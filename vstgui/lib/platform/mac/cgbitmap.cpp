//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "cgbitmap.h"

#if MAC
#include "macglobals.h"
#include <Accelerate/Accelerate.h>

namespace VSTGUI {

//-----------------------------------------------------------------------------
IPlatformBitmap* IPlatformBitmap::create (CPoint* size)
{
	if (size)
		return new CGBitmap (*size);
	return new CGBitmap ();
}

//-----------------------------------------------------------------------------
CGBitmap::CGBitmap (const CPoint& inSize)
: image (0)
, imageSource (0)
, bits (0)
, dirty (false)
, bytesPerRow (0)
{
	size = inSize;
	allocBits ();
}

//-----------------------------------------------------------------------------
CGBitmap::CGBitmap ()
: image (0)
, imageSource (0)
, bits (0)
, dirty (false)
, bytesPerRow (0)
{
}

//-----------------------------------------------------------------------------
CGBitmap::~CGBitmap ()
{
	if (image)
		CGImageRelease (image);
	if (imageSource)
		CFRelease (imageSource);
	if (bits)
		free (bits);
}

//-----------------------------------------------------------------------------
bool CGBitmap::load (const CResourceDescription& desc)
{
	if (bits)
		return false;

	bool result = false;
	if (getBundleRef ())
	{
		// find the bitmap in our Bundle.
		// If the resource description is of type integer, it must be in the form of bmp00123.png, where the resource id would be 123.
		// else it just uses the name
		char filename [PATH_MAX];
		if (desc.type == CResourceDescription::kIntegerType)
			sprintf (filename, "bmp%05d", (int32_t)desc.u.id);
		else
			strcpy (filename, desc.u.name);
		CFStringRef cfStr = CFStringCreateWithCString (NULL, filename, kCFStringEncodingUTF8);
		if (cfStr)
		{
			CFURLRef url = NULL;
			int32_t i = 0;
			while (url == NULL)
			{
				static CFStringRef resTypes [] = { CFSTR("png"), CFSTR("bmp"), CFSTR("jpg"), CFSTR("pict"), NULL };
				url = CFBundleCopyResourceURL (getBundleRef (), cfStr, desc.type == CResourceDescription::kIntegerType ? resTypes[i] : 0, NULL);
				if (resTypes[++i] == NULL)
					break;
			}
			CFRelease (cfStr);
			if (url)
			{
				CGImageSourceRef source = CGImageSourceCreateWithURL (url, NULL);
				CFRelease (url);
				result = loadFromImageSource (source);
				CFRelease (source);
			}
		}
	}
#if DEBUG
	if (result == false)
	{
		if (desc.type == CResourceDescription::kIntegerType)
			DebugPrint ("*** Bitmap Nr.:%d not found.\n", desc.u.id);
		else
			DebugPrint ("*** Bitmap '%s' not found.\n", desc.u.name);
	}
#endif
	return result;
}

static CFStringRef kCGImageSourceShouldPreferRGB32 = CFSTR("kCGImageSourceShouldPreferRGB32");

//-----------------------------------------------------------------------------
bool CGBitmap::loadFromImageSource (CGImageSourceRef source)
{
	imageSource = source;
	if (imageSource)
	{
		CFDictionaryRef properties = CGImageSourceCopyPropertiesAtIndex (imageSource, 0, 0);
		if (properties == 0)
			return false;
		CFNumberRef value = (CFNumberRef)CFDictionaryGetValue (properties, kCGImagePropertyPixelHeight);
		if (value)
		{
			double fValue = 0;
			if (CFNumberGetValue (value, kCFNumberDoubleType, &fValue))
				size.y = fValue;
		}
		value = (CFNumberRef)CFDictionaryGetValue (properties, kCGImagePropertyPixelWidth);
		if (value)
		{
			double fValue = 0;
			if (CFNumberGetValue (value, kCFNumberDoubleType, &fValue))
				size.x = fValue;
		}
		CFRelease (properties);
		CFRetain (imageSource);
	}
	return (size.x != 0 && size.y != 0);
}

//-----------------------------------------------------------------------------
CGImageRef CGBitmap::getCGImage ()
{
	if (image == 0 && imageSource)
	{
		const void* keys[] = {kCGImageSourceShouldCache, kCGImageSourceShouldPreferRGB32};
		const void* values[] = {kCFBooleanTrue, kCFBooleanTrue};
		CFDictionaryRef options = CFDictionaryCreate (NULL, keys, values, 2, NULL, NULL);
		image = CGImageSourceCreateImageAtIndex (imageSource, 0, options);
		CFRelease (imageSource);
		CFRelease (options);
		imageSource = 0;
	}
	if (dirty && bits)
	{
		freeCGImage ();

		size_t rowBytes = getBytesPerRow ();
		size_t byteCount = rowBytes * size.y;
		size_t bitDepth = 32;

		CGDataProviderRef provider = CGDataProviderCreateWithData (NULL, bits, byteCount, NULL);
		CGBitmapInfo alphaInfo = kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Big;
		image = CGImageCreate (size.x, size.y, 8, bitDepth, rowBytes, GetGenericRGBColorSpace (), alphaInfo, provider, NULL, false, kCGRenderingIntentDefault);
		CGDataProviderRelease (provider);
		dirty = false;
	}
	return image;
}

//-----------------------------------------------------------------------------
CGContextRef CGBitmap::createCGContext ()
{
	CGContextRef context = 0;
	if (bits == 0)
	{
		allocBits ();
		if (imageSource)
			getCGImage ();
		if (image)
		{
			context = createCGContext ();
			if (context)
			{
				CGContextScaleCTM (context, 1, -1);
				CGContextDrawImage (context, CGRectMake (0, -size.y, size.x, size.y), image);
				CGContextScaleCTM (context, 1, -1);
				return context;
			}
		}
	}
	if (bits)
	{
		CGBitmapInfo bitmapInfo = kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Big;
		context = CGBitmapContextCreate (bits,
						size.x,
						size.y,
						8,
						getBytesPerRow (),
						GetGenericRGBColorSpace (),
						bitmapInfo);
		CGContextTranslateCTM (context, 0, (CGFloat)size.y);
		CGContextScaleCTM (context, 1, -1);
	}
	return context;
}

//-----------------------------------------------------------------------------
void CGBitmap::allocBits ()
{
	if (bits == 0)
	{
		bytesPerRow = size.x * 4;
		if (bytesPerRow % 16)
			bytesPerRow += 16 - (bytesPerRow % 16);
		int32_t bitmapByteCount     = bytesPerRow * size.y; 
		bits = calloc (1, bitmapByteCount);
	}
}

//-----------------------------------------------------------------------------
void CGBitmap::freeCGImage ()
{
	if (image)
		CFRelease (image);
	image = 0;
}

//-----------------------------------------------------------------------------
class CGBitmapPixelAccess : public IPlatformBitmapPixelAccess
{
public:
	CGBitmapPixelAccess (CGBitmap* bitmap, bool alphaPremultiplied)
	: bitmap (bitmap)
	, alphaPremultiplied (alphaPremultiplied)
	{
		if (!alphaPremultiplied)
		{
			vImage_Buffer buffer;
			buffer.data = bitmap->getBits ();
			buffer.width = bitmap->getSize ().x;
			buffer.height = bitmap->getSize ().y;
			buffer.rowBytes = bitmap->getBytesPerRow ();
			vImage_Error error = vImageUnpremultiplyData_ARGB8888 (&buffer, &buffer, kvImageNoFlags);
			assert (error == kvImageNoError);
		}
		bitmap->remember ();
	}
	
