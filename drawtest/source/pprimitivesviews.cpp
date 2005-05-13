#ifndef __pprimitivesviews__
#include "pprimitivesviews.h"
#endif

const CColor kBlueAlphaCColor	= {   0,   0, 255, 200};
const CColor kRedAlphaCColor	= { 255,   0,   0, 200};
const CColor kYellowAlphaCColor = { 255, 255,   0, 200};
const CColor kGreenAlphaCColor  = {   0, 255,   0, 200};
const CColor kMagentaAlphaCColor= { 255,   0, 255, 200};

PMiscView::PMiscView (const CRect& size)
: CView (size), xOffset (0), yOffset (0)
{
	setTransparency (true);
}

void PMiscView::mouse (CDrawContext* pContext, CPoint &where, long buttons)
{
	if (pContext->waitDrag ())
	{
		while (pContext->getMouseButtons () == kLButton)
		{
			CPoint cl;
			getMouseLocation (pContext, cl);
			if (cl != where)
			{
				xOffset -= (where.x - cl.x);
				yOffset -= (where.y - cl.y);
				where = cl;
				setDirty (true);
			}
			getFrame ()->doIdleStuff ();
		}
	}
}

static inline void offsetPoints (CPoint* points, long numPoints, CPoint offset)
{
	for (long i = 0; i < numPoints; i++)
		points[i].offset (offset.x, offset.y);
}

void PMiscView::draw (CDrawContext* pContext)
{
	drawGrid (pContext);
	CPoint polyPoints[10];
	polyPoints[0] = CPoint (50,0);
	polyPoints[1] = CPoint (100,50);
	polyPoints[2] = CPoint (50,100);
	polyPoints[3] = CPoint (0,50);
	polyPoints[4] = CPoint (50,0);
	offsetPoints (polyPoints, 5, CPoint (size.left + xOffset, size.top + yOffset));
	pContext->setDrawMode (kAntialias);
	pContext->setFrameColor (kBlackCColor);
	pContext->drawPolygon (polyPoints, 5, kDrawStroked);
	offsetPoints (polyPoints, 5, CPoint (50, 0));
	pContext->setFillColor (kBlueAlphaCColor);
	pContext->drawPolygon (polyPoints, 5, kDrawFilled);
	offsetPoints (polyPoints, 5, CPoint (50, 0));
	pContext->setFillColor (kRedAlphaCColor);
	pContext->drawPolygon (polyPoints, 5, kDrawFilledAndStroked);

	CRect ellipseRect (200, 0, 300, 100);
	ellipseRect.offset (size.left + xOffset, size.top + yOffset);
	pContext->drawEllipse (ellipseRect, kDrawStroked);
	ellipseRect.offset (50, 0);
	pContext->setFillColor (kBlueAlphaCColor);
	pContext->drawEllipse (ellipseRect, kDrawFilled);
	ellipseRect.offset (50, 0);
	pContext->setFillColor (kRedAlphaCColor);
	pContext->drawEllipse (ellipseRect, kDrawFilledAndStroked);

	setDirty (false);
}

void PMiscView::drawGrid (CDrawContext* pContext)
{
	static long gridOffset = 10;
	long x = size.left;
	long y = size.top;
	long numHLines = size.getWidth () / gridOffset + 1;
	long numVLines = size.getHeight () / gridOffset + 1;
	CPoint* points = new CPoint [numHLines*2 + numVLines*2];
	for (long i = 0; i < numHLines*2; i+=2, x += gridOffset)
	{
		points[i] = CPoint (x, size.top);
		points[i+1] = CPoint (x, size.bottom);
	}
	for (long i = 0; i < numVLines*2; i+=2, y += gridOffset)
	{
		points[numHLines*2+i] = CPoint (size.left, y);
		points[numHLines*2+i+1] = CPoint (size.right, y);
	}
	pContext->setDrawMode (kCopyMode);
	pContext->setFrameColor (kGreyCColor);
	pContext->drawLines (points, numHLines+numVLines);
	delete [] points;
}

PLinesView::PLinesView (const CRect& size)
: CView (size)
{
	setTransparency (true);
}

