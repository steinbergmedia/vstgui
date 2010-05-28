//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
// CDataBrowser written 2006 by Arne Scheffler
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#ifndef __cdatabrowser__
#define __cdatabrowser__

#include "cscrollview.h"

namespace VSTGUI {

class CTextEdit;
class CDataBrowser;
class CDataBrowserView;
class CDataBrowserHeader;

//-----------------------------------------------------------------------------
// IDataBrowser Declaration
//! @brief DataBrowser Interface
//-----------------------------------------------------------------------------------------------
class IDataBrowser
{
public:
	virtual int32_t dbGetNumRows (CDataBrowser* browser) = 0;	///< return number of rows for CDataBrowser browser
	virtual int32_t dbGetNumColumns (CDataBrowser* browser) = 0;	///< return number of columns for CDataBrowser browser
	virtual bool dbGetColumnDescription (int32_t index, CCoord& minWidth, CCoord& maxWidth, CDataBrowser* browser) { return false; }
	virtual CCoord dbGetCurrentColumnWidth (int32_t index, CDataBrowser* browser) = 0;	///< return current width of index column
	virtual void dbSetCurrentColumnWidth (int32_t index, const CCoord& width, CDataBrowser* browser) {}	///< the width of a column has changed
	virtual CCoord dbGetRowHeight (CDataBrowser* browser) = 0;	///< return height of one row
	virtual bool dbGetLineWidthAndColor (CCoord& width, CColor& color, CDataBrowser* browser) { return false; } ///< return the line width and color

	virtual void dbDrawHeader (CDrawContext* context, const CRect& size, int32_t column, int32_t flags, CDataBrowser* browser) = 0;	///< draw the db header
	virtual void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser) = 0;	///< draw a db cell

	virtual CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) { return kMouseDownEventHandledButDontNeedMovedOrUpEvents; } ///< mouse button was pressed on a cell
	virtual CMouseEventResult dbOnMouseMoved (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) { return kMouseEventNotHandled; } ///< mouse was moved over a cell
	virtual CMouseEventResult dbOnMouseUp (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) { return kMouseEventNotHandled; } ///< mouse button was released on a cell

	virtual void dbSelectionChanged (CDataBrowser* browser) {}	///< the selection of the db changed

	virtual void dbCellTextChanged (int32_t row, int32_t column, UTF8StringPtr newText, CDataBrowser* browser) {} ///< the text of the cell changed beginTextEdit was called for
	virtual void dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* textEditControl, CDataBrowser* browser) {} ///< beginTextEdit calls this, so you can setup the textedit control

	virtual int32_t dbOnKeyDown (const VstKeyCode& key, CDataBrowser* browser) { return -1; }

	enum {
		kRowSelected = 1 << 1,
	};
};

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
		kDrawHeaderFlag
	};
	
public:
	CDataBrowser (const CRect& size, CFrame* pParent, IDataBrowser* db, int32_t style = 0, CCoord scrollbarWidth = 16, CBitmap* pBackground = 0);

	enum CDataBrowserStyle 
	{
		// see CScrollView for more styles
		kDrawRowLines			= 1 << kDrawRowLinesFlag,
		kDrawColumnLines		= 1 << kDrawColumnLinesFlag,
		kDrawHeader				= 1 << kDrawHeaderFlag,
	};
	
	enum 
	{
		kNoSelection	= -1
	};

	//-----------------------------------------------------------------------------
	/// @name CDataBrowser Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void recalculateLayout (bool rememberSelection = false);				///< trigger recalculation, call if numRows or numColumns changed
	virtual void invalidate (int32_t row, int32_t column);								///< invalidates an individual cell
	virtual void invalidateRow (int32_t row);											///< invalidates a complete row
	virtual void makeRowVisible (int32_t row);											///< scrolls the scrollview so that row is visible

	virtual CRect getCellBounds (int32_t row, int32_t column);							///< get bounds of a cell

	virtual int32_t getSelectedRow () const { return selectedRow; }					///< get selected row
	virtual void setSelectedRow (int32_t row, bool makeVisible = false);				///< set the exclusive selected row

	virtual void beginTextEdit (int32_t row, int32_t column, UTF8StringPtr initialText);	///< starts a text edit for a cell
	//@}

	void setAutosizeFlags (int32_t flags);
	void setViewSize (CRect& size, bool invalid);
protected:
	~CDataBrowser ();
	void valueChanged (CControl *pControl);
	CMessageResult notify (CBaseObject* sender, IdStringPtr message);
	bool attached (CView *parent);

	IDataBrowser* db;
	CDataBrowserView* dbView;
	CDataBrowserHeader* dbHeader;
	CViewContainer* dbHeaderContainer;
	int32_t selectedRow;
};

} // namespace

#endif
