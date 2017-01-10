//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins :
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

#include "clayeredviewcontainer.h"
#include "cframe.h"
#include "cdrawcontext.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
CLayeredViewContainer::CLayeredViewContainer (const CRect& r)
: CViewContainer (r)
, parentLayerView (0)
, zIndex (0)
{
}

//-----------------------------------------------------------------------------
CLayeredViewContainer::~CLayeredViewContainer ()
{
	
}

//-----------------------------------------------------------------------------
void CLayeredViewContainer::setZIndex (uint32_t _zIndex)
{
	if (_zIndex != zIndex)
	{
		zIndex = _zIndex;
		if (layer)
			layer->setZIndex (zIndex);
	}
}

//-----------------------------------------------------------------------------
void CLayeredViewContainer::updateLayerSize ()
{
	CRect newSize = getViewSize ();
	getTransform ().transform (newSize);

	CViewContainer* parent = static_cast<CViewContainer*> (getParentView ());
	while (parent)
	{
		CRect parentSize = parent->getViewSize ();
		parent->getTransform ().transform (newSize);
		newSize.offset (parentSize.left, parentSize.top);
		newSize.bound (parentSize);
		parent = static_cast<CViewContainer*> (parent->getParentView ());
	}

	if (parentLayerView)
	{
		CPoint p (parentLayerView->getVisibleViewSize ().getTopLeft ());
		parentLayerView->translateToGlobal (p);
		newSize.offsetInverse (p);
	}
	layer->setSize (newSize);
}

//-----------------------------------------------------------------------------
bool CLayeredViewContainer::removed (CView* parent)
{
	if (!isAttached ())
		return false;
	registerListeners (false);
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
		{
			layer->setZIndex (zIndex);
			layer->setAlpha (getAlphaValue ());
			updateLayerSize ();
		}
	}
	parent = pParentView;
	
	registerListeners (true);
	
	pParentView = 0;
	pParentFrame = 0;

	return CViewContainer::attached (parent);
}

//-----------------------------------------------------------------------------
void CLayeredViewContainer::registerListeners (bool state)
{
	CViewContainer* parent = static_cast<CViewContainer*> (getParentView ());
	while (parent)
	{
		if (state)
			parent->registerViewContainerListener (this);
		else
			parent->unregisterViewContainerListener (this);
		parent = static_cast<CViewContainer*> (parent->getParentView ());
	}
}

//-----------------------------------------------------------------------------
void CLayeredViewContainer::viewContainerTransformChanged (CViewContainer* container)
{
	if (layer)
		updateLayerSize ();
}

//-----------------------------------------------------------------------------
void CLayeredViewContainer::invalid ()
{
	CRect r = getViewSize ();
	r.originize ();
	invalidRect (r);
}

//-----------------------------------------------------------------------------
void CLayeredViewContainer::invalidRect (const CRect& rect)
{
	if (layer)
	{
		CRect r (rect);
		getDrawTransform ().transform (r);
		layer->invalidRect (r);
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
	{
		updateLayerSize ();
		invalid ();
	}
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
void CLayeredViewContainer::setAlphaValue (float alpha)
{
	if (layer)
	{
		alphaValue = alpha;
		layer->setAlpha (alpha);
	}
	else
		CViewContainer::setAlphaValue (alpha);
}

//-----------------------------------------------------------------------------
void CLayeredViewContainer::drawRect (CDrawContext* pContext, const CRect& updateRect)
{
	if (layer)
		layer->draw (pContext, updateRect);
	else
		CViewContainer::drawRect (pContext, updateRect);
}

//-----------------------------------------------------------------------------
void CLayeredViewContainer::drawViewLayer (CDrawContext* context, const CRect& _dirtyRect)
{
	CRect dirtyRect (_dirtyRect);

	CGraphicsTransform drawTransform = getDrawTransform ();
	drawTransform.inverse ().transform (dirtyRect);

	CRect visibleSize = getVisibleViewSize ();
	CPoint p (visibleSize.left, visibleSize.top);
	dirtyRect.offset (p.x, p.y);
	CDrawContext::Transform transform (*context, drawTransform * CGraphicsTransform ().translate (-p.x, -p.y));
	CViewContainer::drawRect (context, dirtyRect);
}

//-----------------------------------------------------------------------------
CGraphicsTransform CLayeredViewContainer::getDrawTransform () const
{
	CGraphicsTransform transform;
	typedef std::list<CViewContainer*> ParentViews;
	ParentViews parents;

	CViewContainer* parent = static_cast<CViewContainer*> (getParentView ());
	while (parent)
	{
		parents.push_front (parent);
		parent = static_cast<CViewContainer*> (parent->getParentView ());
	}
	VSTGUI_RANGE_BASED_FOR_LOOP (ParentViews, parents, CViewContainer*, parent)
		transform = parent->getTransform () * transform;
	VSTGUI_RANGE_BASED_FOR_LOOP_END
	
	const CViewContainer* This = static_cast<const CViewContainer*> (this);
	if (This)
		transform = This->getTransform () * transform;
	return transform;
}

}
