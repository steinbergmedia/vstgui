
#include "cgbitmap.h"

#if MAC
#include "macglobals.h"

BEGIN_NAMESPACE_VSTGUI

//-----------------------------------------------------------------------------
IPlatformBitmap* IPlatformBitmap::create (CPoint* size)
{
	if (size)
		return new CGOffscreenBitmap (*size);
	return new CGBitmap ();
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
	if (getBundleRef ())
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
				result = loadFromUrl (url);
				CFRelease (url);
			}
			else
			{
				#if DEBUG
				DebugPrint ("*** Bitmap Nr.:%d not found.\n", desc.u.id);
				#endif
			}
		}
	}
	return result;
}

static CFStringRef kCGImageSourceShouldPreferRGB32 = CFSTR("kCGImageSourceShouldPreferRGB32");

//-----------------------------------------------------------------------------
bool CGBitmap::loadFromUrl (CFURLRef url)
{
	imageSource = CGImageSourceCreateWithURL (url, NULL);
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
	}
	#if DEBUG
	else
	{
		DebugPrint ("*** Bitmap not found :"); 
		CFShow (url);
	}
	#endif
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
		imageSource = 0;
	}
	return image;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CDataProvider 
{
public:
	CDataProvider (void* bitmap) : ptr ((unsigned char*)bitmap)
	{
	}

	static size_t getBytes (void *info, void* buffer, size_t count)
	{	// this could be optimized ;-)
		CDataProvider* p = (CDataProvider*)info;
		unsigned char* dst = (unsigned char*)buffer;
		unsigned char* src = p->ptr + p->pos;
		memcpy (dst, src, count);
		#if 0
		for (unsigned long i = 0; i < count / 4; i++)
		{
			*dst++ = *src++;
			*dst++ = *src++;
			*dst++ = *src++;
			*dst++ = *src++;
		}
		#endif
		p->pos += count;
		return count;
	}

	static void skipBytes (void* info, size_t count)
	{
		CDataProvider* p = (CDataProvider*)info;
		p->pos += count;
	}

	#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5
	static off_t skipForward (void *info, off_t count)
	{
		CDataProvider* p = (CDataProvider*)info;
		p->pos += count;
		return p->pos;
	}
	#endif

	static void rewind (void* info)
	{
		CDataProvider* p = (CDataProvider*)info;
		p->pos = 0;
	}

	static void releaseProvider (void* info)
	{
		CDataProvider* p = (CDataProvider*)info;
		delete p;
	}

	unsigned long pos;
	unsigned char* ptr;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CGOffscreenBitmap::CGOffscreenBitmap (const CPoint& inSize)
: bits (0)
, dirty (false)
{
	size = inSize;
	allocBits ();
}

//-----------------------------------------------------------------------------
CGOffscreenBitmap::~CGOffscreenBitmap ()
{
	if (bits)
		free (bits);
}

//-----------------------------------------------------------------------------
CGImageRef CGOffscreenBitmap::getCGImage ()
{
	if (image == 0 || dirty)
	{
		if (image)
			CGImageRelease (image);

		size_t rowBytes = getBytesPerRow ();
		size_t byteCount = rowBytes * size.y;
		short bitDepth = 32;

		CGDataProviderRef provider = 0;
		#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5
		static CGDataProviderSequentialCallbacks callbacks = { 0, CDataProvider::getBytes, CDataProvider::skipForward, CDataProvider::rewind, CDataProvider::releaseProvider };
		if (transparent)
			provider = CGDataProviderCreateSequential (new CDataProvider (bits), &callbacks);
		#else
		static CGDataProviderCallbacks callbacks = { CDataProvider::getBytes, CDataProvider::skipBytes, CDataProvider::rewind, CDataProvider::releaseProvider };
		if (transparent)
			provider = CGDataProviderCreate (new CDataProvider (bits), &callbacks);
		#endif
		else
			provider = CGDataProviderCreateWithData (NULL, bits, byteCount, NULL);
		CGImageAlphaInfo alphaInfo = kCGImageAlphaFirst;
		image = CGImageCreate (size.x, size.y, 8, bitDepth, rowBytes, GetGenericRGBColorSpace (), alphaInfo, provider, NULL, false, kCGRenderingIntentDefault);
		CGDataProviderRelease (provider);
	}
	return image;
}

//-----------------------------------------------------------------------------
CGContextRef CGOffscreenBitmap::createCGContext ()
{
	CGContextRef context = 0;
	if (bits)
	{
		context = CGBitmapContextCreate (bits,
						size.x,
						size.y,
						8,
						getBytesPerRow (),
						GetGenericRGBColorSpace (),
						kCGImageAlphaPremultipliedFirst);
		CGContextTranslateCTM (context, 0, (CGFloat)size.y);
		CGContextScaleCTM (context, 1, -1);
		dirty = true;
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

//-----------------------------------------------------------------------------
int CGOffscreenBitmap::getBytesPerRow () const
{
	return getSize ().x * 4;
}

END_NAMESPACE_VSTGUI

#endif // MAC
