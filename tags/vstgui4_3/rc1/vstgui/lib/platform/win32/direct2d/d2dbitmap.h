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

#ifndef __d2dbitmap__
#define __d2dbitmap__

#include "../win32bitmapbase.h"

#if WINDOWS && VSTGUI_DIRECT2D_SUPPORT

#include "../../../cpoint.h"

struct IWICBitmapSource;
struct ID2D1Bitmap;
struct ID2D1RenderTarget;
struct IWICBitmap;
struct IWICBitmapLock;

#include <map>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class D2DBitmap : public Win32BitmapBase
{
public:
	D2DBitmap ();
	D2DBitmap (const CPoint& size);
	~D2DBitmap ();

	bool load (const CResourceDescription& desc) VSTGUI_OVERRIDE_VMETHOD;
	const CPoint& getSize () const VSTGUI_OVERRIDE_VMETHOD { return size; }
	IPlatformBitmapPixelAccess* lockPixels (bool alphaPremultiplied) VSTGUI_OVERRIDE_VMETHOD;
	void setScaleFactor (double factor) VSTGUI_OVERRIDE_VMETHOD { scaleFactor = factor; }
	double getScaleFactor () const VSTGUI_OVERRIDE_VMETHOD { return scaleFactor; }

	HBITMAP createHBitmap ();
	bool loadFromStream (IStream* stream);

	IWICBitmapSource* getSource () const { return source; }
	IWICBitmap* getBitmap ();
	bool createMemoryPNGRepresentation (void** ptr, uint32_t& size);
//-----------------------------------------------------------------------------
protected:
	void replaceBitmapSource (IWICBitmapSource* newSourceBitmap);

	class PixelAccess : public IPlatformBitmapPixelAccess
	{
	public:
		PixelAccess ();
		~PixelAccess ();

		bool init (D2DBitmap* bitmap, bool alphaPremultiplied);

		uint8_t* getAddress () const { return (uint8_t*)ptr; }
		uint32_t getBytesPerRow () const { return bytesPerRow; }
		PixelFormat getPixelFormat () const { return kBGRA; }

	protected:
		static void premultiplyAlpha (BYTE* ptr, UINT bytesPerRow, const CPoint& size);
		static void unpremultiplyAlpha (BYTE* ptr, UINT bytesPerRow, const CPoint& size);

		D2DBitmap* bitmap;
		IWICBitmapLock* bLock;
		BYTE* ptr;
		UINT bytesPerRow;
		bool alphaPremultiplied;
	};

	CPoint size;
	double scaleFactor;
	IWICBitmapSource* source;
};

//-----------------------------------------------------------------------------
class D2DBitmapCache
{
public:
	ID2D1Bitmap* getBitmap (D2DBitmap* bitmap, ID2D1RenderTarget* renderTarget);
	
	void removeBitmap (D2DBitmap* bitmap);
	void removeRenderTarget (ID2D1RenderTarget* renderTarget);

	static D2DBitmapCache* instance ();
//-----------------------------------------------------------------------------
protected:
	D2DBitmapCache ();
	~D2DBitmapCache ();
	ID2D1Bitmap* createBitmap (D2DBitmap* bitmap, ID2D1RenderTarget* renderTarget);
	typedef std::map<ID2D1RenderTarget*, ID2D1Bitmap*> RenderTargetBitmapMap;
	typedef std::map<D2DBitmap*, RenderTargetBitmapMap> BitmapCache;
	BitmapCache cache;
};

} // namespace

#endif // WINDOWS

#endif // __d2dbitmap__
