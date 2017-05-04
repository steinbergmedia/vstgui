// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cviewcontainer.h"
#include "coffscreencontext.h"
#include "cbitmap.h"
#include "cframe.h"
#include "ccolor.h"
#include "ifocusdrawing.h"
#include "itouchevent.h"
#include "iviewlistener.h"
#include "cgraphicspath.h"
#include "controls/ccontrol.h"

#include <algorithm>
#include <cassert>

namespace VSTGUI {

#ifndef FOREACHSUBVIEW_REVERSE
	#define FOREACHSUBVIEW_REVERSE(reverse) ChildViewConstIterator it; ChildViewConstReverseIterator rit; if (reverse) rit = pImpl->children.rbegin (); else it = pImpl->children.begin (); while (reverse ? rit != pImpl->children.rend () : it != pImpl->children.end ()) { CView* pV; if (reverse) {	pV = (*rit); ++rit; } else { pV = (*it); ++it; } {
#endif
#ifndef ENDFOREACHSUBVIEW
	#define ENDFOREACHSUBVIEW } }
#endif

IdStringPtr kMsgLooseFocus = "LooseFocus";

//-----------------------------------------------------------------------------
// CViewContainer Implementation
//-----------------------------------------------------------------------------
struct CViewContainer::Impl
{
	using ViewContainerListenerDispatcher = DispatchList<IViewContainerListener*>;
	
	ViewContainerListenerDispatcher viewContainerListeners;
	CGraphicsTransform transform;
	
	/// @cond ignore
	ViewList children;
	/// @endcond
	
	CDrawStyle backgroundColorDrawStyle {kDrawFilledAndStroked};
	CColor backgroundColor {kBlackCColor};
	CPoint backgroundOffset;
	
	CRect lastDrawnFocus;
	
