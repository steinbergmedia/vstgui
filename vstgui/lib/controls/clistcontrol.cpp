// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../cbitmap.h"
#include "../cdrawcontext.h"
#include "../cframe.h"
#include "../cgraphicspath.h"
#include "../cscrollview.h"
#include "../events.h"
#include "clistcontrol.h"
#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
struct CListControl::Impl
{
	SharedPointer<IListControlDrawer> drawer;
	SharedPointer<IListControlConfigurator> configurator;

	std::vector<CListControlRowDesc> rowDescriptions;
	Optional<int32_t> hoveredRow {};
	bool doHoverCheck {false};
	CCoord minHeight {0.};
};

//------------------------------------------------------------------------
CListControl::CListControl (const CRect& size, IControlListener* listener, int32_t tag)
: CControl (size, listener, tag)
{
	impl = std::unique_ptr<Impl> (new Impl);
}

//------------------------------------------------------------------------
CListControl::~CListControl () = default;

//------------------------------------------------------------------------
void CListControl::setDrawer (IListControlDrawer* d)
{
	impl->drawer = d;
}

//------------------------------------------------------------------------
void CListControl::setConfigurator (IListControlConfigurator* c)
{
	impl->configurator = c;
	recalculateLayout ();
}

//------------------------------------------------------------------------
IListControlDrawer* CListControl::getDrawer () const
{
	return impl->drawer;
}

//------------------------------------------------------------------------
IListControlConfigurator* CListControl::getConfigurator () const
{
	return impl->configurator;
}

//------------------------------------------------------------------------
Optional<int32_t> CListControl::getHoveredRow () const
{
	if (impl->hoveredRow)
	 	return makeOptional (*impl->hoveredRow);
	return {};
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
	return static_cast<int32_t> (std::round (getValue ()));
}

//------------------------------------------------------------------------
void CListControl::recalculateLayout ()
{
	if (!impl->configurator)
		return;

	CCoord height = 0.;
	auto numRows = getNumRows ();
	impl->rowDescriptions.resize (static_cast<size_t> (numRows));
	impl->doHoverCheck = false;

	for (auto row = 0; row < numRows; ++row)
	{
		impl->rowDescriptions[row] = impl->configurator->getRowDesc (row);
		height += impl->rowDescriptions[row].height;
		impl->doHoverCheck |= (impl->rowDescriptions[row].flags & CListControlRowDesc::Hoverable) != 0;
	}

	if (impl->minHeight > 0 && height < impl->minHeight)
		height = impl->minHeight;

	auto viewSize = getViewSize ();
	if (viewSize.getHeight () != height)
	{
		viewSize.setHeight (height);
		setViewSize (viewSize);
		setMouseableArea (viewSize);
	}
}

//------------------------------------------------------------------------
Optional<CRect> CListControl::getRowRect (int32_t row) const
{
	if (row < getMinRowIndex () || row > getMaxRowIndex ())
		return {};
	row -= getMinRowIndex ();
	CRect rowSize;
	rowSize.setWidth (getWidth ());
	for (auto i = 0u; i < impl->rowDescriptions.size (); ++i)
	{
		rowSize.setHeight (impl->rowDescriptions[i].height);
		if (i == row)
			break;
		rowSize.offset (0, impl->rowDescriptions[i].height);
	}
	rowSize.offset (getViewSize ().getTopLeft ());
	return makeOptional (rowSize);
}

//------------------------------------------------------------------------
void CListControl::invalidRow (int32_t row)
{
	if (auto rect = getRowRect (row))
		invalidRect (*rect);
}

//------------------------------------------------------------------------
Optional<int32_t> CListControl::getRowAtPoint (CPoint where) const
{
	where.offsetInverse (getViewSize ().getTopLeft ());

	auto numRows = getNumRows ();
	for (auto row = 0; row < numRows; ++row)
	{
		if (where.y < impl->rowDescriptions[row].height)
			return {row + getMinRowIndex ()};
		where.y -= impl->rowDescriptions[row].height;
	}
	return {};
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

	if (!impl->drawer)
		return;

	if (!getTransparency ())
		impl->drawer->drawBackground (context, getViewSize ());

	CRect rowSize;
	rowSize.setTopLeft (getViewSize ().getTopLeft ());
	rowSize.setWidth (getWidth ());
	rowSize.setHeight (0);
	auto numRows = getNumRows ();
	auto selectedRow = static_cast<int32_t> (getNormalizedRowIndex (getIntValue ()));
	for (auto row = 0; row < numRows; ++row)
	{
		rowSize.setHeight (impl->rowDescriptions[row].height);
		if (updateRect.rectOverlap (rowSize))
		{
			int32_t flags = selectedRow == row ? IListControlDrawer::Row::Selected : 0;
			if (impl->rowDescriptions[row].flags & CListControlRowDesc::Selectable)
				flags |= IListControlDrawer::Row::Selectable;
			if (impl->hoveredRow && *impl->hoveredRow == row + getMinRowIndex ())
				flags |= IListControlDrawer::Row::Hovered;
			if (row == numRows - 1)
				flags |= IListControlDrawer::Row::LastRow;
			impl->drawer->drawRow (context, rowSize, {row + getMinRowIndex (), flags});
		}
		rowSize.offset (0, impl->rowDescriptions[row].height);
	}
}

