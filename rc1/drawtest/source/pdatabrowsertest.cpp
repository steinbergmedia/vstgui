/*
 *  pdatabrowsertest.cpp
 *  drawtest
 *
 *  Created by Arne Scheffler on 4/26/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "pdatabrowsertest.h"
#include <stdio.h>

#define kCellByteSize	256
#define kLineWidth		4

PDataBrowserTest::PDataBrowserTest ()
{
	for (long r = 0; r < kNumRows; r++)
	{
		for (long c = 0; c < kNumCols; c++)
		{
			cell [r][c] = (char*)malloc (kCellByteSize);
			sprintf (cell [r][c], "cell %03d|%03d", r,c);
		}
	}
	for (long i = 0; i < kNumCols; i++)
	{
		colWidth[i] = 80;
	}
}

PDataBrowserTest::~PDataBrowserTest ()
{
	for (long r = 0; r < kNumRows; r++)
	{
		for (long c = 0; c < kNumCols; c++)
		{
			free (cell [r][c]);
		}
	}
}

// IDataBrowser
long PDataBrowserTest::dbGetNumRows (CDataBrowser* browser)
{
	return kNumRows;
}

long PDataBrowserTest::dbGetNumColumns (CDataBrowser* browser)
{
	return kNumCols;
}

bool PDataBrowserTest::dbGetColumnDescription (long index, CCoord& minWidth, CCoord& maxWidth, CDataBrowser* browser)
{
	minWidth = 20;
	maxWidth = 1000;
	return true;
}

CCoord PDataBrowserTest::dbGetCurrentColumnWidth (long index, CDataBrowser* browser)
{
	return colWidth [index];
}

void PDataBrowserTest::dbSetCurrentColumnWidth (long index, const CCoord& width, CDataBrowser* browser)
{
	colWidth[index] = width;
}

CCoord PDataBrowserTest::dbGetRowHeight (CDataBrowser* browser)
{
	return kSystemFont->getSize () + 4;
}

void PDataBrowserTest::dbDrawHeader (CDrawContext* context, const CRect& size, long column, long flags, CDataBrowser* browser)
{
	context->setLineWidth (1);
	context->setFillColor (kGreyCColor);
	context->setFrameColor (kGreyCColor);
	context->drawRect (size, kDrawFilled);//AndStroked);
	context->setFrameColor (kBlackCColor);
	context->setLineWidth (kLineWidth);
	context->moveTo (CPoint (size.right + kLineWidth/2, size.top));
	context->lineTo (CPoint (size.right + kLineWidth/2, size.bottom));
	char text[40];
	sprintf (text, "Col %d", column);
	context->setFont (kSystemFont);
	context->setFontColor (kBlackCColor);
	context->drawStringUTF8 (text, size);
}

void PDataBrowserTest::dbDrawCell (CDrawContext* context, const CRect& size, long row, long column, long flags, CDataBrowser* browser)
{
	if (flags & IDataBrowser::kRowSelected)
	{
		context->setLineWidth (1);
		context->setFillColor (kYellowCColor);
		context->drawRect (size, kDrawFilled);
	}
	context->setFont (kSystemFont);
	context->setFontColor (kBlackCColor);
	context->drawStringUTF8 (cell[row][column], size, kLeftText);
}

CMouseEventResult PDataBrowserTest::dbOnMouseDown (const CPoint& where, const long& buttons, long row, long column, CDataBrowser* browser)
{
	if (buttons & kDoubleClick)
		browser->beginTextEdit (row, column, cell[row][column]);
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

void PDataBrowserTest::dbSelectionChanged (CDataBrowser* browser)
{
}

void PDataBrowserTest::dbCellTextChanged (long row, long column, const char* newText, CDataBrowser* browser)
{
	strcpy (cell[row][column], newText);
}

void PDataBrowserTest::dbCellSetupTextEdit (long row, long column, CTextEdit* textEditControl, CDataBrowser* browser)
{
	textEditControl->setHoriAlign (kLeftText);
	textEditControl->setFont (kSystemFont);
}

bool PDataBrowserTest::dbGetLineWidthAndColor (CCoord& width, CColor& color, CDataBrowser* browser)
{
	width = kLineWidth;
	color = kBlueCColor;
	return true;
}

