// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cdatabrowser.h"
#include "vstkeycode.h"
#include "cframe.h"
#include "controls/ctextedit.h"
#include "controls/cscrollbar.h"
#include "ifocusdrawing.h"
#include "cvstguitimer.h"
#include "cgraphicspath.h"
#include "platform/iplatformfont.h"
#include <cmath>
#include <algorithm>

namespace VSTGUI {

/// @cond ignore
//-----------------------------------------------------------------------------------------------
class CDataBrowserView : public CView, public IFocusDrawing
//-----------------------------------------------------------------------------------------------
{
public:
	CDataBrowserView (const CRect& size, IDataBrowserDelegate* db, CDataBrowser* browser);

	void draw (CDrawContext* context) override;
	void drawRect (CDrawContext* context, const CRect& updateRect) override;
	CMouseEventResult onMouseDown (CPoint &where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint &where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint &where, const CButtonState& buttons) override;
	CMouseEventResult onMouseExited (CPoint &where, const CButtonState& buttons) override;

	bool onDrop (IDataPackage* drag, const CPoint& where) override;
	void onDragEnter (IDataPackage* drag, const CPoint& where) override;
	void onDragLeave (IDataPackage* drag, const CPoint& where) override;
	void onDragMove (IDataPackage* drag, const CPoint& where) override;
	
	int32_t onKeyDown (VstKeyCode& keyCode) override;

	CRect getRowBounds (int32_t row);
	void invalidateRow (int32_t row);

	bool getCell (const CPoint& where, CDataBrowser::Cell& cell);

	bool drawFocusOnTop () override;
	bool getFocusPath (CGraphicsPath& outPath) override;
protected:

	IDataBrowserDelegate* db;
	CDataBrowser* browser;
};

//-----------------------------------------------------------------------------------------------
class CDataBrowserHeader : public CView
//-----------------------------------------------------------------------------------------------
{
public:
	CDataBrowserHeader (const CRect& size, IDataBrowserDelegate* db, CDataBrowser* browser);

	void draw (CDrawContext* context) override;
	void drawRect (CDrawContext* context, const CRect& updateRect) override;
	CMouseEventResult onMouseDown (CPoint &where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint &where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint &where, const CButtonState& buttons) override;
	CMouseEventResult onMouseExited (CPoint &where, const CButtonState& buttons) override;

protected:
	int32_t getColumnAtPoint (CPoint& where);

	IDataBrowserDelegate* db;
	CDataBrowser* browser;

	CPoint startMousePoint;
	int32_t mouseColumn;
	CCoord startWidth;
};
/// @endcond

//-----------------------------------------------------------------------------------------------
/**
 * @param size size of data browser
 * @param db data browser interface. If db is inheritated from CBaseObject it will be remembered and released if data browser is destroyed
 * @param style data browser and scroll view style see #CDataBrowserStyle and #CScrollViewStyle
 * @param scrollbarWidth width of scrollbars
 * @param pBackground background bitmap
 */
CDataBrowser::CDataBrowser (const CRect& size, IDataBrowserDelegate* db, int32_t style, CCoord scrollbarWidth, CBitmap* pBackground)
: CScrollView (size, CRect (0, 0, 0, 0), style, scrollbarWidth, pBackground)
, db (db)
, dbView (nullptr)
, dbHeader (nullptr)
, dbHeaderContainer (nullptr)
{
	setTransparency (true);
	dbView = new CDataBrowserView (CRect (0, 0, 0, 0), db, this);
	dbView->setAutosizeFlags (kAutosizeLeft|kAutosizeRight|kAutosizeBottom);
	addView (dbView);
	auto obj = dynamic_cast<IReference*>(db);
	if (obj)
		obj->remember ();
}

//-----------------------------------------------------------------------------------------------
CDataBrowser::~CDataBrowser () noexcept
{
	auto obj = dynamic_cast<IReference*>(db);
	if (obj)
		obj->forget ();
}

//-----------------------------------------------------------------------------------------------
void CDataBrowser::setAutosizeFlags (int32_t flags)
{
	CScrollView::setAutosizeFlags (flags);
}

//-----------------------------------------------------------------------------------------------
void CDataBrowser::setViewSize (const CRect& size, bool invalid)
{
	if (getViewSize () != size)
	{
		CScrollView::setViewSize (size, invalid);
		recalculateLayout (true);
	}
}

//-----------------------------------------------------------------------------------------------
void CDataBrowser::setWantsFocus (bool state)
{
	if (dbView)
	{
		dbView->setWantsFocus (state);
	}
}

//------------------------------------------------------------------------
bool CDataBrowser::wantsFocus () const
{
	return dbView ? dbView->wantsFocus () : false;
}

//-----------------------------------------------------------------------------------------------
bool CDataBrowser::attached (CView *parent)
{
	bool result = CScrollView::attached (parent);
	if (result)
	{
		recalculateLayout (true);
		db->dbAttached (this);
	}
	return result;
}

//-----------------------------------------------------------------------------------------------
bool CDataBrowser::removed (CView* parent)
{
	if (isAttached ())
		db->dbRemoved (this);
	return CScrollView::removed (parent);
}

//-----------------------------------------------------------------------------------------------
int32_t CDataBrowser::onKeyDown (VstKeyCode& keyCode)
{
	return dbView ? dbView->onKeyDown (keyCode) : -1;
}

//-----------------------------------------------------------------------------
CMouseEventResult CDataBrowser::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	CMouseEventResult result = CViewContainer::onMouseDown (where, buttons);
	if (auto frame = getFrame ())
	{
		CView* focusView = frame->getFocusView ();
		if (focusView != dbView && !isChild (focusView, true))
			frame->setFocusView (dbView);
	}
	return result;
}