	CView* currentDragView {nullptr};
	CView* mouseDownView {nullptr};
	
};

//-----------------------------------------------------------------------------
/**
 * CViewContainer constructor.
 * @param rect the size of the container
 */
CViewContainer::CViewContainer (const CRect &rect)
: CView (rect)
{
	pImpl = std::unique_ptr<Impl> (new Impl ());
	setAutosizingEnabled (true);
}

//-----------------------------------------------------------------------------
CViewContainer::CViewContainer (const CViewContainer& v)
: CView (v)
{
	pImpl = std::unique_ptr<Impl> (new Impl ());
	pImpl->transform = v.pImpl->transform;
	pImpl->backgroundColorDrawStyle = v.pImpl->backgroundColorDrawStyle;
	pImpl->backgroundColor = v.pImpl->backgroundColor;
	pImpl->backgroundOffset = v.pImpl->backgroundOffset;
	ViewIterator it (&v);
	while (*it)
	{
		addView (static_cast<CView*>((*it)->newCopy ()));
		++it;
	}
}

//-----------------------------------------------------------------------------
CViewContainer::~CViewContainer () noexcept
{
	vstgui_assert (pImpl->viewContainerListeners.empty ());
}

//-----------------------------------------------------------------------------
void CViewContainer::beforeDelete ()
{
	// remove all views
	CViewContainer::removeAll (true);
	CView::beforeDelete ();
}

//-----------------------------------------------------------------------------
void CViewContainer::registerViewContainerListener (IViewContainerListener* listener)
{
	pImpl->viewContainerListeners.add (listener);
}

//-----------------------------------------------------------------------------
void CViewContainer::unregisterViewContainerListener (IViewContainerListener* listener)
{
	pImpl->viewContainerListeners.remove (listener);
}

//-----------------------------------------------------------------------------
void CViewContainer::parentSizeChanged ()
{
	for (const auto& pV : pImpl->children)
		pV->parentSizeChanged ();	// notify children that the size of the parent or this container has changed
}

//-----------------------------------------------------------------------------
void CViewContainer::setMouseDownView (CView* view)
{
	pImpl->mouseDownView = view;
}

//-----------------------------------------------------------------------------
CView* CViewContainer::getMouseDownView () const
{
	return pImpl->mouseDownView;
}

//-----------------------------------------------------------------------------
auto CViewContainer::getChildren () const -> const ViewList&
{
	return pImpl->children;
}

//-----------------------------------------------------------------------------
void CViewContainer::setTransform (const CGraphicsTransform& t)
{
	if (pImpl->transform != t)
	{
		pImpl->transform = t;
		pImpl->viewContainerListeners.forEach ([this] (IViewContainerListener* listener) {
			listener->viewContainerTransformChanged (this);
		});
	}
}

//-----------------------------------------------------------------------------
const CGraphicsTransform& CViewContainer::getTransform () const
{
	return pImpl->transform;
}

//-----------------------------------------------------------------------------
void CViewContainer::setAutosizingEnabled (bool state)
{
	setViewFlag (kAutosizeSubviews, state);
}

//-----------------------------------------------------------------------------
/**
 * @param rect the new size of the container
 * @param invalid the views to dirty
 */
void CViewContainer::setViewSize (const CRect &rect, bool invalid)
{
	if (rect == getViewSize ())
		return;

	CRect oldSize (getViewSize ());
	CView::setViewSize (rect, invalid);

	if (getAutosizingEnabled ())
	{
		CCoord widthDelta = rect.getWidth () - oldSize.getWidth ();
		CCoord heightDelta = rect.getHeight () - oldSize.getHeight ();
		getTransform ().inverse ().transform (widthDelta, heightDelta);

		if (widthDelta != 0 || heightDelta != 0)
		{
			uint32_t numSubviews = getNbViews ();
			uint32_t counter = 0;
			bool treatAsColumn = (getAutosizeFlags () & kAutosizeColumn) != 0;
			bool treatAsRow = (getAutosizeFlags () & kAutosizeRow) != 0;
			for (const auto& pV : pImpl->children)
			{
				int32_t autosize = pV->getAutosizeFlags ();
				CRect viewSize (pV->getViewSize ());
				CRect mouseSize (pV->getMouseableArea ());
				if (treatAsColumn)
				{
					if (counter)
					{
						viewSize.offset (counter * (widthDelta / (numSubviews)), 0);
						mouseSize.offset (counter * (widthDelta / (numSubviews)), 0);
					}
					viewSize.setWidth (viewSize.getWidth () + (widthDelta / (numSubviews)));
					mouseSize.setWidth (mouseSize.getWidth () + (widthDelta / (numSubviews)));
				}
				else if (widthDelta != 0 && autosize & kAutosizeRight)
				{
					viewSize.right += widthDelta;
					mouseSize.right += widthDelta;
					if (!(autosize & kAutosizeLeft))
					{
						viewSize.left += widthDelta;
						mouseSize.left += widthDelta;
					}
				}
				if (treatAsRow)
				{
					if (counter)
					{
						viewSize.offset (0, counter * (heightDelta / (numSubviews)));
						mouseSize.offset (0, counter * (heightDelta / (numSubviews)));
					}
					viewSize.setHeight (viewSize.getHeight () + (heightDelta / (numSubviews)));
					mouseSize.setHeight (mouseSize.getHeight () + (heightDelta / (numSubviews)));
				}
				else if (heightDelta != 0 && autosize & kAutosizeBottom)
				{
					viewSize.bottom += heightDelta;
					mouseSize.bottom += heightDelta;
					if (!(autosize & kAutosizeTop))
					{
						viewSize.top += heightDelta;
						mouseSize.top += heightDelta;
					}
				}
				if (viewSize != pV->getViewSize ())
				{
					pV->setViewSize (viewSize);
					pV->setMouseableArea (mouseSize);
				}
				counter++;
			}
		}
	}
	parentSizeChanged ();
}

//-----------------------------------------------------------------------------
/**
 * @param rect size to get visible size of
 * @return visible size of rect
 */
CRect CViewContainer::getVisibleSize (const CRect& rect) const
{
	CRect viewSize (getViewSize ());
	getTransform ().inverse ().transform (viewSize);
	CRect result (rect);
	result.offset (viewSize.left, viewSize.top);
	result.bound (viewSize);
	if (getFrame () == this)
	{}
	else if (getParentView ())
		result = static_cast<CViewContainer*> (getParentView ())->getVisibleSize (result);
	result.offset (-viewSize.left, -viewSize.top);
	return result;
}

//-----------------------------------------------------------------------------
bool CViewContainer::sizeToFit ()
{
	bool treatAsColumn = (getAutosizeFlags () & kAutosizeColumn) != 0;
	bool treatAsRow = (getAutosizeFlags () & kAutosizeRow) != 0;
	if (treatAsColumn || treatAsRow)
		return false;

	CRect bounds (50000, 50000, -50000, -50000);
	for (const auto& pV : pImpl->children)
	{
		if (pV->isVisible ())
		{
			CRect vs (pV->getViewSize ());
			if (vs.left < bounds.left)
				bounds.left = vs.left;
			if (vs.right > bounds.right)
				bounds.right = vs.right;
			if (vs.top < bounds.top)
				bounds.top = vs.top;
			if (vs.bottom > bounds.bottom)
				bounds.bottom = vs.bottom;
		}
	}
	
	CRect vs (getViewSize ());
	vs.right = vs.left + bounds.right + bounds.left;
	vs.bottom = vs.top + bounds.bottom + bounds.top;
	
	setViewSize (vs);
	setMouseableArea (vs);

	return true;
}

//-----------------------------------------------------------------------------
/**
 * @param color the new background color of the container
 */
void CViewContainer::setBackgroundColor (const CColor& color)
{
	if (color != pImpl->backgroundColor)
	{
		pImpl->backgroundColor = color;
		setDirty (true);
	}
}

//------------------------------------------------------------------------------
CColor CViewContainer::getBackgroundColor () const
{
	return pImpl->backgroundColor;
}

//------------------------------------------------------------------------------
void CViewContainer::setBackgroundOffset (const CPoint& p)
{
	pImpl->backgroundOffset = p;
}

//------------------------------------------------------------------------------
const CPoint& CViewContainer::getBackgroundOffset () const
{
	return pImpl->backgroundOffset;
}

//------------------------------------------------------------------------------
void CViewContainer::setBackgroundColorDrawStyle (CDrawStyle style)
{
	if (pImpl->backgroundColorDrawStyle != style)
	{
		pImpl->backgroundColorDrawStyle = style;
		setDirty (true);
	}
}

//------------------------------------------------------------------------------
CDrawStyle CViewContainer::getBackgroundColorDrawStyle () const
{
	return pImpl->backgroundColorDrawStyle;
}

//------------------------------------------------------------------------------
CMessageResult CViewContainer::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == kMsgNewFocusView)
	{
		CView* view = dynamic_cast<CView*> (sender);
		if (view && isChild (view, false) && getFrame ()->focusDrawingEnabled ())
		{
			CCoord width = getFrame ()->getFocusWidth ();
			CRect viewSize (view->getViewSize ());
			viewSize.extend (width, width);
			invalidRect (viewSize);
		}
	}
	else if (message == kMsgOldFocusView)
	{
		if (!pImpl->lastDrawnFocus.isEmpty ())
			invalidRect (pImpl->lastDrawnFocus);
		pImpl->lastDrawnFocus = CRect (0, 0, 0, 0);
	}
	return kMessageUnknown;
}

