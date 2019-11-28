// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uicrosslines.h"

#if VSTGUI_LIVE_EDITING

#include "../../lib/cviewcontainer.h"
#include "../../lib/cdrawcontext.h"
#include "uiselection.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
UICrossLines::UICrossLines (CViewContainer* view, int32_t style, const CColor& background, const CColor& foreground)
: UIOverlayView (view)
, style (style)
, background (background)
, foreground (foreground)
{
}

//----------------------------------------------------------------------------------------------------
UICrossLines::~UICrossLines ()
{
}

//----------------------------------------------------------------------------------------------------
void UICrossLines::update (UISelection* selection)
{
	invalid ();

	CPoint p;
	currentRect = selection->getBounds ();
	localToFrame (p);
	currentRect.offset (-p.x, -p.y);

	invalid ();
}

//----------------------------------------------------------------------------------------------------
void UICrossLines::update (const CPoint& point)
{
	invalid ();

	currentRect.left = point.x-1;
	currentRect.top = point.y-1;
	currentRect.setWidth (1);
	currentRect.setHeight (1);
	getTargetView ()->getTransform ().transform (currentRect);
	CPoint p;
	getParentView ()->frameToLocal (p);
	currentRect.offset (p.x, p.y);
	getTargetView ()->localToFrame (p);
	currentRect.offset (p.x, p.y);

	invalid ();
}

//----------------------------------------------------------------------------------------------------
void UICrossLines::update (const CRect& rect)
{
	invalid ();
	currentRect = rect;
	getTargetView ()->getTransform ().transform (currentRect);
	CPoint p;
	getParentView ()->frameToLocal (p);
	currentRect.offset (p.x, p.y);
	getTargetView ()->localToFrame (p);
	currentRect.offset (p.x, p.y);
	invalid ();
}

//----------------------------------------------------------------------------------------------------
void UICrossLines::invalid ()
{
	if (style == kLassoStyle)
	{
		auto r = currentRect;
		r.makeIntegral ();
		if (!r.isEmpty ())
			invalidRect (r);
		return;
	}
	CRect frameRect = getViewSize ();
	invalidRect (CRect (currentRect.left-3, frameRect.top, currentRect.left+3, frameRect.bottom));
	invalidRect (CRect (frameRect.left, currentRect.top-3, frameRect.right, currentRect.top+3));
	if (style == kSelectionStyle)
	{
		invalidRect (CRect (currentRect.right-3, frameRect.top, currentRect.right+3, frameRect.bottom));
		invalidRect (CRect (frameRect.left, currentRect.bottom-3, frameRect.right, currentRect.bottom+3));
	}
}

//----------------------------------------------------------------------------------------------------
void UICrossLines::drawLines (CDrawContext* pContext, const CRect& size, const CRect& selectionSize)
{
	pContext->drawLine (CPoint (size.left, selectionSize.top), CPoint (size.right, selectionSize.top));
	pContext->drawLine (CPoint (selectionSize.left, size.top), CPoint (selectionSize.left, size.bottom));
	if (style == kSelectionStyle)
	{
		pContext->drawLine (CPoint (size.left, selectionSize.bottom - 1), CPoint (size.right, selectionSize.bottom - 1));
		pContext->drawLine (CPoint (selectionSize.right-1, size.top), CPoint (selectionSize.right-1, size.bottom));
	}
}

//----------------------------------------------------------------------------------------------------
void UICrossLines::draw (CDrawContext* pContext)
{
	CRect size = getViewSize ();

	pContext->setDrawMode (kAliasing);
	pContext->setLineStyle (kLineSolid);
	pContext->setFrameColor (background);
	pContext->setLineWidth (1);

	if (style == kLassoStyle)
	{
		auto r = currentRect;
		r.makeIntegral ();
		if (r.isEmpty ())
			return;
		pContext->setFillColor (foreground);
		pContext->drawRect (r, kDrawFilledAndStroked);
		return;
	}

	CRect selectionSize (currentRect);
	drawLines (pContext, size, selectionSize);

	static const CCoord dashLength [] = {3,3};
	static const CLineStyle lineStyle (CLineStyle::kLineCapButt, CLineStyle::kLineJoinMiter, 0, 2, dashLength);

	pContext->setLineStyle (lineStyle);
	pContext->setFrameColor (foreground);
	drawLines (pContext, size, selectionSize);
}

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
