// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "csegmentbutton.h"
#include "../cdrawcontext.h"
#include "../cframe.h"
#include "../cgraphicspath.h"
#include <algorithm>

namespace VSTGUI {

uint32_t CSegmentButton::kPushBack = std::numeric_limits<uint32_t>::max ();

//-----------------------------------------------------------------------------
CSegmentButton::CSegmentButton (const CRect& size, IControlListener* listener, int32_t tag)
: CControl (size, listener, tag)
, font (kNormalFont)
{
	setWantsFocus (true);
}

//-----------------------------------------------------------------------------
void CSegmentButton::addSegment (Segment segment, uint32_t index)
{
	if (index == kPushBack && segments.size () < kPushBack)
		segments.emplace_back (segment);
	else if (index < segments.size ())
	{
		Segments::iterator it = segments.begin ();
		std::advance (it, index);
		segments.insert (it, segment);
	}
	updateSegmentSizes ();
}

//-----------------------------------------------------------------------------
void CSegmentButton::removeSegment (uint32_t index)
{
	if (index < segments.size ())
	{
		Segments::iterator it = segments.begin ();
		std::advance (it, index);
		segments.erase (it);
	}
	updateSegmentSizes ();
}

//-----------------------------------------------------------------------------
void CSegmentButton::removeAllSegments ()
{
	segments.clear ();
	invalid ();
}

//-----------------------------------------------------------------------------
void CSegmentButton::setSelectedSegment (uint32_t index)
{
	setValueNormalized (static_cast<float> (index) / static_cast<float> (segments.size () - 1));
	invalid ();
}

//-----------------------------------------------------------------------------
uint32_t CSegmentButton::getSelectedSegment () const
{
	return getSegmentIndex (getValueNormalized ());
}

//-----------------------------------------------------------------------------
void CSegmentButton::setStyle (Style newStyle)
{
	if (style != newStyle)
	{
		style = newStyle;
		updateSegmentSizes ();
		invalid ();
	}
}

//-----------------------------------------------------------------------------
void CSegmentButton::setTextTruncateMode (CDrawMethods::TextTruncateMode mode)
{
	if (textTruncateMode != mode)
	{
		textTruncateMode = mode;
		invalid ();
	}
}

//-----------------------------------------------------------------------------
void CSegmentButton::setGradient (CGradient* newGradient)
{
	if (gradient != newGradient)
	{
		gradient = newGradient;
		invalid ();
	}
}

//-----------------------------------------------------------------------------
void CSegmentButton::setGradientHighlighted (CGradient* newGradient)
{
	if (gradientHighlighted != newGradient)
	{
		gradientHighlighted = newGradient;
		invalid ();
	}
}

//-----------------------------------------------------------------------------
void CSegmentButton::setRoundRadius (CCoord newRoundRadius)
{
	if (roundRadius != newRoundRadius)
	{
		roundRadius = newRoundRadius;
		invalid ();
	}
}

//-----------------------------------------------------------------------------
void CSegmentButton::setFont (CFontRef newFont)
{
	if (font != newFont)
	{
		font = newFont;
		invalid ();
	}
}

//-----------------------------------------------------------------------------
void CSegmentButton::setTextAlignment (CHoriTxtAlign newAlignment)
{
	if (textAlignment != newAlignment)
	{
		textAlignment = newAlignment;
		invalid ();
	}
}

//-----------------------------------------------------------------------------
void CSegmentButton::setTextMargin (CCoord newMargin)
{
	if (textMargin != newMargin)
	{
		textMargin = newMargin;
		invalid ();
	}
}

//-----------------------------------------------------------------------------
void CSegmentButton::setTextColor (CColor newColor)
{
	if (textColor != newColor)
	{
		textColor = newColor;
		invalid ();
	}
}

//-----------------------------------------------------------------------------
void CSegmentButton::setTextColorHighlighted (CColor newColor)
{
	if (textColorHighlighted != newColor)
	{
		textColorHighlighted = newColor;
		invalid ();
	}
}

//-----------------------------------------------------------------------------
void CSegmentButton::setFrameColor (CColor newColor)
{
	if (frameColor != newColor)
	{
		frameColor = newColor;
		invalid ();
	}
}

//-----------------------------------------------------------------------------
void CSegmentButton::setFrameWidth (CCoord newWidth)
{
	if (frameWidth != newWidth)
	{
		frameWidth = newWidth;
		invalid ();
	}
}

//-----------------------------------------------------------------------------
bool CSegmentButton::attached (CView *parent)
{
	if (CControl::attached (parent))
	{
		updateSegmentSizes ();
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void CSegmentButton::setViewSize (const CRect& rect, bool invalid)
{
	CControl::setViewSize (rect, invalid);
	updateSegmentSizes ();
}

//-----------------------------------------------------------------------------
CMouseEventResult CSegmentButton::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton ())
	{
		float newValue = 0;
		float valueOffset = 1.f / (segments.size () - 1);
		uint32_t currentIndex = getSegmentIndex (getValueNormalized ());
		for (auto& segment : segments)
		{
			if (segment.rect.pointInside (where))
			{
				uint32_t newIndex = getSegmentIndex (newValue);
				if (newIndex != currentIndex)
				{
					beginEdit ();
					setSelectedSegment (newIndex);
					valueChanged ();
					endEdit ();
					invalid ();
				}
				break;
			}
			newValue += valueOffset;
		}
	}
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//-----------------------------------------------------------------------------
int32_t CSegmentButton::onKeyDown (VstKeyCode& keyCode)
{
	int32_t result = -1;
	if (keyCode.modifier == 0 && keyCode.character == 0)
	{
		uint32_t newIndex = getSegmentIndex (getValueNormalized ());
		uint32_t oldIndex = newIndex;
		switch (keyCode.virt)
		{
			case VKEY_LEFT:
			{
				if (style == kHorizontal && newIndex > 0)
					newIndex--;
				result = 1;
				break;
			}
			case VKEY_RIGHT:
			{
				if (style == kHorizontal && newIndex < segments.size () - 1)
					newIndex++;
				result = 1;
				break;
			}
			case VKEY_UP:
			{
				if (style == kVertical && newIndex > 0)
					newIndex--;
				result = 1;
				break;
			}
			case VKEY_DOWN:
			{
				if (style == kVertical && newIndex < segments.size () - 1)
					newIndex++;
				result = 1;
				break;
			}
		}
		if (newIndex != oldIndex)
		{
			beginEdit ();
			setSelectedSegment (newIndex);
			valueChanged ();
			endEdit ();
			invalid ();
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
void CSegmentButton::draw (CDrawContext* pContext)
{
	CView::draw (pContext);
}

//-----------------------------------------------------------------------------
void CSegmentButton::drawRect (CDrawContext* pContext, const CRect& dirtyRect)
{
	bool isHorizontal = style == kHorizontal;
	SharedPointer<CGraphicsPath> path;
	if (gradient || gradientHighlighted || (getFrameWidth () > 0. && getFrameColor ().alpha != 0))
	{
		CRect r (getViewSize ());
		r.inset (getFrameWidth () / 2., getFrameWidth () / 2.);
		path = owned (pContext->createGraphicsPath ());
		path->addRoundRect (r, getRoundRadius ());
	}
	pContext->setDrawMode (kAntiAliasing);
	bool drawLines = getFrameWidth () > 0. && getFrameColor ().alpha != 0;
	if (drawLines)
	{
		pContext->setLineStyle (kLineSolid);
		pContext->setLineWidth (getFrameWidth ());
		pContext->setFrameColor (getFrameColor ());
	}
	if (gradient)
	{
		pContext->fillLinearGradient (path, *gradient, getViewSize ().getTopLeft (), getViewSize ().getBottomLeft ());
	}
	uint32_t selectedIndex = getSelectedSegment ();
	for (uint32_t index = 0; index < segments.size (); ++index)
	{
		Segment& segment = segments[index];
		if (!dirtyRect.rectOverlap (segment.rect))
			continue;
		CRect oldClip;
		pContext->getClipRect (oldClip);
		CRect clipRect (segment.rect);
		clipRect.bound (oldClip);
		pContext->setClipRect (clipRect);
		bool selected = selectedIndex == index;
		if (selected && gradientHighlighted)
			pContext->fillLinearGradient (path, *gradientHighlighted, segment.rect.getTopLeft (), segment.rect.getBottomLeft ());
		if (selected && segment.backgroundHighlighted)
			segment.backgroundHighlighted->draw (pContext, segment.rect);
		else if (segment.background)
			segment.background->draw (pContext, segment.rect);
		CDrawMethods::drawIconAndText (pContext, selected ? segment.iconHighlighted : segment.icon, segment.iconPosition, textAlignment, textMargin, segment.rect, segment.name, font, selected ? textColorHighlighted : textColor, textTruncateMode);
		pContext->setClipRect (oldClip);
		if (drawLines && index > 0 && index < segments.size ())
		{
			path->beginSubpath (segment.rect.getTopLeft ());
			path->addLine (isHorizontal ? segment.rect.getBottomLeft () : segment.rect.getTopRight ());
		}
	}
	if (drawLines)
		pContext->drawGraphicsPath (path, CDrawContext::kPathStroked);
	setDirty (false);
}

//-----------------------------------------------------------------------------
uint32_t CSegmentButton::getSegmentIndex (float value) const
{
	if (value < 0.f || value > 1.f)
		return kPushBack;
	return static_cast<uint32_t> (static_cast<float> (segments.size () - 1) * value);
}

//-----------------------------------------------------------------------------
void CSegmentButton::updateSegmentSizes ()
{
	if (isAttached () && !segments.empty ())
	{
		if (style == kHorizontal)
		{
			CCoord width = getWidth () / segments.size ();
			CRect r (getViewSize ());
			r.setWidth (width);
			for (auto& segment : segments)
			{
				segment.rect = r;
				r.offset (width, 0);
			}
		}
		else
		{
			CCoord height = getHeight () / segments.size ();
			CRect r (getViewSize ());
			r.setHeight (height);
			for (auto& segment : segments)
			{
				segment.rect = r;
				r.offset (0, height);
			}
		}
	}
}

//-----------------------------------------------------------------------------
bool CSegmentButton::drawFocusOnTop ()
{
	return false;
}

//-----------------------------------------------------------------------------
bool CSegmentButton::getFocusPath (CGraphicsPath& outPath)
{
	CRect r (getViewSize ());
	r.inset (getFrameWidth () / 2., getFrameWidth () / 2.);
	outPath.addRoundRect (r, getRoundRadius ());
	CCoord focusWidth = getFrame ()->getFocusWidth ();
	r.extend (focusWidth, focusWidth);
	outPath.addRoundRect (r, getRoundRadius ());
	return true;
}

} // namespace