//-----------------------------------------------------------------------------
/**
 * @param pView the view object to add to this container
 * @return true on success. false if view was already attached
 */
bool CViewContainer::addView (CView* pView)
{
	return addView (pView, nullptr);
}

//-----------------------------------------------------------------------------
/**
 * @param pView the view object to add to this container
 * @param pBefore the view object
 * @return true on success. false if view was already attached
 */
bool CViewContainer::addView (CView *pView, CView* pBefore)
{
	if (!pView)
		return false;

	vstgui_assert (!pView->isSubview (), "view is already added to a container view");
	vstgui_assert(!pView->isSubview());

	if (pBefore)
	{
		ViewList::iterator it = std::find (pImpl->children.begin (), pImpl->children.end (), pBefore);
		pImpl->children.insert (it, pView);
	}
	else
	{
		pImpl->children.emplace_back (pView);
	}

	pView->setSubviewState (true);

	pImpl->viewContainerListeners.forEach ([&] (IViewContainerListener* listener) {
		listener->viewContainerViewAdded (this, pView);
	});

	if (isAttached ())
	{
		pView->attached (this);
		pView->invalid ();
	}
	return true;
}

//-----------------------------------------------------------------------------
/**
 * @param pView the view object to add to this container
 * @param mouseableArea the view area in where the view will get mouse events
 * @param mouseEnabled bool to set if view will get mouse events
 * @return true on success. false if view was already attached
 */
