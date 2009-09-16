//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2008, Steinberg Media Technologies, All Rights Reserved
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

#include "cbitmap.h"
#include "cdrawcontext.h"
#include "win32support.h"

BEGIN_NAMESPACE_VSTGUI

#if DEBUG
	long gNbCBitmap = 0;
#endif // DEBUG

#if VSTGUI_USES_COREGRAPHICS
	#ifndef CGFLOAT_DEFINED
		#define CGFLOAT_DEFINED
		typedef float CGFloat;
	#endif // CGFLOAT_DEFINED
#endif // VSTGUI_USES_COREGRAPHICS

#if WINDOWS && USE_LIBPNG
	class PNGResourceStream
	{
	public:
		PNGResourceStream ();
		~PNGResourceStream ();
		bool open (const CResourceDescription& resourceDesc);
		void read (unsigned char* ptr, size_t size);
		static void readCallback (png_struct* pngPtr, unsigned char* ptr, size_t size);
	protected:
		HGLOBAL resData;
		unsigned long streamPos;
		unsigned long resSize;
	};
#endif

//-----------------------------------------------------------------------------
// CBitmap Implementation
//-----------------------------------------------------------------------------
/*! @class CBitmap
@section cbitmap_alphablend Alpha Blend and Transparency
With Version 3.0 of VSTGUI it is possible to use alpha blended bitmaps. This comes free on Mac OS X and with Windows you need to include libpng.
Per default PNG images will be rendered alpha blended. If you want to use a transparency color with PNG Bitmaps, you need to call setNoAlpha(true) on the bitmap and set the transparency color.
@section cbitmap_macosx Apple Mac OS X
The Bitmaps can be of type PNG, JPEG, PICT, BMP and are stored in the Resources folder of the plugin bundle. 
With the latest version VSTGUI supports all image types supported by the Image I/O Framework.
@section cbitmap_windows Microsoft Windows
The Bitmaps are .bmp files and must be included in the plug (usually using a .rc file).
It's also possible to use png as of version 3.0 if you define the macro USE_LIBPNG and include the libpng and zlib libraries/sources to your project.
@section new New since 3.0
There is a new way to name the bitmaps in the latest version. Instead of using a number identifier for the bitmaps you can now use real names for it.
The CResourceDescription works with both names and numbers. If you use names, you need to use the real filename with extension. Then it gets automaticly
loaded on Mac OS X out of the Resources folder of the vst bundle. On Windows you also specify the resource in the .rc file with the real filename.
\code
// Old way
1001                    BITMAP  DISCARDABLE     "bmp01001.bmp"
// New way
RealFileName.bmp        BITMAP  DISCARDABLE     "RealFileName.bmp"
\endcode
\code
CBitmap* bitmap1 = new CBitmap (1001);
CBitmap* bitmap2 = new CBitmap ("RealFileName.bmp");
\endcode
*/
CBitmap::CBitmap (const CResourceDescription& desc)
: resourceDesc (desc)
, width (0)
, height (0)
, noAlpha (true)
#if VSTGUI_USES_COREGRAPHICS
, cgImage (0)
#endif
{
	#if DEBUG
	gNbCBitmap++;
	#endif

	setTransparentColor (kTransparentCColor);
	
#if GDIPLUS
	GDIPlusGlobals::enter ();
	pBitmap = 0;
	bits = 0;
#endif

	pMask = 0;
	pHandle = 0;

	loadFromResource (resourceDesc);

	#if DEBUG
	gBitmapAllocation += (long)height * (long)width;
	#endif
}

//-----------------------------------------------------------------------------
CBitmap::CBitmap (CCoord width, CCoord height)
: width (width)
, height (height)
, noAlpha (true)
#if VSTGUI_USES_COREGRAPHICS
, cgImage (0)
#endif
{
	#if DEBUG
	gNbCBitmap++;
	#endif

	setTransparentColor (kTransparentCColor);

#if WINDOWS
#if GDIPLUS
	GDIPlusGlobals::enter ();
	pBitmap = 0;
	pHandle = 0;
	bits = 0;
	pBitmap = new Gdiplus::Bitmap ((INT)width, (INT)height, PixelFormat32bppARGB);
#else
	HDC hScreen = GetDC (0);
	pHandle = CreateCompatibleBitmap (hScreen, width, height);
	ReleaseDC (0, hScreen);	
#endif
	pMask = 0;

#elif VSTGUI_USES_COREGRAPHICS
	pHandle = 0;
	pMask = 0;
	
	pHandle = malloc (width * 4 * height);

#endif
}

