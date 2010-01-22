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

namespace VSTGUI {

//-----------------------------------------------------------------------------
IPlatformBitmap* IPlatformBitmap::create (CPoint* size)
{
	if (size)
		return new CGOffscreenBitmap (*size);
	return new CGBitmap ();
}

//-----------------------------------------------------------------------------
static size_t DataProviderGetBytesCallback (void *info, void *buffer, size_t count)
{
	IBitmapReader* reader = (IBitmapReader*)info;
	return reader->readBytes (buffer, count);
}

//-----------------------------------------------------------------------------
static off_t DataProviderSkipForwardCallback (void *info, off_t count)
{
	IBitmapReader* reader = (IBitmapReader*)info;
	off_t skipped = 0;
	char buffer;
	for (off_t i = 0; i < count; i++)
	{
		int read = reader->readBytes (&buffer, 1);
		if (read != 0)
			break;
		skipped += read;
	}
	return skipped;
}

//-----------------------------------------------------------------------------
static void DataProviderRewindCallback (void *info)
{
	IBitmapReader* reader = (IBitmapReader*)info;
	reader->rewind ();
}

//-----------------------------------------------------------------------------
static void DataProviderReleaseInfoCallback (void *info)
{
	IBitmapReader* reader = (IBitmapReader*)info;
	reader->forget ();
}

//-----------------------------------------------------------------------------
CGBitmap::CGBitmap ()
: image (0)
, imageSource (0)
{
}

//-----------------------------------------------------------------------------
CGBitmap::~CGBitmap ()
{
	if (image)
		CGImageRelease (image);
	if (imageSource)
		CFRelease (imageSource);
}

//-----------------------------------------------------------------------------
bool CGBitmap::load (const CResourceDescription& desc)
{
	bool result = false;
	if (gCustomBitmapReaderCreator)
	{
		IBitmapReader* reader = gCustomBitmapReaderCreator->createBitmapReader (desc);
		if (reader)
		{
			static CGDataProviderSequentialCallbacks callbacks = {
				0,
				DataProviderGetBytesCallback,
				DataProviderSkipForwardCallback,
				DataProviderRewindCallback,
				DataProviderReleaseInfoCallback
			};
			
			CGDataProviderRef dataProvider = CGDataProviderCreateSequential (reader, &callbacks);
			if (dataProvider)
			{
				CGImageSourceRef source = CGImageSourceCreateWithDataProvider (dataProvider, 0);
				if (source)
				{
					result = loadFromImageSource (source);
					CFRelease (source);
				}
				CFRelease (dataProvider);
			}
			reader->forget ();
		}
	}
	if (!result && getBundleRef ())
	{
		// find the bitmap in our Bundle.
		// If the resource description is of type integer, it must be in the form of bmp00123.png, where the resource id would be 123.
		// else it just uses the name
		char filename [PATH_MAX];
		if (desc.type == CResourceDescription::kIntegerType)
			sprintf (filename, "bmp%05d", (int)desc.u.id);
		else
			strcpy (filename, desc.u.name);
		CFStringRef cfStr = CFStringCreateWithCString (NULL, filename, kCFStringEncodingUTF8);
		if (cfStr)
		{
			CFURLRef url = NULL;
			int i = 0;
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
	return image;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CGOffscreenBitmap::CGOffscreenBitmap (const CPoint& inSize)
: bits (0)
, dirty (false)
{
	size = inSize;
	bytesPerRow = size.x * 4;
	if (bytesPerRow % 16)
		bytesPerRow += 16 - (bytesPerRow % 16);
	allocBits ();
}

//-----------------------------------------------------------------------------
CGOffscreenBitmap::~CGOffscreenBitmap ()
{
	if (bits)
		free (bits);
}

//-----------------------------------------------------------------------------
void CGOffscreenBitmap::freeCGImage ()
{
	if (image)
		CGImageRelease (image);
	image = 0;
}

//-----------------------------------------------------------------------------
CGImageRef CGOffscreenBitmap::getCGImage ()
{
	if (dirty)
		freeCGImage ();
	if (image == 0)
	{
		size_t rowBytes = getBytesPerRow ();
		size_t byteCount = rowBytes * size.y;
		short bitDepth = 32;

		CGDataProviderRef provider = CGDataProviderCreateWithData (NULL, bits, byteCount, NULL);
	#if defined (__BIG_ENDIAN__)
		CGImageAlphaInfo alphaInfo = kCGImageAlphaFirst;
	#else
		CGImageAlphaInfo alphaInfo = kCGImageAlphaLast;
	#endif
		image = CGImageCreate (size.x, size.y, 8, bitDepth, rowBytes, GetGenericRGBColorSpace (), alphaInfo, provider, NULL, false, kCGRenderingIntentDefault);
		CGDataProviderRelease (provider);
		dirty = false;
	}
	return image;
}

//-----------------------------------------------------------------------------
CGContextRef CGOffscreenBitmap::createCGContext ()
{
	CGContextRef context = 0;
	if (bits)
	{
	#if defined (__BIG_ENDIAN__)
		CGBitmapInfo bitmapInfo = kCGImageAlphaPremultipliedFirst;
	#else
		CGBitmapInfo bitmapInfo = kCGImageAlphaPremultipliedLast;
	#endif
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
void CGOffscreenBitmap::allocBits ()
{
	if (bits == 0)
	{
		int bitmapBytesPerRow   = getBytesPerRow ();
		int bitmapByteCount     = bitmapBytesPerRow * size.y; 
		bits = calloc (1, bitmapByteCount);
	}
}

} // namespace

#endif // MAC
