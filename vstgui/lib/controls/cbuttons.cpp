// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cbuttons.h"
#include "../cdrawcontext.h"
#include "../cbitmap.h"
#include "../cframe.h"
#include "../cgraphicspath.h"
#include "../platform/iplatformfont.h"
#include <cmath>

namespace VSTGUI {

//------------------------------------------------------------------------
// COnOffButton
//------------------------------------------------------------------------
/*! @class COnOffButton
Define a button with 2 positions.
The bitmap includes the 2 subbitmaps (i.e the rectangle used for the display of this button is half-height of the bitmap).
When its value changes, the listener is called.
*/
//------------------------------------------------------------------------
/**
 * COnOffButton constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background bitmap of the on/off button
 * @param style style, currently not used
 */
//------------------------------------------------------------------------
COnOffButton::COnOffButton (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, int32_t style)
: CControl (size, listener, tag, background)
, style (style)
{
	setWantsFocus (true);
}

//------------------------------------------------------------------------
COnOffButton::COnOffButton (const COnOffButton& v)
: CControl (v)
, style (v.style)
{
	setWantsFocus (true);
}

//------------------------------------------------------------------------
void COnOffButton::draw (CDrawContext *pContext)
{
	if (getDrawBackground ())
	{
		CCoord off;

		if (value == getMax ())
			off = getDrawBackground ()->getHeight () / 2;
		else
			off = 0;

		getDrawBackground ()->draw (pContext, getViewSize (), CPoint (0, off));
	}
	setDirty (false);
}

//------------------------------------------------------------------------
CMouseEventResult COnOffButton::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	beginEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult COnOffButton::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult COnOffButton::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	if (isEditing ())
	{
		if (getViewSize ().pointInside (where))
		{
			value = (value == getMax ()) ? getMin () : getMax ();
			invalid ();
			valueChanged ();
		}
		endEdit ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult COnOffButton::onMouseCancel ()
{
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
int32_t COnOffButton::onKeyDown (VstKeyCode& keyCode)
{
	if (keyCode.virt == VKEY_RETURN && keyCode.modifier == 0)
	{
		value = (value == getMax ()) ? getMin () : getMax ();
		invalid ();
		beginEdit ();
		valueChanged ();
		endEdit ();
		return 1;
	}
	return -1;
}

//------------------------------------------------------------------------
bool COnOffButton::sizeToFit ()
{
	if (getDrawBackground ())
	{
		CRect vs (getViewSize ());
		vs.setWidth (getDrawBackground ()->getWidth ());
		vs.setHeight (getDrawBackground ()->getHeight () / 2.);
		setViewSize (vs, true);
		setMouseableArea (vs);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
// CKickButton
//------------------------------------------------------------------------
/*! @class CKickButton
Define a button with 2 states using 2 subbitmaps.
One click on it, then the second subbitmap is displayed.
When the mouse button is relaxed, the first subbitmap is framed.
*/
//------------------------------------------------------------------------
/**
 * CKickButton constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background the bitmap
 * @param offset unused
 */
//------------------------------------------------------------------------
CKickButton::CKickButton (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset)
: CControl (size, listener, tag, background)
, offset (offset)
{
	heightOfOneImage = size.getHeight ();
	setWantsFocus (true);
}

//------------------------------------------------------------------------
/**
 * CKickButton constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param heightOfOneImage height of one sub bitmap in background
 * @param background the bitmap
 * @param offset of background
 */
//------------------------------------------------------------------------
CKickButton::CKickButton (const CRect& size, IControlListener* listener, int32_t tag, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset)
: CControl (size, listener, tag, background)
, offset (offset)
{
	setHeightOfOneImage (heightOfOneImage);
	setWantsFocus (true);
}

//------------------------------------------------------------------------
CKickButton::CKickButton (const CKickButton& v)
: CControl (v)
, offset (v.offset)
{
	setHeightOfOneImage (v.heightOfOneImage);
	setWantsFocus (true);
}

//------------------------------------------------------------------------
void CKickButton::draw (CDrawContext *pContext)
{
	CPoint where (offset.x, offset.y);

	bounceValue ();

	if (value == getMax ())
		where.y += heightOfOneImage;

	if (getDrawBackground ())
	{
		getDrawBackground ()->draw (pContext, getViewSize (), where);
	}
	setDirty (false);
}

//------------------------------------------------------------------------
CMouseEventResult CKickButton::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;
	fEntryState = value;
	beginEdit ();
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CKickButton::onMouseCancel ()
{
	value = getMin ();
	if (isDirty ())
		invalid ();
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CKickButton::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	if (value > 0.f)
		valueChanged ();
	value = getMin ();
	valueChanged ();
	if (isDirty ())
		invalid ();
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CKickButton::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (isEditing ())
	{
		if (where.x >= getViewSize ().left && where.y >= getViewSize ().top  &&
			where.x <= getViewSize ().right && where.y <= getViewSize ().bottom)
			value = getMax ();
		else
			value = getMin ();
		
		if (isDirty ())
			invalid ();
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
int32_t CKickButton::onKeyDown (VstKeyCode& keyCode)
{
	if (keyCode.modifier == 0 && keyCode.virt == VKEY_RETURN)
	{
		if (value != getMax ())
		{
			beginEdit ();
			value = getMax ();
			invalid ();
			valueChanged ();
		}
		return 1;
	}
	return -1;
}

//------------------------------------------------------------------------
int32_t CKickButton::onKeyUp (VstKeyCode& keyCode)
{
	if (keyCode.modifier == 0 && keyCode.virt == VKEY_RETURN)
	{
		value = getMin ();
		invalid ();
		valueChanged ();
		endEdit ();
		return 1;
	}
	return -1;
}

//------------------------------------------------------------------------
bool CKickButton::sizeToFit ()
{
	if (getDrawBackground ())
	{
		CRect vs (getViewSize ());
		vs.setHeight (heightOfOneImage);
		vs.setWidth (getDrawBackground ()->getWidth ());
		setViewSize (vs, true);
		setMouseableArea (vs);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
// CCheckBox
//------------------------------------------------------------------------
/*! @class CCheckBox
A checkbox control with a title and 3 states : checked, half checked, not checked

- if value is < 0.5 the checkbox is not checked
- if value is 0.5 the checkbox is half checked
- if value is > 0.5 the checkbox is checked

the user can only switch between checked and not checked state.

If the bitmap is set, the bitmap must contain 6 states of the checkbox in the following order:
- not checked
- half checked
- checked
- not checked hilighted
- half checked hilighted
- checked hilighted
*/
//------------------------------------------------------------------------
//------------------------------------------------------------------------
CCheckBox::CCheckBox (const CRect& size, IControlListener* listener, int32_t tag, UTF8StringPtr title, CBitmap* bitmap, int32_t style)
: CControl (size, listener, tag, bitmap)
, style (style)
, fontColor (kWhiteCColor)
, font (kSystemFont)
{
	setTitle (title);
	setBoxFillColor (kWhiteCColor);
	setBoxFrameColor (kBlackCColor);
	setCheckMarkColor (kRedCColor);
	setWantsFocus (true);
	if (style & kAutoSizeToFit)
		sizeToFit ();
}

//------------------------------------------------------------------------
CCheckBox::CCheckBox (const CCheckBox& checkbox)
: CControl (checkbox)
, style (checkbox.style)
, fontColor (checkbox.fontColor)
, font (checkbox.font)
{
	setTitle (checkbox.title);
	setBoxFillColor (checkbox.boxFillColor);
	setBoxFrameColor (checkbox.boxFrameColor);
	setCheckMarkColor (checkbox.checkMarkColor);
	setWantsFocus (true);
}

//------------------------------------------------------------------------
void CCheckBox::setTitle (const UTF8String& newTitle)
{
	title = newTitle;
	if (style & kAutoSizeToFit)
		sizeToFit ();
}

//------------------------------------------------------------------------
void CCheckBox::setFont (CFontRef newFont)
{
	font = newFont;
	if (font && style & kAutoSizeToFit)
		sizeToFit ();
}

//------------------------------------------------------------------------
void CCheckBox::setBackground (CBitmap *background)
{
	CView::setBackground (background);
	if (style & kAutoSizeToFit)
		sizeToFit ();
}

//------------------------------------------------------------------------
void CCheckBox::setStyle (int32_t newStyle)
{
	if (style != newStyle)
	{
		style = newStyle;
		if (style & kAutoSizeToFit)
			sizeToFit ();
		invalid ();
	}
}

/// @cond ignore
//------------------------------------------------------------------------
static CCoord getFontCapHeight (CFontRef font)
{
	CCoord c = font->getSize ();
	IPlatformFont* pf = font->getPlatformFont ();
	if (pf)
	{
		CCoord capHeight = pf->getCapHeight ();
		if (capHeight <= 0)
			capHeight = pf->getAscent ();
		if (capHeight > 0)
			c = capHeight;
	}
	return c;
}
/// @endcond

//------------------------------------------------------------------------
static CCoord kCheckBoxTitleMargin = 5;

//------------------------------------------------------------------------
bool CCheckBox::sizeToFit ()
{
	if (title.empty ())
		return false;
	if (auto painter = font->getFontPainter ())
	{
		CRect fitSize (getViewSize ());
		if (getDrawBackground ())
		{
			fitSize.setWidth (getDrawBackground ()->getWidth ());
			fitSize.setHeight (getDrawBackground ()->getHeight () / 6);
		}
		else
		{
			fitSize.setWidth (fitSize.getHeight ());
		}
		fitSize.right += kCheckBoxTitleMargin;
		fitSize.right += painter->getStringWidth (nullptr, UTF8String (title).getPlatformString (), true);
		setViewSize (fitSize);
		setMouseableArea (fitSize);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
void CCheckBox::draw (CDrawContext* context)
{
	float norm = getValueNormalized ();
	CRect checkBoxSize (getViewSize ());
	if (getDrawBackground ())
	{
		CPoint off;

		checkBoxSize.setWidth (getDrawBackground ()->getWidth ());
		checkBoxSize.setHeight (getDrawBackground ()->getHeight () / 6);

		if (norm == 0.5)
			off.y = checkBoxSize.getHeight ();
		else if (norm > 0.5)
			off.y = checkBoxSize.getHeight () * 2;
		else
			off.y = 0;
		if (hilight)
			off.y += getDrawBackground ()->getHeight () / 2.;

		getDrawBackground ()->draw (context, checkBoxSize, off);
	}
	else
	{
		checkBoxSize.setHeight (std::floor (getFontCapHeight (font) + 2.5));
		checkBoxSize.setWidth (checkBoxSize.getHeight ());
		checkBoxSize.offset (1., std::ceil ((getViewSize ().getHeight () - checkBoxSize.getHeight ()) / 2.));
		context->setLineWidth (1);
		context->setLineStyle (kLineSolid);
		context->setDrawMode (kAntiAliasing);
		context->setFrameColor (boxFrameColor);
		context->setFillColor (boxFillColor);
		context->drawRect (checkBoxSize, kDrawFilledAndStroked);

		if (hilight)
		{
			CColor hilightColor = boxFrameColor;
			hilightColor.alpha /= 2;
			context->setFrameColor (hilightColor);
			CRect r (checkBoxSize);
			r.inset (1., 1.);
			context->drawRect (r, kDrawStroked);
		}

		context->setDrawMode (kAntiAliasing);
		context->setFrameColor (checkMarkColor);
		context->setLineWidth (2.);

		const CCoord cbInset = 2;
		
		if (style & kDrawCrossBox)
		{
			if (norm == 0.5f)
			{
				context->drawLine (CPoint (checkBoxSize.left + cbInset, checkBoxSize.top + checkBoxSize.getHeight () / 2.), CPoint (checkBoxSize.right - cbInset, checkBoxSize.top + checkBoxSize.getHeight () / 2));
			}
			else if (norm > 0.5f)
			{
				context->drawLine (CPoint (checkBoxSize.left + cbInset, checkBoxSize.top + cbInset), CPoint (checkBoxSize.right - cbInset, checkBoxSize.bottom - cbInset));
				context->drawLine (CPoint (checkBoxSize.left + cbInset, checkBoxSize.bottom - cbInset), CPoint (checkBoxSize.right - cbInset, checkBoxSize.top + cbInset));
			}
		}
		else
		{
			if (norm == 0.5f)
			{
				context->drawLine (CPoint (checkBoxSize.left + cbInset, checkBoxSize.top + checkBoxSize.getHeight () / 2.), CPoint (checkBoxSize.right - cbInset, checkBoxSize.top + checkBoxSize.getHeight () / 2));
			}
			else if (norm > 0.5f)
			{
				SharedPointer<CGraphicsPath> path = owned (context->createGraphicsPath ());
				if (path)
				{
					path->beginSubpath (CPoint (checkBoxSize.left + cbInset, checkBoxSize.top + checkBoxSize.getHeight () / 2.));
					path->addLine (CPoint (checkBoxSize.left + checkBoxSize.getWidth () / 2, checkBoxSize.bottom - cbInset));
					path->addLine (CPoint (checkBoxSize.right + 1, checkBoxSize.top - 1));
					context->drawGraphicsPath (path, CDrawContext::kPathStroked);
				}
				else
				{
					context->drawLine (CPoint (checkBoxSize.left + cbInset, checkBoxSize.top + checkBoxSize.getHeight () / 2.), CPoint (checkBoxSize.left + checkBoxSize.getWidth () / 2, checkBoxSize.bottom - cbInset));
					context->drawLine (CPoint (checkBoxSize.left + checkBoxSize.getWidth () / 2., checkBoxSize.bottom - cbInset), CPoint (checkBoxSize.right + 1, checkBoxSize.top - 1));
				}
			}
		}
	}
	
	if (title.empty() == false)
	{
		CPoint p (checkBoxSize.getBottomRight ());
		p.offset (kCheckBoxTitleMargin, -1.);
		
		context->setFont (font);
		context->setFontColor (fontColor);
		context->setDrawMode (kAntiAliasing);
		
		context->drawString (title.getPlatformString (), p, true);
	}
	
	setDirty (false);
}

//------------------------------------------------------------------------
bool CCheckBox::getFocusPath (CGraphicsPath& outPath)
{
	if (wantsFocus ())
	{
		CCoord focusWidth = getFrame ()->getFocusWidth ();
		CRect checkBoxSize (getViewSize ());
		if (getDrawBackground ())
		{
			checkBoxSize.setWidth (getDrawBackground ()->getWidth ());
			checkBoxSize.setHeight (getDrawBackground ()->getHeight () / 6);
		}
		else
		{
			checkBoxSize.setHeight (std::floor (getFontCapHeight (font) + 2.5));
			checkBoxSize.setWidth (checkBoxSize.getHeight ());
			checkBoxSize.offset (1, std::ceil ((getViewSize ().getHeight () - checkBoxSize.getHeight ()) / 2));
		}
		outPath.addRect (checkBoxSize);
		checkBoxSize.extend (focusWidth, focusWidth);
		outPath.addRect (checkBoxSize);
	}
	return true;
}

//------------------------------------------------------------------------
CMouseEventResult CCheckBox::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton ())
	{
		beginEdit ();
		previousValue = value;
		return onMouseMoved (where, buttons);
	}
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//------------------------------------------------------------------------
CMouseEventResult CCheckBox::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (isEditing ())
	{
		bool wasHilighted = hilight;
		if (getViewSize ().pointInside (where))
			hilight = true;
		else
			hilight = false;
		if (wasHilighted != hilight)
			invalid ();
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CCheckBox::onMouseCancel ()
{
	hilight = false;
	value = previousValue;
	if (isDirty ())
		invalid ();
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CCheckBox::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	hilight = false;
	if (getViewSize ().pointInside (where))
		value = (previousValue < getMax ()) ? getMax () : getMin ();
	else
		value = previousValue;
	if (isDirty ())
	{
		valueChanged ();
		invalid ();
	}
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
int32_t CCheckBox::onKeyDown (VstKeyCode& keyCode)
{
	if (keyCode.virt == VKEY_RETURN && keyCode.modifier == 0)
	{
		value = (value < getMax ()) ? getMax () : getMin ();
		invalid ();
		beginEdit ();
		valueChanged ();
		endEdit ();
		return 1;
	}
	return -1;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
CTextButton::CTextButton (const CRect& size, IControlListener* listener, int32_t tag, UTF8StringPtr title, Style style)
: CControl (size, listener, tag, nullptr)
, font (kSystemFont)
, frameWidth (1.)
, roundRadius (6.)
, textMargin (0.)
, horiTxtAlign (kCenterText)
, iconPosition (CDrawMethods::kIconLeft)
, style (style)
, title (title)
{
	setTextColor (kBlackCColor);
	setTextColorHighlighted (kWhiteCColor);
	
	gradient = owned (CGradient::create (0, 1, CColor (220, 220, 220, 255), CColor (180, 180, 180, 255)));
	gradientHighlighted = owned (CGradient::create (0, 1, CColor (180, 180, 180, 255), CColor (100, 100, 100, 255)));
	
	setFrameColor (kBlackCColor);
	setFrameColorHighlighted (kBlackCColor);
	setWantsFocus (true);
}

//------------------------------------------------------------------------
bool CTextButton::removed (CView* parent)
{
	invalidPath ();
	return CControl::removed (parent);
}

//------------------------------------------------------------------------
void CTextButton::setViewSize (const CRect& rect, bool invalid)
{
	invalidPath ();
	CControl::setViewSize (rect, invalid);
}

//------------------------------------------------------------------------
CGradient* CTextButton::getGradient () const
{
	return gradient;
}

//------------------------------------------------------------------------
CGradient* CTextButton::getGradientHighlighted () const
{
	return gradientHighlighted;
}

//------------------------------------------------------------------------
CBitmap* CTextButton::getIcon () const
{
	return icon;
}

//------------------------------------------------------------------------
CBitmap* CTextButton::getIconHighlighted () const
{
	return iconHighlighted;
}

//------------------------------------------------------------------------
void CTextButton::setTitle (const UTF8String& newTitle)
{
	title = newTitle;
	invalid ();
}

//------------------------------------------------------------------------
void CTextButton::setFont (CFontRef newFont)
{
	font = newFont;
	invalid ();
}

//------------------------------------------------------------------------
void CTextButton::setTextColor (const CColor& color)
{
	textColor = color;
	invalid ();
}

//------------------------------------------------------------------------
void CTextButton::setGradient (CGradient* newGradient)
{
	gradient = newGradient;
	invalid ();
}

//------------------------------------------------------------------------
void CTextButton::setGradientHighlighted (CGradient* newGradient)
{
	gradientHighlighted = newGradient;
	invalid ();
}

//------------------------------------------------------------------------
void CTextButton::setFrameColor (const CColor& color)
{
	frameColor = color;
	invalid ();
}

//------------------------------------------------------------------------
void CTextButton::setTextColorHighlighted (const CColor& color)
{
	textColorHighlighted = color;
	invalid ();
}

//------------------------------------------------------------------------
void CTextButton::setFrameColorHighlighted (const CColor& color)
{
	frameColorHighlighted = color;
	invalid ();
}

//------------------------------------------------------------------------
void CTextButton::setFrameWidth (CCoord width)
{
	frameWidth = width;
	invalid ();
}

//------------------------------------------------------------------------
void CTextButton::setRoundRadius (CCoord radius)
{
	roundRadius = radius;
	invalidPath ();
	invalid ();
}

//------------------------------------------------------------------------
void CTextButton::setStyle (Style _style)
{
	style = _style;
}

//------------------------------------------------------------------------
void CTextButton::setIcon (CBitmap* bitmap)
{
	if (icon != bitmap)
	{
		icon = bitmap;
		invalid ();
	}
}

//------------------------------------------------------------------------
void CTextButton::setIconHighlighted (CBitmap* bitmap)
{
	if (iconHighlighted != bitmap)
	{
		iconHighlighted = bitmap;
		invalid ();
	}
}

//------------------------------------------------------------------------
void CTextButton::setIconPosition (CDrawMethods::IconPosition pos)
{
	if (iconPosition != pos)
	{
		iconPosition = pos;
		invalid ();
	}
}

//------------------------------------------------------------------------
void CTextButton::setTextMargin (CCoord margin)
{
	if (textMargin != margin)
	{
		textMargin = margin;
		invalid ();
	}
}

//------------------------------------------------------------------------
void CTextButton::setTextAlignment (CHoriTxtAlign hAlign)
{
	// to force the redraw
	if (horiTxtAlign != hAlign)
	{
		horiTxtAlign = hAlign;
		invalid ();
	}
}


//------------------------------------------------------------------------
bool CTextButton::sizeToFit ()
{
	if (title.empty ())
		return false;
	if (auto painter = font->getFontPainter ())
	{
		CRect fitSize (getViewSize ());
		fitSize.right = fitSize.left + (roundRadius + 1.) * 4.;
		fitSize.right += painter->getStringWidth (nullptr, title.getPlatformString (), true);
		setViewSize (fitSize);
		setMouseableArea (fitSize);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
void CTextButton::draw (CDrawContext* context)
{
	bool highlight = value > 0.5 ? true : false;
	context->setDrawMode (kAntiAliasing);
	context->setLineWidth (frameWidth);
	context->setLineStyle (CLineStyle (CLineStyle::kLineCapRound, CLineStyle::kLineJoinRound));
	context->setFrameColor (highlight ? frameColorHighlighted : frameColor);
	CRect r (getViewSize ());
	r.inset (frameWidth / 2., frameWidth / 2.);
	if (gradient && gradientHighlighted)
	{
		CGraphicsPath* path = getPath (context);
		if (path)
		{
			CGradient* drawGradient = highlight ? gradientHighlighted : gradient;
			if (drawGradient)
				context->fillLinearGradient (path, *drawGradient, r.getTopLeft (), r.getBottomLeft (), false);
			context->drawGraphicsPath (path, CDrawContext::kPathStroked);
		}
	}
	CRect titleRect = getViewSize ();
	titleRect.inset (frameWidth / 2., frameWidth / 2.);

	CBitmap* iconToDraw = highlight ? (iconHighlighted ? iconHighlighted : icon) : (icon ? icon : iconHighlighted);
	CDrawMethods::drawIconAndText (context, iconToDraw, iconPosition, getTextAlignment (), getTextMargin (), titleRect, title, getFont (), highlight ? getTextColorHighlighted () : getTextColor ());
	setDirty (false);
}

//------------------------------------------------------------------------
bool CTextButton::getFocusPath (CGraphicsPath& outPath)
{
	CRect r (getViewSize ());
	CCoord focusWidth = getFrame ()->getFocusWidth ();
	r.inset (-focusWidth, -focusWidth);
	outPath.addRoundRect (r, roundRadius);
	outPath.closeSubpath ();
	r = getViewSize ();
	outPath.addRoundRect (r, roundRadius);
	return true;
}

//------------------------------------------------------------------------
bool CTextButton::drawFocusOnTop ()
{
	return false;
}

//------------------------------------------------------------------------
CGraphicsPath* CTextButton::getPath (CDrawContext* context)
{
	if (_path == nullptr)
	{
		CRect r (getViewSize ());
		r.inset (frameWidth / 2., frameWidth / 2.);
		_path = owned (context->createRoundRectGraphicsPath (r, roundRadius));
	}
	return _path;
}

//------------------------------------------------------------------------
void CTextButton::invalidPath ()
{
	_path = nullptr;
}

//------------------------------------------------------------------------
CMouseEventResult CTextButton::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;
	fEntryState = value;
	beginEdit ();
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CTextButton::onMouseCancel ()
{
	value = fEntryState;
	if (isDirty ())
		invalid ();
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CTextButton::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	if (isEditing ())
	{
		if (value != fEntryState)
		{
			valueChanged ();
			if (style == kKickStyle)
			{
				value = getMin ();  // set button to UNSELECTED state
				valueChanged ();
			}
			if (isDirty ())
				invalid ();
		}
		endEdit ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CTextButton::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (isEditing ())
	{
		if (where.x >= getViewSize ().left && where.y >= getViewSize ().top  &&
			where.x <= getViewSize ().right && where.y <= getViewSize ().bottom)
			value = fEntryState == getMin () ? getMax () : getMin ();
		else
			value = fEntryState == getMin () ? getMin () : getMax ();
		
		if (isDirty ())
			invalid ();
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
int32_t CTextButton::onKeyDown (VstKeyCode& keyCode)
{
	if (keyCode.modifier == 0 && keyCode.virt == VKEY_RETURN)
	{
		if (style == kKickStyle)
		{
			if (value != getMax ())
			{
				beginEdit ();
				value = getMax ();
				invalid ();
				valueChanged ();
				value = getMin ();
				invalid ();
				valueChanged ();
				endEdit ();
			}
		}
		else
		{
			beginEdit ();
			if (value == getMin ())
				value = getMax ();
			else
				value = getMin ();
			invalid ();
			valueChanged ();
			endEdit ();
		}
		return 1;
	}
	return -1;
}

//------------------------------------------------------------------------
int32_t CTextButton::onKeyUp (VstKeyCode& keyCode)
{
	return -1;
}

} // namespace
