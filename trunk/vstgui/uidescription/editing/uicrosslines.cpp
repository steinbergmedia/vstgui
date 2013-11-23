//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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

#include "../../lib/cview.h"
#include "../../lib/cdrawcontext.h"
#include "uiselection.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
UICrossLines::UICrossLines (CView* view, int32_t style, const CColor& background, const CColor& foreground)
: view (view)
, style (style)
, background (background)
, foreground (foreground)
{
}

//----------------------------------------------------------------------------------------------------
UICrossLines::~UICrossLines ()
{
	invalid ();
}

//----------------------------------------------------------------------------------------------------
void UICrossLines::update (UISelection* selection)
{
	invalid ();
	currentRect = selection->getBounds ();
	CPoint p;
	view->frameToLocal (p);
	currentRect.offset (p.x, p.y);
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
	invalid ();
}

//----------------------------------------------------------------------------------------------------
void UICrossLines::invalid ()
{
	CRect frameRect = view->getViewSize (frameRect);
	view->invalid ();
	view->invalidRect (CRect (currentRect.left-3, frameRect.top, currentRect.left+3, frameRect.bottom));
	view->invalidRect (CRect (frameRect.left, currentRect.top-3, frameRect.right, currentRect.top+3));
	if (style == kSelectionStyle)
	{
		view->invalidRect (CRect (currentRect.right-3, frameRect.top, currentRect.right+3, frameRect.bottom));
		view->invalidRect (CRect (frameRect.left, currentRect.bottom-3, frameRect.right, currentRect.bottom+3));
	}
}

//----------------------------------------------------------------------------------------------------
void UICrossLines::draw (CDrawContext* pContext)
{
	CRect size = view->getViewSize (size);
	size.originize ();
	CRect selectionSize (currentRect);

	pContext->setFrameColor (background);
	pContext->setLineWidth (3);
	pContext->setLineStyle (kLineSolid);
	pContext->setDrawMode (kAliasing);
	pContext->moveTo (CPoint (size.left, selectionSize.top+1));
	pContext->lineTo (CPoint (size.right, selectionSize.top+1));
	pContext->moveTo (CPoint (selectionSize.left, size.top+1));
	pContext->lineTo (CPoint (selectionSize.left, size.bottom));
	if (style == kSelectionStyle)
	{
		pContext->moveTo (CPoint (size.left, selectionSize.bottom));
		pContext->lineTo (CPoint (size.right, selectionSize.bottom));
		pContext->moveTo (CPoint (selectionSize.right-1, size.top));
		pContext->lineTo (CPoint (selectionSize.right-1, size.bottom));
	}
	pContext->setFrameColor (foreground);
	pContext->setLineWidth (1);
	pContext->setLineStyle (kLineOnOffDash);
	pContext->moveTo (CPoint (size.left, selectionSize.top+1));
	pContext->lineTo (CPoint (size.right, selectionSize.top+1));
	pContext->moveTo (CPoint (selectionSize.left, size.top));
	pContext->lineTo (CPoint (selectionSize.left, size.bottom));
	if (style == kSelectionStyle)
	{
		pContext->moveTo (CPoint (size.left, selectionSize.bottom));
		pContext->lineTo (CPoint (size.right, selectionSize.bottom));
		pContext->moveTo (CPoint (selectionSize.right-1, size.top));
		pContext->lineTo (CPoint (selectionSize.right-1, size.bottom));
	}
}

}

#endif // VSTGUI_LIVE_EDITING
