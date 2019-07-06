// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../cbitmap.h"
#include "../cdrawcontext.h"
#include "clistcontrol.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
CListControl::CListControl (const CRect& size, IControlListener* listener, int32_t tag)
: CControl (size, listener, tag)
{
}

//------------------------------------------------------------------------
void CListControl::setDrawer (IListControlDrawer* d)
{
	drawer = d;
}

//------------------------------------------------------------------------
void CListControl::setConfigurator (IListControlConfigurator* c)
{
	configurator = c;
	if (configurator)
		recalculateHeight ();
}

//------------------------------------------------------------------------
int32_t CListControl::getNumRows () const
{
	auto numRows = static_cast<int32_t> (std::round (getRange ())) + 1;
	return std::max (0, numRows);
}

//------------------------------------------------------------------------
int32_t CListControl::getIntValue () const
{
	return static_cast<int32_t> (std::round (getValue () - getMin ()));
}

//------------------------------------------------------------------------
void CListControl::recalculateHeight ()
{
	if (!configurator)
		return;

	CCoord height = 0.;
	auto numRows = getNumRows ();
	rowDescriptions.resize (static_cast<size_t> (numRows));

	for (auto row = 0; row < numRows; ++row)
	{
		rowDescriptions[row] = configurator->getRowDesc (row);
		height += rowDescriptions[row].height;
	}

	auto viewSize = getViewSize ();
	if (viewSize.getHeight () != height)
	{
		viewSize.setHeight (height);
		setMouseableArea (viewSize);
		setViewSize (viewSize);
	}
}

//------------------------------------------------------------------------
void CListControl::invalidRow (int32_t row)
{
	CRect rowSize;
	rowSize.setWidth (getWidth ());
	for (auto i = 0u; i < rowDescriptions.size (); ++i)
	{
		rowSize.setHeight (rowDescriptions[row].height);
		if (i == row)
			break;
		rowSize.offset (0, rowDescriptions[row].height);
	}
	rowSize.offset (getViewSize ().getTopLeft ());
	invalidRect (rowSize);
}

//------------------------------------------------------------------------
int32_t CListControl::getRowAtPoint (CPoint where) const
{
	where.offsetInverse (getViewSize ().getTopLeft ());

	auto numRows = getNumRows ();
	for (auto row = 0; row < numRows; ++row)
	{
		if (where.y < rowDescriptions[row].height)
			return row;
		where.y -= rowDescriptions[row].height;
	}
	return -1;
}

//------------------------------------------------------------------------
void CListControl::draw (CDrawContext* context)
{
	drawRect (context, getViewSize ());
}

//------------------------------------------------------------------------
void CListControl::drawRect (CDrawContext* context, const CRect& updateRect)
{
	setDirty (false);
	ConcatClip cc (*context, updateRect);
	if (cc.isEmpty ())
		return;

	if (auto bitmap = getDrawBackground ())
		bitmap->draw (context, getViewSize ());

	if (!drawer)
		return;

	auto topLeft = getViewSize ().getTopLeft ();
	CDrawContext::Transform t (*context, CGraphicsTransform ().translate (topLeft.x, topLeft.y));
	CRect rowSize;
	rowSize.setWidth (getWidth ());
	auto numRows = getNumRows ();
	for (auto row = 0; row < numRows; ++row)
	{
		rowSize.setHeight (rowDescriptions[row].height);
		// TODO: if row is not covered by update rect do not draw
		int32_t flags = getIntValue () == row ? IListControlDrawer::Selected : 0;
		if (rowDescriptions[row].flags & CListControlRowDesc::Selectable)
			flags |= IListControlDrawer::Selectable;
		if (hoveredRow == row)
			flags |= IListControlDrawer::Hovered;
		if (row == numRows)
			flags |= IListControlDrawer::LastRow;
		drawer->drawRow (context, rowSize, row, flags);
		rowSize.offset (0, rowDescriptions[row].height);
	}
}

//------------------------------------------------------------------------
void CListControl::setMin (float val)
{
	CControl::setMin (val);
	recalculateHeight ();
}

//------------------------------------------------------------------------
void CListControl::setMax (float val)
{
	CControl::setMax (val);
	recalculateHeight ();
}

//------------------------------------------------------------------------
CMouseEventResult CListControl::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton ())
		return kMouseEventHandled;
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//------------------------------------------------------------------------
CMouseEventResult CListControl::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	// TODO: row hover
	auto row = getRowAtPoint (where);
	if (row != -1 && row != hoveredRow)
	{
		if (rowDescriptions[row].flags & CListControlRowDesc::Hoverable)
		{
			if (hoveredRow != -1)
				invalidRow (hoveredRow);
			hoveredRow = row;
			invalidRow (row);
		}
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CListControl::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	if (rowDescriptions.empty () || !buttons.isLeftButton ())
		return kMouseEventHandled;

	auto row = getRowAtPoint (where);
	if (row != -1 && getIntValue () != row)
	{
		if (rowDescriptions[row].flags & CListControlRowDesc::Selectable)
		{
			invalidRow (getIntValue ());
			beginEdit ();
			setValue (row);
			valueChanged ();
			endEdit ();
			invalidRow (getIntValue ());
		}
	}

	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CListControl::onMouseExited (CPoint& where, const CButtonState& buttons)
{
	if (hoveredRow != -1)
	{
		invalidRow (hoveredRow);
		hoveredRow = -1;
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
} // VSTGUI
