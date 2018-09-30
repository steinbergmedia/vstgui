// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "dragging.h"
#include "vstkeycode.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
// IDataBrowserDelegate Declaration
//! @brief DataBrowser Interface
//-----------------------------------------------------------------------------------------------
class IDataBrowserDelegate
{
public:
	enum Flags
	{
		kRowSelected = 1 << 1
	};
	
	/** @name Setup */
	///	@{
	/** return number of rows for CDataBrowser browser */
	virtual int32_t dbGetNumRows (CDataBrowser* browser) = 0;
	/** return number of columns for CDataBrowser browser */
	virtual int32_t dbGetNumColumns (CDataBrowser* browser) = 0;
	/** */
	virtual bool dbGetColumnDescription (int32_t index, CCoord& minWidth, CCoord& maxWidth,
	                                     CDataBrowser* browser) = 0;
	/** return current width of index column */
	virtual CCoord dbGetCurrentColumnWidth (int32_t index, CDataBrowser* browser) = 0;
	/** the width of a column has changed */
	virtual void dbSetCurrentColumnWidth (int32_t index, const CCoord& width,
	                                      CDataBrowser* browser) = 0;
	/** return height of one row */
	virtual CCoord dbGetRowHeight (CDataBrowser* browser) = 0;
	/** return height of header */
	virtual CCoord dbGetHeaderHeight (CDataBrowser* browser) = 0;
	/** return the line width and color */
	virtual bool dbGetLineWidthAndColor (CCoord& width, CColor& color, CDataBrowser* browser) = 0;
	/** databrowser view was attached to a parent */
	virtual void dbAttached (CDataBrowser* browser) = 0;
	/** databrowser view will be removed from its parent */
	virtual void dbRemoved (CDataBrowser* browser) = 0;
	///	@}

	/** @name Drawing */
	///	@{
	/** draw the db header */
	virtual void dbDrawHeader (CDrawContext* context, const CRect& size, int32_t column,
	                           int32_t flags, CDataBrowser* browser) = 0;
	/** draw a db cell */
	virtual void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column,
	                         int32_t flags, CDataBrowser* browser) = 0;
	///	@}

	/** @name Mouse Handling */
	///	@{
	/** mouse button was pressed on a cell */
	virtual CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons,
	                                         int32_t row, int32_t column,
	                                         CDataBrowser* browser) = 0;
	/** mouse was moved over a cell */
	virtual CMouseEventResult dbOnMouseMoved (const CPoint& where, const CButtonState& buttons,
	                                          int32_t row, int32_t column,
	                                          CDataBrowser* browser) = 0;
	/** mouse button was released on a cell */
	virtual CMouseEventResult dbOnMouseUp (const CPoint& where, const CButtonState& buttons,
	                                       int32_t row, int32_t column, CDataBrowser* browser) = 0;
	///	@}

	/** @name Drag'n Drop Handling */
	///	@{
	virtual void dbOnDragEnterBrowser (IDataPackage* drag, CDataBrowser* browser) = 0;
	virtual void dbOnDragExitBrowser (IDataPackage* drag, CDataBrowser* browser) = 0;
	virtual DragOperation dbOnDragEnterCell (int32_t row, int32_t column, const CPoint& where,
	                                         IDataPackage* drag, CDataBrowser* browser) = 0;
	virtual DragOperation dbOnDragMoveInCell (int32_t row, int32_t column, const CPoint& where,
	                                          IDataPackage* drag, CDataBrowser* browser) = 0;
	virtual void dbOnDragExitCell (int32_t row, int32_t column, IDataPackage* drag,
	                               CDataBrowser* browser) = 0;
	virtual bool dbOnDropInCell (int32_t row, int32_t column, const CPoint& where,
	                             IDataPackage* drag, CDataBrowser* browser) = 0;
	///	@}

	/** @name Selection  */
	///	@{
	/** the selection of the db changed */
	virtual void dbSelectionChanged (CDataBrowser* browser) = 0;
	///	@}

	/** @name Cell Text Editing */
	///	@{
	/** the text of the cell changed beginTextEdit was called for */
	virtual void dbCellTextChanged (int32_t row, int32_t column, UTF8StringPtr newText,
	                                CDataBrowser* browser) = 0;
	/** beginTextEdit calls this, so you can setup the textedit control */
	virtual void dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* textEditControl,
	                                  CDataBrowser* browser) = 0;
	//@}

	/** @name Keyboard Handling */
	///	@{
	virtual int32_t dbOnKeyDown (const VstKeyCode& key, CDataBrowser* browser) = 0;
	///	@}

	virtual ~IDataBrowserDelegate () noexcept = default;
};

