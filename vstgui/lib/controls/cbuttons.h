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

#ifndef __cbuttons__
#define __cbuttons__

#include "ccontrol.h"
#include "../cfont.h"
#include "../ccolor.h"
#include "../cbitmap.h"
#include "../cgradient.h"
#include "../cgraphicspath.h"
#include "../cstring.h"
#include "../cdrawmethods.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// COnOffButton Declaration
//! @brief a button control with 2 states
/// @ingroup controls
//-----------------------------------------------------------------------------
class COnOffButton : public CControl
{
public:
	COnOffButton (const CRect& size, IControlListener* listener = nullptr, int32_t tag = -1, CBitmap* background = nullptr, int32_t style = 0);
	COnOffButton (const COnOffButton& onOffButton);

	//-----------------------------------------------------------------------------
	/// @name COnOffButton Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual int32_t getStyle () const { return style; }
	virtual void setStyle (int32_t newStyle) { style = newStyle; }
	//@}

	// overrides
	virtual void draw (CDrawContext*) override;
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	virtual CMouseEventResult onMouseCancel () override;
	virtual int32_t onKeyDown (VstKeyCode& keyCode) override;
	virtual bool sizeToFit () override;

	CLASS_METHODS(COnOffButton, CControl)
protected:
	~COnOffButton () noexcept override = default;
	int32_t style;
};

//-----------------------------------------------------------------------------
// CCheckBox Declaration
/// @brief a check box control with a title and 3 states
/// @ingroup controls
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class CCheckBox : public CControl
{
public:
	CCheckBox (const CRect& size, IControlListener* listener = nullptr, int32_t tag = -1, UTF8StringPtr title = nullptr, CBitmap* bitmap = nullptr, int32_t style = 0);
	CCheckBox (const CCheckBox& checkbox);

	enum Styles {
		kAutoSizeToFit = 1 << 0, ///< automatically adjusts the width so that the label is completely visible
		kDrawCrossBox  = 1 << 1	 ///< draws a crossbox instead of a checkmark if no bitmap is provided
	};

	//-----------------------------------------------------------------------------
	/// @name CCheckBox Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setTitle (const UTF8String& newTitle);
	const UTF8String& getTitle () const { return title; }
	
	virtual void setFont (CFontRef newFont);
	const CFontRef getFont () const { return font; }
	
	virtual void setFontColor (const CColor& newColor) { fontColor = newColor; invalid (); }
	const CColor& getFontColor () const { return fontColor; }

	virtual void setBoxFrameColor (const CColor& newColor) { boxFrameColor = newColor; invalid (); }
	const CColor& getBoxFrameColor () const { return boxFrameColor; }
	virtual void setBoxFillColor (const CColor& newColor) { boxFillColor = newColor; invalid (); }
	const CColor& getBoxFillColor () const { return boxFillColor; }
	virtual void setCheckMarkColor (const CColor& newColor) { checkMarkColor = newColor; invalid (); }
	const CColor& getCheckMarkColor () const { return checkMarkColor; }

	virtual int32_t getStyle () const { return style; }
	virtual void setStyle (int32_t newStyle);
	//@}

	// overrides
	virtual void draw (CDrawContext* context) override;
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	virtual CMouseEventResult onMouseCancel () override;
	virtual int32_t onKeyDown (VstKeyCode& keyCode) override;
	virtual bool sizeToFit () override;
	virtual void setBackground (CBitmap *background) override;
	virtual bool getFocusPath (CGraphicsPath& outPath) override;

	CLASS_METHODS(CCheckBox, CControl)
protected:
	~CCheckBox () noexcept override = default;

	UTF8String title;
	int32_t style;
	CColor fontColor;
	CColor boxFrameColor;
	CColor boxFillColor;
	CColor checkMarkColor;
	SharedPointer<CFontDesc> font;

private:
	float previousValue {0.f};
	bool hilight {false};
};

//-----------------------------------------------------------------------------
// CKickButton Declaration
//!
/// @ingroup controls
//-----------------------------------------------------------------------------
class CKickButton : public CControl, public IMultiBitmapControl
{
public:
	CKickButton (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset = CPoint (0, 0));
	CKickButton (const CRect& size, IControlListener* listener, int32_t tag, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset = CPoint (0, 0));
	CKickButton (const CKickButton& kickButton);

