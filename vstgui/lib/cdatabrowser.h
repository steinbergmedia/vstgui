// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __cdatabrowser__
#define __cdatabrowser__

#include "vstguifwd.h"
#include "cscrollview.h"
#include "cfont.h"
#include "ccolor.h"
#include "cstring.h"
#include <vector>

namespace VSTGUI {

//-----------------------------------------------------------------------------
// IDataBrowserDelegate Declaration
//! @brief DataBrowser Interface
//-----------------------------------------------------------------------------------------------
class IDataBrowserDelegate
{
public:
	/** @name Setup */
	///	@{
	virtual int32_t dbGetNumRows (CDataBrowser* browser) = 0;	///< return number of rows for CDataBrowser browser
	virtual int32_t dbGetNumColumns (CDataBrowser* browser) = 0;	///< return number of columns for CDataBrowser browser
	virtual bool dbGetColumnDescription (int32_t index, CCoord& minWidth, CCoord& maxWidth, CDataBrowser* browser) { return false; }
	virtual CCoord dbGetCurrentColumnWidth (int32_t index, CDataBrowser* browser) = 0;	///< return current width of index column
	virtual void dbSetCurrentColumnWidth (int32_t index, const CCoord& width, CDataBrowser* browser) {}	///< the width of a column has changed
	virtual CCoord dbGetRowHeight (CDataBrowser* browser) = 0;	///< return height of one row
	virtual CCoord dbGetHeaderHeight (CDataBrowser* browser) { return dbGetRowHeight (browser); } ///< return height of header
	virtual bool dbGetLineWidthAndColor (CCoord& width, CColor& color, CDataBrowser* browser) { return false; } ///< return the line width and color
	virtual void dbAttached (CDataBrowser* browser) {}	///< databrowser view was attached to a parent
	virtual void dbRemoved (CDataBrowser* browser) {}		///< databrowser view will be removed from its parent
	///	@}

	/** @name Drawing */
	///	@{
	virtual void dbDrawHeader (CDrawContext* context, const CRect& size, int32_t column, int32_t flags, CDataBrowser* browser) = 0;	///< draw the db header
	virtual void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser) = 0;	///< draw a db cell
	///	@}

	/** @name Mouse Handling */
	///	@{
	virtual CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) { return kMouseDownEventHandledButDontNeedMovedOrUpEvents; } ///< mouse button was pressed on a cell
	virtual CMouseEventResult dbOnMouseMoved (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) { return kMouseEventNotHandled; } ///< mouse was moved over a cell
	virtual CMouseEventResult dbOnMouseUp (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) { return kMouseEventNotHandled; } ///< mouse button was released on a cell
	///	@}

	/** @name Drag'n Drop Handling */
	///	@{
	virtual void dbOnDragEnterBrowser (IDataPackage* drag, CDataBrowser* browser) {}
	virtual void dbOnDragExitBrowser (IDataPackage* drag, CDataBrowser* browser) {}
	virtual void dbOnDragEnterCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser) {}
	virtual void dbOnDragMoveInCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser) {}
	virtual void dbOnDragExitCell (int32_t row, int32_t column, IDataPackage* drag, CDataBrowser* browser) {}
	virtual bool dbOnDropInCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser) { return false; }
	///	@}

	/** @name Selection  */
	///	@{
	virtual void dbSelectionChanged (CDataBrowser* browser) {}	///< the selection of the db changed
	///	@}

	/** @name Cell Text Editing */
	///	@{
	virtual void dbCellTextChanged (int32_t row, int32_t column, UTF8StringPtr newText, CDataBrowser* browser) {} ///< the text of the cell changed beginTextEdit was called for
	virtual void dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* textEditControl, CDataBrowser* browser) {} ///< beginTextEdit calls this, so you can setup the textedit control
	//@}

	/** @name Keyboard Handling */
	///	@{
	virtual int32_t dbOnKeyDown (const VstKeyCode& key, CDataBrowser* browser) { return -1; }
	///	@}

	enum {
		kRowSelected = 1 << 1
	};
};

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
	virtual void recalculateLayout (bool rememberSelection = false);						///< trigger recalculation, call if numRows or numColumns changed
	virtual void invalidate (const Cell& cell);												///< invalidates an individual cell
	virtual void invalidateRow (int32_t row);												///< invalidates a complete row
	virtual void makeRowVisible (int32_t row);												///< scrolls the scrollview so that row is visible

	virtual CRect getCellBounds (const Cell& cell);											///< get bounds of a cell
	virtual Cell getCellAt (const CPoint& where) const;										///< get the cell at position where

	virtual int32_t getSelectedRow () const;												///< get first selected row
	virtual void setSelectedRow (int32_t row, bool makeVisible = false);					///< set the exclusive selected row

	const Selection& getSelection () const { return selection; }							///< get all selected rows
	virtual void selectRow (int32_t row);													///< add row to selection
	virtual void unselectRow (int32_t row);													///< remove row from selection
	virtual void unselectAll ();															///< empty selection

	virtual void beginTextEdit (const Cell& cell, UTF8StringPtr initialText);				///< starts a text edit for a cell

	IDataBrowserDelegate* getDelegate () const { return db; }								///< get delegate object
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
class IGenericStringListDataBrowserSourceSelectionChanged
{
public:
	virtual void dbSelectionChanged (int32_t selectedRow, GenericStringListDataBrowserSource* source) = 0;
	virtual void dbRowDoubleClick (int32_t row, GenericStringListDataBrowserSource* source) {};
};

