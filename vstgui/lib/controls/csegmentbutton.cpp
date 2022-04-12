// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "csegmentbutton.h"
#include "../cdrawcontext.h"
#include "../cframe.h"
#include "../cgraphicspath.h"
#include "../events.h"
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
	return (getSelectionMode () != SelectionMode::kMultiple || segments.size () < 32);
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
		case SelectionMode::kSingleToggle:
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
				switch (selectionMode)
				{
					case SelectionMode::kSingle:
					{
						uint32_t currentIndex = getSegmentIndex (getValueNormalized ());
						if (newIndex != currentIndex)
							setSelectedSegment (newIndex);
						break;
					}
					case SelectionMode::kSingleToggle:
					{
						uint32_t currentIndex = getSegmentIndex (getValueNormalized ());
						if (newIndex != currentIndex)
							setSelectedSegment (newIndex);
						else
						{
							++currentIndex;
							if (getSegments ().size () - 1 < currentIndex)
								currentIndex = 0;
							setSelectedSegment (currentIndex);
						}
						break;
					}
					case SelectionMode::kMultiple:
					{
						selectSegment (newIndex, !segment.selected);
						break;
					}
				}
				break; // out of for loop
			}
			newValue += valueOffset;

			// Last segment can lead to newValue > 1.0
			newValue = std::min(newValue, 1.f);
		}
	}
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//-----------------------------------------------------------------------------
void CSegmentButton::onKeyboardEvent (KeyboardEvent& event)
{
	if (event.type != EventType::KeyDown || event.modifiers.empty () == false ||
	    event.character != 0)
		return;
	if (selectionMode != SelectionMode::kMultiple)
	{
		uint32_t newIndex = getSegmentIndex (getValueNormalized ());
		uint32_t oldIndex = newIndex;
		switch (event.virt)
		{
			case VirtualKey::Left:
			{
				if (style == Style::kHorizontal && newIndex > 0)
					newIndex--;
				else if (style == Style::kHorizontalInverse && newIndex < segments.size () - 1)
					newIndex++;
				event.consumed = true;
				break;
			}
			case VirtualKey::Right:
			{
				if (style == Style::kHorizontal && newIndex < segments.size () - 1)
					newIndex++;
				else if (style == Style::kHorizontalInverse && newIndex > 0)
					newIndex--;
				event.consumed = true;
				break;
			}
			case VirtualKey::Up:
			{
				if (style == Style::kVertical && newIndex > 0)
					newIndex--;
				else if (style == Style::kVerticalInverse && newIndex < segments.size () - 1)
					newIndex++;
				event.consumed = true;
				break;
			}
			case VirtualKey::Down:
			{
				if (style == Style::kVertical && newIndex < segments.size () - 1)
					newIndex++;
				else if (style == Style::kVerticalInverse && newIndex > 0)
					newIndex--;
				event.consumed = true;
				break;
			}
			default: return;
		}
		if (newIndex != oldIndex)
		{
			setSelectedSegment (newIndex);
		}
	}
}

//-----------------------------------------------------------------------------
void CSegmentButton::draw (CDrawContext* pContext)
{
	CView::draw (pContext);
}

//-----------------------------------------------------------------------------
void CSegmentButton::drawRect (CDrawContext* pContext, const CRect& dirtyRect)
{
	if (getOldValue () != getValue ())
		verifySelections ();

	bool isHorizontal = isHorizontalStyle (style);
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
		if (!path)
			return;
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
	auto lineIndexStart = 1u;
	auto lineIndexEnd = segments.size ();
	if (isInverseStyle (style))
	{
		--lineIndexStart;
		--lineIndexEnd;
	}

	for (uint32_t index = 0u, end = static_cast<uint32_t> (segments.size ()); index < end; ++index)
	{
		const auto& segment = segments[index];
		if (!dirtyRect.rectOverlap (segment.rect))
			continue;

		drawClipped (pContext, segment.rect, [&] () {
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
		});
		if (drawLines && index >= lineIndexStart && index < lineIndexEnd)
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

	return std::min<uint32_t> (static_cast<uint32_t> (segments.size () - 1),
							   static_cast<uint32_t> (value * (segments.size ())));
}

//-----------------------------------------------------------------------------
void CSegmentButton::updateSegmentSizes ()
{
	if (isAttached () && !segments.empty ())
	{
		switch (style)
		{
			case Style::kHorizontal:
			{
				CCoord width = getWidth () / segments.size ();
				CRect r (getViewSize ());
				r.setWidth (width);
				for (auto& segment : segments)
				{
					segment.rect = r;
					r.offset (width, 0);
				}
				break;
			}
			case Style::kHorizontalInverse:
			{
				CCoord width = getWidth () / segments.size ();
				CRect r (getViewSize ());
				r.setWidth (width);
				for (auto it = segments.rbegin(); it != segments.rend(); ++it)
				{
					(*it).rect = r;
					r.offset (width, 0);
				}
				break;
			}
			case Style::kVertical:
			{
				CCoord height = getHeight () / segments.size ();
				CRect r (getViewSize ());
				r.setHeight (height);
				for (auto& segment : segments)
				{
					segment.rect = r;
					r.offset (0, height);
				}
				break;
			}
			case Style::kVerticalInverse:
			{
				CCoord height = getHeight () / segments.size ();
				CRect r (getViewSize ());
				r.setHeight (height);
				for (auto it = segments.rbegin(); it != segments.rend(); ++it)
				{
					(*it).rect = r;
					r.offset (0, height);
				}
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CSegmentButton::verifySelections ()
{
	if (selectionMode == SelectionMode::kMultiple)
	{
		auto bitset = static_cast<uint32_t> (value);
		for (auto index = 0u; index < segments.size (); ++index)
		{
			segments[index].selected = (bitset & (1 << index)) != 0;
		}
	}
	else
	{
		auto selectedIndex = getSelectedSegment ();
		if (selectedIndex > segments.size ())
			selectedIndex = 0;
		for (auto& segment : segments)
			segment.selected = false;
		segments[selectedIndex].selected = true;
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
