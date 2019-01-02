// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "csegmentbutton.h"
#include "../cdrawcontext.h"
#include "../cframe.h"
#include "../cgraphicspath.h"
#include <algorithm>

namespace VSTGUI {

//-----------------------------------------------------------------------------
CSegmentButton::CSegmentButton (const CRect& size, IControlListener* listener, int32_t tag)
: CControl (size, listener, tag), font (kNormalFont)
{
	setWantsFocus (true);
}

//-----------------------------------------------------------------------------
bool CSegmentButton::canAddOneMoreSegment () const
{
	return (getSelectionMode () == SelectionMode::kSingle || segments.size () < 32);
}

//-----------------------------------------------------------------------------
bool CSegmentButton::addSegment (const Segment& segment, uint32_t index)
{
	if (!canAddOneMoreSegment ())
		return false;
	if (index == kPushBack && segments.size () < kPushBack)
		segments.emplace_back (segment);
	else if (index < segments.size ())
	{
		auto it = segments.begin ();
		std::advance (it, index);
		segments.insert (it, segment);
	}
	updateSegmentSizes ();
	return true;
}

//-----------------------------------------------------------------------------
bool CSegmentButton::addSegment (Segment&& segment, uint32_t index)
{
	if (!canAddOneMoreSegment ())
		return false;
	if (index == kPushBack && segments.size () < kPushBack)
		segments.emplace_back (std::move (segment));
	else if (index < segments.size ())
	{
		auto it = segments.begin ();
		std::advance (it, index);
		segments.insert (it, std::move (segment));
	}
	updateSegmentSizes ();
	return true;
}

//-----------------------------------------------------------------------------
void CSegmentButton::removeSegment (uint32_t index)
{
	if (index < segments.size ())
	{
		auto it = segments.begin ();
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
void CSegmentButton::valueChanged ()
{
	switch (getSelectionMode ())
	{
		case SelectionMode::kSingle:
		{
			auto index = static_cast<int64_t> (getSelectedSegment ());
			for (auto& segment : segments)
			{
				bool state = index == 0;
				if (state != segment.selected)
				{
					segment.selected = state;
					invalidRect (segment.rect);
				}
				--index;
			}
			break;
		}
		case SelectionMode::kMultiple:
		{
			auto bitset = static_cast<uint32_t> (value);
			size_t index = 0;
			for (auto& segment : segments)
			{
				bool state = (hasBit (bitset, 1 << index));
				if (state != segment.selected)
				{
					segment.selected = state;
					invalidRect (segment.rect);
				}
				++index;
			}
			break;
		}
	}
	CControl::valueChanged ();
}

//-----------------------------------------------------------------------------
void CSegmentButton::setSelectedSegment (uint32_t index)
{
	if (index >= segments.size ())
		return;
	beginEdit ();
	setValueNormalized (static_cast<float> (index) / static_cast<float> (segments.size () - 1));
	valueChanged ();
	endEdit ();
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
void CSegmentButton::setSelectionMode (SelectionMode mode)
{
	if (mode != selectionMode)
	{
		selectionMode = mode;
		if (isAttached ())
		{
			verifySelections ();
			invalid ();
		}
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
bool CSegmentButton::attached (CView* parent)
{
	if (CControl::attached (parent))
	{
		verifySelections ();
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

//------------------------------------------------------------------------
void CSegmentButton::selectSegment (uint32_t index, bool state)
{
	beginEdit ();
	auto bitset = static_cast<uint32_t> (value);
	setBit (bitset, (1 << index), state);
	value = static_cast<float> (bitset);
	valueChanged ();
	endEdit ();
}

//------------------------------------------------------------------------
bool CSegmentButton::isSegmentSelected (uint32_t index) const
{
	return segments[index].selected;
}

//-----------------------------------------------------------------------------
CMouseEventResult CSegmentButton::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton ())
	{
		float newValue = 0;
		float valueOffset = 1.f / (segments.size () - 1);
		for (auto& segment : segments)
		{
			if (segment.rect.pointInside (where))
			{
				uint32_t newIndex = getSegmentIndex (newValue);
				if (selectionMode == SelectionMode::kSingle)
				{
					uint32_t currentIndex = getSegmentIndex (getValueNormalized ());
					if (newIndex != currentIndex)
					{
						setSelectedSegment (newIndex);
					}
				}
				else
				{
					selectSegment (newIndex, !segment.selected);
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
	if (selectionMode == SelectionMode::kSingle && keyCode.modifier == 0 && keyCode.character == 0)
	{
		uint32_t newIndex = getSegmentIndex (getValueNormalized ());
		uint32_t oldIndex = newIndex;
		switch (keyCode.virt)
		{
			case VKEY_LEFT:
			{
				if (style == Style::kHorizontal && newIndex > 0)
					newIndex--;
				result = 1;
				break;
			}
			case VKEY_RIGHT:
			{
				if (style == Style::kHorizontal && newIndex < segments.size () - 1)
					newIndex++;
				result = 1;
				break;
			}
			case VKEY_UP:
			{
				if (style == Style::kVertical && newIndex > 0)
					newIndex--;
				result = 1;
				break;
			}
			case VKEY_DOWN:
			{
				if (style == Style::kVertical && newIndex < segments.size () - 1)
					newIndex++;
				result = 1;
				break;
			}
		}
		if (newIndex != oldIndex)
		{
			setSelectedSegment (newIndex);
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
	bool isHorizontal = style == Style::kHorizontal;
	bool drawLines = getFrameWidth () != 0. && getFrameColor ().alpha != 0;
	auto lineWidth = getFrameWidth ();
	if (lineWidth < 0.)
	{
		lineWidth = pContext->getHairlineSize ();
	}
	SharedPointer<CGraphicsPath> path;
	if (gradient || gradientHighlighted || drawLines)
	{
		CRect r (getViewSize ());
		r.inset (lineWidth / 2., lineWidth / 2.);
		path = owned (pContext->createGraphicsPath ());
		path->addRoundRect (r, getRoundRadius ());
	}
	pContext->setDrawMode (kAntiAliasing);
	if (drawLines)
	{
		pContext->setLineStyle (kLineSolid);
		pContext->setLineWidth (lineWidth);
		pContext->setFrameColor (getFrameColor ());
	}
	if (gradient)
	{
		if (isHorizontal)
		{
			pContext->fillLinearGradient (path, *gradient, getViewSize ().getTopLeft (),
			                              getViewSize ().getBottomLeft ());
		}
		else
		{
			pContext->fillLinearGradient (path, *gradient, getViewSize ().getTopLeft (),
			                              getViewSize ().getTopRight ());
		}
	}
	for (uint32_t index = 0u, end = static_cast<uint32_t> (segments.size ()); index < end; ++index)
	{
		const auto& segment = segments[index];
		if (!dirtyRect.rectOverlap (segment.rect))
			continue;
		CRect oldClip;
		pContext->getClipRect (oldClip);
		CRect clipRect (segment.rect);
		clipRect.bound (oldClip);
		pContext->setClipRect (clipRect);
		if (segment.selected && gradientHighlighted)
		{
			if (isHorizontal)
			{
				pContext->fillLinearGradient (path, *gradientHighlighted,
				                              segment.rect.getTopLeft (),
				                              segment.rect.getBottomLeft ());
			}
			else
			{
				pContext->fillLinearGradient (path, *gradientHighlighted,
				                              segment.rect.getTopLeft (),
				                              segment.rect.getTopRight ());
			}
		}
		if (segment.selected && segment.backgroundHighlighted)
		{
			segment.backgroundHighlighted->draw (pContext, segment.rect);
		}
		else if (segment.background)
		{
			segment.background->draw (pContext, segment.rect);
		}
		CDrawMethods::drawIconAndText (
		    pContext, segment.selected ? segment.iconHighlighted : segment.icon,
		    segment.iconPosition, textAlignment, textMargin, segment.rect, segment.name, font,
		    segment.selected ? textColorHighlighted : textColor, textTruncateMode);
		pContext->setClipRect (oldClip);
		if (drawLines && index > 0 && index < segments.size ())
		{
			path->beginSubpath (segment.rect.getTopLeft ());
			path->addLine (isHorizontal ? segment.rect.getBottomLeft () :
			                              segment.rect.getTopRight ());
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
		if (style == Style::kHorizontal)
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
void CSegmentButton::verifySelections ()
{
	if (selectionMode == SelectionMode::kSingle)
	{
		auto selectedIndex = getSelectedSegment ();
		if (selectedIndex > segments.size ())
			selectedIndex = 0;
		for (auto& segment : segments)
			segment.selected = false;
		setSelectedSegment (selectedIndex);
	}
	else
	{
		auto bitset = static_cast<uint32_t> (value);
		for (auto index = 0u; index < segments.size (); ++index)
		{
			segments[index].selected = (bitset & (1 << index)) != 0;
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
	auto lineWidth = getFrameWidth ();
	if (lineWidth < 0.)
		lineWidth = 1.;
	CRect r (getViewSize ());
	r.inset (lineWidth / 2., lineWidth / 2.);
	outPath.addRoundRect (r, getRoundRadius ());
	CCoord focusWidth = getFrame ()->getFocusWidth ();
	r.extend (focusWidth, focusWidth);
	outPath.addRoundRect (r, getRoundRadius ());
	return true;
}

} // VSTGUI
