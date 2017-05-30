// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cgbitmap.h"
#include "../../cresourcedescription.h"

#if MAC
#include "macglobals.h"
#include <Accelerate/Accelerate.h>
#include <AssertMacros.h>
#if TARGET_OS_IPHONE
	#include <MobileCoreServices/MobileCoreServices.h>
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
SharedPointer<IPlatformBitmap> IPlatformBitmap::create (CPoint* size)
{
	if (size)
		return makeOwned<CGBitmap> (*size);
	return makeOwned<CGBitmap> ();
}

//-----------------------------------------------------------------------------
SharedPointer<IPlatformBitmap> IPlatformBitmap::createFromPath (UTF8StringPtr absolutePath)
{
	SharedPointer<IPlatformBitmap> bitmap;
	CFURLRef url = CFURLCreateFromFileSystemRepresentation (nullptr, (const UInt8*)absolutePath, static_cast<CFIndex> (strlen (absolutePath)), false);
	if (url)
	{
		CGImageSourceRef source = CGImageSourceCreateWithURL (url, nullptr);
		if (source)
		{
			auto cgBitmap = makeOwned<CGBitmap> ();
			bool result = cgBitmap->loadFromImageSource (source);
			if (result)
				bitmap = std::move (cgBitmap);
			CFRelease (source);
		}
		CFRelease (url);
	}
	return bitmap;
}

//-----------------------------------------------------------------------------
SharedPointer<IPlatformBitmap> IPlatformBitmap::createFromMemory (const void* ptr, uint32_t memSize)
{
	SharedPointer<IPlatformBitmap> bitmap;
	CFDataRef data = CFDataCreate (nullptr, (const UInt8*)ptr, static_cast<CFIndex> (memSize));
	if (data)
	{
		CGImageSourceRef source = CGImageSourceCreateWithData (data, nullptr);
		if (source)
		{
			auto cgBitmap = makeOwned<CGBitmap> ();
			bool result = cgBitmap->loadFromImageSource (source);
			if (result)
				bitmap = std::move (cgBitmap);
			CFRelease (source);
		}
		CFRelease (data);
	}
	return bitmap;
}