//-----------------------------------------------------------------------------
// IDataBrowserDelegateAdapter
//-----------------------------------------------------------------------------------------------
class DataBrowserDelegateAdapter : public IDataBrowserDelegate
{
public:
	int32_t dbGetNumRows (CDataBrowser* browser) override = 0;
	int32_t dbGetNumColumns (CDataBrowser* browser) override = 0;
	CCoord dbGetRowHeight (CDataBrowser* browser) override = 0;
	CCoord dbGetCurrentColumnWidth (int32_t index, CDataBrowser* browser) override = 0;
	void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column,
	                 int32_t flags, CDataBrowser* browser) override = 0;

	bool dbGetColumnDescription (int32_t index, CCoord& minWidth, CCoord& maxWidth,
	                             CDataBrowser* browser) override
	{
		return false;
	}
	void dbSetCurrentColumnWidth (int32_t index, const CCoord& width,
	                              CDataBrowser* browser) override
	{
	}
	CCoord dbGetHeaderHeight (CDataBrowser* browser) override { return dbGetRowHeight (browser); }
	bool dbGetLineWidthAndColor (CCoord& width, CColor& color, CDataBrowser* browser) override
	{
		return false;
	}
	void dbDrawHeader (CDrawContext* context, const CRect& size, int32_t column, int32_t flags,
	                   CDataBrowser* browser) override {}
	void dbAttached (CDataBrowser* browser) override {}
	void dbRemoved (CDataBrowser* browser) override {}
	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row,
	                                 int32_t column, CDataBrowser* browser) override
	{
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}
	CMouseEventResult dbOnMouseMoved (const CPoint& where, const CButtonState& buttons, int32_t row,
	                                  int32_t column, CDataBrowser* browser) override
	{
		return kMouseEventNotHandled;
	}
	CMouseEventResult dbOnMouseUp (const CPoint& where, const CButtonState& buttons, int32_t row,
	                               int32_t column, CDataBrowser* browser) override
	{
		return kMouseEventNotHandled;
	}
	void dbOnDragEnterBrowser (IDataPackage* drag, CDataBrowser* browser) override {}
	void dbOnDragExitBrowser (IDataPackage* drag, CDataBrowser* browser) override {}
	DragOperation dbOnDragEnterCell (int32_t row, int32_t column, const CPoint& where,
	                                 IDataPackage* drag, CDataBrowser* browser) override
	{
		return DragOperation::None;
	}
	DragOperation dbOnDragMoveInCell (int32_t row, int32_t column, const CPoint& where,
	                                  IDataPackage* drag, CDataBrowser* browser) override
	{
		return DragOperation::None;
	}
	void dbOnDragExitCell (int32_t row, int32_t column, IDataPackage* drag,
	                       CDataBrowser* browser) override
	{
	}
	bool dbOnDropInCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag,
	                     CDataBrowser* browser) override
	{
		return false;
	}
	void dbSelectionChanged (CDataBrowser* browser) override {}
	void dbCellTextChanged (int32_t row, int32_t column, UTF8StringPtr newText,
	                        CDataBrowser* browser) override
	{
	}
	void dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* textEditControl,
	                          CDataBrowser* browser) override
	{
	}
	int32_t dbOnKeyDown (const VstKeyCode& key, CDataBrowser* browser) override { return -1; }
};

//------------------------------------------------------------------------
} // VSTGUI
