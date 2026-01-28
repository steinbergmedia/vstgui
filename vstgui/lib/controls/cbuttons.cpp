// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cbuttons.h"
#include "../cdrawcontext.h"
#include "../cbitmap.h"
#include "../cframe.h"
#include "../cgraphicspath.h"
#include "../events.h"
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
COnOffButton::COnOffButton (const CRect& size, IControlListener* listener, int32_t tag,
							const SharedPointer<CBitmap>& background, int32_t style)
: CControl (size, listener, tag, background), style (style)
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

		if (getValue () == getMax ())
			off = getDrawBackground ()->getHeight () / 2;
		else
			off = 0;

		getDrawBackground ()->draw (pContext, getViewSize (), CPoint (0, off));
	}
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
			if (setValue ((getValue () == getMax ()) ? getMin () : getMax ()))
				valueChanged ();
		}
		endEdit ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult COnOffButton::onMouseCancel ()
{
	if (isEditing ())
		endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
void COnOffButton::onKeyboardEvent (KeyboardEvent& event)
{
	if (event.type != EventType::KeyDown)
		return;
	if (event.virt == VirtualKey::Return && event.modifiers.empty ())
	{
		setValue ((getValue () == getMax ()) ? getMin () : getMax ());
		invalid ();
		beginEdit ();
		valueChanged ();
		endEdit ();
		event.consumed = true;
	}
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
Use a CMultiFrameBitmap for its background bitmap.
*/
//------------------------------------------------------------------------
/**
 * CKickButton constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background the bitmap
 */
//------------------------------------------------------------------------
CKickButton::CKickButton (const CRect& size, IControlListener* listener, int32_t tag,
						  const SharedPointer<CBitmap>& background)
: CControl (size, listener, tag, background)
{
	setWantsFocus (true);
}

//------------------------------------------------------------------------
CKickButton::CKickButton (const CKickButton& v) : CControl (v) { setWantsFocus (true); }

//------------------------------------------------------------------------
void CKickButton::draw (CDrawContext *pContext)
{
	bounceValue ();

	if (auto bitmap = getDrawBackground ())
	{
		if (auto mfb = bitmap.cast<CMultiFrameBitmap> ())
		{
			auto index = getMultiFrameBitmapIndex (*mfb.get (), getValueNormalized ());
			mfb->drawFrame (pContext, index, getViewSize ().getTopLeft ());
		}
		else
		{
			bitmap->draw (pContext, getViewSize ());
		}
	}
}

//------------------------------------------------------------------------
CMouseEventResult CKickButton::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;
	beginEdit ();
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CKickButton::onMouseCancel ()
{
	if (isEditing ())
	{
		if (setValue (getMin ()))
		{
			valueChanged ();
			invalid ();
		}
		endEdit ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CKickButton::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	if (isEditing ())
	{
		if (getValue () > 0.f)
			valueChanged ();
		setValue (getMin ());
		valueChanged ();
		endEdit ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CKickButton::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (isEditing ())
	{
		if (where.x >= getViewSize ().left && where.y >= getViewSize ().top  &&
			where.x <= getViewSize ().right && where.y <= getViewSize ().bottom)
			setValue (getMax ());
		else
			setValue (getMin ());

		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
void CKickButton::onKeyboardEvent (KeyboardEvent& event)
{
	if (event.modifiers.empty () && event.virt == VirtualKey::Return)
	{
		if (event.type == EventType::KeyDown)
		{
			if (getValue () != getMax ())
			{
				beginEdit ();
				setValue (getMax ());
				valueChanged ();
			}
			event.consumed = true;
		}
		else if (event.type == EventType::KeyUp && isEditing ())
		{
			setValue (getMin ());
			valueChanged ();
			endEdit ();
			event.consumed = true;
		}
	}
}

//------------------------------------------------------------------------
bool CKickButton::sizeToFit ()
{
	if (auto bitmap = getDrawBackground ())
	{
		CRect vs (getViewSize ());
		if (auto mfb = bitmap.cast<CMultiFrameBitmap> ())
		{
			vs.setSize (mfb->getFrameSize ());
		}
		else
		{
			vs.setHeight (bitmap->getHeight ());
			vs.setWidth (bitmap->getWidth ());
		}
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
- not checked highlighted
- half checked highlighted
- checked highlighted
*/
//------------------------------------------------------------------------
//------------------------------------------------------------------------
CCheckBox::CCheckBox (const CRect& size, IControlListener* listener, int32_t tag,
					  UTF8StringPtr title, const SharedPointer<CBitmap>& bitmap, int32_t style)
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
void CCheckBox::setFont (const SharedPointer<CFontDesc>& newFont)
{
	font = newFont;
	if (font && style & kAutoSizeToFit)
		sizeToFit ();
}

//------------------------------------------------------------------------
void CCheckBox::setBackground (const SharedPointer<CBitmap>& background)
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

//------------------------------------------------------------------------
void CCheckBox::setFrameWidth (CCoord width)
{
	if (frameWidth != width)
	{
		frameWidth = width;
		invalid ();
	}
}

//------------------------------------------------------------------------
void CCheckBox::setRoundRectRadius (CCoord radius)
{
	if (roundRectRadius != radius)
	{
		roundRectRadius = radius;
		invalid ();
	}
}

/// @cond ignore
//------------------------------------------------------------------------
static CCoord getFontCapHeight (const SharedPointer<CFontDesc>& font)
{
	CCoord c = font->getSize ();
	auto pf = font->getPlatformFont ();
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
		if (highlight)
			off.y += getDrawBackground ()->getHeight () / 2.;

		getDrawBackground ()->draw (context, checkBoxSize, off);
	}
	else
	{
		auto lineWidth = frameWidth;
		if (lineWidth < 0)
			lineWidth = context->getHairlineSize ();
		if (!(style & kIgnoreCapHeightOnDraw))
			checkBoxSize.setHeight (std::floor (getFontCapHeight (font) + 2.5));
		else
			checkBoxSize.bottom -= 2.;
		checkBoxSize.setWidth (checkBoxSize.getHeight ());
		checkBoxSize.offset (1., std::ceil ((getViewSize ().getHeight () - checkBoxSize.getHeight ()) / 2.));
		context->setLineWidth (lineWidth);
		context->setLineStyle (kLineSolid);
		context->setDrawMode (kAntiAliasing);
		context->setFrameColor (boxFrameColor);
		context->setFillColor (boxFillColor);
		if (auto path = context->createRoundRectGraphicsPath (checkBoxSize, roundRectRadius))
		{
			context->drawGraphicsPath (path, CDrawContext::kPathFilled);
			context->drawGraphicsPath (path, CDrawContext::kPathStroked);
		}
		else
		{
			context->drawRect (checkBoxSize, kDrawFilledAndStroked);
		}

		if (highlight)
		{
			CColor highlightColor = boxFrameColor;
			highlightColor.alpha /= 2;
			context->setFrameColor (highlightColor);
			CRect r (checkBoxSize);
			r.inset (lineWidth, lineWidth);
			if (auto path = context->createRoundRectGraphicsPath (r, roundRectRadius))
			{
				context->drawGraphicsPath (path, CDrawContext::kPathStroked);
			}
			else
			{
				context->drawRect (r, kDrawStroked);
			}
		}

		context->setDrawMode (kAntiAliasing);
		context->setFrameColor (checkMarkColor);
		context->setLineWidth (2.);

		const CCoord cbInset = 2.;
		
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
				auto path = context->createGraphicsPath ();
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
}

//------------------------------------------------------------------------
bool CCheckBox::getFocusPath (CGraphicsPath& outPath, CCoord focusLineWidth)
{
	if (wantsFocus ())
	{
		CRect checkBoxSize (getViewSize ());
		if (getDrawBackground ())
		{
			checkBoxSize.setWidth (getDrawBackground ()->getWidth ());
			checkBoxSize.setHeight (getDrawBackground ()->getHeight () / 6);
		}
		else
		{
			if (!(style & kIgnoreCapHeightOnDraw))
				checkBoxSize.setHeight (std::floor (getFontCapHeight (font) + 2.5));
			else
				checkBoxSize.bottom -= 2.;
			checkBoxSize.setWidth (checkBoxSize.getHeight ());
			checkBoxSize.offset (1, std::ceil ((getViewSize ().getHeight () - checkBoxSize.getHeight ()) / 2));
		}
		outPath.addRoundRect (checkBoxSize, roundRectRadius);
		checkBoxSize.extend (focusLineWidth, focusLineWidth);
		outPath.addRoundRect (checkBoxSize, roundRectRadius);
	}
	return true;
}

//------------------------------------------------------------------------
CMouseEventResult CCheckBox::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton ())
	{
		beginEdit ();
		previousValue = getValue ();
		return onMouseMoved (where, buttons);
	}
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//------------------------------------------------------------------------
CMouseEventResult CCheckBox::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (isEditing ())
	{
		bool wasHighlighted = highlight;
		if (getViewSize ().pointInside (where))
			highlight = true;
		else
			highlight = false;
		if (wasHighlighted != highlight)
			invalid ();
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CCheckBox::onMouseCancel ()
{
	if (isEditing ())
	{
		highlight = false;
		if (setValue (previousValue))
			valueChanged ();
		invalid ();
		endEdit ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CCheckBox::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	highlight = false;
	bool changed = false;
	if (getViewSize ().pointInside (where))
		changed = setValue ((previousValue < getMax ()) ? getMax () : getMin ());
	else
		changed = setValue (previousValue);
	if (changed)
		valueChanged ();
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
void CCheckBox::onKeyboardEvent (KeyboardEvent& event)
{
	if (event.type == EventType::KeyDown && event.virt == VirtualKey::Return &&
	    event.modifiers.empty ())
	{
		setValue ((getValue () < getMax ()) ? getMax () : getMin ());
		beginEdit ();
		valueChanged ();
		endEdit ();
		event.consumed = true;
	}
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

	gradient = CGradient::create (0, 1, CColor (220, 220, 220, 255), CColor (180, 180, 180, 255));
	gradientHighlighted =
		CGradient::create (0, 1, CColor (180, 180, 180, 255), CColor (100, 100, 100, 255));

	setFrameColor (kBlackCColor);
	setFrameColorHighlighted (kBlackCColor);
	setWantsFocus (true);
}

//------------------------------------------------------------------------
bool CTextButton::removed (const SharedPointer<CViewContainer>& parent)
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
SharedPointer<CGradient> CTextButton::getGradient () const { return gradient; }

//------------------------------------------------------------------------
SharedPointer<CGradient> CTextButton::getGradientHighlighted () const
{
	return gradientHighlighted;
}

//------------------------------------------------------------------------
SharedPointer<CBitmap> CTextButton::getIcon () const { return icon; }

//------------------------------------------------------------------------
SharedPointer<CBitmap> CTextButton::getIconHighlighted () const { return iconHighlighted; }

//------------------------------------------------------------------------
void CTextButton::setTitle (const UTF8String& newTitle)
{
	title = newTitle;
	invalid ();
}

//------------------------------------------------------------------------
void CTextButton::setFont (const SharedPointer<CFontDesc>& newFont)
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
void CTextButton::setGradient (const SharedPointer<CGradient>& newGradient)
{
	gradient = newGradient;
	invalid ();
}

//------------------------------------------------------------------------
void CTextButton::setGradientHighlighted (const SharedPointer<CGradient>& newGradient)
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
void CTextButton::setIcon (const SharedPointer<CBitmap>& bitmap)
{
	if (icon != bitmap)
	{
		icon = bitmap;
		invalid ();
	}
}

//------------------------------------------------------------------------
void CTextButton::setIconHighlighted (const SharedPointer<CBitmap>& bitmap)
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
	bool highlight = getValue () == getMax () ? true : false;
	auto lineWidth = getFrameWidth ();
	if (lineWidth < 0.)
		lineWidth = context->getHairlineSize ();
	context->setDrawMode (kAntiAliasing);
	context->setLineWidth (lineWidth);
	context->setLineStyle (CLineStyle (CLineStyle::kLineCapRound, CLineStyle::kLineJoinRound));
	context->setFrameColor (highlight ? frameColorHighlighted : frameColor);
	CRect r (getViewSize ());
	r.inset (lineWidth / 2., lineWidth / 2.);
	if (gradient && gradientHighlighted)
	{
		auto path = getPath (context, lineWidth);
		if (path)
		{
			if (auto drawGradient = highlight ? gradientHighlighted : gradient)
			{
				context->fillLinearGradient (path, *drawGradient.get (), r.getTopLeft (),
											 r.getBottomLeft (), false);
			}
			context->drawGraphicsPath (path, CDrawContext::kPathStroked);
		}
	}
	CRect titleRect = getViewSize ();
	titleRect.inset (lineWidth / 2., lineWidth / 2.);

	SharedPointer<CBitmap> iconToDraw;
	if (!getMouseEnabled () && getDisabledBackground ())
		iconToDraw = getDisabledBackground ();
	else
		iconToDraw = highlight ? (iconHighlighted ? iconHighlighted : icon) : (icon ? icon : iconHighlighted);
	CDrawMethods::drawIconAndText (context, iconToDraw, iconPosition, getTextAlignment (),
								   getTextMargin (), titleRect, title, getFont (),
								   highlight ? getTextColorHighlighted () : getTextColor ());
}

//------------------------------------------------------------------------
bool CTextButton::getFocusPath (CGraphicsPath& outPath, CCoord focusLineWidth)
{
	CRect r (getViewSize ());
	r.inset (-focusLineWidth, -focusLineWidth);
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
SharedPointer<CGraphicsPath> CTextButton::getPath (CDrawContext* context, CCoord lineWidth)
{
	if (_path == nullptr)
	{
		CRect r (getViewSize ());
		r.inset (lineWidth / 2., lineWidth / 2.);
		_path = context->createRoundRectGraphicsPath (r, roundRadius);
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
	fEntryState = getValue ();
	beginEdit ();
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CTextButton::onMouseCancel ()
{
	if (isEditing ())
	{
		setValue (fEntryState);
		endEdit ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CTextButton::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	if (isEditing ())
	{
		if (getValue () != fEntryState)
		{
			valueChanged ();
			if (style == kKickStyle)
			{
				if (setValue (getMin ())) // set button to UNSELECTED state
					valueChanged ();
			}
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
			setValue (fEntryState == getMin () ? getMax () : getMin ());
		else
			setValue (fEntryState == getMin () ? getMin () : getMax ());

		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
void CTextButton::onKeyboardEvent (KeyboardEvent& event)
{
	if (event.type != EventType::KeyDown)
		return;
	if (event.modifiers.empty () && event.virt == VirtualKey::Return)
	{
		if (style == kKickStyle)
		{
			if (getValue () != getMax ())
			{
				beginEdit ();
				if (setValue (getMax ()))
					valueChanged ();
				if (setValue (getMin ()))
					valueChanged ();
				endEdit ();
			}
		}
		else
		{
			beginEdit ();
			if (getValue () == getMin ())
				setValue (getMax ());
			else
				setValue (getMin ());
			valueChanged ();
			endEdit ();
		}
		event.consumed = true;
	}
}

} // VSTGUI