//-----------------------------------------------------------------------------
void CDataBrowser::valueChanged (CControl *pControl)
{
	CPoint origOffset = getScrollOffset ();
	CScrollView::valueChanged (pControl);
	CPoint offset = getScrollOffset ();
	if (origOffset != offset)
	{
		switch (pControl->getTag ())
		{
			case kHSBTag:
			{
				if (dbHeader)
				{
					CRect viewSize = dbHeader->getViewSize ();
					CCoord width = viewSize.getWidth ();
					viewSize.left = offset.x;
					viewSize.setWidth (width);
					dbHeader->setViewSize (viewSize);
					dbHeader->setMouseableArea (viewSize);
					dbHeader->invalid ();
				}
				break;
			}
		}
		if (isAttached () && (getMouseDownView () == dbView || getMouseDownView () == nullptr))
		{
			CPoint where;
			getFrame ()->getCurrentMouseLocation (where);
			if (getFrame ()->getViewAt (where, GetViewOptions (GetViewOptions::kDeep)) == dbView)
			{
				CDataBrowser::Cell cell;
				dbView->frameToLocal (where);
				dbView->getCell (where, cell);
				db->dbOnMouseMoved (where, getFrame ()->getCurrentMouseButtons (), cell.row, cell.column, this);
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------
void CDataBrowser::recalculateSubViews ()
{
	CScrollView::recalculateSubViews ();
}

//-----------------------------------------------------------------------------------------------
/**
 * @param rememberSelection if true selection will be remembered
 */
void CDataBrowser::recalculateLayout (bool rememberSelection)
{
	CCoord lineWidth = 0;
	CColor lineColor;
	db->dbGetLineWidthAndColor (lineWidth, lineColor, this);
	CCoord rowHeight = db->dbGetRowHeight (this);
	CCoord headerHeight = db->dbGetHeaderHeight (this);
	int32_t numRows = db->dbGetNumRows (this);
	int32_t numColumns = db->dbGetNumColumns (this);
	CCoord allRowsHeight = rowHeight * numRows;
	if (style & kDrawRowLines)
		allRowsHeight += numRows * lineWidth;
	CCoord allColumnsWidth = 0;
	for (int32_t i = 0; i < numColumns; i++)
		allColumnsWidth += db->dbGetCurrentColumnWidth (i, this);
	if (style & kDrawColumnLines)
		allColumnsWidth += numColumns * lineWidth;
	CRect newContainerSize (0, 0, allColumnsWidth, allRowsHeight);
	if (style & kDrawHeader)
	{

		newContainerSize.offset (0, headerHeight+lineWidth);

		CRect headerSize (0, 0, newContainerSize.getWidth (), headerHeight+lineWidth);
		if (style & kHorizontalScrollbar && hsb)
			headerSize.right += hsb->getWidth ();
		if (dbHeader == nullptr)
		{
			CRect hcs (headerSize);
			if (!(style & kDontDrawFrame))
				hcs.left = hcs.top = 1;
			hcs.setWidth (getViewSize ().getWidth () - ((style & kDontDrawFrame) ? 0 : 2));
			dbHeaderContainer = new CViewContainer (hcs);
			dbHeaderContainer->setAutosizeFlags (kAutosizeLeft|kAutosizeRight|kAutosizeTop);
			dbHeaderContainer->setTransparency (true);
			headerSize.offset (-headerSize.left, -headerSize.top);
			dbHeader = new CDataBrowserHeader (headerSize, db, this);
			dbHeader->setAutosizeFlags (kAutosizeLeft|kAutosizeRight|kAutosizeTop);
			dbHeaderContainer->addView (dbHeader);
			CViewContainer::addView (dbHeaderContainer, nullptr);
		}
		else
		{
			headerSize.offset (getScrollOffset ().x, 0);
			dbHeader->setViewSize (headerSize);
			dbHeader->setMouseableArea (headerSize);
		}
	}
	setContainerSize (newContainerSize, true);
	if (dbView->getParentView ())
	{
		CRect ps = dbView->getParentView ()->getViewSize ();
		if (newContainerSize.getWidth () < ps.getWidth ())
			newContainerSize.setWidth (ps.getWidth ());
		if (newContainerSize.getHeight () < ps.getHeight ())
			newContainerSize.setHeight (ps.getHeight ());
		if (newContainerSize != getContainerSize ())
			setContainerSize (newContainerSize, true);
	}
	newContainerSize.offset (getScrollOffset ().x, -getScrollOffset ().y);
	dbView->setViewSize (newContainerSize);
	dbView->setMouseableArea (newContainerSize);

	CControl* scrollbar = getVerticalScrollbar ();
	if (scrollbar && newContainerSize.getHeight () > 0.)
	{
		float wheelInc = (float) (rowHeight / newContainerSize.getHeight ());
		scrollbar->setWheelInc (wheelInc);
	}

	if (style & kDrawHeader)
	{
		for (const auto& pV : getChildren ())
		{
			CRect viewSize = pV->getViewSize ();
			if (pV != dbHeaderContainer && viewSize.top < headerHeight+lineWidth)
			{
				if (style & kOverlayScrollbars && pV.cast<CScrollView> ())
					continue;

				bool autoSizingEnabled = false;
				if (auto container = pV->asViewContainer ())
				{
					autoSizingEnabled = container->getAutosizingEnabled ();
					container->setAutosizingEnabled (false);
				}
				viewSize.top += headerHeight+lineWidth;
				pV->setViewSize (viewSize);
				pV->setMouseableArea (viewSize);
				if (auto container = pV->asViewContainer ())
					container->setAutosizingEnabled (autoSizingEnabled);
			}
		}
	}
	
	if (isAttached ())
		invalid ();
		
	validateSelection ();
 
	if (!rememberSelection)
		unselectAll ();
}

//-----------------------------------------------------------------------------------------------
/**
 * @param cell cell to invalidate
 */
void CDataBrowser::invalidate (const Cell& cell)
{
	invalidRect (getCellBounds (cell));
}

//-----------------------------------------------------------------------------------------------
/**
 * @param row row to invalidate
 */
void CDataBrowser::invalidateRow (int32_t row)
{
	dbView->invalidateRow (row);
}

//-----------------------------------------------------------------------------------------------
/**
 * @param row row to make visible
 */
void CDataBrowser::makeRowVisible (int32_t row)
{
	CRect r = dbView->getRowBounds (row);
	makeRectVisible (r);
}

//-----------------------------------------------------------------------------------------------
/**
 * @param index row to select
 * @param makeVisible if true makes row visible
 */
void CDataBrowser::setSelectedRow (int32_t index, bool makeVisible)
{
	if (index == kNoSelection)
	{
		unselectAll ();
		return;
	}
	int32_t numRows = db->dbGetNumRows (this);
	if (index >= numRows)
		index = numRows-1;

	bool hasChanged = true;
	Selection::iterator alreadySelected = std::find (selection.begin (), selection.end (), index);
	if (alreadySelected != selection.end ())
	{
		selection.erase (alreadySelected);
		hasChanged = selection.size () > 0;
	}
	else
	{
		invalidateRow (index);
	}
	
	for (auto row : selection)
	{
		dbView->invalidateRow (row);
	}
	selection.clear ();
	
	selection.emplace_back (index);
	if (hasChanged)
		db->dbSelectionChanged (this);
	
	if (makeVisible)
		makeRowVisible (index);
}

//-----------------------------------------------------------------------------------------------
int32_t CDataBrowser::getSelectedRow () const
{
	if (selection.size () > 0)
		return selection[0];
	return kNoSelection;
}

//-----------------------------------------------------------------------------------------------
void CDataBrowser::selectRow (int32_t row)
{
	if (row > db->dbGetNumRows (this))
		return;
	Selection::iterator alreadySelected = std::find (selection.begin (), selection.end (), row);
	if (alreadySelected == selection.end ())
	{
		if (getStyle () & kMultiSelectionStyle)
		{
			selection.emplace_back (row);
			dbView->invalidateRow (row);
			db->dbSelectionChanged (this);
		}
		else
		{
			setSelectedRow (row);
		}
	}
}

//-----------------------------------------------------------------------------------------------
void CDataBrowser::unselectRow (int32_t row)
{
	if (row > db->dbGetNumRows (this))
		return;
	Selection::iterator alreadySelected = std::find (selection.begin (), selection.end (), row);
	if (alreadySelected != selection.end ())
	{
		if (getStyle () & kMultiSelectionStyle)
		{
			selection.erase (alreadySelected);
			dbView->invalidateRow (row);
			db->dbSelectionChanged (this);
		}
		else
		{
			unselectAll ();
		}
	}
}

//-----------------------------------------------------------------------------------------------
void CDataBrowser::unselectAll ()
{
	if (selection.size () > 0)
	{
		for (auto row : selection)
		{
			dbView->invalidateRow (row);
		}
		selection.clear ();
		db->dbSelectionChanged (this);
	}
}

//-----------------------------------------------------------------------------------------------
void CDataBrowser::validateSelection ()
{
	bool selectionChanged = false;
	int32_t numRows = db->dbGetNumRows (this);
	for (Selection::iterator it = selection.begin (); it != selection.end ();)
	{
		if (*it >= numRows)
		{
			it = selection.erase (it);
			selectionChanged = true;
		}
		else
		{
			++it;
		}
	}
	if (selectionChanged)
		db->dbSelectionChanged (this);
}

//-----------------------------------------------------------------------------------------------
/**
 * @param cell cell
 * @return bounds of cell
 */
CRect CDataBrowser::getCellBounds (const Cell& cell)
{
	CCoord lineWidth = 0;
	if (style & kDrawRowLines || style & kDrawColumnLines)
	{
		CColor lineColor;
		db->dbGetLineWidthAndColor (lineWidth, lineColor, this);
	}
	CCoord rowHeight = db->dbGetRowHeight (this);
	if (style & kDrawRowLines)
		rowHeight += lineWidth;
	CRect result (0, rowHeight * cell.row, 0, rowHeight * (cell.row+1));
	for (int32_t i = 0; i <= cell.column; i++)
	{
		CCoord colWidth = db->dbGetCurrentColumnWidth (i, this);
		if (i != cell.column)
		{
			result.offset (colWidth, 0);
			if (style & kDrawColumnLines)
				result.offset (lineWidth, 0);
		}
		result.setWidth (colWidth);
	}
	CRect viewSize = dbView->getViewSize ();
	result.offset (viewSize.left, viewSize.top);
	return result;
}

//-----------------------------------------------------------------------------------------------
CDataBrowser::Cell CDataBrowser::getCellAt (const CPoint& where) const
{
	Cell pos;
	if (dbView)
	{
		CPoint w (where);
		localToFrame (w);
		dbView->frameToLocal (w);
		if (dbView->hitTest (w))
			dbView->getCell (w, pos);
	}
	return pos;
}

//-----------------------------------------------------------------------------------------------
/**
 * @param cell cell
 * @param initialText UTF-8 string the text edit field will be initialized with
 */
void CDataBrowser::beginTextEdit (const Cell& cell, UTF8StringPtr initialText)
{
	CRect r = getCellBounds (cell);
	makeRectVisible (r);
	CRect cellRect = getCellBounds (cell);
	CTextEdit* te = new CTextEdit (cellRect, nullptr, -1, initialText);
	db->dbCellSetupTextEdit (cell.row, cell.column, te, this);
	addView (te);
	getFrame ()->setFocusView (te);
	// save row and column
	te->setAttribute ('row ', sizeof (int32_t), &cell.row);
	te->setAttribute ('col ', sizeof (int32_t), &cell.column);
}

//-----------------------------------------------------------------------------------------------
CMessageResult CDataBrowser::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == kMsgLooseFocus)
	{
		CTextEdit* te = dynamic_cast<CTextEdit*>(sender);
		if (te)
		{
			// get row and column
			int32_t row = kNoSelection;
			int32_t col = kNoSelection;
			uint32_t outSize;
			te->getAttribute ('row ', sizeof (int32_t), &row, outSize);
			te->getAttribute ('col ', sizeof (int32_t), &col, outSize);
			UTF8StringPtr newText = te->getText ();
			db->dbCellTextChanged (row, col, newText, this);
			removeView (te);
			getFrame ()->setFocusView (dbView);
			return kMessageNotified;
		}
	}
	return CViewContainer::notify (sender, message);
}

/// @cond ignore
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
CDataBrowserHeader::CDataBrowserHeader (const CRect& size, IDataBrowserDelegate* db, CDataBrowser* browser)
: CView (size)
, db (db)
, browser (browser)
{
	setTransparency (true);
}

//-----------------------------------------------------------------------------------------------
void CDataBrowserHeader::draw (CDrawContext* context)
{
	drawRect (context, getViewSize ());
}

//-----------------------------------------------------------------------------------------------
void CDataBrowserHeader::drawRect (CDrawContext* context, const CRect& updateRect)
{
	CColor lineColor;
	CCoord lineWidth = 0;
	if (browser->getStyle () & CDataBrowser::kDrawRowLines || browser->getStyle () & CDataBrowser::kDrawColumnLines)
	{
		db->dbGetLineWidthAndColor (lineWidth, lineColor, browser);
	}
	CCoord headerHeight = db->dbGetHeaderHeight (browser);
	if (browser->getStyle () & CDataBrowser::kDrawRowLines)
		headerHeight += lineWidth;
	int32_t numColumns = db->dbGetNumColumns (browser);

	CRect r (getViewSize ().left, getViewSize ().top, 0, 0);
	r.setHeight (headerHeight);
	for (int32_t col = 0; col < numColumns; col++)
	{
		CCoord columnWidth = db->dbGetCurrentColumnWidth (col, browser);
		r.setWidth (columnWidth);
		if (browser->getStyle () & CDataBrowser::kDrawColumnLines)
			r.right += lineWidth;
		CRect testRect (r);
		testRect.bound (updateRect);
		if (!testRect.isEmpty ())
		{
			db->dbDrawHeader (context, r, col, 0, browser);
		}
		r.offset (r.getWidth (), 0);
	}
	setDirty (false);
}

//-----------------------------------------------------------------------------------------------
int32_t CDataBrowserHeader::getColumnAtPoint (CPoint& where)
{
	// calculate column at point
	CCoord lineWidth = 0;
	if (browser->getStyle () & CDataBrowser::kDrawRowLines || browser->getStyle () & CDataBrowser::kDrawColumnLines)
	{
		CColor lineColor;
		db->dbGetLineWidthAndColor (lineWidth, lineColor, browser);
	}
	int32_t col = -1;
	int32_t numColumns = db->dbGetNumColumns (browser);
	CRect r (getViewSize ().left, getViewSize ().top, 0, getViewSize ().bottom);
	for (int32_t c = 0; c < numColumns; c++)
	{
		CCoord columnWidth = db->dbGetCurrentColumnWidth (c, browser);
		if (browser->getStyle () & CDataBrowser::kDrawColumnLines)
			columnWidth += lineWidth;
		r.setWidth (columnWidth);
		if (r.pointInside (where))
		{
			if (r.right - where.x < 5)
				col = c;
			break;
		}
		r.offset (columnWidth, 0);
	}
	return col;
}

//-----------------------------------------------------------------------------------------------
CMouseEventResult CDataBrowserHeader::onMouseDown (CPoint &where, const CButtonState& buttons)
{
	mouseColumn = getColumnAtPoint (where);
	CCoord minWidth;
	CCoord maxWidth;
	if (mouseColumn >= 0 && db->dbGetColumnDescription (mouseColumn, minWidth, maxWidth, browser) && minWidth != maxWidth)
	{
		startWidth = db->dbGetCurrentColumnWidth (mouseColumn, browser);
		startMousePoint = where;
		getFrame ()->setCursor (kCursorHSize);
		return onMouseMoved (where, buttons);
	}
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//-----------------------------------------------------------------------------------------------
CMouseEventResult CDataBrowserHeader::onMouseMoved (CPoint &where, const CButtonState& buttons)
{
	if (buttons & kLButton)
	{
		CCoord xOffset = where.x - startMousePoint.x;
		if (xOffset != 0)
		{
			CCoord minWidth;
			CCoord maxWidth;
			if (db->dbGetColumnDescription (mouseColumn, minWidth, maxWidth, browser))
			{
				CCoord columnWidth = db->dbGetCurrentColumnWidth (mouseColumn, browser);
				CCoord newWidth = startWidth + xOffset;
				if (newWidth < minWidth)
					newWidth = minWidth;
				if (newWidth > maxWidth)
					newWidth = maxWidth;
				if (newWidth != columnWidth)
				{
					db->dbSetCurrentColumnWidth (mouseColumn, newWidth, browser);
					browser->recalculateLayout (true);
				}
			}
		}
		return kMouseEventHandled;
	}
	else
	{
		int32_t col = getColumnAtPoint (where);
		CCoord minWidth;
		CCoord maxWidth;
		if (col >= 0 && db->dbGetColumnDescription (col, minWidth, maxWidth, browser) && minWidth != maxWidth)
			getFrame ()->setCursor (kCursorHSize);
		else
			getFrame ()->setCursor (kCursorDefault);
	}
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------------------------
CMouseEventResult CDataBrowserHeader::onMouseExited (CPoint &where, const CButtonState& buttons)
{
	getFrame ()->setCursor (kCursorDefault);
	return kMouseEventHandled;
}

//-----------------------------------------------------------------------------------------------
CMouseEventResult CDataBrowserHeader::onMouseUp (CPoint &where, const CButtonState& buttons)
{
	getFrame ()->setCursor (kCursorDefault);
	return kMouseEventHandled;
}

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
CDataBrowserView::CDataBrowserView (const CRect& size, IDataBrowserDelegate* db, CDataBrowser* browser)
: CView (size)
, db (db)
, browser (browser)
{
	setTransparency (true);
	setWantsFocus (true);
}

//-----------------------------------------------------------------------------------------------
CRect CDataBrowserView::getRowBounds (int32_t row)
{
	CCoord lineWidth = 0;
	if (browser->getStyle () & CDataBrowser::kDrawRowLines || browser->getStyle () & CDataBrowser::kDrawColumnLines)
	{
		CColor lineColor;
		db->dbGetLineWidthAndColor (lineWidth, lineColor, browser);
	}
	CCoord rowHeight = db->dbGetRowHeight (browser);

	CRect where (getViewSize ());
	where.originize ();
	if (browser->getStyle () & CDataBrowser::kDrawRowLines)
		rowHeight+=lineWidth;

	CRect r (getViewSize ().left, getViewSize ().top + rowHeight * row, getViewSize ().right, getViewSize ().top + rowHeight * (row+1));
	return r;
}

//-----------------------------------------------------------------------------------------------
void CDataBrowserView::invalidateRow (int32_t row)
{
	if (row == CDataBrowser::kNoSelection)
		return;

	CRect ia = getRowBounds (row);
	invalidRect (ia);
}

//-----------------------------------------------------------------------------------------------
void CDataBrowserView::draw (CDrawContext* context)
{
	drawRect (context, getViewSize ());
}

//-----------------------------------------------------------------------------------------------
void CDataBrowserView::drawRect (CDrawContext* context, const CRect& updateRect)
{
	const bool drawRowLines = (browser->getStyle () & CDataBrowser::kDrawRowLines) ? true : false;
	CCoord lineWidth = 0;
	CColor lineColor;
	if (browser->getStyle () & CDataBrowser::kDrawRowLines || browser->getStyle () & CDataBrowser::kDrawColumnLines)
	{
		db->dbGetLineWidthAndColor (lineWidth, lineColor, browser);
	}

	CCoord rowHeight = db->dbGetRowHeight (browser);
	if (drawRowLines)
		rowHeight += lineWidth;
	int32_t numRows = db->dbGetNumRows (browser);
	int32_t numColumns = db->dbGetNumColumns (browser);

	const CDataBrowser::Selection& selection = browser->getSelection ();

	CDrawContext::LineList lines;

	CRect r (getViewSize ());
	r.setHeight (rowHeight - lineWidth);
	for (int32_t row = 0; row < numRows; row++)
	{
		CRect testRect (r);
		testRect.bound (updateRect);
		if (testRect.isEmpty () == false)
		{
			bool isSelected = std::find (selection.begin (), selection.end (), row) != selection.end ();
			for (int32_t col = 0; col < numColumns; col++)
			{
				CCoord columnWidth = db->dbGetCurrentColumnWidth (col, browser);
				r.setWidth (columnWidth);
				testRect = r;
				testRect.bound (updateRect);
				if (testRect.isEmpty () == false)
				{
					context->setClipRect (testRect);
					CRect cellSize (r);
					cellSize.bottom++;
					cellSize.right++;
					db->dbDrawCell (context, cellSize, row, col, isSelected ? IDataBrowserDelegate::kRowSelected : 0, browser);
				}
				r.offset (columnWidth, 0);
				if (browser->getStyle () & CDataBrowser::kDrawColumnLines)
					r.offset (lineWidth, 0);
			}
		}
		r.left = getViewSize ().left;
		r.setWidth (getWidth ());
		if (drawRowLines)
			lines.emplace_back (r.getBottomLeft (), r.getBottomRight ());
		r.offset (0, rowHeight);
	}
	if (browser->getStyle () & CDataBrowser::kDrawColumnLines)
	{
		
		CPoint p1;
		CPoint p2;
		if (browser->getStyle () & CDataBrowser::kDrawColumnLines)
		{
			p1 (getViewSize ().left - lineWidth, getViewSize ().top);
			p2 (getViewSize ().left - lineWidth, getViewSize ().bottom);
			for (int32_t col = 0; col < numColumns - 1; col++)
			{
				p1.x = p2.x = p1.x + db->dbGetCurrentColumnWidth (col, browser) + lineWidth;
				lines.emplace_back (p1, p2);
			}
		}
	}
	if (lines.size ())
	{
		context->setClipRect (updateRect);
		context->setDrawMode (kAntiAliasing);
		context->setLineWidth (lineWidth);
		context->setFrameColor (lineColor);
		context->setLineStyle (kLineSolid);
		context->drawLines (lines);
	}
	setDirty (false);
}

//-----------------------------------------------------------------------------------------------
bool CDataBrowserView::getCell (const CPoint& where, CDataBrowser::Cell& cell)
{
	CCoord lineWidth = 0;
	if (browser->getStyle () & CDataBrowser::kDrawRowLines || browser->getStyle () & CDataBrowser::kDrawColumnLines)
	{
		CColor lineColor;
		db->dbGetLineWidthAndColor (lineWidth, lineColor, browser);
	}
	CCoord rowHeight = db->dbGetRowHeight (browser);
	int32_t numColumns = db->dbGetNumColumns (browser);

	CPoint _where (where);
	_where.offset (-getViewSize ().left, -getViewSize ().top);
	if (browser->getStyle () & CDataBrowser::kDrawRowLines)
		rowHeight += lineWidth;
	int32_t rowNum = (int32_t)(_where.y / rowHeight);
	int32_t colNum = 0;
	CCoord cw = 0;
	for (int32_t i = 0; i < numColumns; i++)
	{
		cw += db->dbGetCurrentColumnWidth (i, browser);
		if (browser->getStyle () & CDataBrowser::kDrawColumnLines)
			cw += lineWidth;
		if (_where.x < cw && rowNum < db->dbGetNumRows (browser) && colNum < db->dbGetNumColumns (browser))
		{
			colNum = i;
			cell.row = rowNum;
			cell.column = colNum;
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------------------------
CMouseEventResult CDataBrowserView::onMouseDown (CPoint &where, const CButtonState& buttons)
{
	getFrame ()->setFocusView (this);
	CDataBrowser::Cell cell;
	if (getCell (where, cell))
	{
		const CDataBrowser::Selection& selection = browser->getSelection ();
		bool alreadySelected = std::find (selection.begin (), selection.end (), cell.row) != selection.end ();
		if (browser->getStyle () & CDataBrowser::kMultiSelectionStyle)
		{
			if (buttons.getModifierState () == kControl)
			{
				if (alreadySelected)
					browser->unselectRow (cell.row);
				else
					browser->selectRow (cell.row);
			}
			else if (buttons.getModifierState () == kShift)
			{
				int32_t lastSelectedRow = selection.size () > 0 ? selection.back () : -1;
				if (lastSelectedRow < cell.row)
				{
					for (int32_t i = lastSelectedRow; i <= cell.row; i++)
					{
						browser->selectRow (i);
					}
				}
				else if (lastSelectedRow > cell.row)
				{
					for (int32_t i = cell.row; i < lastSelectedRow; i++)
					{
						browser->selectRow (i);
					}
				}
			}
			else
			{
				browser->setSelectedRow (cell.row);
			}
		}
		else if (!alreadySelected)
		{
			invalidateRow (cell.row);
			invalidateRow (browser->getSelectedRow ());
			browser->setSelectedRow (cell.row);
		}
		return db->dbOnMouseDown (where, buttons, cell.row, cell.column, browser);
	}
	return kMouseEventHandled;
}

//-----------------------------------------------------------------------------------------------
CMouseEventResult CDataBrowserView::onMouseMoved (CPoint &where, const CButtonState& buttons)
{
	CDataBrowser::Cell cell;
	if (getCell (where, cell))
	{
		return db->dbOnMouseMoved (where, buttons, cell.row, cell.column, browser);
	}
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------------------------
CMouseEventResult CDataBrowserView::onMouseUp (CPoint &where, const CButtonState& buttons)
{
	CDataBrowser::Cell cell;
	if (getCell (where, cell))
	{
		return db->dbOnMouseUp (where, buttons, cell.row, cell.column, browser);
	}
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------------------------
CMouseEventResult CDataBrowserView::onMouseExited (CPoint &where, const CButtonState& buttons)
{
	return db->dbOnMouseMoved (where, buttons, -1, -1, browser);
}

//-----------------------------------------------------------------------------------------------
bool CDataBrowserView::onDrop (IDataPackage* drag, const CPoint& where)
{
	CPoint cellPoint (where);
	CDataBrowser::Cell cell;
	if (getCell (where, cell))
	{
		CRect r = browser->getCellBounds (cell);
		cellPoint.x -= r.left;
		cellPoint.y -= r.top;
	}
	return db->dbOnDropInCell (cell.row, cell.column, cellPoint, drag, browser);
}

//-----------------------------------------------------------------------------------------------
static const CViewAttributeID kDataBrowserViewDragRow = 'vddr';
static const CViewAttributeID kDataBrowserViewDragColumn = 'vddc';

//-----------------------------------------------------------------------------------------------
void CDataBrowserView::onDragEnter (IDataPackage* drag, const CPoint& where)
{
	db->dbOnDragEnterBrowser (drag, browser);
	CDataBrowser::Cell cell;
	if (getCell (where, cell))
	{
		CRect r = browser->getCellBounds (cell);
		CPoint cellPoint (where);
		cellPoint.x -= r.left;
		cellPoint.y -= r.top;
		db->dbOnDragEnterCell (cell.row, cell.column, cellPoint, drag, browser);
		setAttribute (kDataBrowserViewDragRow, sizeof (int32_t), &cell.row);
		setAttribute (kDataBrowserViewDragColumn, sizeof (int32_t), &cell.column);
	}
}

//-----------------------------------------------------------------------------------------------
void CDataBrowserView::onDragLeave (IDataPackage* drag, const CPoint& where)
{
	uint32_t size;
	int32_t oldRowNum = -1;
	int32_t oldColNum = -1;
	getAttribute (kDataBrowserViewDragRow, sizeof (int32_t), &oldRowNum, size);
	getAttribute (kDataBrowserViewDragColumn, sizeof (int32_t), &oldColNum, size);
	if (oldRowNum != -1 && oldColNum != -1)
	{
		db->dbOnDragExitCell (oldRowNum, oldColNum, drag, browser);
		removeAttribute (kDataBrowserViewDragRow);
		removeAttribute (kDataBrowserViewDragColumn);
	}
	db->dbOnDragExitBrowser (drag, browser);
}

//-----------------------------------------------------------------------------------------------
void CDataBrowserView::onDragMove (IDataPackage* drag, const CPoint& where)
{
	uint32_t size;
	int32_t oldRowNum = -1;
	int32_t oldColNum = -1;
	getAttribute (kDataBrowserViewDragRow, sizeof (int32_t), &oldRowNum, size);
	getAttribute (kDataBrowserViewDragColumn, sizeof (int32_t), &oldColNum, size);
	CDataBrowser::Cell cell;
	if (getCell (where, cell))
	{
		CRect r = browser->getCellBounds (cell);
		CPoint cellPoint (where);
		cellPoint.x -= r.left;
		cellPoint.y -= r.top;
		if (oldRowNum != cell.row || oldColNum != cell.column)
		{
			if (oldRowNum != -1 && oldColNum != -1)
				db->dbOnDragExitCell (oldRowNum, oldColNum, drag, browser);
			db->dbOnDragEnterCell (cell.row, cell.column, cellPoint, drag, browser);
			setAttribute (kDataBrowserViewDragRow, sizeof (int32_t), &cell.row);
			setAttribute (kDataBrowserViewDragColumn, sizeof (int32_t), &cell.column);
		}
		else
		{
			db->dbOnDragMoveInCell (cell.row, cell.column, cellPoint, drag, browser);
		}
	}
	else if (oldRowNum != -1 && oldColNum != -1)
	{
		db->dbOnDragExitCell (oldRowNum, oldColNum, drag, browser);
		removeAttribute (kDataBrowserViewDragRow);
		removeAttribute (kDataBrowserViewDragColumn);
	}
}

//-----------------------------------------------------------------------------------------------
int32_t CDataBrowserView::onKeyDown (VstKeyCode& keyCode)
{
	int32_t res = db->dbOnKeyDown (keyCode, browser);
	if (res != -1)
		return res;
	if (keyCode.modifier != 0)
		return -1;
	if (keyCode.virt == VKEY_UP || keyCode.virt == VKEY_DOWN || keyCode.virt == VKEY_PAGEUP || keyCode.virt == VKEY_PAGEDOWN)
	{
		int32_t numRows = db->dbGetNumRows (browser);
		int32_t selRow = browser->getSelectedRow ();
		int32_t changeRow = 0;
		if (keyCode.virt == VKEY_UP)
			changeRow = -1;
		else if (keyCode.virt == VKEY_DOWN)
			changeRow = 1;
		else if (keyCode.virt == VKEY_PAGEUP)
			changeRow = (int32_t) (-browser->getHeight () / db->dbGetRowHeight (browser));
		else if (keyCode.virt == VKEY_PAGEDOWN)
			changeRow = (int32_t) (browser->getHeight () / db->dbGetRowHeight (browser));
		int32_t newSelRow = selRow + changeRow;
		newSelRow = std::min<int32_t> (newSelRow, numRows);
		newSelRow = std::max<int32_t> (newSelRow, 0);
		if (newSelRow != selRow)
		{
			invalidateRow (selRow);
			invalidateRow (newSelRow);
			browser->setSelectedRow (newSelRow, true);
			CRect rect = getRowBounds (newSelRow);
			browser->makeRectVisible (rect);
		}
		return 1;
	}
	return -1;
}

//-----------------------------------------------------------------------------------------------
bool CDataBrowserView::drawFocusOnTop ()
{
	return true;
}

//-----------------------------------------------------------------------------------------------
bool CDataBrowserView::getFocusPath (CGraphicsPath& outPath)
{
	CRect r = getVisibleViewSize ();
	outPath.addRect (r);
	CCoord focusWidth = getFrame ()->getFocusWidth ();
	r.inset (focusWidth, focusWidth);
	outPath.addRect (r);
	return true;
}

/// @endcond

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
GenericStringListDataBrowserSource::GenericStringListDataBrowserSource (const StringVector* stringList, IGenericStringListDataBrowserSourceSelectionChanged* delegate)
: stringList (stringList)
, rowHeight (-1)
, fontColor (kWhiteCColor)
, selectionColor (kBlueCColor)
, rowlineColor (kGreyCColor)
, rowBackColor (kTransparentCColor)
, rowAlternateBackColor (kTransparentCColor)
, textInset (2., 0.)
, textAlignment (kLeftText)
, drawFont (kSystemFont)
, dataBrowser (nullptr)
, delegate (delegate)
{
}

//-----------------------------------------------------------------------------
GenericStringListDataBrowserSource::~GenericStringListDataBrowserSource () noexcept = default;

//-----------------------------------------------------------------------------
void GenericStringListDataBrowserSource::dbAttached (CDataBrowser* browser)
{
	dataBrowser = browser;
}

//-----------------------------------------------------------------------------
void GenericStringListDataBrowserSource::dbRemoved (CDataBrowser* browser)
{
	dataBrowser = nullptr;
}

//-----------------------------------------------------------------------------
void GenericStringListDataBrowserSource::setStringList (const StringVector* stringList)
{
	this->stringList = stringList;
	if (dataBrowser)
		dataBrowser->recalculateLayout (true);
}

//-----------------------------------------------------------------------------
void GenericStringListDataBrowserSource::setupUI (const CColor& _selectionColor, const CColor& _fontColor, const CColor& _rowlineColor, const CColor& _rowBackColor, const CColor& _rowAlternateBackColor, CFontRef _font, int32_t _rowHeight, CCoord _textInset)
{
	if (_font)
		drawFont = _font;
	textInset = CPoint (_textInset, 0);
	rowHeight = _rowHeight;
	selectionColor = _selectionColor;
	fontColor = _fontColor;
	rowlineColor = _rowlineColor;
	rowBackColor = _rowBackColor;
	rowAlternateBackColor = _rowAlternateBackColor;
	if (dataBrowser)
		dataBrowser->recalculateLayout (true);
}

//-----------------------------------------------------------------------------
void GenericStringListDataBrowserSource::dbSelectionChanged (CDataBrowser* browser)
{
	if (delegate)
		delegate->dbSelectionChanged (browser->getSelectedRow (), this);
}

//-----------------------------------------------------------------------------
int32_t GenericStringListDataBrowserSource::dbGetNumRows (CDataBrowser* browser)
{
	return stringList ? (int32_t)stringList->size () : 0;
}

//-----------------------------------------------------------------------------
CCoord GenericStringListDataBrowserSource::dbGetCurrentColumnWidth (int32_t index, CDataBrowser* browser)
{
	return browser->getWidth () - ((browser->getStyle () & CScrollView::kOverlayScrollbars || (browser->getActiveScrollbars () & CScrollView::kVerticalScrollbar) == 0) ? 0 : browser->getScrollbarWidth ());
}

//-----------------------------------------------------------------------------
bool GenericStringListDataBrowserSource::dbGetLineWidthAndColor (CCoord& width, CColor& color, CDataBrowser* browser)
{
	width = 1.;
	color = rowlineColor;
	return true;
}

//-----------------------------------------------------------------------------
CCoord GenericStringListDataBrowserSource::dbGetRowHeight (CDataBrowser* browser)
{
	if (rowHeight < 0)
	{
		if (drawFont->getPlatformFont ())
		{
			CCoord height = drawFont->getPlatformFont ()->getAscent ();
			height += drawFont->getPlatformFont ()->getDescent ();
			height += drawFont->getPlatformFont ()->getLeading ();
			return std::floor (height + 2.5);
		}
		return drawFont->getSize () + 2.;
	}
	return rowHeight;
}

//-----------------------------------------------------------------------------
void GenericStringListDataBrowserSource::dbDrawHeader (CDrawContext* context, const CRect& size, int32_t column, int32_t flags, CDataBrowser* browser)
{
}

//-----------------------------------------------------------------------------
void GenericStringListDataBrowserSource::drawRowBackground (CDrawContext* context, const CRect& size, int32_t row, int32_t flags, CDataBrowser* browser) const
{
	vstgui_assert (row >= 0 && static_cast<size_t> (row) < stringList->size ());

	context->setDrawMode (kAliasing);
	context->setLineWidth (1.);
	context->setFillColor ((row % 2) ? rowBackColor : rowAlternateBackColor);
	context->drawRect (size, kDrawFilled);
	if (flags & kRowSelected)
	{
		CColor color (selectionColor);
		CView* focusView = browser->getFrame ()->getFocusView ();
		if (!(focusView && browser->isChild (focusView, true)))
		{
			double hue, saturation, value;
			color.toHSV (hue, saturation, value);
			if (saturation > 0.)
			{
				saturation *= 0.5;
				color.fromHSV (hue, saturation, value);
			}
			else
				color.alpha /= 2;
		}
		context->setFillColor (color);
		context->drawRect (size, kDrawFilled);
	}
}

//-----------------------------------------------------------------------------
void GenericStringListDataBrowserSource::drawRowString (CDrawContext* context, const CRect& size, int32_t row, int32_t flags, CDataBrowser* browser) const
{
	vstgui_assert (row >= 0 && static_cast<size_t> (row) < stringList->size ());
	
	context->saveGlobalState ();
	CRect stringSize (size);
	stringSize.inset (textInset.x, textInset.y);
	context->setFont (drawFont);
	context->setFontColor (fontColor);
	ConcatClip cc (*context, stringSize);
	context->drawString ((*stringList)[static_cast<size_t> (row)].getPlatformString (), stringSize, textAlignment);
	context->restoreGlobalState ();
}

//-----------------------------------------------------------------------------
void GenericStringListDataBrowserSource::dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser)
{
	vstgui_assert (row >= 0 && static_cast<size_t> (row) < stringList->size ());
	vstgui_assert (column == 0);

	drawRowBackground (context, size, row, flags, browser);
	drawRowString (context, size, row, flags, browser);
}

//-----------------------------------------------------------------------------
int32_t GenericStringListDataBrowserSource::dbOnKeyDown (const VstKeyCode& _key, CDataBrowser* browser)
{
	VstKeyCode key = _key;
	if (key.virt == VKEY_SPACE)
	{
		key.virt = 0;
		key.character = 0x20;
	}
	if (dataBrowser && key.virt == 0 && key.modifier == 0)
	{
		if (timer == nullptr)
		{
			timer = makeOwned<CVSTGUITimer> (this, 1000);
			timer->start ();
		}
		else
		{
			timer->stop ();
			timer->start ();
		}
		keyDownFindString += static_cast<char> (toupper (key.character));
		StringVector::const_iterator it = stringList->begin ();
		int32_t row = 0;
		while (it != stringList->end ())
		{
			std::string str ((*it).getString (), 0, keyDownFindString.length ());
			std::transform (str.begin (), str.end (), str.begin (), ::toupper);
			if (str == keyDownFindString)
			{
				dataBrowser->setSelectedRow (row, true);
				return 1;
			}
			row++;
			++it;
		}
	}
	return -1;
}

//-----------------------------------------------------------------------------
CMouseEventResult GenericStringListDataBrowserSource::dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser)
{
	if (delegate && buttons.isDoubleClick ())
		delegate->dbRowDoubleClick (row, this);
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//-----------------------------------------------------------------------------
CMessageResult GenericStringListDataBrowserSource::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == CVSTGUITimer::kMsgTimer)
	{
		keyDownFindString = "";
		timer = nullptr;
		return kMessageNotified;
	}
	return kMessageUnknown;
}

} // namespace

