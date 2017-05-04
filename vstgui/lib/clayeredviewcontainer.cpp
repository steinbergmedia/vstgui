// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "clayeredviewcontainer.h"
#include "cframe.h"
#include "cdrawcontext.h"
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
	if (getFrame ())
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
		layer = getFrame ()->getPlatformFrame ()->createPlatformViewLayer (this, parentLayerView ? parentLayerView->layer : nullptr);
		if (layer)
		{
			layer->setZIndex (zIndex);
			layer->setAlpha (getAlphaValue ());
			updateLayerSize ();
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
		setAlphaValueNoInvalidate (alpha);
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
	using ParentViews = std::list<CViewContainer*>;

	CGraphicsTransform transform;
	ParentViews parents;

	CViewContainer* parent = static_cast<CViewContainer*> (getParentView ());
	while (parent)
	{
		parents.push_front (parent);
		parent = static_cast<CViewContainer*> (parent->getParentView ());
	}
	for (const auto& parent : parents)
		transform = parent->getTransform () * transform;
	
	const CViewContainer* This = static_cast<const CViewContainer*> (this);
	if (This)
		transform = This->getTransform () * transform;
	return transform;
}

}