//-----------------------------------------------------------------------------
PNGBitmapBuffer IPlatformBitmap::createMemoryPNGRepresentation (const SharedPointer<IPlatformBitmap>& bitmap)
{
	PNGBitmapBuffer buffer;
#if !TARGET_OS_IPHONE
	if (auto cgBitmap = bitmap.cast<CGBitmap> ())
	{
		CGImageRef image = cgBitmap->getCGImage ();
		if (image)
		{
			CFMutableDataRef data = CFDataCreateMutable (nullptr, 0);
			if (data)
			{
				CGImageDestinationRef dest = CGImageDestinationCreateWithData (data, kUTTypePNG, 1, nullptr);
				if (dest)
				{
					auto scaleFactor = bitmap->getScaleFactor ();
					CFMutableDictionaryRef properties = nullptr;
					if (scaleFactor != 1.)
					{
						properties = CFDictionaryCreateMutable (nullptr, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
						double dpi = 72 * scaleFactor;
						auto number = CFNumberCreate(nullptr, kCFNumberDoubleType, &dpi);
						CFDictionaryAddValue (properties, kCGImagePropertyDPIWidth, number);
						CFDictionaryAddValue (properties, kCGImagePropertyDPIHeight, number);
						CFRelease (number);
					}
					CGImageDestinationAddImage (dest, image, properties);
					if (CGImageDestinationFinalize (dest))
					{
						buffer.resize(CFDataGetLength (data));
						CFDataGetBytes (data, CFRangeMake (0, CFDataGetLength (data)), buffer.data ());
					}
					if (properties)
						CFRelease (properties);
					CFRelease (dest);
				}
				CFRelease (data);
			}
		}
	}
#endif
	return buffer;
}

//-----------------------------------------------------------------------------
CGBitmap::CGBitmap (const CPoint& inSize)
: size (inSize)
, image (nullptr)
, imageSource (nullptr)
, layer (nullptr)
, bits (nullptr)
, dirty (false)
, bytesPerRow (0)
, scaleFactor (1.)
{
	allocBits ();
}

//-----------------------------------------------------------------------------
CGBitmap::CGBitmap (CGImageRef image)
: image (image)
, imageSource (nullptr)
, layer (nullptr)
, bits (nullptr)
, dirty (false)
, bytesPerRow (0)
, scaleFactor (1.)
{
	CGImageRetain (image);
	size.x = CGImageGetWidth (image);
	size.y = CGImageGetHeight (image);
}

//-----------------------------------------------------------------------------
CGBitmap::CGBitmap ()
: image (nullptr)
, imageSource (nullptr)
, layer (nullptr)
, bits (nullptr)
, dirty (false)
, bytesPerRow (0)
, scaleFactor (1.)
{
}

//-----------------------------------------------------------------------------
CGBitmap::~CGBitmap () noexcept
{
	if (image)
		CGImageRelease (image);
	if (layer)
		CFRelease (layer);
	if (imageSource)
		CFRelease (imageSource);
	if (bits)
		std::free (bits);
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
			std::strcpy (filename, desc.u.name);
		CFStringRef cfStr = CFStringCreateWithCString (nullptr, filename, kCFStringEncodingUTF8);
		if (cfStr)
		{
			CFURLRef url = nullptr;
			if (filename[0] == '/')
				url = CFURLCreateFromFileSystemRepresentation (nullptr, (const UInt8*)filename, static_cast<CFIndex> (strlen (filename)), false);
			int32_t i = 0;
			while (url == nullptr)
			{
				static CFStringRef resTypes [] = { CFSTR("png"), CFSTR("bmp"), CFSTR("jpg"), CFSTR("pict"), nullptr };
				url = CFBundleCopyResourceURL (getBundleRef (), cfStr, desc.type == CResourceDescription::kIntegerType ? resTypes[i] : nullptr, nullptr);
				if (resTypes[++i] == nullptr)
					break;
			}
			CFRelease (cfStr);
			if (url)
			{
				CGImageSourceRef source = CGImageSourceCreateWithURL (url, nullptr);
				if (source)
				{
					result = loadFromImageSource (source);
					CFRelease (source);
				}
				CFRelease (url);
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

//-----------------------------------------------------------------------------
static CFStringRef kCGImageSourceShouldPreferRGB32 = CFSTR("kCGImageSourceShouldPreferRGB32");

#define VSTGUI_QUARTZ_WORKAROUND_PNG_DECODE_ON_DRAW_BUG __i386__ || TARGET_OS_IPHONE

//-----------------------------------------------------------------------------
bool CGBitmap::loadFromImageSource (CGImageSourceRef source)
{
	imageSource = source;
	if (imageSource)
	{
		CFRetain (imageSource);
		CFDictionaryRef properties = CGImageSourceCopyPropertiesAtIndex (imageSource, 0, nullptr);
		if (properties == nullptr)
		{
			return false;
		}
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
	#if VSTGUI_QUARTZ_WORKAROUND_PNG_DECODE_ON_DRAW_BUG
		// workaround a bug in Mac OS X 10.6 (32 bit), where PNG bitmaps were decoded all the time when drawn.
		// we fix this by copying the pixels of the bitmap into our own buffer.
		CFStringRef imageType = CGImageSourceGetType (imageSource);
		if (imageType && CFStringCompare (imageType, kUTTypePNG, 0) == kCFCompareEqualTo)
		{
			CGContextRef context = createCGContext ();
			if (context)
			{
				dirty = true;
				CFRelease (context);
			}
		}
	#endif
		CFRelease (properties);
	}
	return (size.x != 0 && size.y != 0);
}

//-----------------------------------------------------------------------------
CGImageRef CGBitmap::getCGImage ()
{
	if (image == nullptr && imageSource)
	{
		const void* keys[] = {kCGImageSourceShouldCache, kCGImageSourceShouldPreferRGB32};
		const void* values[] = {kCFBooleanTrue, kCFBooleanTrue};
		CFDictionaryRef options = CFDictionaryCreate (nullptr, keys, values, 2, nullptr, nullptr);
		image = CGImageSourceCreateImageAtIndex (imageSource, 0, options);
		CFRelease (imageSource);
		CFRelease (options);
		imageSource = nullptr;
	}
	if ((dirty || image == nullptr) && bits)
	{
		freeCGImage ();

		size_t rowBytes = getBytesPerRow ();
		size_t byteCount = rowBytes * static_cast<size_t> (size.y);
		size_t bitDepth = 32;

		CGDataProviderRef provider = CGDataProviderCreateWithData (nullptr, bits, byteCount, nullptr);
		CGBitmapInfo bitmapInfo = kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Big;
		image = CGImageCreate (static_cast<size_t> (size.x), static_cast<size_t> (size.y), 8, bitDepth, rowBytes, GetCGColorSpace (), bitmapInfo, provider, nullptr, false, kCGRenderingIntentDefault);
		CGDataProviderRelease (provider);
		dirty = false;
	}
	return image;
}

//-----------------------------------------------------------------------------
CGContextRef CGBitmap::createCGContext ()
{
	CGContextRef context = nullptr;
	if (bits == nullptr)
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
				CGContextDrawImage (context, CGRectMake (0, static_cast<CGFloat> (-size.y), static_cast<CGFloat> (size.x), static_cast<CGFloat> (size.y)), image);
				CGContextScaleCTM (context, 1, -1);
				return context;
			}
		}
	}
	if (bits)
	{
		CGBitmapInfo bitmapInfo = kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Big;
		context = CGBitmapContextCreate (bits,
						static_cast<size_t> (size.x),
						static_cast<size_t> (size.y),
						8,
						getBytesPerRow (),
						GetCGColorSpace (),
						bitmapInfo);
		CGContextTranslateCTM (context, 0, (CGFloat)size.y);
		CGContextScaleCTM (context, 1, -1);
	}
	return context;
}

//-----------------------------------------------------------------------------
CGLayerRef CGBitmap::createCGLayer (CGContextRef context)
{
	if (layer && !dirty)
		return layer;
	CGImageRef image = getCGImage ();
	layer = image ? CGLayerCreateWithContext (context, CGSizeFromCPoint (size), nullptr) : nullptr;
	if (layer)
	{
		CGContextRef layerContext = CGLayerGetContext (layer);
		CGContextDrawImage (layerContext, CGRectMake (0, 0, static_cast<CGFloat> (size.x), static_cast<CGFloat> (size.y)), image);
	}
	return layer;
}

//-----------------------------------------------------------------------------
void CGBitmap::allocBits ()
{
	if (bits == nullptr)
	{
		bytesPerRow = static_cast<uint32_t> (size.x * 4);
		if (bytesPerRow % 16)
			bytesPerRow += 16 - (bytesPerRow % 16);
		uint32_t bitmapByteCount = bytesPerRow * static_cast<uint32_t> (size.y);
		bits = calloc (1, bitmapByteCount);
	}
}

//-----------------------------------------------------------------------------
void CGBitmap::freeCGImage ()
{
	if (image)
		CFRelease (image);
	image = nullptr;
	if (layer)
		CFRelease (layer);
	layer = nullptr;
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
			buffer.width = static_cast<vImagePixelCount> (bitmap->getSize ().x);
			buffer.height = static_cast<vImagePixelCount> (bitmap->getSize ().y);
			buffer.rowBytes = bitmap->getBytesPerRow ();
			vImage_Error error = vImageUnpremultiplyData_ARGB8888 (&buffer, &buffer, kvImageNoFlags);
			assert (error == kvImageNoError);
		}
		bitmap->remember ();
	}
	
	~CGBitmapPixelAccess () noexcept override
	{
		if (!alphaPremultiplied)
		{
			vImage_Buffer buffer;
			buffer.data = bitmap->getBits ();
			buffer.width = static_cast<vImagePixelCount> (bitmap->getSize ().x);
			buffer.height = static_cast<vImagePixelCount> (bitmap->getSize ().y);
			buffer.rowBytes = bitmap->getBytesPerRow ();
			vImage_Error error = vImagePremultiplyData_ARGB8888 (&buffer, &buffer, kvImageNoFlags);
			assert (error == kvImageNoError);
		}
		bitmap->setDirty ();
		bitmap->forget ();
	}

	uint8_t* getAddress () const override
	{
		return (uint8_t*)bitmap->getBits ();
	}
	
	uint32_t getBytesPerRow () const override
	{
		return bitmap->getBytesPerRow ();
	}
	
	PixelFormat getPixelFormat () const override
	{
		#ifdef __BIG_ENDIAN__
		return kRGBA;
		#else
		return kARGB;
		#endif
	}
	
protected:
	CGBitmap* bitmap;
	bool alphaPremultiplied;
};

//-----------------------------------------------------------------------------
SharedPointer<IPlatformBitmapPixelAccess> CGBitmap::lockPixels (bool alphaPremultiplied)
{
	if (bits == nullptr)
	{
		CGContextRef context = createCGContext ();
		if (context)
			CFRelease (context);
	}
	if (bits)
	{
		return makeOwned<CGBitmapPixelAccess> (this, alphaPremultiplied);
	}
	return nullptr;
}

} // namespace

#endif // MAC