	virtual void draw (CDrawContext*) override;

	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	virtual CMouseEventResult onMouseCancel () override;
	virtual int32_t onKeyDown (VstKeyCode& keyCode) override;
	virtual int32_t onKeyUp (VstKeyCode& keyCode) override;

	virtual bool sizeToFit () override;

	void setNumSubPixmaps (int32_t numSubPixmaps) override { IMultiBitmapControl::setNumSubPixmaps (numSubPixmaps); invalid (); }

	CLASS_METHODS(CKickButton, CControl)
protected:
	~CKickButton () noexcept override = default;
	CPoint	offset;

private:
	float   fEntryState;
};

//-----------------------------------------------------------------------------
// CTextButton Declaration
/// @brief a button which renders without bitmaps
/// @ingroup controls
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class CTextButton : public CControl
{
public:
	enum Style ///< CTextButton style
	{
		kKickStyle = 0,
		kOnOffStyle
	};

	CTextButton (const CRect& size, IControlListener* listener = nullptr, int32_t tag = -1, UTF8StringPtr title = nullptr, Style = kKickStyle);

	//-----------------------------------------------------------------------------
	/// @name CTextButton Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setTitle (const UTF8String& newTitle);
	const UTF8String& getTitle () const { return title; }

	virtual void setFont (CFontRef newFont);
	CFontRef getFont () const { return font; }
	
	virtual void setTextColor (const CColor& color);
	const CColor& getTextColor () const { return textColor; }
	virtual void setTextColorHighlighted (const CColor& color);
	const CColor& getTextColorHighlighted () const { return textColorHighlighted; }
	
	virtual void setGradient (CGradient* gradient);
	CGradient* getGradient () const;
	virtual void setGradientHighlighted (CGradient* gradient);
	CGradient* getGradientHighlighted () const;
	
	virtual void setFrameColor (const CColor& color);
	const CColor& getFrameColor () const { return frameColor; }
	virtual void setFrameColorHighlighted (const CColor& color);
	const CColor& getFrameColorHighlighted () const { return frameColorHighlighted; }

	virtual void setFrameWidth (CCoord width);
	CCoord getFrameWidth () const { return frameWidth; }

	virtual void setRoundRadius (CCoord radius);
	CCoord getRoundRadius () const { return roundRadius; }
	
	virtual void setStyle (Style style);
	Style getStyle () const { return style; }

	virtual void setIcon (CBitmap* bitmap);
	CBitmap* getIcon () const;
	
	virtual void setIconHighlighted (CBitmap* bitmap);
	CBitmap* getIconHighlighted () const;

	virtual void setIconPosition (CDrawMethods::IconPosition pos);
	CDrawMethods::IconPosition getIconPosition () const { return iconPosition; }
	
	virtual void setTextMargin (CCoord margin);
	CCoord getTextMargin () const { return textMargin; }

	virtual void setTextAlignment (CHoriTxtAlign hAlign);
	CHoriTxtAlign getTextAlignment () const { return horiTxtAlign; }
	//@}

	// overrides
	void draw (CDrawContext* context) override;
	bool getFocusPath (CGraphicsPath& outPath) override;
	bool drawFocusOnTop () override;
	void setViewSize (const CRect& rect, bool invalid = true) override;
	bool removed (CView* parent) override;
	virtual bool sizeToFit () override;
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	virtual CMouseEventResult onMouseCancel () override;
	virtual int32_t onKeyDown (VstKeyCode& keyCode) override;
	virtual int32_t onKeyUp (VstKeyCode& keyCode) override;
	
	CLASS_METHODS(CTextButton, CControl)
protected:
	~CTextButton () noexcept override = default;

	void invalidPath ();
	CGraphicsPath* getPath (CDrawContext* context);

	SharedPointer<CFontDesc> font;
	SharedPointer<CGraphicsPath> _path;
	SharedPointer<CBitmap> icon;
	SharedPointer<CBitmap> iconHighlighted;
	SharedPointer<CGradient> gradient;
	SharedPointer<CGradient> gradientHighlighted;
	
	CColor textColor;
	CColor frameColor;

	CColor textColorHighlighted;
	CColor frameColorHighlighted;

	CCoord frameWidth;
	CCoord roundRadius;
	CCoord textMargin;
	
	CHoriTxtAlign horiTxtAlign;
	CDrawMethods::IconPosition iconPosition;
	Style style;
	UTF8String title;
private:
	float fEntryState;
};

} // namespace

#endif
