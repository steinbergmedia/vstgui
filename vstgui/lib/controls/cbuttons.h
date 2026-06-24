// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

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
	COnOffButton (const CRect& size, IControlListener* listener = nullptr, int32_t tag = -1,
				  const SharedPointer<CBitmap>& background = {}, int32_t style = 0);

	//-----------------------------------------------------------------------------
	/// @name COnOffButton Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual int32_t getStyle () const { return style; }
	virtual void setStyle (int32_t newStyle) { style = newStyle; }
	//@}

	// overrides
	void draw (CDrawContext&) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;
	void onKeyboardEvent (KeyboardEvent& event) override;
	bool sizeToFit () override;

protected:
	VSTGUI_SHAREDPTR_FRIEND (COnOffButton)
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
	CCheckBox (const CRect& size, IControlListener* listener = nullptr, int32_t tag = -1,
			   UTF8StringPtr title = nullptr, const SharedPointer<CBitmap>& background = {},
			   int32_t style = 0);

	enum Styles
	{
		/** automatically adjusts the width so that the label is completely visible */
		kAutoSizeToFit = 1 << 0,
		/** draws a crossbox instead of a checkmark if no bitmap is provided */
		kDrawCrossBox = 1 << 1,
		/** do not limit the box drawing to the cap height */
		kIgnoreCapHeightOnDraw = 1 << 2,
	};

	//-----------------------------------------------------------------------------
	/// @name CCheckBox Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setTitle (const UTF8String& newTitle);
	const UTF8String& getTitle () const { return title; }

	virtual void setFont (const SharedPointer<CFontDesc>& newFont);
	SharedPointer<CFontDesc> getFont () const { return font; }

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

    CCoord getFrameWidth () const { return frameWidth; }
    virtual void setFrameWidth (CCoord width);
    CCoord getRoundRectRadius () const { return roundRectRadius; }
    virtual void setRoundRectRadius (CCoord radius);

    //@}

	// overrides
	void draw (CDrawContext& context) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;
	void onKeyboardEvent (KeyboardEvent& event) override;
	bool sizeToFit () override;
	void setBackground (const SharedPointer<CBitmap>& background) override;
	bool getFocusPath (CGraphicsPath& outPath, CCoord focusLineWidth) override;

protected:
	VSTGUI_SHAREDPTR_FRIEND (CCheckBox)

	~CCheckBox () noexcept override = default;

	UTF8String title;
	int32_t style;
	CColor fontColor;
	CColor boxFrameColor;
	CColor boxFillColor;
	CColor checkMarkColor;
    CCoord frameWidth {1};
    CCoord roundRectRadius {0};
	SharedPointer<CFontDesc> font;

private:
	float previousValue {0.f};
	bool highlight {false};
};

//-----------------------------------------------------------------------------
// CKickButton Declaration
//!
/// @ingroup controls uses_multi_frame_bitmaps
//-----------------------------------------------------------------------------
class CKickButton : public CControl,
					public MultiFrameBitmapView<CKickButton>
{
public:
	CKickButton (const CRect& size, IControlListener* listener, int32_t tag,
				 const SharedPointer<CBitmap>& background);

	void draw (CDrawContext&) override;

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;
	void onKeyboardEvent (KeyboardEvent& event) override;

	bool sizeToFit () override;

protected:
	VSTGUI_SHAREDPTR_FRIEND (CKickButton)
	~CKickButton () noexcept override = default;
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
	/** CTextButton style */
	enum Style
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

	virtual void setFont (const SharedPointer<CFontDesc>& newFont);
	SharedPointer<CFontDesc> getFont () const { return font; }

	virtual void setTextColor (const CColor& color);
	const CColor& getTextColor () const { return textColor; }
	virtual void setTextColorHighlighted (const CColor& color);
	const CColor& getTextColorHighlighted () const { return textColorHighlighted; }

	virtual void setGradient (const SharedPointer<CGradient>& gradient);
	SharedPointer<CGradient> getGradient () const;
	virtual void setGradientHighlighted (const SharedPointer<CGradient>& gradient);
	SharedPointer<CGradient> getGradientHighlighted () const;

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

	virtual void setIcon (const SharedPointer<CBitmap>& bitmap);
	SharedPointer<CBitmap> getIcon () const;

	virtual void setIconHighlighted (const SharedPointer<CBitmap>& bitmap);
	SharedPointer<CBitmap> getIconHighlighted () const;

	virtual void setIconPosition (CDrawMethods::IconPosition pos);
	CDrawMethods::IconPosition getIconPosition () const { return iconPosition; }
	
	virtual void setTextMargin (CCoord margin);
	CCoord getTextMargin () const { return textMargin; }

	virtual void setTextAlignment (CHoriTxtAlign hAlign);
	CHoriTxtAlign getTextAlignment () const { return horiTxtAlign; }
	//@}

	// overrides
	void draw (CDrawContext& context) override;
	bool getFocusPath (CGraphicsPath& outPath, CCoord focusLineWidth) override;
	bool drawFocusOnTop () override;
	void setViewSize (const CRect& rect, bool invalid = true) override;
	bool removed (CViewContainer& parent) override;
	bool sizeToFit () override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;
	void onKeyboardEvent (KeyboardEvent& event) override;

protected:
	VSTGUI_SHAREDPTR_FRIEND (CTextButton)
	~CTextButton () noexcept override = default;

	void invalidPath ();
	SharedPointer<CGraphicsPath> getPath (CDrawContext& context, CCoord lineWidth);

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

} // VSTGUI
