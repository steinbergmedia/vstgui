//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
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

#include "cviewcontainer.h"
#include "coffscreencontext.h"
#include "cbitmap.h"
#include "cframe.h"
#include "ifocusdrawing.h"
#include "itouchevent.h"
#include "iviewlistener.h"
#include "cgraphicspath.h"
#include "controls/ccontrol.h"

#include <algorithm>
#include <cassert>

namespace VSTGUI {

#ifndef FOREACHSUBVIEW_REVERSE
	#define FOREACHSUBVIEW_REVERSE(reverse) ChildViewConstIterator it; ChildViewConstReverseIterator rit; if (reverse) rit = children.rbegin (); else it = children.begin (); while (reverse ? rit != children.rend () : it != children.end ()) { CView* pV; if (reverse) {	pV = (*rit); rit++; } else { pV = (*it); it++; } {
#endif

#if VSTGUI_ENABLE_DEPRECATED_METHODS
IdStringPtr kMsgCheckIfViewContainer	= "kMsgCheckIfViewContainer";
#endif
IdStringPtr kMsgLooseFocus = "LooseFocus";

namespace CViewContainerPrivate {

//-----------------------------------------------------------------------------
struct ViewContainerListenerCall
{
	CViewContainer* container;
	ViewContainerListenerCall (CViewContainer* container) : container (container) {}
};

//-----------------------------------------------------------------------------
struct ViewContainerTransformChanged : ViewContainerListenerCall
{
	ViewContainerTransformChanged (CViewContainer* container) : ViewContainerListenerCall (container) {}
	void operator () (IViewContainerListener* listener) const
	{
		listener->viewContainerTransformChanged (container);
	}
};

//-----------------------------------------------------------------------------
struct ViewContainerViewAdded : ViewContainerListenerCall
{
	CView* view;
	ViewContainerViewAdded (CViewContainer* container, CView* view) : ViewContainerListenerCall (container), view (view) {}
	void operator () (IViewContainerListener* listener) const
	{
		listener->viewContainerViewAdded (container, view);
	}
};

//-----------------------------------------------------------------------------
struct ViewContainerViewRemoved : ViewContainerListenerCall
{
	CView* view;
	ViewContainerViewRemoved (CViewContainer* container, CView* view) : ViewContainerListenerCall (container), view (view) {}
	void operator () (IViewContainerListener* listener) const
	{
		listener->viewContainerViewRemoved (container, view);
	}
};

//-----------------------------------------------------------------------------
struct ViewContainerViewZOrderChanged : ViewContainerListenerCall
{
	CView* view;
	ViewContainerViewZOrderChanged (CViewContainer* container, CView* view) : ViewContainerListenerCall (container), view (view) {}
	void operator () (IViewContainerListener* listener) const
	{
		listener->viewContainerViewZOrderChanged (container, view);
	}
};

}

//-----------------------------------------------------------------------------
// CViewContainer Implementation
//-----------------------------------------------------------------------------
/**
 * CViewContainer constructor.
 * @param rect the size of the container
 */
CViewContainer::CViewContainer (const CRect &rect)
: CView (rect)
, currentDragView (0)
, mouseDownView (0)
, backgroundColorDrawStyle (kDrawFilledAndStroked)
{
	backgroundOffset (0, 0);
	backgroundColor = kBlackCColor;
	setAutosizingEnabled (true);
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
/**
 * CViewContainer constructor.
 * @param rect the size of the container
 * @param pParent (unused)
 * @param pBackground the background bitmap, can be NULL
 */
CViewContainer::CViewContainer (const CRect &rect, CFrame* pParent, CBitmap* pBackground)
: CView (rect)
, currentDragView (0)
, mouseDownView (0)
, backgroundColorDrawStyle (kDrawFilledAndStroked)
{
	backgroundOffset (0, 0);
	setBackground (pBackground);
	backgroundColor = kBlackCColor;
	setAutosizingEnabled (true);
}
#endif

//-----------------------------------------------------------------------------
CViewContainer::CViewContainer (const CViewContainer& v)
: CView (v)
, backgroundColor (v.backgroundColor)
, backgroundOffset (v.backgroundOffset)
, backgroundColorDrawStyle (v.backgroundColorDrawStyle)
, currentDragView (0)
, mouseDownView (0)
{
	ViewIterator it (&v);
	while (*it)
	{
		addView (static_cast<CView*>((*it)->newCopy ()));
		++it;
	}
}

//-----------------------------------------------------------------------------
CViewContainer::~CViewContainer ()
{
	// remove all views
	removeAll (true);
	vstgui_assert (viewContainerListeners.empty ());
}

//-----------------------------------------------------------------------------
void CViewContainer::registerViewContainerListener (IViewContainerListener* listener)
{
	viewContainerListeners.add (listener);
}

//-----------------------------------------------------------------------------
void CViewContainer::unregisterViewContainerListener (IViewContainerListener* listener)
{
	viewContainerListeners.remove (listener);
}

//-----------------------------------------------------------------------------
void CViewContainer::parentSizeChanged ()
{
	FOREACHSUBVIEW
		pV->parentSizeChanged ();	// notify children that the size of the parent or this container has changed
	ENDFOREACHSUBVIEW
}

//-----------------------------------------------------------------------------
void CViewContainer::setTransform (const CGraphicsTransform& t)
{
	if (transform != t)
	{
		transform = t;
		viewContainerListeners.forEach (CViewContainerPrivate::ViewContainerTransformChanged (this));
	}
}

//-----------------------------------------------------------------------------
void CViewContainer::setAutosizingEnabled (bool state)
{
	if (state)
		viewFlags |= kAutosizeSubviews;
	else
		viewFlags &= ~kAutosizeSubviews;
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
			FOREACHSUBVIEW
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
			ENDFOREACHSUBVIEW
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
	if (pParentFrame == this)
	{}
	else if (pParentView)
		result = static_cast<CViewContainer*> (pParentView)->getVisibleSize (result);
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
	FOREACHSUBVIEW
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
	ENDFOREACHSUBVIEW
	
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
	if (color != backgroundColor)
	{
		backgroundColor = color;
		setDirty (true);
	}
}

//------------------------------------------------------------------------------
void CViewContainer::setBackgroundColorDrawStyle (CDrawStyle style)
{
	if (backgroundColorDrawStyle != style)
	{
		backgroundColorDrawStyle = style;
		setDirty (true);
	}
}

//------------------------------------------------------------------------------
CMessageResult CViewContainer::notify (CBaseObject* sender, IdStringPtr message)
{
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	if (message == kMsgCheckIfViewContainer)
		return kMessageNotified;
	else 
#endif
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
		if (!lastDrawnFocus.isEmpty ())
			invalidRect (lastDrawnFocus);
		lastDrawnFocus = CRect (0, 0, 0, 0);
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
	return addView (pView, 0);
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
		ViewList::iterator it = std::find (children.begin (), children.end (), pBefore);
		children.insert (it, pView);
	}
	else
	{
		children.push_back (pView);
	}

	pView->setSubviewState (true);

	viewContainerListeners.forEach (CViewContainerPrivate::ViewContainerViewAdded (this, pView));

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
	if (addView (pView, 0))
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
	if (mouseDownView)
		mouseDownView = 0;
	currentDragView = 0;
	
	ViewList::iterator it = children.begin ();
	while (it != children.end ())
	{
		CView* view = *it;
		children.erase (it);
		if (isAttached ())
			view->removed (this);
		view->setSubviewState (false);
		viewContainerListeners.forEach (CViewContainerPrivate::ViewContainerViewRemoved (this, view));
		if (withForget)
			view->forget ();
		it = children.begin ();
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
	ViewList::iterator it = std::find (children.begin (), children.end (), pView);
	if (it != children.end ())
	{
		pView->invalid ();
		if (pView == mouseDownView)
			mouseDownView = 0;
		if (pView == currentDragView)
			currentDragView = 0;
		children.erase (it);
		if (isAttached ())
			pView->removed (this);
		pView->setSubviewState (false);
		viewContainerListeners.forEach (CViewContainerPrivate::ViewContainerViewRemoved (this, pView));
		if (withForget)
			pView->forget ();
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
		ChildViewConstIterator it = children.begin ();
		while (!found && it != children.end ())
		{
			CView* v = (*it);
			if (pView == v)
			{
				found = true;
				break;
			}
			if (CViewContainer* container = dynamic_cast<CViewContainer*>(v))
				found = container->isChild (pView, true);
			it++;
		}
	}
	else
	{
		found = std::find (children.begin (), children.end (), pView) != children.end ();
	}
	return found;
}

//-----------------------------------------------------------------------------
bool CViewContainer::hasChildren () const
{
	return children.size () > 0;
}

//-----------------------------------------------------------------------------
/**
 * @return number of subviews
 */
uint32_t CViewContainer::getNbViews () const
{
	return static_cast<uint32_t> (children.size ());
}

//-----------------------------------------------------------------------------
/**
 * @param index the index of the view to return
 * @return view at index. NULL if view at index does not exist.
 */
CView* CViewContainer::getView (uint32_t index) const
{
	ChildViewConstIterator it = children.begin ();
	std::advance (it, index);
	if (it != children.end ())
		return *it;
	return 0;
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
		ViewList::iterator it = std::find (children.begin (), children.end (), view);
		if (it != children.end ())
		{
			children.erase (it);
			it = children.begin ();
			std::advance (it, newIndex);
			bool result = children.insert (it, view) != children.end ();
			if (result)
				viewContainerListeners.forEach (CViewContainerPrivate::ViewContainerViewZOrderChanged (this, view));
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
		if (pParentView)
			pParentView->invalidRect (getViewSize ());
		return true;
	}
	FOREACHSUBVIEW
		if (pV->isDirty () && pV->isVisible ())
		{
			if (CViewContainer* container = dynamic_cast<CViewContainer*> (pV))
				container->invalidateDirtyViews ();
			else
				pV->invalid ();
		}
	ENDFOREACHSUBVIEW
	return true;
}

//-----------------------------------------------------------------------------
void CViewContainer::invalid ()
{
	if (!isVisible ())
		return;
	CRect _rect (getViewSize ());
	if (pParentView)
		pParentView->invalidRect (_rect);
}

//-----------------------------------------------------------------------------
void CViewContainer::invalidRect (const CRect& rect)
{
	if (!isVisible ())
		return;
	CRect _rect (rect);
	transform.transform (_rect);
	_rect.offset (getViewSize ().left, getViewSize ().top);
	_rect.bound (getViewSize ());
	if (_rect.isEmpty ())
		return;
	if (pParentView)
		pParentView->invalidRect (_rect);
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
		getDrawBackground ()->draw (pContext, tr, backgroundOffset);
		pContext->setClipRect (oldClip);
	}
	else if ((backgroundColor.alpha != 255 && getTransparency ()) || !getTransparency ())
	{
		pContext->setDrawMode (kAliasing);
		pContext->setLineWidth (1);
		pContext->setFillColor (backgroundColor);
		pContext->setFrameColor (backgroundColor);
		pContext->setLineStyle (kLineSolid);
		CRect r;
		if (backgroundColorDrawStyle == kDrawFilled || (backgroundColorDrawStyle == kDrawFilledAndStroked && backgroundColor.alpha == 255))
		{
			r = _updateRect;
			r.extend (1, 1);
		}
		else
		{
			r = getViewSize ();
			r.offset (-r.left, -r.top);
		}
		pContext->drawRect (r, backgroundColorDrawStyle);
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
	
	CView* _focusView = 0;
	IFocusDrawing* _focusDrawing = 0;
	if (getFrame ()->focusDrawingEnabled () && isChild (getFrame ()->getFocusView (), false) && getFrame ()->getFocusView ()->isVisible () && getFrame ()->getFocusView ()->wantsFocus ())
	{
		_focusView = getFrame ()->getFocusView ();
		_focusDrawing = dynamic_cast<IFocusDrawing*> (_focusView);
	}

	{
	CDrawContext::Transform tr (*pContext, transform);
	transform.inverse ().transform (newClip);
	transform.inverse ().transform (clientRect);
	transform.transform (oldClip2);
	
	// draw each view
	FOREACHSUBVIEW
		if (pV->isVisible ())
		{
			if (_focusDrawing && _focusView == pV && !_focusDrawing->drawFocusOnTop ())
			{
				SharedPointer<CGraphicsPath> focusPath = owned (pContext->createGraphicsPath ());
				if (focusPath)
				{
					if (_focusDrawing->getFocusPath (*focusPath))
					{
						lastDrawnFocus = focusPath->getBoundingBox ();
						if (!lastDrawnFocus.isEmpty ())
						{
							pContext->setClipRect (oldClip2);
							pContext->setDrawMode (kAntiAliasing|kNonIntegralMode);
							pContext->setFillColor (getFrame ()->getFocusColor ());
							pContext->drawGraphicsPath (focusPath, CDrawContext::kPathFilledEvenOdd);
							lastDrawnFocus = focusPath->getBoundingBox ();
							lastDrawnFocus.extend (1, 1);
						}
						_focusDrawing = 0;
						_focusView = 0;
					}
				}
			}

			if (checkUpdateRect (pV, clientRect))
			{
				CRect viewSize = pV->getViewSize (viewSize);
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
	ENDFOREACHSUBVIEW
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
			lastDrawnFocus = focusPath->getBoundingBox ();
			if (!lastDrawnFocus.isEmpty ())
			{
				pContext->setDrawMode (kAntiAliasing|kNonIntegralMode);
				pContext->setFillColor (getFrame ()->getFocusColor ());
				pContext->drawGraphicsPath (focusPath, CDrawContext::kPathFilledEvenOdd);
				lastDrawnFocus = focusPath->getBoundingBox ();
				lastDrawnFocus.extend (1, 1);
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
bool CViewContainer::hitTestSubViews (const CPoint& where, const CButtonState buttons)
{
	CPoint where2 (where);
	transform.transform (where2);
	where2.offset (-getViewSize ().left, -getViewSize ().top);

	FOREACHSUBVIEW_REVERSE(true)
		if (pV && pV->isVisible () && pV->getMouseEnabled () && pV->hitTest (where2, buttons))
			return true;
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
	transform.inverse ().transform (where2);

	FOREACHSUBVIEW_REVERSE(true)
		if (pV && pV->isVisible () && pV->getMouseEnabled () && pV->hitTest (where2, buttons))
		{
			CControl* control = dynamic_cast<CControl*> (pV);
			if (control && control->getListener () && buttons & (kAlt | kShift | kControl | kApple | kRButton))
			{
				if (control->getListener ()->controlModifierClicked ((CControl*)pV, buttons) != 0)
					return kMouseEventHandled;
			}
			CBaseObjectGuard crg (pV);

			if (pV->wantsFocus ())
				getFrame ()->setFocusView (pV);

			CMouseEventResult result = pV->onMouseDown (where2, buttons);
			if (result != kMouseEventNotHandled && result != kMouseEventNotImplemented)
			{
				if (pV->getNbReference () > 1 && result == kMouseEventHandled)
					mouseDownView = pV;
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
	if (mouseDownView)
	{
		CBaseObjectGuard crg (mouseDownView);

		CPoint where2 (where);
		where2.offset (-getViewSize ().left, -getViewSize ().top);
		transform.inverse ().transform (where2);
		mouseDownView->onMouseUp (where2, buttons);
		mouseDownView = 0;
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CViewContainer::onMouseMoved (CPoint &where, const CButtonState& buttons)
{
	if (mouseDownView)
	{
		CBaseObjectGuard crg (mouseDownView);

		CPoint where2 (where);
		where2.offset (-getViewSize ().left, -getViewSize ().top);
		transform.inverse ().transform (where2);
		CMouseEventResult mouseResult = mouseDownView->onMouseMoved (where2, buttons);
		if (mouseResult != kMouseEventHandled && mouseResult != kMouseEventNotImplemented)
		{
			mouseDownView = 0;
			return kMouseEventNotHandled;
		}
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CViewContainer::onMouseCancel ()
{
	if (mouseDownView)
	{
		CBaseObjectGuard crg (mouseDownView);
		return mouseDownView->onMouseCancel ();
	}
	return kMouseEventHandled;
}

//-----------------------------------------------------------------------------
bool CViewContainer::onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const CButtonState &buttons)
{
	FOREACHSUBVIEW_REVERSE(true)
		CPoint where2 (where);
		where2.offset (-getViewSize ().left, -getViewSize ().top);
		transform.inverse ().transform (where2);
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
	transform.inverse ().transform (where2);

	CView* view = getViewAt (where, GetViewOptions (GetViewOptions::kMouseEnabled|GetViewOptions::kIncludeViewContainer));
	if (view != currentDragView)
	{
		if (currentDragView)
			currentDragView->onDragLeave (drag, where2);
		currentDragView = view;
	}
	if (currentDragView)
	{
		result = currentDragView->onDrop (drag, where2);
		currentDragView->onDragLeave (drag, where2);
	}
	currentDragView = 0;
	
	return result;
}

//-----------------------------------------------------------------------------
void CViewContainer::onDragEnter (IDataPackage* drag, const CPoint& where)
{
	CPoint where2 (where);
	where2.offset (-getViewSize ().left, -getViewSize ().top);
	transform.inverse ().transform (where2);

	if (currentDragView)
		currentDragView->onDragLeave (drag, where2);
	CView* view = getViewAt (where, GetViewOptions (GetViewOptions::kMouseEnabled|GetViewOptions::kIncludeViewContainer));
	currentDragView = view;
	if (view)
		view->onDragEnter (drag, where2);
}

//-----------------------------------------------------------------------------
void CViewContainer::onDragLeave (IDataPackage* drag, const CPoint& where)
{
	CPoint where2 (where);
	where2.offset (-getViewSize ().left, -getViewSize ().top);
	transform.inverse ().transform (where2);

	if (currentDragView)
		currentDragView->onDragLeave (drag, where2);
	currentDragView = 0;
}

//-----------------------------------------------------------------------------
void CViewContainer::onDragMove (IDataPackage* drag, const CPoint& where)
{
	CPoint where2 (where);
	where2.offset (-getViewSize ().left, -getViewSize ().top);
	transform.inverse ().transform (where2);

	CView* view = getViewAt (where, GetViewOptions (GetViewOptions::kMouseEnabled|GetViewOptions::kIncludeViewContainer));
	if (view != currentDragView)
	{
		if (currentDragView)
			currentDragView->onDragLeave (drag, where2);
		if (view)
			view->onDragEnter (drag, where2);
		currentDragView = view;
	}
	else if (currentDragView)
		currentDragView->onDragMove (drag, where2);
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
			CViewContainer* container = dynamic_cast<CViewContainer*>(view);
			if (container)
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
	FOREACHSUBVIEW
		pV->looseFocus ();
	ENDFOREACHSUBVIEW
}

//-----------------------------------------------------------------------------
void CViewContainer::takeFocus ()
{
	FOREACHSUBVIEW
		pV->takeFocus ();
	ENDFOREACHSUBVIEW
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
				else if (CViewContainer* container = dynamic_cast<CViewContainer*> (pV))
				{
					if (container->advanceNextFocusView (0, reverse))
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

	FOREACHSUBVIEW
		if (pV->isDirty () && pV->isVisible ())
		{
			CRect r = pV->getViewSize (r);
			r.bound (viewSize);
			if (r.getWidth () > 0 && r.getHeight () > 0)
				return true;
		}
	ENDFOREACHSUBVIEW
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
	transform.inverse ().transform (where);

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
				if (CViewContainer* container = dynamic_cast<CViewContainer*> (pV))
				{
					CView* view = container->getViewAt (where, options);
					return options.includeViewContainer () ? (view ? view : container) : view;
				}
			}
			if (!options.includeViewContainer () && dynamic_cast<CViewContainer*> (pV))
				continue;
			return pV;
		}
	ENDFOREACHSUBVIEW

	return 0;
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
	transform.inverse ().transform (where);

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
				if (CViewContainer* container = dynamic_cast<CViewContainer*> (pV))
					result |= container->getViewsAt (where, views, options);
			}
			if (options.includeViewContainer () == false)
			{
				if (dynamic_cast<CViewContainer*> (pV))
					continue;
			}
			views.push_back (pV);
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
	transform.inverse ().transform (where);

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
				if (CViewContainer* container = dynamic_cast<CViewContainer*> (pV))
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
	if (pParentView)
		return pParentView->frameToLocal (point);
	return point;
}

//-----------------------------------------------------------------------------
CPoint& CViewContainer::localToFrame (CPoint& point) const
{
	point.offset (getViewSize ().left, getViewSize ().top);
	if (pParentView)
		return pParentView->localToFrame (point);
	return point;
}

//-----------------------------------------------------------------------------
bool CViewContainer::removed (CView* parent)
{
	if (!isAttached ())
		return false;

	FOREACHSUBVIEW
		pV->removed (this);
	ENDFOREACHSUBVIEW
	
	return CView::removed (parent);
}

//-----------------------------------------------------------------------------
bool CViewContainer::attached (CView* parent)
{
	if (isAttached ())
		return false;

	pParentFrame = parent->getFrame ();

	bool result = CView::attached (parent);
	if (result)
	{
		FOREACHSUBVIEW
			pV->attached (this);
		ENDFOREACHSUBVIEW
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
	FOREACHSUBVIEW
		for (int32_t i = 0; i < _debugDumpLevel; i++)
			DebugPrint ("\t");
		pV->dumpInfo ();
		DebugPrint ("\n");
		if (CViewContainer* container = dynamic_cast<CViewContainer*> (pV))
			container->dumpHierarchy ();
	ENDFOREACHSUBVIEW
	_debugDumpLevel--;
}

#endif

} // namespace
