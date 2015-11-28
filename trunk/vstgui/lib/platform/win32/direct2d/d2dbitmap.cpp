//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "d2dbitmap.h"

#if WINDOWS && VSTGUI_DIRECT2D_SUPPORT

#include "../win32support.h"
#include <wincodec.h>
#include <d2d1.h>
#include <shlwapi.h>
#include <cassert>

#pragma comment (lib,"windowscodecs.lib")

namespace VSTGUI {

//-----------------------------------------------------------------------------
class WICGlobal
{
public:
	static IWICImagingFactory* getFactory ();
protected:
	WICGlobal ();
	~WICGlobal ();
	IWICImagingFactory* factory;
};

//-----------------------------------------------------------------------------
D2DBitmap::D2DBitmap ()
: source (0)
, scaleFactor (1.)
{
}

//-----------------------------------------------------------------------------
D2DBitmap::D2DBitmap (const CPoint& size)
: source (0)
, size (size)
, scaleFactor (1.)
{
	REFWICPixelFormatGUID pixelFormat = GUID_WICPixelFormat32bppPBGRA;
	WICBitmapCreateCacheOption options = WICBitmapCacheOnLoad;
	IWICBitmap* bitmap = 0;
	HRESULT hr = WICGlobal::getFactory ()->CreateBitmap ((UINT)size.x, (UINT)size.y, pixelFormat, options, &bitmap);
	if (hr == S_OK && bitmap)
	{
		source = bitmap;
	}
	else
	{
	#if DEBUG
		DebugPrint ("Could not create Bitmap with size : %d, %d\n", (int)size.x, (int)size.y);
	#endif
	}
}

//-----------------------------------------------------------------------------
D2DBitmap::~D2DBitmap ()
{
	D2DBitmapCache* gCache = D2DBitmapCache::instance ();
	if (gCache)
		gCache->removeBitmap (this);
	if (source)
		source->Release ();
}

//-----------------------------------------------------------------------------
IWICBitmap* D2DBitmap::getBitmap ()
{
	if (getSource () == 0)
		return 0;

	IWICBitmap* icBitmap = 0;
	if (!SUCCEEDED (getSource ()->QueryInterface (IID_IWICBitmap, (void**)&icBitmap)))
	{
		if (SUCCEEDED (WICGlobal::getFactory ()->CreateBitmapFromSource (getSource (), WICBitmapCacheOnDemand, &icBitmap)))
		{
			replaceBitmapSource (icBitmap);
		}
	}
	if (icBitmap)
		icBitmap->Release ();
	return icBitmap;
}

//-----------------------------------------------------------------------------
bool D2DBitmap::createMemoryPNGRepresentation (void** ptr, uint32_t& size)
{
	if (getSource () == 0)
		return false;

	bool result = false;
	IWICBitmapEncoder* encoder = 0;
	if (SUCCEEDED (WICGlobal::getFactory ()->CreateEncoder (GUID_ContainerFormatPng, NULL, &encoder)))
	{
		IStream* stream = 0;
		if (SUCCEEDED (CreateStreamOnHGlobal (NULL, TRUE, &stream)))
		{
			if (SUCCEEDED (encoder->Initialize (stream, WICBitmapEncoderNoCache)))
			{
				IWICBitmapFrameEncode* frame = 0;
				if (SUCCEEDED (encoder->CreateNewFrame (&frame, NULL)))
				{
					if (SUCCEEDED (frame->Initialize (NULL)))
					{
						if (SUCCEEDED (frame->WriteSource (this->getSource (), NULL)))
						{
							if (SUCCEEDED (frame->Commit ()))
							{
								if (SUCCEEDED (encoder->Commit ()))
								{
									HGLOBAL hGlobal;
									if (SUCCEEDED (GetHGlobalFromStream (stream, &hGlobal)))
									{
										void* globalAddress = GlobalLock (hGlobal);
										SIZE_T globalSize = GlobalSize (hGlobal);
										if (globalSize && globalAddress)
										{
											*ptr = std::malloc (globalSize);
											size = (uint32_t)globalSize;
											memcpy (*ptr, globalAddress, globalSize);
											result = true;
										}
										GlobalUnlock (hGlobal);
									}
								}
							}
						}
					}
					frame->Release ();
				}
			}

			stream->Release ();
		}
		encoder->Release ();
	}
	return result;
}

//-----------------------------------------------------------------------------
bool D2DBitmap::loadFromStream (IStream* iStream)
{
	IWICBitmapDecoder* decoder = 0;
	IWICStream* stream = 0;
	if (SUCCEEDED (WICGlobal::getFactory ()->CreateStream (&stream)))
	{
		if (SUCCEEDED (stream->InitializeFromIStream (iStream)))
		{
			WICGlobal::getFactory ()->CreateDecoderFromStream (stream, NULL, WICDecodeMetadataCacheOnLoad, &decoder);
		}
		stream->Release ();
	}
	if (decoder)
	{
		IWICBitmapFrameDecode* frame;
		if (SUCCEEDED (decoder->GetFrame (0, &frame)))
		{
			UINT w = 0;
			UINT h = 0;
			frame->GetSize (&w, &h);
			size.x = w;
			size.y = h;
			IWICFormatConverter* converter = 0;
			WICGlobal::getFactory ()->CreateFormatConverter (&converter);
			if (converter)
			{
				if (!SUCCEEDED (converter->Initialize (frame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut)))
				{
					converter->Release ();
					converter = 0;
				}
				else
					source = converter;
			}
			frame->Release ();
		}
		decoder->Release ();
	}
	return source != 0;
}

//-----------------------------------------------------------------------------
bool D2DBitmap::load (const CResourceDescription& resourceDesc)
{
	if (source)
		return true;

	bool result = false;
	ResourceStream* resourceStream = new ResourceStream;
	if (resourceStream->open (resourceDesc, "PNG"))
	{
		result = loadFromStream (resourceStream);
	}
	resourceStream->Release ();

#if DEBUG
	if (result == false && resourceDesc.type == CResourceDescription::kStringType)
	{
		// In DEBUG mode we allow to load the bitmap from a path so that the WYSIWYG editor is usable
		UTF8StringHelper path (resourceDesc.u.name);
		IStream* stream = 0;
		if (SUCCEEDED (SHCreateStreamOnFileEx (path, STGM_READ|STGM_SHARE_DENY_WRITE, 0, false, 0, &stream)))
		{
			result = loadFromStream (stream);
			stream->Release ();
		}
	}
#endif
	return result;
}

//-----------------------------------------------------------------------------
HBITMAP D2DBitmap::createHBitmap ()
{
	if (getSource () == 0)
		return 0;

	BITMAPINFO pbmi = {0};
	pbmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi.bmiHeader.biPlanes = 1;
	pbmi.bmiHeader.biCompression = BI_RGB;
	pbmi.bmiHeader.biWidth = (LONG)size.x;
	pbmi.bmiHeader.biHeight = (LONG)size.y;
	pbmi.bmiHeader.biBitCount = 32;

	HDC hdc = GetDC (NULL);
	if (hdc == 0)
		return 0;
	BYTE* bits = 0;
	HBITMAP result = CreateDIBSection (hdc, &pbmi, DIB_RGB_COLORS, reinterpret_cast<void**> (&bits), 0, 0);
	if (result)
	{
		getSource ()->CopyPixels (NULL, (INT)size.x * sizeof (DWORD), (INT)size.x * sizeof (DWORD) * (INT)size.y, bits);
	}
	return result;
}

//-----------------------------------------------------------------------------
void D2DBitmap::replaceBitmapSource (IWICBitmapSource* newSourceBitmap)
{
	if (source)
	{
		D2DBitmapCache::instance ()->removeBitmap (this);
		source->Release ();
	}
	source = newSourceBitmap;
	if (source)
		source->AddRef ();
}

//-----------------------------------------------------------------------------
IPlatformBitmapPixelAccess* D2DBitmap::lockPixels (bool alphaPremultiplied)
{
	if (getSource () == 0)
		return 0;
	PixelAccess* pixelAccess = new PixelAccess;
	if (pixelAccess->init (this, alphaPremultiplied))
		return pixelAccess;
	pixelAccess->forget ();
	return 0;
}

//-----------------------------------------------------------------------------
D2DBitmap::PixelAccess::PixelAccess ()
: bitmap (0)
, bLock (0)
, ptr (0)
, bytesPerRow (0)
{
}

//-----------------------------------------------------------------------------
D2DBitmap::PixelAccess::~PixelAccess ()
{
	if (bLock)
	{
		if (!alphaPremultiplied)
			premultiplyAlpha (ptr, bytesPerRow, bitmap->getSize ());
		bLock->Release ();
	}
	if (bitmap)
	{
		D2DBitmapCache::instance ()->removeBitmap (bitmap);
		bitmap->forget ();
	}
}

//-----------------------------------------------------------------------------
bool D2DBitmap::PixelAccess::init (D2DBitmap* inBitmap, bool _alphaPremultiplied)
{
	bool result = false;
	vstgui_assert (inBitmap);
	IWICBitmap* icBitmap = inBitmap->getBitmap ();
	if (icBitmap)
	{
		WICRect rcLock = { 0, 0, (INT)inBitmap->getSize ().x, (INT)inBitmap->getSize ().y };
		if (SUCCEEDED (icBitmap->Lock (&rcLock, WICBitmapLockRead | WICBitmapLockWrite, &bLock)))
		{
			bLock->GetStride (&bytesPerRow);
			UINT bufferSize;
			bLock->GetDataPointer (&bufferSize, &ptr);

			bitmap = inBitmap;
			bitmap->remember ();
			alphaPremultiplied = _alphaPremultiplied;
			if (!alphaPremultiplied)
				unpremultiplyAlpha (ptr, bytesPerRow, bitmap->getSize ());
			result = true;
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
void D2DBitmap::PixelAccess::premultiplyAlpha (BYTE* ptr, UINT bytesPerRow, const CPoint& size)
{
	for (int32_t y = 0; y < (int32_t)size.y; y++, ptr += bytesPerRow)
	{
		uint32_t* pixelPtr = (uint32_t*)ptr;
		for (int32_t x = 0; x < (int32_t)size.x; x++, pixelPtr++)
		{
			uint8_t* pixel = (uint8_t*)pixelPtr;
			if (pixel[3] == 0)
			{
				*pixelPtr = 0;
				continue;
			}
			pixel[0] = (uint32_t)((pixel[0] * pixel[3]) >> 8);
			pixel[1] = (uint32_t)((pixel[1] * pixel[3]) >> 8);
			pixel[2] = (uint32_t)((pixel[2] * pixel[3]) >> 8);
		}
	}
}

//-----------------------------------------------------------------------------
void D2DBitmap::PixelAccess::unpremultiplyAlpha (BYTE* ptr, UINT bytesPerRow, const CPoint& size)
{
	for (int32_t y = 0; y < (int32_t)size.y; y++, ptr += bytesPerRow)
	{
		uint32_t* pixelPtr = (uint32_t*)ptr;
		for (int32_t x = 0; x < (int32_t)size.x; x++, pixelPtr++)
		{
			uint8_t* pixel = (uint8_t*)pixelPtr;
			if (pixel[3] == 0)
				continue;
			pixel[0] = (uint32_t)(pixel[0] * 255) / pixel[3];
			pixel[1] = (uint32_t)(pixel[1] * 255) / pixel[3];
			pixel[2] = (uint32_t)(pixel[2] * 255) / pixel[3];
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ID2D1Bitmap* D2DBitmapCache::getBitmap (D2DBitmap* bitmap, ID2D1RenderTarget* renderTarget)
{
	BitmapCache::iterator it = cache.find (bitmap);
	if (it != cache.end ())
	{
		RenderTargetBitmapMap::iterator it2 = it->second.find (renderTarget);
		if (it2 != it->second.end ())
		{
			return it2->second;
		}
		ID2D1Bitmap* b = createBitmap (bitmap, renderTarget);
		if (b)
			it->second.insert (std::make_pair (renderTarget, b));
		return b;
	}
	std::pair<BitmapCache::iterator, bool> insertSuccess = cache.insert (std::make_pair (bitmap, RenderTargetBitmapMap ()));
	if (insertSuccess.second == true)
	{
		ID2D1Bitmap* b = createBitmap (bitmap, renderTarget);
		if (b)
		{
			insertSuccess.first->second.insert (std::make_pair (renderTarget, b));
			return b;
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
void D2DBitmapCache::removeBitmap (D2DBitmap* bitmap)
{
	BitmapCache::iterator it = cache.find (bitmap);
	if (it != cache.end ())
	{
		RenderTargetBitmapMap::iterator it2 = it->second.begin ();
		while (it2 != it->second.end ())
		{
			it2->second->Release ();
			it2++;
		}
		cache.erase (it);
	}
}

//-----------------------------------------------------------------------------
void D2DBitmapCache::removeRenderTarget (ID2D1RenderTarget* renderTarget)
{
	BitmapCache::iterator it = cache.begin ();
	while (it != cache.end ())
	{
		RenderTargetBitmapMap::iterator it2 = it->second.begin ();
		while (it2 != it->second.end ())
		{
			if (it2->first == renderTarget)
			{
				it2->second->Release ();
				it->second.erase (it2++);
			}
			else
				it2++;
		}
		it++;
	}
}

//-----------------------------------------------------------------------------
ID2D1Bitmap* D2DBitmapCache::createBitmap (D2DBitmap* bitmap, ID2D1RenderTarget* renderTarget)
{
	if (bitmap->getSource () == 0)
		return 0;
	ID2D1Bitmap* d2d1Bitmap = 0; 
	renderTarget->CreateBitmapFromWicBitmap (bitmap->getSource (), &d2d1Bitmap);
	return d2d1Bitmap;
}

static D2DBitmapCache* gD2DBitmapCache = 0;
//-----------------------------------------------------------------------------
D2DBitmapCache::D2DBitmapCache ()
{
	gD2DBitmapCache = this;
}

//-----------------------------------------------------------------------------
D2DBitmapCache::~D2DBitmapCache ()
{
#if DEBUG
	for (BitmapCache::const_iterator it = cache.begin (); it != cache.end (); it++)
	{
		vstgui_assert (it->second.size () == 0);
	}
#endif
	gD2DBitmapCache = 0;
}

//-----------------------------------------------------------------------------
D2DBitmapCache* D2DBitmapCache::instance ()
{
	static D2DBitmapCache gInstance;
	return gD2DBitmapCache;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
WICGlobal::WICGlobal ()
: factory (0)
{

#if _WIN32_WINNT > 0x601
// make sure when building with the Win 8.0 SDK we work on Win7
#define VSTGUI_WICImagingFactory CLSID_WICImagingFactory1
#else
#define VSTGUI_WICImagingFactory CLSID_WICImagingFactory
#endif

	CoCreateInstance (VSTGUI_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (void**)&factory);
	vstgui_assert (factory);
}

//-----------------------------------------------------------------------------
WICGlobal::~WICGlobal ()
{
	if (factory)
		factory->Release ();
}

//-----------------------------------------------------------------------------
IWICImagingFactory* WICGlobal::getFactory ()
{
	static WICGlobal wicGlobal;
	return wicGlobal.factory;
}

} // namespace

#endif // WINDOWS
