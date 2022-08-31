// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "clayeredviewcontainer.h"
#include "cframe.h"
#include "cdrawcontext.h"
#include "coffscreencontext.h"
#include "platform/iplatformframe.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
CLayeredViewContainer::CLayeredViewContainer (const CRect& r)
: CViewContainer (r)
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
	if (!layer)
		return;

	CRect newSize = getViewSize ();
	getTransform ().transform (newSize);
	auto frame = getFrame ();

	auto* parent = static_cast<CViewContainer*> (getParentView ());
	while (parent && parent != frame)
	{
		CRect parentSize = parent->getViewSize ();
		parent->getTransform ().transform (newSize);
		newSize.offset (parentSize.left, parentSize.top);
		newSize.bound (parentSize);
		parent = static_cast<CViewContainer*> (parent->getParentView ());
	}

	frame->getTransform ().transform (newSize);

	if (parentLayerView)
	{
		CPoint p (parentLayerView->getVisibleViewSize ().getTopLeft ());
		parentLayerView->translateToGlobal (p);
		newSize.offsetInverse (p);
	}
	if (layer)
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
		layer = nullptr;
		parentLayerView = nullptr;
		getFrame ()->unregisterScaleFactorChangedListener (this);
	}
	return CViewContainer::removed (parent);
}

//-----------------------------------------------------------------------------
bool CLayeredViewContainer::attached (CView* parent)
{
	if (isAttached ())
		return false;

	setParentView (parent);
	setParentFrame (parent->getFrame ());
	if (auto frame = getFrame ())
	{
		while (parent && dynamic_cast<CFrame*>(parent) == nullptr)
		{
			parentLayerView = dynamic_cast<CLayeredViewContainer*>(parent);
			if (parentLayerView)
			{
				break;
			}
			parent = parent->getParentView ();
		}
		layer = frame->getPlatformFrame ()->createPlatformViewLayer (this, parentLayerView ? parentLayerView->layer : nullptr);
		if (layer)
		{
			layer->setZIndex (zIndex);
			layer->setAlpha (getAlphaValue ());
			updateLayerSize ();
			frame->registerScaleFactorChangedListener (this);
		}
	}
	parent = getParentView ();
	
	registerListeners (true);
	
	setParentView (nullptr);
	setParentFrame (nullptr);

	return CViewContainer::attached (parent);
}

//-----------------------------------------------------------------------------
void CLayeredViewContainer::registerListeners (bool state)
{
	auto* parent = static_cast<CViewContainer*> (getParentView ());
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
	updateLayerSize ();
}

//-----------------------------------------------------------------------------
void CLayeredViewContainer::setAlphaValue (float alpha)
{
	if (layer)
	{
		setAlphaValueNoInvalidate (alpha);
		layer->setAlpha (alpha);
	}
	else
		CViewContainer::setAlphaValue (alpha);
}

//-----------------------------------------------------------------------------
void CLayeredViewContainer::drawRect (CDrawContext* pContext, const CRect& updateRect)
{
	auto drawsIntoBitmap = false;
	if (auto offscreenContext = dynamic_cast<COffscreenContext*> (pContext))
		drawsIntoBitmap = offscreenContext->getBitmap () != nullptr;
	if (layer && !drawsIntoBitmap)
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
	CRect viewSize = getViewSize ();
	CPoint p (viewSize.left < 0 ? viewSize.left - visibleSize.left : visibleSize.left,
	          viewSize.top < 0 ? viewSize.top - visibleSize.top : visibleSize.top);
	dirtyRect.offset (p.x, p.y);
	CDrawContext::Transform transform (*context, drawTransform * CGraphicsTransform ().translate (-p.x, -p.y));
	CViewContainer::drawRect (context, dirtyRect);
}

//-----------------------------------------------------------------------------
CGraphicsTransform CLayeredViewContainer::getDrawTransform () const
{
	using ParentViews = std::list<CViewContainer*>;

	CGraphicsTransform transform;
	ParentViews parents;
	auto frame = getFrame ();
	
	auto* parent = static_cast<CViewContainer*> (getParentView ());
	while (parent && parent != frame)
	{
		parents.push_front (parent);
		parent = static_cast<CViewContainer*> (parent->getParentView ());
	}
	for (const auto& p : parents)
		transform = p->getTransform () * transform;
	
	auto self = static_cast<const CViewContainer*> (this);
	if (self)
		transform = self->getTransform () * transform;

	if (frame)
		transform = frame->getTransform () * transform;

	return transform;
}

//-----------------------------------------------------------------------------
void CLayeredViewContainer::onScaleFactorChanged (CFrame* frame, double newScaleFactor)
{
	if (layer)
		layer->onScaleFactorChanged (newScaleFactor);
}

}