//-----------------------------------------------------------------------------
CBitmap::CBitmap ()
: width (0)
, height (0)
, noAlpha (true)
#if VSTGUI_USES_COREGRAPHICS
, cgImage (0)
#endif
{
	#if GDIPLUS
	GDIPlusGlobals::enter ();
	pBitmap = 0;
	bits = 0;
	#endif
	
	pHandle = 0;
	pMask = 0;
}

#if WINDOWS && GDIPLUS
//-----------------------------------------------------------------------------
CBitmap::CBitmap (Gdiplus::Bitmap* platformBitmap)
: width (0)
, height (0)
, noAlpha (true)
{
	pHandle = 0;
	pMask = 0;
	GDIPlusGlobals::enter ();
	pBitmap = platformBitmap->Clone (0, 0, platformBitmap->GetWidth (), platformBitmap->GetHeight (), PixelFormat32bppARGB);
	width = (CCoord)pBitmap->GetWidth ();
	height = (CCoord)pBitmap->GetHeight ();
	bits = 0;
}
#endif

#if VSTGUI_USES_COREGRAPHICS
//-----------------------------------------------------------------------------
CBitmap::CBitmap (CGImageRef platformBitmap)
: width (0)
, height (0)
, noAlpha (true)
{
	pHandle = 0;
	pMask = 0;
	cgImage = platformBitmap;
	CGImageRetain (platformBitmap);
	width = CGImageGetWidth (platformBitmap);
	height = CGImageGetHeight (platformBitmap);
}
#endif

//-----------------------------------------------------------------------------
CBitmap::~CBitmap ()
{
	dispose ();
	#if GDIPLUS
	GDIPlusGlobals::exit ();
	#endif
}

//-----------------------------------------------------------------------------
void CBitmap::dispose ()
{
	#if DEBUG
	gNbCBitmap--;
	gBitmapAllocation -= (long)height * (long)width;
	#endif

	#if WINDOWS
	#if GDIPLUS
	if (pBitmap)
		delete pBitmap;
	pBitmap = 0;
	bits = 0;
	#endif
	if (pHandle)
		DeleteObject (pHandle);
	if (pMask)
		DeleteObject (pMask);

	noAlpha = false;
		
	#elif VSTGUI_USES_COREGRAPHICS
	if (cgImage)
		CGImageRelease ((CGImageRef)cgImage);
	cgImage = 0;

	if (pHandle)
		free (pHandle);
	
	#endif // VSTGUI_USES_COREGRAPHICS

	pHandle = 0;
	pMask = 0;

	width = 0;
	height = 0;

}

//-----------------------------------------------------------------------------
void* CBitmap::getHandle () const
 {
	return pHandle; 
}

