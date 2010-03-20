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

#ifndef __d2dbitmap__
#define __d2dbitmap__

#include "../../iplatformbitmap.h"

#if WINDOWS && VSTGUI_DIRECT2D_SUPPORT

struct IWICFormatConverter;
struct IWICBitmapSource;
struct ID2D1Bitmap;
struct ID2D1RenderTarget;
struct IWICBitmap;

#include <map>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class D2DBitmap : public IPlatformBitmap
{
public:
	D2DBitmap ();
	~D2DBitmap ();

	bool load (const CResourceDescription& desc);
	const CPoint& getSize () const { return size; }

	virtual IWICBitmapSource* getSource ();
//-----------------------------------------------------------------------------
protected:
	D2DBitmap (const CPoint& size);

	CPoint size;
	IWICFormatConverter* converter;
};

//-----------------------------------------------------------------------------
class D2DOffscreenBitmap : public D2DBitmap
{
public:
	D2DOffscreenBitmap (const CPoint& size);
	bool load (const CResourceDescription& desc) { return false; }

	IWICBitmapSource* getSource ();
	IWICBitmap* getBitmap () const { return bitmap; }
protected:
	IWICBitmap* bitmap;
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
	D2DBitmapCache () {};
	~D2DBitmapCache ();
	ID2D1Bitmap* createBitmap (D2DBitmap* bitmap, ID2D1RenderTarget* renderTarget);
	std::map<D2DBitmap*, std::map<ID2D1RenderTarget*, ID2D1Bitmap*> > cache;
};

} // namespace

#endif // WINDOWS

#endif // __d2dbitmap__