//-----------------------------------------------------------------------------
// GenericStringListDataBrowserSource Declaration
//! @brief Generic string list data browser source
//-----------------------------------------------------------------------------------------------
class GenericStringListDataBrowserSource : public IDataBrowserDelegate, public CBaseObject
{
public:
	using StringVector = std::vector<UTF8String>;

	GenericStringListDataBrowserSource (const StringVector* stringList, IGenericStringListDataBrowserSourceSelectionChanged* delegate = nullptr);
	~GenericStringListDataBrowserSource () noexcept override;

	void setStringList (const StringVector* stringList);
	const StringVector* getStringList () const { return stringList; }

	void setupUI (const CColor& selectionColor, const CColor& fontColor, const CColor& rowlineColor, const CColor& rowBackColor, const CColor& rowAlteranteBackColor, CFontRef font = nullptr, int32_t rowHeight = -1, CCoord textInset = 2.);

protected:
	void drawRowBackground (CDrawContext* context, const CRect& size, int32_t row, int32_t flags, CDataBrowser* browser) const;
	void drawRowString (CDrawContext* context, const CRect& size, int32_t row, int32_t flags, CDataBrowser* browser) const;

	int32_t dbGetNumRows (CDataBrowser* browser) override;
	int32_t dbGetNumColumns (CDataBrowser* browser) override { return 1; }
	bool dbGetColumnDescription (int32_t index, CCoord& minWidth, CCoord& maxWidth, CDataBrowser* browser)  override{ return false; }
	CCoord dbGetCurrentColumnWidth (int32_t index, CDataBrowser* browser) override;
	void dbSetCurrentColumnWidth (int32_t index, const CCoord& width, CDataBrowser* browser) override {}
	CCoord dbGetRowHeight (CDataBrowser* browser) override;
	bool dbGetLineWidthAndColor (CCoord& width, CColor& color, CDataBrowser* browser) override;

	void dbDrawHeader (CDrawContext* context, const CRect& size, int32_t column, int32_t flags, CDataBrowser* browser) override;
	void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser) override;

	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) override;
	CMouseEventResult dbOnMouseMoved (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) override { return kMouseEventNotHandled; }
	CMouseEventResult dbOnMouseUp (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) override { return kMouseEventNotHandled; }

	void dbSelectionChanged (CDataBrowser* browser) override;

	void dbCellTextChanged (int32_t row, int32_t column, UTF8StringPtr newText, CDataBrowser* browser) override {}
	void dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* textEditControl, CDataBrowser* browser) override {}

	int32_t dbOnKeyDown (const VstKeyCode& key, CDataBrowser* browser) override;

	void dbAttached (CDataBrowser* browser) override;
	void dbRemoved (CDataBrowser* browser) override;

	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;

	const StringVector* stringList;
	int32_t rowHeight;
	CColor fontColor;
	CColor selectionColor;
	CColor rowlineColor;
	CColor rowBackColor;
	CColor rowAlternateBackColor;
	CPoint textInset;
	CHoriTxtAlign textAlignment;
	SharedPointer<CFontDesc> drawFont;
	CDataBrowser* dataBrowser;
	IGenericStringListDataBrowserSourceSelectionChanged* delegate;

	SharedPointer<CVSTGUITimer> timer;
	std::string keyDownFindString;
};

} // namespace

#endif