//-----------------------------------------------------------------------------
bool CBitmap::loadFromResource (const CResourceDescription& resourceDesc)
{
	bool result = false;

	dispose ();
	
	//---------------------------------------------------------------------------------------------
	#if WINDOWS
	//---------------------------------------------------------------------------------------------
	#if USE_LIBPNG
	PNGResourceStream resStream;
	if (resStream.open (resourceDesc))
	{
		// setup libpng
		png_structp png_ptr;
		png_infop info_ptr;
		png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (png_ptr)
		{
			info_ptr = png_create_info_struct (png_ptr);
			if (info_ptr)
			{
				if (setjmp (png_jmpbuf (png_ptr)) == 0)
				{
					int bit_depth, color_type;
					png_set_read_fn (png_ptr, (void *)&resStream, PNGResourceStream::readCallback);
					png_read_info (png_ptr, info_ptr);
					png_get_IHDR (png_ptr, info_ptr, (png_uint_32*)&width, (png_uint_32*)&height, &bit_depth, &color_type, 0, 0, 0);
					long bytesPerRow = width * (32 / 8);
					while (bytesPerRow & 0x03)
						bytesPerRow++;
					// create BITMAP
					BITMAPINFO* bmInfo = new BITMAPINFO;
					BITMAPINFOHEADER* header = (BITMAPINFOHEADER*)bmInfo;
					memset (header, 0, sizeof(BITMAPINFOHEADER));
					header->biSize = sizeof(BITMAPINFOHEADER);
					header->biWidth = width;
					header->biHeight = height;
					header->biPlanes = 1;
					header->biBitCount = 32;
					header->biCompression = BI_RGB;
					header->biClrUsed = 0;
					#if !GDIPLUS
					void* bits;
					#endif
					HDC dstDC = 0; //CreateCompatibleDC (0);
					pHandle = CreateDIBSection (dstDC, bmInfo, DIB_RGB_COLORS, &bits, NULL, 0);
					delete bmInfo;
					if (pHandle)
					{
						if (color_type == PNG_COLOR_TYPE_PALETTE)
							png_set_palette_to_rgb (png_ptr);
						if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
							png_set_gray_to_rgb (png_ptr);
						if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
							png_set_gray_1_2_4_to_8 (png_ptr);
						if (png_get_valid (png_ptr, info_ptr, PNG_INFO_tRNS))
							png_set_tRNS_to_alpha (png_ptr);
						else
							png_set_filler (png_ptr, 0xFF, PNG_FILLER_AFTER);
						if (bit_depth == 16)
						{
							png_set_swap (png_ptr);
							png_set_strip_16 (png_ptr);
						}
						if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA)
							png_set_bgr (png_ptr);
						int number_passes = png_set_interlace_handling(png_ptr); // suggested by Hermann Seib!!
						png_read_update_info (png_ptr, info_ptr);

						unsigned char** rows = new unsigned char*[1];
						for (int pass = number_passes - 1; pass >= 0; pass--) // suggested by Hermann Seib!!
						{ // suggested by Hermann Seib!!
						#if GDIPLUS
						rows[0] = (unsigned char*)bits;
						for (long i = 0; i < height; i++)
						{
							png_read_rows (png_ptr, rows, NULL, 1);
							rows[0] += bytesPerRow;
						}
						#else
						rows[0] = (unsigned char*)bits + (height-1) * bytesPerRow;
						for (long i = 0; i < height; i++)
						{
							png_read_rows (png_ptr, rows, NULL, 1);
							rows[0] -= bytesPerRow;
						}
						#endif
						} // suggested by Hermann Seib!!
						delete [] rows;
						png_read_end (png_ptr, 0);
						#if 1 //!GDIPLUS
						// premultiply alpha
						unsigned long* pixelPtr = (unsigned long*)bits;
						for (int y = 0; y < height; y++)
						{
							for (int x = 0; x < width; x++)
							{
								unsigned char* pixel = (unsigned char*)pixelPtr;
								if (pixel[3] != 0)
								{
									pixel[0] = ((pixel[0] * pixel[3]) >> 8);
									pixel[1] = ((pixel[1] * pixel[3]) >> 8);
									pixel[2] = ((pixel[2] * pixel[3]) >> 8);
								}
								else
									*pixelPtr = 0UL;
								pixelPtr++;
							}
						}
						#endif
						if (dstDC)
							DeleteDC (dstDC);
#if 0
						HDC srcDC = CreateCompatibleDC (0);
						SelectObject (srcDC, pHandle);

						HDC dstDC = CreateCompatibleDC (0);
						this->pHandle = CreateCompatibleBitmap (dstDC, width, height);
						SelectObject (dstDC, this->pHandle);

						BLENDFUNCTION blendFunction;
						blendFunction.BlendOp = AC_SRC_OVER;
						blendFunction.BlendFlags = 0;
						blendFunction.SourceConstantAlpha = 255;
						#if USE_ALPHA_BLEND
						if (noAlpha)
							blendFunction.AlphaFormat = 0;//AC_SRC_NO_ALPHA;
						else
							blendFunction.AlphaFormat = AC_SRC_ALPHA;
						#else
						blendFunction.AlphaFormat = 0;//AC_SRC_NO_ALPHA;
						#endif
						#if DYNAMICALPHABLEND
						(*pfnAlphaBlend) (dstDC, 
									0, 0,
									width, height, 
									srcDC,
									0, 0,
									width, height,
									blendFunction);
						#else
						#endif

						DeleteDC (srcDC);
						DeleteDC (dstDC);
						DeleteObject (pHandle);
#endif
					}
				}
			}
			png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
		}
		noAlpha = false;
		result = true;
	}
	#endif
	if (!result)
	{
#if GDIPLUS
		ResourceStream* resourceStream = new ResourceStream;
		if (resourceStream->open (resourceDesc, "PNG"))
			pBitmap = Gdiplus::Bitmap::FromStream (resourceStream, TRUE);
		resourceStream->Release ();
		if (!pBitmap)
			pBitmap = Gdiplus::Bitmap::FromResource (GetInstance (), resourceDesc.type == CResourceDescription::kIntegerType ? (WCHAR*)MAKEINTRESOURCE(resourceDesc.u.id) : (WCHAR*)resourceDesc.u.name);
		if (pBitmap)
		{
			result = true;
			width = (CCoord)pBitmap->GetWidth ();
			height = (CCoord)pBitmap->GetHeight ();
		}
#else
		pHandle = LoadBitmap (GetInstance (), resourceDesc.type == CResourceDescription::kIntegerType ? MAKEINTRESOURCE (resourceDesc.u.id) : resourceDesc.u.name);
		BITMAP bm;
		if (pHandle && GetObject (pHandle, sizeof (bm), &bm))
		{
			width  = bm.bmWidth; 
			height = bm.bmHeight;
			noAlpha = true;
			result = true;
		}
#endif
	}
	
	//---------------------------------------------------------------------------------------------
	#elif VSTGUI_USES_COREGRAPHICS
	//---------------------------------------------------------------------------------------------
	pHandle = 0;
	pMask = 0;
	cgImage = 0;
	if (getBundleRef ())
	{
		// find the bitmap in our Bundle.
		// If the resource description is of type integer, it must be in the form of bmp00123.png, where the resource id would be 123.
		// else it just uses the name
		char filename [PATH_MAX];
		if (resourceDesc.type == CResourceDescription::kIntegerType)
			sprintf (filename, "bmp%05d", (int)resourceDesc.u.id);
		else
			strcpy (filename, resourceDesc.u.name);
		CFStringRef cfStr = CFStringCreateWithCString (NULL, filename, kCFStringEncodingUTF8);
		if (cfStr)
		{
			CFURLRef url = NULL;
			int i = 0;
			while (url == NULL)
			{
				static CFStringRef resTypes [] = { CFSTR("png"), CFSTR("bmp"), CFSTR("jpg"), CFSTR("pict"), NULL };
				url = CFBundleCopyResourceURL (getBundleRef (), cfStr, resourceDesc.type == CResourceDescription::kIntegerType ? resTypes[i] : 0, NULL);
				if (resTypes[++i] == NULL)
					break;
			}
			CFRelease (cfStr);
			if (url)
			{
				result = loadFromPath (url);
				CFRelease (url);
			}
			else
			{
				#if DEBUG
				DebugPrint ("*** Bitmap Nr.:%d not found.\n", resourceDesc.u.id);
				#endif
			}
		}
	}
	
	#endif
	return result;
}

