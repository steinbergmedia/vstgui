//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins :
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

#include "clayeredviewcontainer.h"
#include "cframe.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
CLayeredViewContainer::CLayeredViewContainer (const CRect& r)
: CViewContainer (r)
, parentLayerView (0)
{
}

//-----------------------------------------------------------------------------
CLayeredViewContainer::~CLayeredViewContainer ()
{
	
}

//-----------------------------------------------------------------------------
void CLayeredViewContainer::updateLayerSize ()
{
	CRect r (getViewSize ());
	r.originize ();
	CPoint p;
	localToFrame (p);
	if (parentLayerView)
	{
		parentLayerView->frameToLocal (p);
	}
	else
	{
	}
	r.offset (p.x, p.y);
	layer->setSize (r);
}

//-----------------------------------------------------------------------------
bool CLayeredViewContainer::removed (CView* parent)
{
	if (layer)
	{
		layer = 0;
		parentLayerView = 0;
	}
	return CViewContainer::removed (parent);
}

//-----------------------------------------------------------------------------
bool CLayeredViewContainer::attached (CView* parent)
{
	if (isAttached ())
		return false;

	pParentView = parent;
	pParentFrame = parent->getFrame ();
	if (pParentFrame)
	{
		while (parent && dynamic_cast<CFrame*>(parent) == 0)
		{
			parentLayerView = dynamic_cast<CLayeredViewContainer*>(parent);
			if (parentLayerView)
			{
				break;
			}
			parent = parent->getParentView ();
		}
		layer = pParentFrame->getPlatformFrame ()->createPlatformViewLayer (this, parentLayerView ? parentLayerView->layer : 0);
		if (layer)
			updateLayerSize ();
	}
	parent = pParentView;
	pParentView = 0;
	pParentFrame = 0;

	return CViewContainer::attached (parent);
}

//-----------------------------------------------------------------------------
void CLayeredViewContainer::invalidRect (const CRect& rect)
{
	if (layer)
	{
		layer->invalidRect (rect);
	}
	else
	{
		CViewContainer::invalidRect (rect);
	}
}

//-----------------------------------------------------------------------------
void CLayeredViewContainer::parentSizeChanged ()
{
	CViewContainer::parentSizeChanged ();
	if (layer)
		updateLayerSize ();
}

//-----------------------------------------------------------------------------
void CLayeredViewContainer::setViewSize (const CRect& rect, bool invalid)
{
	CViewContainer::setViewSize (rect, invalid);
	if (layer)
	{
		updateLayerSize ();
	}
}

//-----------------------------------------------------------------------------
void CLayeredViewContainer::drawRect (CDrawContext* pContext, const CRect& updateRect)
{
	if (layer == 0)
		CViewContainer::drawRect (pContext, updateRect);
}

//-----------------------------------------------------------------------------
void CLayeredViewContainer::drawViewLayer (CDrawContext* context, const CRect& _dirtyRect)
{
	CRect dirtyRect (_dirtyRect);
	CPoint p (getViewSize ().left, getViewSize ().top);
	dirtyRect.offset (p.x, p.y);
	p.x *= -1.;
	p.y *= -1.;
	context->setOffset (p);
	CViewContainer::drawRect (context, dirtyRect);
}

}