// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "idatabrowserdelegate.h"
#include "cfont.h"
#include "ccolor.h"
#include "cdrawdefs.h"

//-----------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
class GenericStringListDataBrowserSourceSelectionChanged
{
public:
	virtual void dbSelectionChanged (int32_t selectedRow,
	                                 GenericStringListDataBrowserSource* source) = 0;
	virtual void dbRowDoubleClick (int32_t row, GenericStringListDataBrowserSource* source) {};
};

//-----------------------------------------------------------------------------
// GenericStringListDataBrowserSource Declaration
//! @brief Generic string list data browser source
//-----------------------------------------------------------------------------
class GenericStringListDataBrowserSource : public DataBrowserDelegateAdapter, public CBaseObject
{
public:
	using StringVector = std::vector<UTF8String>;

	GenericStringListDataBrowserSource (
	    const StringVector* stringList,
	    GenericStringListDataBrowserSourceSelectionChanged* delegate = nullptr);
	~GenericStringListDataBrowserSource () noexcept override;

	void setStringList (const StringVector* stringList);
	const StringVector* getStringList () const { return stringList; }

	void setupUI (const CColor& selectionColor, const CColor& fontColor, const CColor& rowlineColor,
	              const CColor& rowBackColor, const CColor& rowAlteranteBackColor,
	              CFontRef font = nullptr, int32_t rowHeight = -1, CCoord textInset = 2.);

protected:
	void drawRowBackground (CDrawContext* context, const CRect& size, int32_t row, int32_t flags,
	                        CDataBrowser* browser) const;
	void drawRowString (CDrawContext* context, const CRect& size, int32_t row, int32_t flags,
	                    CDataBrowser* browser) const;

	int32_t dbGetNumRows (CDataBrowser* browser) override;
	int32_t dbGetNumColumns (CDataBrowser* browser) override { return 1; }
	bool dbGetColumnDescription (int32_t index, CCoord& minWidth, CCoord& maxWidth,
	                             CDataBrowser* browser) override
	{
		return false;
	}
	CCoord dbGetCurrentColumnWidth (int32_t index, CDataBrowser* browser) override;
	void dbSetCurrentColumnWidth (int32_t index, const CCoord& width,
	                              CDataBrowser* browser) override
	{
	}
	CCoord dbGetRowHeight (CDataBrowser* browser) override;
	bool dbGetLineWidthAndColor (CCoord& width, CColor& color, CDataBrowser* browser) override;

	void dbDrawHeader (CDrawContext* context, const CRect& size, int32_t column, int32_t flags,
	                   CDataBrowser* browser) override;
	void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column,
	                 int32_t flags, CDataBrowser* browser) override;

	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row,
	                                 int32_t column, CDataBrowser* browser) override;
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

	void dbSelectionChanged (CDataBrowser* browser) override;

	void dbCellTextChanged (int32_t row, int32_t column, UTF8StringPtr newText,
	                        CDataBrowser* browser) override
	{
	}
	void dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* textEditControl,
	                          CDataBrowser* browser) override
	{
	}

	void dbOnKeyboardEvent (KeyboardEvent& event, CDataBrowser* browser) override;

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
	GenericStringListDataBrowserSourceSelectionChanged* delegate;

	SharedPointer<CVSTGUITimer> timer;
	std::string keyDownFindString;
};

//------------------------------------------------------------------------
} // VSTGUI
