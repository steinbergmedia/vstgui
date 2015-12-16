//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "uicrosslines.h"

#if VSTGUI_LIVE_EDITING

#include "../../lib/cviewcontainer.h"
#include "../../lib/cdrawcontext.h"
#include "uiselection.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
UICrossLines::UICrossLines (CViewContainer* editView, int32_t style, const CColor& background, const CColor& foreground)
: CView (CRect (0, 0, 0, 0))
, editView (editView)
, style (style)
, background (background)
, foreground (foreground)
{
	setMouseEnabled (false);
	viewSizeChanged (editView, CRect (0, 0, 0, 0));
	editView->registerViewListener (this);
}

//----------------------------------------------------------------------------------------------------
UICrossLines::~UICrossLines ()
{
	editView->unregisterViewListener (this);
}

//----------------------------------------------------------------------------------------------------
void UICrossLines::viewSizeChanged (CView* view, const CRect& oldSize)
{
	CRect r = editView->getVisibleViewSize ();
	r.originize ();
	CPoint p;
	editView->getParentView ()->localToFrame (p);
	r.offset (p.x, p.y);
	if (getViewSize () != r)
	{
		invalidRect (getViewSize ());
		setViewSize (r);
	}
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
	editView->getTransform ().transform (currentRect);
	CPoint p;
	getParentView ()->frameToLocal (p);
	currentRect.offset (p.x, p.y);
	editView->localToFrame (p);
	currentRect.offset (p.x, p.y);

	invalid ();
}

//----------------------------------------------------------------------------------------------------
void UICrossLines::invalid ()
{
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
	pContext->drawLine (CPoint (size.left, selectionSize.top+1), CPoint (size.right, selectionSize.top+1));
	pContext->drawLine (CPoint (selectionSize.left, size.top), CPoint (selectionSize.left, size.bottom));
	if (style == kSelectionStyle)
	{
		pContext->drawLine (CPoint (size.left, selectionSize.bottom), CPoint (size.right, selectionSize.bottom));
		pContext->drawLine (CPoint (selectionSize.right-1, size.top), CPoint (selectionSize.right-1, size.bottom));
	}
}

//----------------------------------------------------------------------------------------------------
void UICrossLines::draw (CDrawContext* pContext)
{
	CRect size = getViewSize ();

	CRect selectionSize (currentRect);

	pContext->setDrawMode (kAliasing);
	pContext->setLineStyle (kLineSolid);
	pContext->setFrameColor (background);
	pContext->setLineWidth (1);
	drawLines (pContext, size, selectionSize);

	static const CCoord dashLength [] = {3,3};
	static const CLineStyle lineStyle (CLineStyle::kLineCapButt, CLineStyle::kLineJoinMiter, 0, 2, dashLength);

	pContext->setLineStyle (lineStyle);
	pContext->setFrameColor (foreground);
	drawLines (pContext, size, selectionSize);
}

}

#endif // VSTGUI_LIVE_EDITING
