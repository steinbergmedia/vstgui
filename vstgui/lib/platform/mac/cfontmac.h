//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __cfontmac__
#define __cfontmac__

#include "../../cfont.h"

#if MAC

#include "../../ccolor.h"
#if TARGET_OS_IPHONE
	#include <CoreText/CoreText.h>
#else
	#include <ApplicationServices/ApplicationServices.h>
#endif

namespace VSTGUI {

#define VSTGUI_USES_CORE_TEXT	(MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5 || TARGET_OS_IPHONE)

#if VSTGUI_USES_CORE_TEXT
//-----------------------------------------------------------------------------
class CoreTextFont : public IPlatformFont, public IFontPainter
{
public:
	CoreTextFont (UTF8StringPtr name, const CCoord& size, const int32_t& style);

	CTFontRef getFontRef () const { return fontRef; }

//------------------------------------------------------------------------------------
protected:
	~CoreTextFont ();

	void drawString (CDrawContext* context, const CString& string, const CPoint& p, bool antialias = true) VSTGUI_OVERRIDE_VMETHOD;
	CCoord getStringWidth (CDrawContext* context, const CString& string, bool antialias = true) VSTGUI_OVERRIDE_VMETHOD;
	CFDictionaryRef getStringAttributes (const CGColorRef color = 0);

	double getAscent () const VSTGUI_OVERRIDE_VMETHOD;
	double getDescent () const VSTGUI_OVERRIDE_VMETHOD;
	double getLeading () const VSTGUI_OVERRIDE_VMETHOD;
	double getCapHeight () const VSTGUI_OVERRIDE_VMETHOD;

	IFontPainter* getPainter () VSTGUI_OVERRIDE_VMETHOD { return this; }

	CTFontRef fontRef;
	int32_t style;
	bool underlineStyle;
	CColor lastColor;
	CFMutableDictionaryRef stringAttributes;
};

#else // VSTGUI_USES_CORE_TEXT
//-----------------------------------------------------------------------------
class ATSUFont : public IPlatformFont, public IFontPainter
{
public:
	ATSUFont (UTF8StringPtr name, const CCoord& size, const int32_t& style);

	ATSUStyle getATSUStyle () const { return atsuStyle; }

protected:
	~ATSUFont ();

	void drawString (CDrawContext* context, const CString& string, const CPoint& p, bool antialias = true) VSTGUI_OVERRIDE_VMETHOD;
	CCoord getStringWidth (CDrawContext* context, const CString& string, bool antialias = true) VSTGUI_OVERRIDE_VMETHOD;

	double getAscent () const VSTGUI_OVERRIDE_VMETHOD { return -1.; }
	double getDescent () const VSTGUI_OVERRIDE_VMETHOD { return -1.; }
	double getLeading () const VSTGUI_OVERRIDE_VMETHOD { return -1.; }
	double getCapHeight () const VSTGUI_OVERRIDE_VMETHOD { return -1.; }

	IFontPainter* getPainter () VSTGUI_OVERRIDE_VMETHOD { return this; }

	ATSUStyle atsuStyle;
};

#endif

} // namespace

#endif // MAC

#endif
