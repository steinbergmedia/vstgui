//-------------------------------------------------------------------------------------------------------
//  VSTGUI Test View plugin
//  Copyright (c) 2004 Arne Scheffler. All rights reserved.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//-------------------------------------------------------------------------------------------------------

#ifndef __pdrawtestview__
#include "pdrawtestview.h"
#endif

#include <stdio.h>

CDrawTestView::CDrawTestView (const CRect& size)
: CView (size)
, value (0)
{
}

static inline void testDrawRect (CDrawContext *pContext, CRect r, int _offset = 0)
{
	int offset = _offset ? _offset : pContext->getLineWidth ();
	int i = 0;
	while (r.width () > 1 && r.height () > 1)
	{
		if (i++ % 2)
			pContext->setFrameColor (kBlackCColor);
		else
			pContext->setFrameColor (kRedCColor);
		r.inset (offset, offset);
		pContext->drawRect (r);
	}
}

static inline void testFillRect (CDrawContext *pContext, CRect r)
{
	int i = 0;
	while (r.width () > 1 && r.height () > 1)
	{
		if (i++ % 2)
			pContext->setFillColor (kYellowCColor);
		else
			pContext->setFillColor (kCyanCColor);
		r.inset (1,1);
		pContext->fillRect (r);
	}
}

static inline void testDrawLine (CDrawContext *pContext, CRect r, int _offset = 0)
{
	int offset = _offset ? _offset : pContext->getLineWidth ();
	int i = 0;
	while (r.width () > 1 && r.height () > 1)
	{
		if (i++ % 2)
			pContext->setFrameColor (kBlueCColor);
		else
			pContext->setFrameColor (kGreenCColor);
		r.inset (offset,offset);
		pContext->moveTo (CPoint (r.left, r.top));
		pContext->lineTo (CPoint (r.right, r.top));
		pContext->lineTo (CPoint (r.right, r.bottom));
		pContext->lineTo (CPoint (r.left, r.bottom));
		pContext->lineTo (CPoint (r.left, r.top));
	}
}

static inline void clearRect (CDrawContext* pContext, const CRect& r)
{
	pContext->setFillColor (kWhiteCColor);
	pContext->fillRect (r);
}

static inline void drawLines (CDrawContext* pContext, CRect r, int offset = 2)
{
	CRect size (r);
	pContext->setFrameColor (kBlueCColor);
	int i;
	for (i = 0; i < size.height (); i++)
	{
		pContext->moveTo (CPoint (r.left, r.top));
		pContext->lineTo (CPoint (r.right, r.bottom));
		r.offset (0, offset);
	}
	for (i = 0; i < size.height (); i++)
	{
		size.offset (0, -offset);
		pContext->moveTo (CPoint (size.left, size.top));
		pContext->lineTo (CPoint (size.right, size.bottom));
	}
}


CColor arcColors[] = { {0,0,255,255}, {0,255,0,255}, {255,0,0,255} };

#define numArcColors	2

static inline void drawArcs (CDrawContext* pContext, CRect r, int offset = 2)
{
	int i = 0;
	long n = r.height () > r.width () ? r.width () : r.height ();
	while (r.width () > 1 && r.height () > 1)
	{
		pContext->setFrameColor (arcColors[i++]); if (i > numArcColors) i = 0;
		pContext->drawArc (r, CPoint (r.left, r.top + r.height () / 2), CPoint (r.left + r.width () / 2, r.top));
		pContext->setFrameColor (arcColors[i++]); if (i > numArcColors) i = 0;
		pContext->drawArc (r, CPoint (r.left + r.width () / 2, r.top), CPoint (r.right, r.top + r.height () / 2));
		pContext->setFrameColor (arcColors[i++]); if (i > numArcColors) i = 0;
		pContext->drawArc (r, CPoint (r.right, r.top + r.height () / 2), CPoint (r.left + r.width () / 2, r.bottom));
		pContext->setFrameColor (arcColors[i++]); if (i > numArcColors) i = 0;
		pContext->drawArc (r, CPoint (r.left + r.width () / 2, r.bottom), CPoint (r.left, r.top + r.height () / 2));
		r.inset (offset, offset);
	}
}

static inline void drawEllipses (CDrawContext* pContext, CRect r, int offset = 2)
{
	r.inset (10, 0);
	int i = 0;
	long n = r.height () > r.width () ? r.width () : r.height ();
	while (r.width () > 1 && r.height () > 1)
	{
		pContext->setFrameColor (arcColors[i++]); if (i > numArcColors) i = 0;
		pContext->drawEllipse (r);
		r.inset (offset, offset);
	}
}

#define kMaxValue	9

void CDrawTestView::draw (CDrawContext *pContext)
{
	CRect r (size);
	clearRect (pContext, r);

	AEffGUIEditor* editor = (AEffGUIEditor*)getEditor ();
	unsigned long startTime = editor->getTicks ();
	switch (value)
	{
		case 0:
		{
			pContext->setLineWidth (1);
			testDrawRect (pContext, size);
			break;
		}
		case 1:
		{
			pContext->setLineWidth (1);
			testDrawLine (pContext, size, 2);
			break;
		}
		case 2:
		{
			testFillRect (pContext, size);
			break;
		}
		case 3:
		{
			pContext->setLineWidth (2);
			testDrawLine (pContext, size);
			break;
		}
		case 4:
		{
			pContext->setDrawMode (kAntialias);
			pContext->setLineWidth (2);
			testDrawLine (pContext, size);
			break;
		}
		case 5:
		{
			pContext->setDrawMode (kAntialias);
			pContext->setLineWidth (1);
			testDrawLine (pContext, size);
			break;
		}
		case 6:
		{
			pContext->setLineWidth (1);
			drawLines (pContext, size, 4);
			break;
		}
		case 7:
		{
			pContext->setDrawMode (kAntialias);
			pContext->setLineWidth (1);
			drawLines (pContext, size, 4);
			break;
		}
		case 8:
		{
			pContext->setDrawMode (kAntialias);
			pContext->setLineWidth (4);
			drawArcs (pContext, size, 8);
			break;
		}
		case 9:
		{
			pContext->setDrawMode (kAntialias);
			pContext->setLineWidth (4);
			drawEllipses (pContext, size, 8);
			break;
		}
	}
	unsigned long stopTime = editor->getTicks ();

	char str[256];
	sprintf (str, "Drawing took %d ticks", stopTime-startTime);
	pContext->setFont (kSystemFont);
	pContext->setFontColor (kWhiteCColor);
	pContext->drawString (str, r);
	pContext->setFontColor (kBlackCColor);
	r.offset (-1, -1);
	pContext->drawString (str, r, true);
}

void CDrawTestView::mouse (CDrawContext* pContext, CPoint& where, long buttons)
{
	if (buttons & kLButton)
	{
		value++;
		if (value > kMaxValue)
			value = 0;
	}
	setDirty (true);
}