//------------------------------------------------------------------------
bool CListControl::attached (CView* parent)
{
	if (auto scrollView = dynamic_cast<CScrollView*> (parent->getParentView ()))
	{
		impl->minHeight = scrollView->calculateOptimalContainerSize ().getHeight ();
		struct SizeListener : ViewListenerAdapter
		{
			SizeListener (CListControl* listControl, CScrollView* scrollView)
			: control (listControl), scrollView (scrollView)
			{
				listControl->registerViewListener (this);
				scrollView->registerViewListener (this);
			}
			~SizeListener () noexcept
			{
				control->unregisterViewListener (this);
				scrollView->unregisterViewListener (this);
			}
			void viewSizeChanged (CView* view, const CRect& oldSize) override
			{
				if (view != scrollView)
					return;
				control->impl->minHeight =
					scrollView->calculateOptimalContainerSize ().getHeight ();
				control->recalculateLayout ();
			}
			void viewWillDelete (CView* view) override
			{
				if (view == control || view == scrollView)
					delete this;
			}
			void viewAttached (CView* view) override {}
			void viewRemoved (CView* view) override {}

			CListControl* control {nullptr};
			CScrollView* scrollView {nullptr};
		};
		new SizeListener (this, scrollView);
	}
	recalculateLayout ();
	return CControl::attached (parent);
}

//------------------------------------------------------------------------
void CListControl::setMin (float val)
{
	if (getMin () != val && val < getMax ())
	{
		auto ov = getValue ();
		CControl::setMin (val);
		if (isAttached ())
			recalculateLayout ();
		if (ov != getValue ())
			valueChanged ();
	}
}

//------------------------------------------------------------------------
void CListControl::setMax (float val)
{
	if (getMax () != val && val >= getMin ())
	{
		auto ov = getValue ();
		CControl::setMax (val);
		if (isAttached ())
			recalculateLayout ();
		if (ov != getValue ())
			valueChanged ();
	}
}

//------------------------------------------------------------------------
size_t CListControl::getNormalizedRowIndex (int32_t row) const
{
	vstgui_assert (row >= getMinRowIndex ());
	return row - getMinRowIndex ();
}

//------------------------------------------------------------------------
CMouseEventResult CListControl::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton ())
		return kMouseEventHandled;
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//------------------------------------------------------------------------
void CListControl::clearHoveredRow ()
{
	if (impl->hoveredRow)
	{
		invalidRow (*impl->hoveredRow);
		impl->hoveredRow.reset ();
	}
}