void PLinesView::draw (CDrawContext *context)
{
	int i;
	char text[512];
	text[0] = 0;
	context->setFont (kNormalFontSmall);
	context->setFrameColor (kWhiteCColor);
	context->setFontColor (kBlackCColor);
	CRect r (size);
	r.left += 10;
	r.right -= 2;
	r.setHeight (12);
	context->drawString ("Normal Lines with different line widths", r);
	context->setLineStyle (kLineSolid);
	r.offset (0, 12);
	for (i = 1; i <= 5; i++)
	{
		context->setLineWidth (i);
		CPoint p (r.left, r.bottom - r.getHeight () / 2);
		context->moveTo (p);
		p.x = r.right;
		context->lineTo (p);
		CRect textSize (r.left-10, r.top, r.left, r.bottom);
		sprintf (text, "%d", i);
		context->drawString (text, textSize);
		r.offset (0, 12);
	}
	context->drawString ("Dotted Lines with different line widths", r);
	context->setLineStyle (kLineOnOffDash);
	r.offset (0, 12);
	for (i = 1; i <= 5; i++)
	{
		context->setLineWidth (i);
		CPoint p (r.left, r.bottom - r.getHeight () / 2);
		context->moveTo (p);
		p.x = r.right;
		context->lineTo (p);
		CRect textSize (r.left-10, r.top, r.left, r.bottom);
		sprintf (text, "%d", i);
		context->drawString (text, textSize);
		r.offset (0, 12);
	}
	context->drawString ("Normal Lines Antialiased with different line widths", r);
	context->setLineStyle (kLineSolid);
	context->setDrawMode (kAntialias);
	r.offset (0, 12);
	for (i = 1; i <= 5; i++)
	{
		context->setLineWidth (i);
		CPoint p (r.left, r.bottom - r.getHeight () / 2);
		context->moveTo (p);
		p.x = r.right;
		context->lineTo (p);
		CRect textSize (r.left-10, r.top, r.left, r.bottom);
		sprintf (text, "%d", i);
		context->drawString (text, textSize);
		r.offset (0, 12);
	}
	context->drawString ("Dotted Lines Antialiased with different line widths", r);
	context->setLineStyle (kLineOnOffDash);
	r.offset (0, 12);
	for (i = 1; i <= 5; i++)
	{
		context->setLineWidth (i);
		CPoint p (r.left, r.bottom - r.getHeight () / 2);
		context->moveTo (p);
		p.x = r.right;
		context->lineTo (p);
		CRect textSize (r.left-10, r.top, r.left, r.bottom);
		sprintf (text, "%d", i);
		context->drawString (text, textSize);
		r.offset (0, 12);
	}
	context->setLineStyle (kLineSolid);
	context->setDrawMode (kCopyMode);

	setDirty (false);
}

PRectsView::PRectsView (const CRect& size)
: CView (size)
{
	setTransparency (true);
}

void PRectsView::draw (CDrawContext *context)
{
	char text[512];
	text[0] = 0;
	context->setFont (kNormalFontSmall);
	context->setFrameColor (kRedCColor);
	context->setFontColor (kBlackCColor);
	CRect r (size);
	context->drawRect (r);
//	r.inset (1,1);
	context->setFillColor (kWhiteCColor);
	context->drawRect (r,kDrawFilled);
	r.inset (2, 2);
	context->setDrawMode (kAntialias);
	context->drawRect (r);
	context->setDrawMode (kCopyMode);
	r.inset (5, 5);
	CRect r1 (r);
	r1.setHeight (r1.getHeight () / 2);
	r1.setWidth (r1.getWidth () / 2);
	CRect r2 (r1);
	r2.inset (1, 1);
	context->setFillColor (kBlueAlphaCColor);
	context->drawRect (r2, kDrawFilledAndStroked);	
	r2  = r1;
	r2.offset (r2.getWidth (), 0);
	r2.inset (1,1);
	context->setFillColor (kYellowAlphaCColor);
	context->drawRect (r2, kDrawFilledAndStroked);	
	r2 = r1;
	r2.offset (0, r2.getHeight ());
	r2.inset (1,1);
	context->setFillColor (kGreenAlphaCColor);
	context->drawRect (r2, kDrawFilledAndStroked);	
	r2 = r1;
	r2.offset (r2.getWidth (), r2.getHeight ());
	r2.inset (1,1);
	context->setFillColor (kMagentaAlphaCColor);
	context->drawRect (r2, kDrawFilledAndStroked);	

	r2 = r;
	r2.inset (r2.getWidth () / 4, r2.getHeight () / 4);
	CColor alphaColor = {0, 0, 0, 130};
	context->setFillColor (alphaColor);
	context->fillRect (r2);
	r2.inset (r2.getWidth () / 4, r2.getHeight () / 4);
	CColor alpha2Color = {255, 255, 255, 130};
	context->setFillColor (alpha2Color);
	context->fillRect (r2);
	setDirty (false);
}
