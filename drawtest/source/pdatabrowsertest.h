/*
 *  pdatabrowsertest.h
 *  drawtest
 *
 *  Created by Arne Scheffler on 4/26/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __pdatabrowsertest__
#define __pdatabrowsertest__

#include "cdatabrowser.h"

#define kNumRows	50
#define kNumCols	10

class PDataBrowserTest : public CBaseObject, public IDataBrowser
{
public:
	PDataBrowserTest ();
	~PDataBrowserTest ();

	// IDataBrowser
	long dbGetNumRows (CDataBrowser* browser);
	long dbGetNumColumns (CDataBrowser* browser);
	bool dbGetColumnDescription (long index, CCoord& minWidth, CCoord& maxWidth, CDataBrowser* browser);
	CCoord dbGetCurrentColumnWidth (long index, CDataBrowser* browser);
	void dbSetCurrentColumnWidth (long index, const CCoord& width, CDataBrowser* browser);
	CCoord dbGetRowHeight (CDataBrowser* browser);
	
	void dbDrawHeader (CDrawContext* context, const CRect& size, long column, long flags, CDataBrowser* browser);
	void dbDrawCell (CDrawContext* context, const CRect& size, long row, long column, long flags, CDataBrowser* browser);
	CMouseEventResult dbOnMouseDown (const CPoint& where, const long& buttons, long row, long column, CDataBrowser* browser);

	void dbSelectionChanged (CDataBrowser* browser);

	void dbCellTextChanged (long row, long column, const char* newText, CDataBrowser* browser);
	void dbCellSetupTextEdit (long row, long column, CTextEdit* textEditControl, CDataBrowser* browser);

	bool dbGetLineWidthAndColor (CCoord& width, CColor& color, CDataBrowser* browser);
protected:
	char* cell [kNumRows][kNumCols];
	char* colName [kNumCols];
	CCoord colWidth [kNumCols];
};

#endif