//------------------------------------------------------------------------
CMouseEventResult CListControl::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (impl->doHoverCheck)
	{
		auto row = getRowAtPoint (where);
		if (row)
		{
			if (impl->rowDescriptions[getNormalizedRowIndex (*row)].flags &
			    CListControlRowDesc::Hoverable)
			{
				if (!impl->hoveredRow || *impl->hoveredRow != *row)
				{
					clearHoveredRow ();
					impl->hoveredRow = makeOptional (*row);
					invalidRow (*row);
				}
			}
			else
				clearHoveredRow ();
		}
		else
			clearHoveredRow ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CListControl::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	if (impl->rowDescriptions.empty () || !buttons.isLeftButton ())
		return kMouseEventHandled;

	auto row = getRowAtPoint (where);
	if (row && getIntValue () != *row)
	{
		if (rowSelectable (*row))
		{
			invalidRow (getIntValue ());
			beginEdit ();
			setValue (static_cast<float> (*row));
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
	clearHoveredRow ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
int32_t CListControl::getMinRowIndex () const
{
	return static_cast<int32_t> (getMin ());
}

//------------------------------------------------------------------------
int32_t CListControl::getMaxRowIndex () const
{
	return static_cast<int32_t> (getMax ());
}

//------------------------------------------------------------------------
int32_t CListControl::getNextSelectableRow (int32_t r, int32_t direction) const
{
	auto minRowIndex = getMinRowIndex ();
	auto maxRowIndex = getMaxRowIndex ();
	int32_t row = r;
	do
	{
		row += direction;
		if (row > maxRowIndex)
			row = minRowIndex;
		else if (row < minRowIndex)
			row = maxRowIndex;
		if (rowSelectable (row))
			break;
	} while (row != r);
	return row;
}

//------------------------------------------------------------------------
bool CListControl::rowSelectable (int32_t row) const
{
	return (impl->rowDescriptions[getNormalizedRowIndex (row)].flags &
	        CListControlRowDesc::Selectable) != 0;
}

//------------------------------------------------------------------------
void CListControl::onKeyboardEvent (KeyboardEvent& event)
{
	if (event.type != EventType::KeyDown)
		return;
	if (getMouseEnabled () && event.character == 0)
	{
		int32_t newRow = getIntValue ();
		switch (event.virt)
		{
			default: return;
			case VirtualKey::Home:
			{
				if (!event.modifiers.empty ())
					break;
				newRow = getMinRowIndex ();
				if (!rowSelectable (newRow))
					newRow = getNextSelectableRow (newRow, 1);
				break;
			}
			case VirtualKey::End:
			{
				if (!event.modifiers.empty ())
					break;
				newRow = getMaxRowIndex ();
				if (!rowSelectable (newRow))
					newRow = getNextSelectableRow (newRow, -1);
				break;
			}
			case VirtualKey::Up:
			{
				if (!event.modifiers.empty ())
					break;
				newRow = getNextSelectableRow (newRow, -1);
				break;
			}
			case VirtualKey::Down:
			{
				if (!event.modifiers.empty ())
					break;
				newRow = getNextSelectableRow (newRow, 1);
				break;
			}
			case VirtualKey::PageUp:
			{
				if (!event.modifiers.empty ())
					break;
				auto vr = getVisibleViewSize ();
				auto rr = getRowRect (newRow);
				if (rr && !vr.rectOverlap (*rr))
				{
					if (auto parent = getParentView ())
					{
						if (auto scrollView = dynamic_cast<CScrollView*> (parent->getParentView ()))
						{
							scrollView->makeRectVisible (*rr);
							onKeyboardEvent (event);
							return;
						}
					}
				}
				vr.top += 2;
				if (auto firstVisibleRow = getRowAtPoint (vr.getTopLeft ()))
				{
					while (!rowSelectable (*firstVisibleRow))
						*firstVisibleRow += 1;
					if (*firstVisibleRow == getIntValue ())
					{
						vr.offset (0, -vr.getHeight ());
						if ((firstVisibleRow = getRowAtPoint (vr.getTopLeft ())))
							newRow = *firstVisibleRow;
						else
							newRow = getMinRowIndex ();
					}
					else
					{
						newRow = *firstVisibleRow;
					}
				}
				if (!rowSelectable (newRow))
					newRow = getNextSelectableRow (newRow, -1);
				break;
			}
			case VirtualKey::PageDown:
			{
				if (!event.modifiers.empty ())
					break;
				auto vr = getVisibleViewSize ();
				auto rr = getRowRect (newRow);
				if (rr && !vr.rectOverlap (*rr))
				{
					if (auto parent = getParentView ())
					{
						if (auto scrollView = dynamic_cast<CScrollView*> (parent->getParentView ()))
						{
							scrollView->makeRectVisible (*rr);
							onKeyboardEvent (event);
							return;
						}
					}
				}
				vr.bottom -= 2;
				if (auto lastVisibleRow = getRowAtPoint (vr.getBottomLeft ()))
				{
					while (!rowSelectable (*lastVisibleRow))
						*lastVisibleRow -= 1;
					if (*lastVisibleRow == getIntValue ())
					{
						vr.offset (0, vr.getHeight ());
						if ((lastVisibleRow = getRowAtPoint (vr.getBottomLeft ())))
							newRow = *lastVisibleRow;
						else
							newRow = getMaxRowIndex ();
					}
					else
					{
						newRow = *lastVisibleRow;
					}
				}
				if (!rowSelectable (newRow))
					newRow = getNextSelectableRow (newRow, 1);
				break;
			}
		}
		if (newRow != getIntValue () && rowSelectable (newRow))
		{
			invalidRow (getIntValue ());
			beginEdit ();
			setValue (static_cast<float> (newRow));
			valueChanged ();
			endEdit ();
			if (auto rowRect = getRowRect (getIntValue ()))
			{
				invalidRect (*rowRect);
				if (auto parent = getParentView ())
				{
					if (auto scrollView = dynamic_cast<CScrollView*> (parent->getParentView ()))
						scrollView->makeRectVisible (*rowRect);
				}
			}
			event.consumed = true;
		}
	}
}

//------------------------------------------------------------------------
void CListControl::setViewSize (const CRect& rect, bool invalid)
{
	CControl::setViewSize (rect, invalid);
	impl->hoveredRow.reset ();
}

//------------------------------------------------------------------------
bool CListControl::drawFocusOnTop ()
{
	return true;
}

//------------------------------------------------------------------------
bool CListControl::getFocusPath (CGraphicsPath& outPath)
{
	CRect r = getVisibleViewSize ();
	outPath.addRect (r);
	CCoord focusWidth = getFrame ()->getFocusWidth ();
	r.inset (focusWidth, focusWidth);
	outPath.addRect (r);
	return true;
}


//------------------------------------------------------------------------
} // VSTGUI