#if VSTGUI_USES_COREGRAPHICS
const CFStringRef kCGImageSourceShouldPreferRGB32 = CFSTR("kCGImageSourceShouldPreferRGB32");
#endif

//-----------------------------------------------------------------------------
bool CBitmap::loadFromPath (const void* platformPath)
{
	bool result = false;

	dispose ();

	#if VSTGUI_USES_COREGRAPHICS
	CFURLRef url = (CFURLRef)platformPath;

	// use Image I/O
	CGImageSourceRef imageSource = CGImageSourceCreateWithURL (url, NULL);
	if (imageSource)
	{
		const void* keys[] = {kCGImageSourceShouldCache, kCGImageSourceShouldPreferRGB32};
		const void* values[] = {kCFBooleanTrue, kCFBooleanTrue};
		CFDictionaryRef options = CFDictionaryCreate (NULL, keys, values, 2, NULL, NULL);
		cgImage = CGImageSourceCreateImageAtIndex (imageSource, 0, options);
		CFDictionaryRef dictRef = CGImageSourceCopyPropertiesAtIndex (imageSource, 0, options);
		if (dictRef)
		{
			#if DEBUG
			//CFShow (dictRef);
			#endif
			const void* value = CFDictionaryGetValue (dictRef, kCGImagePropertyHasAlpha);
			if (value == kCFBooleanTrue)
				noAlpha = false;
			CFRelease (dictRef);
		}
		CFRelease (options);
		if (cgImage)
		{
			width = CGImageGetWidth ((CGImageRef)cgImage);
			height = CGImageGetHeight ((CGImageRef)cgImage);
			result = true;
		}
		CFRelease (imageSource);
	}

	#if DEBUG
	if (!result)
	{
		DebugPrint ("*** Bitmap not found :"); CFShow (url);
	}
	#endif
	
	#elif WINDOWS
	// todo
	
	#endif
	
	return result;
}

