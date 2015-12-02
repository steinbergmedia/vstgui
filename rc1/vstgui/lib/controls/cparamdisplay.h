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

#ifndef __cparamdisplay__
#define __cparamdisplay__

#include "ccontrol.h"
#include "../cfont.h"
#include "../ccolor.h"
#include "../cdrawdefs.h"
#if VSTGUI_HAS_FUNCTIONAL
#include <functional>
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
typedef bool (*CParamDisplayValueToStringProc) (float value, char utf8String[256], void* userData);

//-----------------------------------------------------------------------------
// CParamDisplay Declaration
//! @brief a parameter display
/// @ingroup views
//-----------------------------------------------------------------------------
class CParamDisplay : public CControl
{
public:
	CParamDisplay (const CRect& size, CBitmap* background = 0, const int32_t style = 0);
	CParamDisplay (const CParamDisplay& paramDisplay);
	
	//-----------------------------------------------------------------------------
	/// @name CParamDisplay Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setFont (CFontRef fontID);
	const CFontRef getFont () const { return fontID; }

	virtual void setFontColor (CColor color);
	CColor getFontColor () const { return fontColor; }

	virtual void setBackColor (CColor color);
	CColor getBackColor () const { return backColor; }

	virtual void setFrameColor (CColor color);
	CColor getFrameColor () const { return frameColor; }

	virtual void setShadowColor (CColor color);
	CColor getShadowColor () const { return shadowColor; }

	virtual void setAntialias (bool state) { bAntialias = state; }
	bool getAntialias () const { return bAntialias; }

	virtual void setHoriAlign (CHoriTxtAlign hAlign);
	CHoriTxtAlign getHoriAlign () const { return horiTxtAlign; }

	virtual void setTextInset (const CPoint& p);
	CPoint getTextInset () const { return textInset; }

	virtual void setTextRotation (double angle);
	double getTextRotation () const { return textRotation; }

	virtual void setRoundRectRadius (const CCoord& radius);
	CCoord getRoundRectRadius () const { return roundRectRadius; }

	virtual void setFrameWidth (const CCoord& width);
	CCoord getFrameWidth () const { return frameWidth; }

#if VSTGUI_HAS_FUNCTIONAL
	typedef CParamDisplay	ValueToStringUserData;
#else
	typedef void			ValueToStringUserData;
#endif

	VSTGUI_DEPRECATED_FUNCTIONAL(virtual void setValueToStringProc (CParamDisplayValueToStringProc proc, void* userData = 0);) ///< deprecated use setValueToStringFunction instead if you use c++11
#if VSTGUI_HAS_FUNCTIONAL
	typedef std::function<bool(float value, char utf8String[256], CParamDisplay* display)> ValueToStringFunction;
	
	void setValueToStringFunction (const ValueToStringFunction& valueToStringFunc);
	void setValueToStringFunction (ValueToStringFunction&& valueToStringFunc);
#endif

	virtual void setStyle (int32_t val);
	int32_t getStyle () const { return style; }

	virtual void setPrecision (uint8_t precision);
	uint8_t getPrecision () const { return valuePrecision; }

	//@}

	void draw (CDrawContext* pContext) VSTGUI_OVERRIDE_VMETHOD;
	bool getFocusPath (CGraphicsPath& outPath) VSTGUI_OVERRIDE_VMETHOD;
	bool removed (CView* parent) VSTGUI_OVERRIDE_VMETHOD;

	CLASS_METHODS(CParamDisplay, CControl)
protected:
	~CParamDisplay ();
	virtual void drawBack (CDrawContext* pContext, CBitmap* newBack = 0);
	VSTGUI_DEPRECATED (void drawText (CDrawContext* pContext, UTF8StringPtr string);)
	VSTGUI_DEPRECATED (void drawText (CDrawContext* pContext, UTF8StringPtr string, const CRect& size);)

	virtual void drawPlatformText (CDrawContext* pContext, IPlatformString* string);
	virtual void drawPlatformText (CDrawContext* pContext, IPlatformString* string, const CRect& size);

	virtual void drawStyleChanged ();

#if VSTGUI_HAS_FUNCTIONAL
	ValueToStringFunction valueToStringFunction;
#else
	CParamDisplayValueToStringProc valueToString;
	void* valueToStringUserData;
#endif

	CHoriTxtAlign horiTxtAlign;
	int32_t		style;
	uint8_t		valuePrecision;

	CFontRef	fontID;
	CColor		fontColor;
	CColor		backColor;
	CColor		frameColor;
	CColor		shadowColor;
	CPoint		textInset;
	CCoord		roundRectRadius;
	CCoord		frameWidth;
	double		textRotation;
	bool		bAntialias;
};

} // namespace

#endif
