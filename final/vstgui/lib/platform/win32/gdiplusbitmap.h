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

#ifndef __gdiplusbitmap__
#define __gdiplusbitmap__

#include "win32bitmapbase.h"

#if WINDOWS

#include "../../cpoint.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class GdiplusBitmap : public Win32BitmapBase
{
public:
	GdiplusBitmap ();
	GdiplusBitmap (const CPoint& size);
	~GdiplusBitmap ();

	bool load (const CResourceDescription& desc) VSTGUI_OVERRIDE_VMETHOD;
	const CPoint& getSize () const VSTGUI_OVERRIDE_VMETHOD { return size; }
	IPlatformBitmapPixelAccess* lockPixels (bool alphaPremultiplied) VSTGUI_OVERRIDE_VMETHOD;
	void setScaleFactor (double factor) VSTGUI_OVERRIDE_VMETHOD {}
	double getScaleFactor () const VSTGUI_OVERRIDE_VMETHOD { return 1.; }

	Gdiplus::Bitmap* getBitmap () const { return bitmap; }
	HBITMAP createHBitmap ();
	bool loadFromStream (IStream* stream);
	bool createMemoryPNGRepresentation (void** ptr, uint32_t& size);

//-----------------------------------------------------------------------------
protected:
	class PixelAccess : public IPlatformBitmapPixelAccess
	{
	public:
		PixelAccess ();
		~PixelAccess ();

		bool init (GdiplusBitmap* bitmap, bool alphaPremulitplied);

		uint8_t* getAddress () const { return (uint8_t*)data.Scan0; }
		uint32_t getBytesPerRow () const { return data.Stride; }
		PixelFormat getPixelFormat () const { return kBGRA; }
	protected:
		GdiplusBitmap* bitmap;
		Gdiplus::BitmapData data;
	};
	Gdiplus::Bitmap* bitmap;
	CPoint size;
	bool allocatedByGdi;
};

} // namespace

#endif // WINDOWS

#endif // __gdiplusbitmap__

