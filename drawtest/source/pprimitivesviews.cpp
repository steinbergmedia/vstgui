#ifndef __pprimitivesviews__
#include "pprimitivesviews.h"
#endif

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
	context->setFrameColor (kBlackCColor);
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

const CColor kBlueAlphaCColor	= {   0,   0, 255, 200};
const CColor kYellowAlphaCColor = { 255, 255,   0, 200};
const CColor kGreenAlphaCColor  = {   0, 255,   0, 200};
const CColor kMagentaAlphaCColor= { 255,   0, 255, 200};

void PRectsView::draw (CDrawContext *context)
{
	int i;
	char text[512];
	text[0] = 0;
	context->setFont (kNormalFontSmall);
	context->setFrameColor (kRedCColor);
	context->setFontColor (kBlackCColor);
	CRect r (size);
	context->drawRect (r);
	r.inset (1,1);
	context->setFillColor (kWhiteCColor);
	context->fillRect (r);
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
	context->fillRect (r2);	
	r2  = r1;
	r2.offset (r2.getWidth (), 0);
	r2.inset (1,1);
	context->setFillColor (kYellowAlphaCColor);
	context->fillRect (r2);	
	r2 = r1;
	r2.offset (0, r2.getHeight ());
	r2.inset (1,1);
	context->setFillColor (kGreenAlphaCColor);
	context->fillRect (r2);	
	r2 = r1;
	r2.offset (r2.getWidth (), r2.getHeight ());
	r2.inset (1,1);
	context->setFillColor (kMagentaAlphaCColor);
	context->fillRect (r2);	

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
