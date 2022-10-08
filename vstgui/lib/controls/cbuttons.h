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
	void draw (CDrawContext*) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;
	void onKeyboardEvent (KeyboardEvent& event) override;
	bool sizeToFit () override;

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
		/** automatically adjusts the width so that the label is completely visible */
		kAutoSizeToFit = 1 << 0,
		/** draws a crossbox instead of a checkmark if no bitmap is provided */
		kDrawCrossBox  = 1 << 1
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

    CCoord getFrameWidth () const { return frameWidth; }
    virtual void setFrameWidth (CCoord width);
    CCoord getRoundRectRadius () const { return roundRectRadius; }
    virtual void setRoundRectRadius (CCoord radius);

    //@}

	// overrides
	void draw (CDrawContext* context) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;
	void onKeyboardEvent (KeyboardEvent& event) override;
	bool sizeToFit () override;
	void setBackground (CBitmap *background) override;
	bool getFocusPath (CGraphicsPath& outPath) override;

	CLASS_METHODS(CCheckBox, CControl)
protected:
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
	bool hilight {false};
};

//-----------------------------------------------------------------------------
// CKickButton Declaration
//!
/// @ingroup controls uses_multi_frame_bitmaps
//-----------------------------------------------------------------------------
class CKickButton : public CControl
#if VSTGUI_ENABLE_DEPRECATED_METHODS
,
					public IMultiBitmapControl
#endif
{
public:
	CKickButton (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background,
				 const CPoint& offset = CPoint (0, 0));
	CKickButton (const CKickButton& kickButton);

	void draw (CDrawContext*) override;

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;
	void onKeyboardEvent (KeyboardEvent& event) override;

	bool sizeToFit () override;

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	void setNumSubPixmaps (int32_t numSubPixmaps) override { IMultiBitmapControl::setNumSubPixmaps (numSubPixmaps); invalid (); }
	CKickButton (const CRect& size, IControlListener* listener, int32_t tag,
				 CCoord heightOfOneImage, CBitmap* background,
				 const CPoint& offset = CPoint (0, 0));
#endif

	CLASS_METHODS(CKickButton, CControl)
protected:
	~CKickButton () noexcept override = default;
	CPoint	offset;
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
	bool sizeToFit () override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;
	void onKeyboardEvent (KeyboardEvent& event) override;
	
	CLASS_METHODS(CTextButton, CControl)
protected:
	~CTextButton () noexcept override = default;

	void invalidPath ();
	CGraphicsPath* getPath (CDrawContext* context, CCoord lineWidth);

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
