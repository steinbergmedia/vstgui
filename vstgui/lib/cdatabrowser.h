// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"
#include "cscrollview.h"
#include "cfont.h"
#include "ccolor.h"
#include "cstring.h"
#include <vector>

namespace VSTGUI {

// forward private internal views
class CDataBrowserView;
class CDataBrowserHeader;

//-----------------------------------------------------------------------------
// CDataBrowser Declaration
//! @brief DataBrowser view
/// @ingroup controls
//-----------------------------------------------------------------------------------------------
class CDataBrowser : public CScrollView
{
protected:
	enum
	{
		kDrawRowLinesFlag = kLastScrollViewStyleFlag,
		kDrawColumnLinesFlag,
		kDrawHeaderFlag,
		kMultiSelectionStyleFlag
	};
	
public:
	CDataBrowser (const CRect& size, IDataBrowserDelegate* db, int32_t style = 0, CCoord scrollbarWidth = 16, CBitmap* pBackground = nullptr);

	enum CDataBrowserStyle 
	{
		// see CScrollView for more styles
		kDrawRowLines			= 1 << kDrawRowLinesFlag,
		kDrawColumnLines		= 1 << kDrawColumnLinesFlag,
		kDrawHeader				= 1 << kDrawHeaderFlag,
		kMultiSelectionStyle	= 1 << kMultiSelectionStyleFlag
	};

	enum
	{
		kNoSelection	= -1
	};

	/// @brief CDataBrowser Cell position description
	struct Cell {
		int32_t row {-1};
		int32_t column {-1};
		
		Cell () = default;
		Cell (int32_t row, int32_t column) : row (row), column (column) {}
		bool isValid () const { return row > -1 && column > -1; }
	};

	using Selection = std::vector<int32_t>;

	//-----------------------------------------------------------------------------
	/// @name CDataBrowser Methods
	//-----------------------------------------------------------------------------
	//@{
	/** trigger recalculation, call if numRows or numColumns changed */
	virtual void recalculateLayout (bool rememberSelection = false);
	/** invalidates an individual cell */
	virtual void invalidate (const Cell& cell);
	/** invalidates a complete row */
	virtual void invalidateRow (int32_t row);
	/** scrolls the scrollview so that row is visible */
	virtual void makeRowVisible (int32_t row);

	/** get bounds of a cell */
	virtual CRect getCellBounds (const Cell& cell);
	/** get the cell at position where */
	virtual Cell getCellAt (const CPoint& where) const;

	/** get first selected row */
	virtual int32_t getSelectedRow () const;
	/** set the exclusive selected row */
	virtual void setSelectedRow (int32_t row, bool makeVisible = false);

	/** get all selected rows */
	const Selection& getSelection () const { return selection; }
	/** add row to selection */
	virtual void selectRow (int32_t row);
	/** remove row from selection */
	virtual void unselectRow (int32_t row);
	/** empty selection */
	virtual void unselectAll ();

	/** starts a text edit for a cell */
	virtual void beginTextEdit (const Cell& cell, UTF8StringPtr initialText);

	/** get delegate object */
	IDataBrowserDelegate* getDelegate () const { return db; }
	//@}

	void setAutosizeFlags (int32_t flags) override;
	void setViewSize (const CRect& size, bool invalid) override;
	void setWantsFocus (bool state) override;

	int32_t onKeyDown (VstKeyCode& keyCode) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
protected:
	~CDataBrowser () noexcept override;
	void valueChanged (CControl *pControl) override;
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;
	bool attached (CView *parent) override;
	bool removed (CView* parent) override;
	bool wantsFocus () const override;

	void recalculateSubViews () override;
	void validateSelection ();

	IDataBrowserDelegate* db;
	CDataBrowserView* dbView;
	CDataBrowserHeader* dbHeader;
	CViewContainer* dbHeaderContainer;
	Selection selection;
};

//-----------------------------------------------------------------------------
} // VSTGUI