	~CGBitmapPixelAccess ()
	{
		if (!alphaPremultiplied)
		{
			vImage_Buffer buffer;
			buffer.data = bitmap->getBits ();
			buffer.width = bitmap->getSize ().x;
			buffer.height = bitmap->getSize ().y;
			buffer.rowBytes = bitmap->getBytesPerRow ();
			vImage_Error error = vImagePremultiplyData_ARGB8888 (&buffer, &buffer, kvImageNoFlags);
			assert (error == kvImageNoError);
		}
		bitmap->setDirty ();
		bitmap->forget ();
	}

	uint8_t* getAddress ()
	{
		return (uint8_t*)bitmap->getBits ();
	}
	
	int32_t getBytesPerRow ()
	{
		return bitmap->getBytesPerRow ();
	}
	
	PixelFormat getPixelFormat () 
	{
		#ifdef __BIG_ENDIAN__
		return kRGBA;
		#else
		return kABGR;
		#endif
	}
	
protected:
	CGBitmap* bitmap;
	bool alphaPremultiplied;
};

//-----------------------------------------------------------------------------
IPlatformBitmapPixelAccess* CGBitmap::lockPixels (bool alphaPremultiplied)
{
	if (bits == 0)
	{
		CGContextRef context = createCGContext ();
		if (context)
			CFRelease (context);
	}
	if (bits)
	{
		return new CGBitmapPixelAccess (this, alphaPremultiplied);
	}
	return 0;
}

} // namespace

#endif // MAC