//-----------------------------------------------------------------------------
bool CBitmap::isLoaded () const
{
	#if VSTGUI_USES_COREGRAPHICS
	if (cgImage || getHandle ())
		return true;
	#elif GDIPLUS
	if (pBitmap)
		return true;
	#else
	if (getHandle ())
		return true;
	#endif

	return false;
}

#if VSTGUI_USES_COREGRAPHICS

class CDataProvider 
{
public:
	CDataProvider (void* bitmap, const CColor& color) : ptr ((unsigned char*)bitmap), color (color) 
	{
	}

	static size_t getBytes (void *info, void* buffer, size_t count)
	{	// this could be optimized ;-)
		CDataProvider* p = (CDataProvider*)info;
		unsigned char* dst = (unsigned char*)buffer;
		unsigned char* src = p->ptr + p->pos;
		for (unsigned long i = 0; i < count / 4; i++)
		{
			if (src[1] == p->color.red && src[2] == p->color.green && src[3] == p->color.blue)
			{
				*dst++ = 0;
				src++;
			}
			else
				*dst++ = *src++;
			*dst++ = *src++;
			*dst++ = *src++;
			*dst++ = *src++;
		}
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
	CColor color;
};

//-----------------------------------------------------------------------------
CGImageRef CBitmap::createCGImage (bool transparent)
{
	if (cgImage)
	{
		CGImageRetain ((CGImageRef)cgImage);
		return (CGImageRef)cgImage;
	}
	if (!pHandle)
		return NULL;

	void* pixels = 0;
	size_t rowBytes = 0;
	short bitDepth = 0;
	size_t size = 0;

	pixels = pHandle;
	rowBytes = width * 4;
	bitDepth = 32;
	size = rowBytes * height;
	
	CGImageRef image = 0;
	CGDataProviderRef provider = 0;
	#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5
	static CGDataProviderSequentialCallbacks callbacks = { 0, CDataProvider::getBytes, CDataProvider::skipForward, CDataProvider::rewind, CDataProvider::releaseProvider };
	if (transparent)
		provider = CGDataProviderCreateSequential (new CDataProvider (pixels, transparentCColor), &callbacks);
	#else
	static CGDataProviderCallbacks callbacks = { CDataProvider::getBytes, CDataProvider::skipBytes, CDataProvider::rewind, CDataProvider::releaseProvider };
	if (transparent)
		provider = CGDataProviderCreate (new CDataProvider (pixels, transparentCColor), &callbacks);
	#endif
	else
		provider = CGDataProviderCreateWithData (NULL, pixels, size, NULL);
	CGImageAlphaInfo alphaInfo = kCGImageAlphaFirst;
	if (bitDepth != 32)
		alphaInfo = kCGImageAlphaNone;
	image = CGImageCreate (width, height, 8, bitDepth, rowBytes, GetGenericRGBColorSpace (), alphaInfo, provider, NULL, false, kCGRenderingIntentDefault);
	CGDataProviderRelease (provider);

	cgImage = image;
	CGImageRetain (image);
	return image;
}

//-----------------------------------------------------------------------------
void CBitmap::setBitsDirty ()
{
	if (pHandle && cgImage)
	{
		CGImageRelease ((CGImageRef)cgImage);
		cgImage = 0;
	}
}

#endif

#if GDIPLUS
//-----------------------------------------------------------------------------
Gdiplus::Bitmap* CBitmap::getBitmap ()
{
	if (pBitmap == 0 && pHandle)
	{
		if (bits) // it¥s a png image
		{
			pBitmap = new Gdiplus::Bitmap ((INT)width, (INT)height, 4*(INT)width, PixelFormat32bppPARGB, (unsigned char*)bits);
		}
		else
			pBitmap = new Gdiplus::Bitmap ((HBITMAP)pHandle, 0);
	}

	return pBitmap;
}

#endif

//-----------------------------------------------------------------------------
void CBitmap::draw (CDrawContext* pContext, CRect &rect, const CPoint &offset)
{
#if WINDOWS
	#if GDIPLUS
	drawAlphaBlend (pContext, rect, offset, 255.f * pContext->getGlobalAlpha ());
	#else
	#if USE_ALPHA_BLEND
	if (!noAlpha)
	{
		drawAlphaBlend (pContext, rect, offset, 255);
		return;
	}
	#endif	

	if (pHandle)
	{
		HGDIOBJ hOldObj;
		HDC hdcMemory = CreateCompatibleDC ((HDC)pContext->pSystemContext);
		hOldObj = SelectObject (hdcMemory, pHandle);
		BitBlt ((HDC)pContext->pSystemContext, 
						rect.left + pContext->offset.h, rect.top + pContext->offset.v, rect.width (), rect.height (), 
						(HDC)hdcMemory, offset.h, offset.v, SRCCOPY);
		SelectObject (hdcMemory, hOldObj);
		DeleteDC (hdcMemory);
	}
	#endif

#elif VSTGUI_USES_COREGRAPHICS
	drawAlphaBlend (pContext, rect, offset, 255);

#endif
}

//-----------------------------------------------------------------------------
void CBitmap::drawTransparent (CDrawContext* pContext, CRect &rect, const CPoint &offset)
{
#if WINDOWS
	#if GDIPLUS
	drawAlphaBlend (pContext, rect, offset, 255.f * pContext->getGlobalAlpha ());
	#else
	#if USE_ALPHA_BLEND
	if (!noAlpha)
	{
		drawAlphaBlend (pContext, rect, offset, 255);
		return;
	}
	#endif	

	BITMAP bm;
	HDC hdcBitmap;
	POINT ptSize;

	hdcBitmap = CreateCompatibleDC ((HDC)pContext->pSystemContext);
	SelectObject (hdcBitmap, pHandle);	 // Select the bitmap

	GetObject (pHandle, sizeof (BITMAP), (LPSTR)&bm);
	ptSize.x = bm.bmWidth;            // Get width of bitmap
	ptSize.y = bm.bmHeight;           // Get height of bitmap
	DPtoLP (hdcBitmap, &ptSize, 1);   // Convert from device to logical points

	DrawTransparent (pContext, rect, offset, hdcBitmap, ptSize, (HBITMAP)pMask, RGB(transparentCColor.red, transparentCColor.green, transparentCColor.blue));

	DeleteDC (hdcBitmap);
	#endif

#elif VSTGUI_USES_COREGRAPHICS
	if (noAlpha)
	{
		CGImageRef image = createCGImage (true);
		if (image)
		{
			drawAlphaBlend (pContext, rect, offset, 255);
			CGImageRelease (image);
		}
	}
	else
		drawAlphaBlend (pContext, rect, offset, 255);
        
#endif
}

//-----------------------------------------------------------------------------
void CBitmap::drawAlphaBlend (CDrawContext* pContext, CRect &rect, const CPoint &offset, unsigned char alpha)
{
#if WINDOWS
	#if GDIPLUS
	Gdiplus::Bitmap* bitmap = getBitmap ();
	if (bitmap)
	{
		Gdiplus::Graphics* graphics = pContext->getGraphics ();
		if (graphics)
		{
			Gdiplus::ImageAttributes imageAtt;
			if (alpha != 255)
			{
				// introducing the alpha blend matrix
				float alfa = ((float)alpha / 255.f);
				Gdiplus::ColorMatrix colorMatrix =
				{
					1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 0.0f, alfa, 0.0f,
					0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
				};
				// create the imageattribute modifier
				imageAtt.SetColorMatrix(&colorMatrix, Gdiplus::ColorMatrixFlagsDefault,
					Gdiplus::ColorAdjustTypeBitmap);
				// create a temporary bitmap to prevent OutOfMemory errors
				Gdiplus::Bitmap myBitmapBuffer((INT)rect.getWidth(), (INT)rect.getHeight(),PixelFormat32bppARGB);
				// create a graphics context for the temporary bitmap
				Gdiplus::Graphics* myGraphicsBuffer = Gdiplus::Graphics::FromImage(&myBitmapBuffer);
				// copy the rectangle we want to paint to the temporary bitmap
				Gdiplus::Rect	myTransRect(
					0,
					0,
					(INT)rect.getWidth (),
					(INT)rect.getHeight ());
				// transfer the bitmap (without modification by imageattr!)
				myGraphicsBuffer->DrawImage(
					pBitmap,myTransRect,
					(INT)offset.x,
					(INT)offset.y,
					(INT)rect.getWidth(),
					(INT)rect.getHeight(),
					Gdiplus::UnitPixel,
					0);
				// now transfer the temporary to the real context at the advised location
				Gdiplus::Rect	myDestRect(
					(INT)rect.left + (INT)pContext->offset.h,
					(INT)rect.top + (INT)pContext->offset.v,
					(INT)rect.getWidth (),
					(INT)rect.getHeight ());
				// transfer from temporary bitmap to real context (with imageattr)
				graphics->DrawImage (
					&myBitmapBuffer,
					myDestRect,
					(INT)0,
					(INT)0,
					(INT)rect.getWidth(),
					(INT)rect.getHeight(),
					Gdiplus::UnitPixel,
					&imageAtt);
				// delete the temporary context of the temporary bitmap
				delete myGraphicsBuffer;
			}
			else
			{
				Gdiplus::Rect	myDestRect(
					(INT)rect.left + (INT)pContext->offset.h,
					(INT)rect.top + (INT)pContext->offset.v,
					(INT)rect.getWidth (),
					(INT)rect.getHeight ());
				graphics->DrawImage (
					pBitmap,
					myDestRect,
					(INT)offset.x,
					(INT)offset.y,
					(INT)rect.getWidth(),
					(INT)rect.getHeight(),
					Gdiplus::UnitPixel,
					0);
			}
		}
	}
	#else
	if (pHandle)
	{
		HGDIOBJ hOldObj;
		HDC hdcMemory = CreateCompatibleDC ((HDC)pContext->pSystemContext);
		hOldObj = SelectObject (hdcMemory, pHandle);

		BLENDFUNCTION blendFunction;
		blendFunction.BlendOp = AC_SRC_OVER;
		blendFunction.BlendFlags = 0;
		blendFunction.SourceConstantAlpha = alpha;
		#if USE_ALPHA_BLEND
		if (noAlpha)
			blendFunction.AlphaFormat = 0;//AC_SRC_NO_ALPHA;
		else
			blendFunction.AlphaFormat = AC_SRC_ALPHA;
		#else
		blendFunction.AlphaFormat = 0;//AC_SRC_NO_ALPHA;
		#endif
		#if DYNAMICALPHABLEND
		// check for Win98 as it has a bug in AlphaBlend
		if (gSystemVersion.dwMajorVersion == 4 && gSystemVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS && gSystemVersion.dwMinorVersion == 10)
		{
			HGDIOBJ hOldObj1;
			HDC hdcMemory1 = CreateCompatibleDC ((HDC)pContext->pSystemContext);
			HBITMAP hbmp = CreateCompatibleBitmap(hdcMemory, rect.width(), rect.height());
			//this does NOT work:
			//HBITMAP hbmp = CreateCompatibleBitmap(hdcMemory1, rect.width(), rect.height());
			hOldObj1 = SelectObject (hdcMemory1, hbmp);

			//copy contents of original picture in hdcMemory 
			//from the offset to hdcMemory1 (0,0)
			long res = BitBlt((HDC)hdcMemory1, 
					0, 0, 
					rect.width(), rect.height(), 
					(HDC)hdcMemory, offset.h, offset.v, SRCCOPY);

			//Copy the resulting image with alpha blending:
			(*pfnAlphaBlend) ((HDC)pContext->pSystemContext, 
						rect.left + pContext->offset.h, rect.top + pContext->offset.v,
						rect.width (), rect.height (), 
						hdcMemory1,
						0, 0,//the offset is done in BitBlt
						rect.width (), rect.height (),
						blendFunction);
			SelectObject (hdcMemory1, hOldObj1);
			DeleteDC(hdcMemory1);
			DeleteObject(hbmp);
		}
		else
		{
			(*pfnAlphaBlend) ((HDC)pContext->pSystemContext, 
						rect.left + pContext->offset.h, rect.top + pContext->offset.v,
						rect.width (), rect.height (), 
						(HDC)hdcMemory,
						offset.h, offset.v,
						rect.width (), rect.height (),
						blendFunction);
		}
		#else
		AlphaBlend ((HDC)pContext->pSystemContext, 
					rect.left + pContext->offset.h, rect.top + pContext->offset.v,
					rect.width (), rect.height (), 
					(HDC)hdcMemory,
					offset.h, offset.v,
					rect.width (), rect.height (),
					blendFunction);
		#endif
		SelectObject (hdcMemory, hOldObj);
		DeleteDC (hdcMemory);
	}
	#endif

#elif VSTGUI_USES_COREGRAPHICS
	if (pHandle || cgImage)
	{
		CGContextRef context = pContext->beginCGContext ();
		if (context)
		{
			if (alpha != 255)
				CGContextSetAlpha (context, (CGFloat)alpha / 255.);
			
			CGImageRef image = createCGImage ();

			if (image)
			{
				CGRect dest;
				dest.origin.x = rect.left - offset.h + pContext->offset.h;
				dest.origin.y = (rect.top + pContext->offset.v) * -1 - (getHeight () - offset.v);
				dest.size.width = getWidth ();
				dest.size.height = getHeight ();
				
				CRect ccr;
				pContext->getClipRect (ccr);
				CGRect cgClipRect = CGRectMake (ccr.left + pContext->offset.h, (ccr.top + pContext->offset.v) * -1 - ccr.height (), ccr.width (), ccr.height ());
				CGContextClipToRect (context, cgClipRect);

				CGRect clipRect;
				clipRect.origin.x = rect.left + pContext->offset.h;
			    clipRect.origin.y = (rect.top + pContext->offset.v) * -1  - rect.height ();
			    clipRect.size.width = rect.width (); 
			    clipRect.size.height = rect.height ();
				
				CGContextClipToRect (context, clipRect);

				CGContextDrawImage (context, dest, image);
				CGImageRelease (image);
			}
			pContext->releaseCGContext (context);
		}
	}
	
#endif
}

//-----------------------------------------------------------------------------
void CBitmap::setTransparentColor (const CColor color)
{
	transparentCColor = color;
#if VSTGUI_USES_COREGRAPHICS
	if (noAlpha)
	{
		if (pHandle)
		{
			if (cgImage)
				CGImageRelease ((CGImageRef)cgImage);
			cgImage = 0;
		}
		else if (cgImage)
		{
			if (CGImageGetBitsPerComponent((CGImageRef)cgImage) == 8)
			{
				CGFloat myMaskingColors[] = { color.red, color.red, color.green, color.green, color.blue, color.blue };
				CGImageRef newImage = CGImageCreateWithMaskingColors ((CGImageRef)cgImage, myMaskingColors);
				if (newImage)
				{
					CGImageRelease ((CGImageRef)cgImage);
					cgImage = newImage;
					noAlpha = false;
				}
			}
			#if DEBUG
			else
			{
				DebugPrint ("Setting a transparent color for an indexed bitmap does not work yet.\n");
			}
			#endif
		}
	}
#endif
}

#if WINDOWS && USE_LIBPNG
PNGResourceStream::PNGResourceStream ()
: streamPos (0)
, resData (0)
, resSize (0)
{
}

PNGResourceStream::~PNGResourceStream ()
{
}

bool PNGResourceStream::open (const CResourceDescription& resourceDesc)
{
	HRSRC rsrc = FindResourceA (GetInstance (), resourceDesc.type == CResourceDescription::kIntegerType ? MAKEINTRESOURCEA (resourceDesc.u.id) : resourceDesc.u.name, "PNG");
	if (rsrc)
	{
		resSize = SizeofResource (GetInstance (), rsrc);
		HGLOBAL resDataLoad = LoadResource (GetInstance (), rsrc);
		if (resDataLoad)
		{
			resData = LockResource (resDataLoad);
			return true;
		}
	}
	return false;
}

void PNGResourceStream::read (unsigned char* ptr, size_t size)
{
	if (streamPos + size <= resSize)
	{
		memcpy (ptr, ((unsigned char*)resData+streamPos), size);
		streamPos += size;
	}
}

static void PNGResourceStream::readCallback (png_struct* pngPtr, unsigned char* ptr, size_t size)
{
	void* obj = png_get_io_ptr (pngPtr);
	if (obj)
		((PNGResourceStream*)obj)->read (ptr, size);
}
#endif // WINDOWS && USE_LIBPNG

END_NAMESPACE_VSTGUI