bool CViewContainer::addView (CView* pView, const CRect &mouseableArea, bool mouseEnabled)
{
	if (addView (pView, nullptr))
	{
		pView->setMouseEnabled (mouseEnabled);
		pView->setMouseableArea (mouseableArea);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
/**
 * @param withForget bool to indicate if the view's reference counter should be decreased after removed from the container
 * @return true on success
 */
bool CViewContainer::removeAll (bool withForget)
{
	if (pImpl->mouseDownView)
		pImpl->mouseDownView = nullptr;
	pImpl->currentDragView = nullptr;
	
	ViewList::iterator it = pImpl->children.begin ();
	while (it != pImpl->children.end ())
	{
		CView* view = *it;
		if (isAttached ())
			view->removed (this);
		pImpl->children.erase (it);
		view->setSubviewState (false);
		pImpl->viewContainerListeners.forEach ([&] (IViewContainerListener* listener) {
			listener->viewContainerViewRemoved (this, view);
		});
		if (withForget)
			view->forget ();
		it = pImpl->children.begin ();
	}
	return true;
}

//-----------------------------------------------------------------------------
/**
 * @param pView the view which should be removed from the container
 * @param withForget bool to indicate if the view's reference counter should be decreased after removed from the container
 * @return true on success
 */
bool CViewContainer::removeView (CView *pView, bool withForget)
{
	ViewList::iterator it = std::find (pImpl->children.begin (), pImpl->children.end (), pView);
	if (it != pImpl->children.end ())
	{
		pView->invalid ();
		if (pView == pImpl->mouseDownView)
			pImpl->mouseDownView = nullptr;
		if (pView == pImpl->currentDragView)
			pImpl->currentDragView = nullptr;
		if (isAttached ())
			pView->removed (this);
		pView->setSubviewState (false);
		pImpl->viewContainerListeners.forEach ([&] (IViewContainerListener* listener) {
			listener->viewContainerViewRemoved (this, pView);
		});
		if (withForget)
			pView->forget ();
		pImpl->children.erase (it);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
/**
 * @param pView the view which should be checked if it is a child of this container
 * @return true on success
 */
bool CViewContainer::isChild (CView* pView) const
{
	return isChild (pView, false);
}

//-----------------------------------------------------------------------------
bool CViewContainer::isChild (CView *pView, bool deep) const
{
	bool found = false;

	if (deep)
	{
		ChildViewConstIterator it = pImpl->children.begin ();
		while (!found && it != pImpl->children.end ())
		{
			CView* v = (*it);
			if (pView == v)
			{
				found = true;
				break;
			}
			if (CViewContainer* container = v->asViewContainer ())
				found = container->isChild (pView, true);
			++it;
		}
	}
	else
	{
		found = std::find (pImpl->children.begin (), pImpl->children.end (), pView) != pImpl->children.end ();
	}
	return found;
}

//-----------------------------------------------------------------------------
bool CViewContainer::hasChildren () const
{
	return pImpl->children.size () > 0;
}

//-----------------------------------------------------------------------------
/**
 * @return number of subviews
 */
uint32_t CViewContainer::getNbViews () const
{
	return static_cast<uint32_t> (pImpl->children.size ());
}

//-----------------------------------------------------------------------------
/**
 * @param index the index of the view to return
 * @return view at index. NULL if view at index does not exist.
 */
CView* CViewContainer::getView (uint32_t index) const
{
	ChildViewConstIterator it = pImpl->children.begin ();
	std::advance (it, index);
	if (it != pImpl->children.end ())
		return *it;
	return nullptr;
}

//-----------------------------------------------------------------------------
/**
 * @param view view which z order position should be changed
 * @param newIndex index of new z position
 * @return true if z order of view changed
 */
bool CViewContainer::changeViewZOrder (CView* view, uint32_t newIndex)
{
	if (newIndex < getNbViews ())
	{
		ViewList::iterator it = std::find (pImpl->children.begin (), pImpl->children.end (), view);
		if (it != pImpl->children.end ())
		{
			pImpl->children.erase (it);
			it = pImpl->children.begin ();
			std::advance (it, newIndex);
			bool result = pImpl->children.insert (it, view) != pImpl->children.end ();
			if (result)
			{
				pImpl->viewContainerListeners.forEach ([&] (IViewContainerListener* listener) {
					listener->viewContainerViewZOrderChanged (this, view);
				});
			}
			return result;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
bool CViewContainer::invalidateDirtyViews ()
{
	if (!isVisible ())
		return true;
	if (CView::isDirty ())
	{
		if (getParentView ())
			getParentView ()->invalidRect (getViewSize ());
		return true;
	}
	for (const auto& pV : pImpl->children)
	{
		if (pV->isDirty () && pV->isVisible ())
		{
			if (CViewContainer* container = pV->asViewContainer ())
				container->invalidateDirtyViews ();
			else
				pV->invalid ();
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
void CViewContainer::invalid ()
{
	if (!isVisible ())
		return;
	CRect _rect (getViewSize ());
	if (getParentView ())
		getParentView ()->invalidRect (_rect);
}

//-----------------------------------------------------------------------------
void CViewContainer::invalidRect (const CRect& rect)
{
	if (!isVisible ())
		return;
	CRect _rect (rect);
	pImpl->transform.transform (_rect);
	_rect.offset (getViewSize ().left, getViewSize ().top);
	_rect.bound (getViewSize ());
	if (_rect.isEmpty ())
		return;
	if (getParentView ())
		getParentView ()->invalidRect (_rect);
}

//-----------------------------------------------------------------------------
/**
 * @param pContext the context which to use to draw this container and its subviews
 */
void CViewContainer::draw (CDrawContext* pContext)
{
	CViewContainer::drawRect (pContext, getViewSize ());
}

//-----------------------------------------------------------------------------
/**
 * @param pContext the context which to use to draw the background
 * @param _updateRect the area which to draw
 */
void CViewContainer::drawBackgroundRect (CDrawContext* pContext, const CRect& _updateRect)
{
	if (getDrawBackground ())
	{
		CRect oldClip;
		pContext->getClipRect (oldClip);
		CRect newClip (_updateRect);
		newClip.bound (oldClip);
		pContext->setClipRect (newClip);
		CRect tr (0, 0, getViewSize ().getWidth (), getViewSize ().getHeight ());
		getDrawBackground ()->draw (pContext, tr, pImpl->backgroundOffset);
		pContext->setClipRect (oldClip);
	}
	else if ((pImpl->backgroundColor.alpha != 255 && getTransparency ()) || !getTransparency ())
	{
		pContext->setDrawMode (kAliasing);
		pContext->setLineWidth (1);
		pContext->setFillColor (pImpl->backgroundColor);
		pContext->setFrameColor (pImpl->backgroundColor);
		pContext->setLineStyle (kLineSolid);
		CRect r;
		if (pImpl->backgroundColorDrawStyle == kDrawFilled || (pImpl->backgroundColorDrawStyle == kDrawFilledAndStroked && pImpl->backgroundColor.alpha == 255))
		{
			r = _updateRect;
			r.extend (1, 1);
		}
		else
		{
			r = getViewSize ();
			r.offset (-r.left, -r.top);
		}
		pContext->drawRect (r, pImpl->backgroundColorDrawStyle);
	}
}

//-----------------------------------------------------------------------------
/**
 * @param pContext the context which to use to draw
 * @param updateRect the area which to draw
 */
void CViewContainer::drawRect (CDrawContext* pContext, const CRect& updateRect)
{
	CPoint offset (getViewSize ().left, getViewSize ().top);
	CDrawContext::Transform offsetTransform (*pContext, CGraphicsTransform ().translate (offset.x, offset.y));

	CRect _updateRect (updateRect);
	_updateRect.bound (getViewSize ());

	CRect clientRect (_updateRect);
	clientRect.offset (-getViewSize ().left, -getViewSize ().top);

	CRect oldClip;
	pContext->getClipRect (oldClip);
	CRect oldClip2 (oldClip);
	
	CRect newClip (clientRect);
	newClip.bound (oldClip);
	pContext->setClipRect (newClip);
	
	// draw the background
	drawBackgroundRect (pContext, clientRect);
	
	CView* _focusView = nullptr;
	IFocusDrawing* _focusDrawing = nullptr;
	if (getFrame ()->focusDrawingEnabled () && isChild (getFrame ()->getFocusView (), false) && getFrame ()->getFocusView ()->isVisible () && getFrame ()->getFocusView ()->wantsFocus ())
	{
		_focusView = getFrame ()->getFocusView ();
		_focusDrawing = dynamic_cast<IFocusDrawing*> (_focusView);
	}

	{
		CDrawContext::Transform tr (*pContext, pImpl->transform);
		pImpl->transform.inverse ().transform (newClip);
		pImpl->transform.inverse ().transform (clientRect);
		pImpl->transform.transform (oldClip2);
		
		// draw each view
		for (const auto& pV : pImpl->children)
		{
			if (pV->isVisible ())
			{
				if (_focusDrawing && _focusView == pV && !_focusDrawing->drawFocusOnTop ())
				{
					SharedPointer<CGraphicsPath> focusPath = owned (pContext->createGraphicsPath ());
					if (focusPath)
					{
						if (_focusDrawing->getFocusPath (*focusPath))
						{
							pImpl->lastDrawnFocus = focusPath->getBoundingBox ();
							if (!pImpl->lastDrawnFocus.isEmpty ())
							{
								pContext->setClipRect (oldClip2);
								pContext->setDrawMode (kAntiAliasing|kNonIntegralMode);
								pContext->setFillColor (getFrame ()->getFocusColor ());
								pContext->drawGraphicsPath (focusPath, CDrawContext::kPathFilledEvenOdd);
								pImpl->lastDrawnFocus = focusPath->getBoundingBox ();
								pImpl->lastDrawnFocus.extend (1, 1);
							}
							_focusDrawing = nullptr;
							_focusView = nullptr;
						}
					}
				}

				if (checkUpdateRect (pV, clientRect))
				{
					CRect viewSize = pV->getViewSize ();
					viewSize.bound (newClip);
					if (viewSize.getWidth () == 0 || viewSize.getHeight () == 0)
						continue;
					pContext->setClipRect (viewSize);
					float globalContextAlpha = pContext->getGlobalAlpha ();
					pContext->setGlobalAlpha (globalContextAlpha * pV->getAlphaValue ());
					pV->drawRect (pContext, viewSize);
					pContext->setGlobalAlpha (globalContextAlpha);
				}
			}
		}
	}
	
	pContext->setClipRect (oldClip2);

	if (_focusView)
	{
		SharedPointer<CGraphicsPath> focusPath = owned (pContext->createGraphicsPath ());
		if (focusPath)
		{
			if (_focusDrawing)
				_focusDrawing->getFocusPath (*focusPath);
			else
			{
				CCoord focusWidth = getFrame ()->getFocusWidth ();
				CRect r (_focusView->getVisibleViewSize ());
				if (!r.isEmpty ())
				{
					focusPath->addRect (r);
					r.extend (focusWidth, focusWidth);
					focusPath->addRect (r);
				}
			}
			pImpl->lastDrawnFocus = focusPath->getBoundingBox ();
			if (!pImpl->lastDrawnFocus.isEmpty ())
			{
				pContext->setDrawMode (kAntiAliasing|kNonIntegralMode);
				pContext->setFillColor (getFrame ()->getFocusColor ());
				pContext->drawGraphicsPath (focusPath, CDrawContext::kPathFilledEvenOdd);
				pImpl->lastDrawnFocus = focusPath->getBoundingBox ();
				pImpl->lastDrawnFocus.extend (1, 1);
			}
		}
	}
	
	setDirty (false);
}

//-----------------------------------------------------------------------------
/**
 * check if view needs to be updated for rect
 * @param view view to check
 * @param rect update rect
 * @return true if view needs update
 */
bool CViewContainer::checkUpdateRect (CView* view, const CRect& rect)
{
	return view->checkUpdate (rect) && view->isVisible ();
}

//-----------------------------------------------------------------------------
/**
 * @param where point
 * @param buttons mouse button and modifier state
 * @return true if any sub view accepts the hit
 */
bool CViewContainer::hitTestSubViews (const CPoint& where, const CButtonState& buttons)
{
	CPoint where2 (where);
	where2.offset (-getViewSize ().left, -getViewSize ().top);
	pImpl->transform.inverse ().transform (where2);

	FOREACHSUBVIEW_REVERSE(true)
		if (pV && pV->isVisible () && pV->getMouseEnabled () && pV->hitTest (where2, buttons))
		{
			if (auto container = pV->asViewContainer ())
			{
				if (container->hitTestSubViews (where2, buttons))
					return true;
			}
			else
				return true;
		}
	ENDFOREACHSUBVIEW
	return false;
}

//-----------------------------------------------------------------------------
/**
 * @param where point
 * @param buttons mouse button and modifier state
 * @return true if container accepts the hit
 */
bool CViewContainer::hitTest (const CPoint& where, const CButtonState& buttons)
{
	CPoint where2 (where);
	//return hitTestSubViews (where); would change default behavior
	return CView::hitTest (where2, buttons);
}

//-----------------------------------------------------------------------------
CMouseEventResult CViewContainer::onMouseDown (CPoint &where, const CButtonState& buttons)
{
	// convert to relativ pos
	CPoint where2 (where);
	where2.offset (-getViewSize ().left, -getViewSize ().top);
	pImpl->transform.inverse ().transform (where2);

	FOREACHSUBVIEW_REVERSE(true)
		if (pV && pV->isVisible () && pV->getMouseEnabled () && pV->hitTest (where2, buttons))
		{
			CControl* control = dynamic_cast<CControl*> (pV);
			if (control && control->getListener () && buttons & (kAlt | kShift | kControl | kApple | kRButton))
			{
				if (control->getListener ()->controlModifierClicked (control, buttons) != 0)
					return kMouseEventHandled;
			}
			CBaseObjectGuard crg (pV);

			if (pV->wantsFocus ())
				getFrame ()->setFocusView (pV);

			CMouseEventResult result = pV->onMouseDown (where2, buttons);
			if (result != kMouseEventNotHandled && result != kMouseEventNotImplemented)
			{
				if (pV->getNbReference () > 1 && result == kMouseEventHandled)
					pImpl->mouseDownView = pV;
				return result;
			}
			if (!pV->getTransparency ())
				return result;
		}
	ENDFOREACHSUBVIEW
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CViewContainer::onMouseUp (CPoint &where, const CButtonState& buttons)
{
	if (pImpl->mouseDownView)
	{
		CBaseObjectGuard crg (pImpl->mouseDownView);

		CPoint where2 (where);
		where2.offset (-getViewSize ().left, -getViewSize ().top);
		pImpl->transform.inverse ().transform (where2);
		pImpl->mouseDownView->onMouseUp (where2, buttons);
		pImpl->mouseDownView = nullptr;
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CViewContainer::onMouseMoved (CPoint &where, const CButtonState& buttons)
{
	if (pImpl->mouseDownView)
	{
		CBaseObjectGuard crg (pImpl->mouseDownView);

		CPoint where2 (where);
		where2.offset (-getViewSize ().left, -getViewSize ().top);
		pImpl->transform.inverse ().transform (where2);
		CMouseEventResult mouseResult = pImpl->mouseDownView->onMouseMoved (where2, buttons);
		if (mouseResult != kMouseEventHandled && mouseResult != kMouseEventNotImplemented)
		{
			pImpl->mouseDownView = nullptr;
			return kMouseEventNotHandled;
		}
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CViewContainer::onMouseCancel ()
{
	if (pImpl->mouseDownView)
	{
		CBaseObjectGuard crg (pImpl->mouseDownView);
		return pImpl->mouseDownView->onMouseCancel ();
	}
	return kMouseEventHandled;
}

//-----------------------------------------------------------------------------
bool CViewContainer::onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const CButtonState &buttons)
{
	FOREACHSUBVIEW_REVERSE(true)
		CPoint where2 (where);
		where2.offset (-getViewSize ().left, -getViewSize ().top);
		pImpl->transform.inverse ().transform (where2);
		if (pV && pV->isVisible () && pV->getMouseEnabled () && pV->getMouseableArea ().pointInside (where2))
		{
			if (pV->onWheel (where2, axis, distance, buttons))
				return true;
			if (!pV->getTransparency ())
				return false;
		}
	ENDFOREACHSUBVIEW
	return false;
}

//-----------------------------------------------------------------------------
bool CViewContainer::onWheel (const CPoint &where, const float &distance, const CButtonState &buttons)
{
	return onWheel (where, kMouseWheelAxisY, distance, buttons);
}

//-----------------------------------------------------------------------------
bool CViewContainer::onDrop (IDataPackage* drag, const CPoint& where)
{
	bool result = false;

	CPoint where2 (where);
	where2.offset (-getViewSize ().left, -getViewSize ().top);
	pImpl->transform.inverse ().transform (where2);

	CView* view = getViewAt (where, GetViewOptions (GetViewOptions::kMouseEnabled|GetViewOptions::kIncludeViewContainer));
	if (view != pImpl->currentDragView)
	{
		if (pImpl->currentDragView)
			pImpl->currentDragView->onDragLeave (drag, where2);
		pImpl->currentDragView = view;
	}
	if (pImpl->currentDragView)
	{
		result = pImpl->currentDragView->onDrop (drag, where2);
		pImpl->currentDragView->onDragLeave (drag, where2);
	}
	pImpl->currentDragView = nullptr;
	
	return result;
}

//-----------------------------------------------------------------------------
void CViewContainer::onDragEnter (IDataPackage* drag, const CPoint& where)
{
	CPoint where2 (where);
	where2.offset (-getViewSize ().left, -getViewSize ().top);
	pImpl->transform.inverse ().transform (where2);

	if (pImpl->currentDragView)
		pImpl->currentDragView->onDragLeave (drag, where2);
	CView* view = getViewAt (where, GetViewOptions (GetViewOptions::kMouseEnabled|GetViewOptions::kIncludeViewContainer));
	pImpl->currentDragView = view;
	if (view)
		view->onDragEnter (drag, where2);
}

//-----------------------------------------------------------------------------
void CViewContainer::onDragLeave (IDataPackage* drag, const CPoint& where)
{
	CPoint where2 (where);
	where2.offset (-getViewSize ().left, -getViewSize ().top);
	pImpl->transform.inverse ().transform (where2);

	if (pImpl->currentDragView)
		pImpl->currentDragView->onDragLeave (drag, where2);
	pImpl->currentDragView = nullptr;
}

//-----------------------------------------------------------------------------
void CViewContainer::onDragMove (IDataPackage* drag, const CPoint& where)
{
	CPoint where2 (where);
	where2.offset (-getViewSize ().left, -getViewSize ().top);
	pImpl->transform.inverse ().transform (where2);

	CView* view = getViewAt (where, GetViewOptions (GetViewOptions::kMouseEnabled|GetViewOptions::kIncludeViewContainer));
	if (view != pImpl->currentDragView)
	{
		if (pImpl->currentDragView)
			pImpl->currentDragView->onDragLeave (drag, where2);
		if (view)
			view->onDragEnter (drag, where2);
		pImpl->currentDragView = view;
	}
	else if (pImpl->currentDragView)
		pImpl->currentDragView->onDragMove (drag, where2);
}

#if VSTGUI_TOUCH_EVENT_HANDLING
//-----------------------------------------------------------------------------
void CViewContainer::onTouchEvent (ITouchEvent& event)
{
	ReverseViewIterator it (this);
	while (*it)
	{
		CView* view = *it;
		CBaseObjectGuard guard (view);
		if (view->wantsMultiTouchEvents ())
		{
			for (const auto& e : event)
			{
				if (e.second.state == ITouchEvent::kBegan && e.second.target == 0)
				{
					CButtonState buttons (kLButton + (e.second.tapCount > 1 ? kDoubleClick : 0));
					CPoint where (e.second.location);
					frameToLocal (where);
					if (view->hitTest (where, buttons))
					{
						view->onTouchEvent (event);
						break;
					}
				}
			}
		}
		it++;
	}
}

//-----------------------------------------------------------------------------
void CViewContainer::findSingleTouchEventTarget (ITouchEvent::Touch& event)
{
	vstgui_assert(event.target == 0);
	vstgui_assert(event.state == ITouchEvent::kBegan);

	CButtonState buttons (kLButton + (event.tapCount > 1 ? kDoubleClick : 0));
	CPoint where (event.location);
	frameToLocal (where);
	ReverseViewIterator it (this);
	while (*it)
	{
		CView* view = *it;
		CBaseObjectGuard guard (view);
		if (view->getMouseEnabled () && view->isVisible () && view->hitTest (where, buttons))
		{
			if (auto container = view->asViewContainer ())
			{
				container->findSingleTouchEventTarget (event);
				if (event.target != 0)
					return;
			}
			else
			{
				CMouseEventResult result = view->onMouseDown (where, buttons);
				if (result == kMouseEventHandled)
				{
					event.target = view;
					event.targetIsSingleTouch = true;
					return;
				}
				else if (result == kMouseDownEventHandledButDontNeedMovedOrUpEvents)
				{
					return;
				}
			}
		}
		it++;
	}
}

#endif

//-----------------------------------------------------------------------------
void CViewContainer::looseFocus ()
{
	CView::looseFocus ();
}

//-----------------------------------------------------------------------------
void CViewContainer::takeFocus ()
{
	CView::takeFocus ();
}

//-----------------------------------------------------------------------------
/**
 * @param oldFocus old focus view
 * @param reverse search order
 * @return true on success
 */
bool CViewContainer::advanceNextFocusView (CView* oldFocus, bool reverse)
{
	if (getFrame ())
	{
		bool foundOld = false;
		FOREACHSUBVIEW_REVERSE(reverse)
			if (oldFocus && !foundOld)
			{
				if (oldFocus == pV)
				{
					foundOld = true;
					continue;
				}
			}
			else
			{
				if (pV->wantsFocus () && pV->getMouseEnabled () && pV->isVisible ())
				{
					getFrame ()->setFocusView (pV);
					return true;
				}
				else if (CViewContainer* container = pV->asViewContainer ())
				{
					if (container->advanceNextFocusView (nullptr, reverse))
						return true;
				}
			}
		ENDFOREACHSUBVIEW
	}
	return false;
}

//-----------------------------------------------------------------------------
bool CViewContainer::isDirty () const
{
	if (CView::isDirty ())
		return true;
		
	CRect viewSize (getViewSize ());
	viewSize.offset (-getViewSize ().left, -getViewSize ().top);

	for (const auto& pV : pImpl->children)
	{
		if (pV->isDirty () && pV->isVisible ())
		{
			CRect r = pV->getViewSize ();
			r.bound (viewSize);
			if (r.getWidth () > 0 && r.getHeight () > 0)
				return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
/**
 * @param p location
 * @param options search options
 * @return view at position p or null
 */
CView* CViewContainer::getViewAt (const CPoint& p, const GetViewOptions& options) const
{
	CPoint where (p);
	where.offset (-getViewSize ().left, -getViewSize ().top);
	pImpl->transform.inverse ().transform (where);

	FOREACHSUBVIEW_REVERSE(true)
		if (pV && pV->getMouseableArea ().pointInside (where))
		{
			if (!options.includeInvisible () && pV->isVisible () == false)
				continue;
			if (options.mouseEnabled ())
			{
				if (pV->getMouseEnabled () == false)
					continue;
			}
			if (options.deep ())
			{
				if (auto container = pV->asViewContainer ())
				{
					CView* view = container->getViewAt (where, options);
					return options.includeViewContainer () ? (view ? view : container) : view;
				}
			}
			if (!options.includeViewContainer () && pV->asViewContainer ())
				continue;
			return pV;
		}
	ENDFOREACHSUBVIEW

	return nullptr;
}

//-----------------------------------------------------------------------------
/**
 * @param p location
 * @param views result list
 * @param options search options
 * @return success
 */
bool CViewContainer::getViewsAt (const CPoint& p, ViewList& views, const GetViewOptions& options) const
{
	bool result = false;

	CPoint where (p);
	where.offset (-getViewSize ().left, -getViewSize ().top);
	pImpl->transform.inverse ().transform (where);

	FOREACHSUBVIEW_REVERSE(true)
		if (pV && pV->getMouseableArea ().pointInside (where))
		{
			if (!options.includeInvisible () && pV->isVisible () == false)
				continue;
			if (options.mouseEnabled ())
			{
				if (pV->getMouseEnabled () == false)
					continue;
			}
			if (options.deep ())
			{
				if (CViewContainer* container = pV->asViewContainer ())
					result |= container->getViewsAt (where, views, options);
			}
			if (options.includeViewContainer () == false)
			{
				if (pV->asViewContainer ())
					continue;
			}
			views.emplace_back (pV);
			result = true;
		}
	ENDFOREACHSUBVIEW

	return result;
}

//-----------------------------------------------------------------------------
/**
 * @param p location
 * @param options search search options
 * @return view container at position p or null
 */
CViewContainer* CViewContainer::getContainerAt (const CPoint& p, const GetViewOptions& options) const
{
	CPoint where (p);
	where.offset (-getViewSize ().left, -getViewSize ().top);
	pImpl->transform.inverse ().transform (where);

	FOREACHSUBVIEW_REVERSE(true)
		if (pV && pV->getMouseableArea ().pointInside (where))
		{
			if (!options.includeInvisible () && pV->isVisible () == false)
				continue;
			if (options.mouseEnabled ())
			{
				if (pV->getMouseEnabled() == false)
					continue;
			}
			if (options.deep ())
			{
				if (CViewContainer* container = pV->asViewContainer ())
					return container->getContainerAt (where, options);
			}
			break;
		}
	ENDFOREACHSUBVIEW

	return const_cast<CViewContainer*>(this);
}

//-----------------------------------------------------------------------------
CPoint& CViewContainer::frameToLocal (CPoint& point) const
{
	point.offset (-getViewSize ().left, -getViewSize ().top);
	if (getParentView ())
		return getParentView ()->frameToLocal (point);
	return point;
}

//-----------------------------------------------------------------------------
CPoint& CViewContainer::localToFrame (CPoint& point) const
{
	point.offset (getViewSize ().left, getViewSize ().top);
	if (getParentView ())
		return getParentView ()->localToFrame (point);
	return point;
}

//-----------------------------------------------------------------------------
bool CViewContainer::removed (CView* parent)
{
	if (!isAttached ())
		return false;

	for (const auto& pV : pImpl->children)
		pV->removed (this);
	
	return CView::removed (parent);
}

//-----------------------------------------------------------------------------
bool CViewContainer::attached (CView* parent)
{
	if (isAttached ())
		return false;

	setParentFrame (parent->getFrame ());

	bool result = CView::attached (parent);
	if (result)
	{
		for (const auto& pV : pImpl->children)
			pV->attached (this);
	}
	return result;
}

#if DEBUG
static int32_t _debugDumpLevel = 0;
//-----------------------------------------------------------------------------
void CViewContainer::dumpInfo ()
{
	CView::dumpInfo ();
}

//-----------------------------------------------------------------------------
void CViewContainer::dumpHierarchy ()
{
	_debugDumpLevel++;
	for (auto& pV : pImpl->children)
	{
		for (int32_t i = 0; i < _debugDumpLevel; i++)
			DebugPrint ("\t");
		pV->dumpInfo ();
		DebugPrint ("\n");
		if (CViewContainer* container = pV->asViewContainer ())
			container->dumpHierarchy ();
	}
	_debugDumpLevel--;
}

#endif

} // namespace
