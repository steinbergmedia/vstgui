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
#include "controls/icontrollistener.h"
#include "cgraphicspath.h"
#include "controls/ccontrol.h"
#include "dragging.h"
#include "dispatchlist.h"
#include "events.h"
#include "finally.h"

#include <algorithm>
#include <cassert>

namespace VSTGUI {

IdStringPtr kMsgLooseFocus = "LooseFocus";

const CViewAttributeID kCViewContainerDropTargetAttribute = 'vcdt';
const CViewAttributeID kCViewContainerMouseDownViewAttribute = 'vcmd';
const CViewAttributeID kCViewContainerLastDrawnFocusAttribute = 'vclf';
const CViewAttributeID kCViewContainerBackgroundOffsetAttribute = 'vcbo';

//-----------------------------------------------------------------------------
// CViewContainer Implementation
//-----------------------------------------------------------------------------
struct CViewContainer::Impl
{
	using ViewContainerListenerDispatcher = DispatchList<IViewContainerListener*>;
	
	ViewContainerListenerDispatcher viewContainerListeners;
	CGraphicsTransform transform;
	
	ViewList children;
	
	CDrawStyle backgroundColorDrawStyle {kDrawFilledAndStroked};
	CColor backgroundColor {kBlackCColor};
};

//------------------------------------------------------------------------
struct CViewContainerDropTarget : public IDropTarget, public NonAtomicReferenceCounted
{
	CViewContainerDropTarget (CViewContainer* container) : container (container) {}

	CPoint getLocalPos (const CPoint& pos) const
	{
		auto viewSize = container->getViewSize ();
		CPoint where2 (pos);
		where2.offset (-viewSize.left, -viewSize.top);
		container->getTransform ().inverse ().transform (where2);
		return where2;
	}
	
	DragOperation onDragEnter (DragEventData data) final
	{
		assert (dropTarget == nullptr);
	
		return onDragMove (data);
	}
	
	DragOperation onDragMove (DragEventData data) final
	{
		CView* view = container->getViewAt (
		    data.pos, GetViewOptions ().mouseEnabled ().includeViewContainer ());
		data.pos = getLocalPos (data.pos);
		if (view == currentDragView)
		{
			if (dropTarget)
				return dropTarget->onDragMove (data);
			return DragOperation::None;
		}
		if (currentDragView)
		{
			if (dropTarget)
				dropTarget->onDragLeave (data);
			dropTarget = nullptr;
			currentDragView = nullptr;
		}
		if (view)
		{
			currentDragView = view;
			if ((dropTarget = currentDragView->getDropTarget ()))
			{
				dropTarget->onDragEnter (data);
				return dropTarget->onDragMove (data);
			}
		}
		return DragOperation::None;
	}

	void onDragLeave (DragEventData data) final
	{
		if (currentDragView)
		{
			if (dropTarget)
			{
				data.pos = getLocalPos (data.pos);
				dropTarget->onDragLeave (data);
				dropTarget = nullptr;
			}
			currentDragView = nullptr;
		}
	}

	bool onDrop (DragEventData data) final
	{
		if (dropTarget)
		{
			data.pos = getLocalPos (data.pos);
			auto result = dropTarget->onDrop (data);
			dropTarget = nullptr;
			currentDragView = nullptr;
			return result;
		}
		currentDragView = nullptr;
		return false;
	}

	CViewContainer* container;
	SharedPointer<IDropTarget> dropTarget;
	SharedPointer<CView> currentDragView;
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
	pImpl->transform = v.getTransform ();
	pImpl->backgroundColorDrawStyle = v.pImpl->backgroundColorDrawStyle;
	pImpl->backgroundColor = v.pImpl->backgroundColor;
	setBackgroundOffset (v.getBackgroundOffset ());
	for (auto& view : v.pImpl->children)
		addView (static_cast<CView*> (view->newCopy ()));
}

//-----------------------------------------------------------------------------
CViewContainer::~CViewContainer () noexcept
{
	vstgui_assert (pImpl->viewContainerListeners.empty ());
}

//-----------------------------------------------------------------------------
void CViewContainer::beforeDelete ()
{
	IDropTarget* dropTarget = nullptr;
	if (getAttribute (kCViewContainerDropTargetAttribute, dropTarget))
	{
		removeAttribute (kCViewContainerDropTargetAttribute);
		dropTarget->forget ();
	}

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
	auto mouseDownView = getMouseDownView ();
	if (mouseDownView && mouseDownView != view)
	{
		// make sure the old mouse down view get a mouse cancel or if not implemented a mouse up
		if (auto cvc = mouseDownView->asViewContainer ())
			cvc->setMouseDownView (nullptr);
		else
		{
			MouseCancelEvent cancelEvent;
			mouseDownView->dispatchEvent (cancelEvent);
			if (!cancelEvent.consumed)
			{
				MouseUpEvent upEvent;
				upEvent.mousePosition = mouseDownView->getViewSize ().getTopLeft () - CPoint (10, 10);
				mouseDownView->dispatchEvent (upEvent);
			}
		}
	}
	setAttribute (kCViewContainerMouseDownViewAttribute, view);
}

//-----------------------------------------------------------------------------
CView* CViewContainer::getMouseDownView () const
{
	CView* view = nullptr;
	if (getAttribute (kCViewContainerMouseDownViewAttribute, view))
		return view;
	return nullptr;
}

//-----------------------------------------------------------------------------
void CViewContainer::clearMouseDownView ()
{
	removeAttribute (kCViewContainerMouseDownViewAttribute);
}

//-----------------------------------------------------------------------------
CRect CViewContainer::getLastDrawnFocus () const
{
	CRect r;
	if (getAttribute (kCViewContainerLastDrawnFocusAttribute, r))
		return r;
	return {};
}

//-----------------------------------------------------------------------------
void CViewContainer::setLastDrawnFocus (CRect r)
{
	if (r.isEmpty ())
		removeAttribute (kCViewContainerLastDrawnFocusAttribute);
	else
		setAttribute (kCViewContainerLastDrawnFocusAttribute, r);
}

//-----------------------------------------------------------------------------
auto CViewContainer::getChildren () const -> const ViewList&
{
	return pImpl->children;
}

//-----------------------------------------------------------------------------
void CViewContainer::setTransform (const CGraphicsTransform& t)
{
	if (getTransform () != t)
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
	else if (auto parent = getParentView ())
		result = static_cast<CViewContainer*> (parent)->getVisibleSize (result);
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

	constexpr auto CoordMax = std::numeric_limits<CCoord>::max ();
	constexpr auto CoordMin = -CoordMax;
	CRect bounds (CoordMax, CoordMax, CoordMin, CoordMin);
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

	if (bounds == CRect (CoordMax, CoordMax, CoordMin, CoordMin))
		return false;

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
	if (p == CPoint (0, 0))
		removeAttribute (kCViewContainerBackgroundOffsetAttribute);
	else
		setAttribute (kCViewContainerBackgroundOffsetAttribute, p);
}

//------------------------------------------------------------------------------
CPoint CViewContainer::getBackgroundOffset () const
{
	CPoint p;
	if (getAttribute (kCViewContainerBackgroundOffsetAttribute, p))
		return p;
	return {};
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
		auto* view = dynamic_cast<CView*> (sender);
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
		auto ldf = getLastDrawnFocus ();
		if (!ldf.isEmpty ())
		{
			invalidRect (ldf);
			setLastDrawnFocus (CRect (0, 0, 0, 0));
		}
	}
	return kMessageUnknown;
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

	if (pBefore)
	{
		auto it = std::find (pImpl->children.begin (), pImpl->children.end (), pBefore);
		vstgui_assert (it != pImpl->children.end ());
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
	clearMouseDownView ();
	
	auto it = pImpl->children.begin ();
	while (it != pImpl->children.end ())
	{
		auto view = *it;
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
	auto it = std::find (pImpl->children.begin (), pImpl->children.end (), pView);
	if (it != pImpl->children.end ())
	{
		pView->invalid ();
		if (pView == getMouseDownView ())
			clearMouseDownView ();
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
		auto it = pImpl->children.begin ();
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
	return !pImpl->children.empty ();
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
	auto it = pImpl->children.begin ();
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
		uint32_t oldIndex = 0;
		auto src = pImpl->children.begin ();
		for (;src != pImpl->children.end () && *src != view; ++src, ++oldIndex);
		if (src != pImpl->children.end ())
		{
			if (newIndex == oldIndex)
				return true;
			if (newIndex > oldIndex)
				++newIndex;

			auto dest = pImpl->children.begin ();
			std::advance (dest, newIndex);

			pImpl->children.insert (dest, view);
			pImpl->children.erase (src);

			pImpl->viewContainerListeners.forEach ([&] (IViewContainerListener* listener) {
				listener->viewContainerViewZOrderChanged (this, view);
			});
			return true;
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
		if (auto parent = getParentView ())
			parent->invalidRect (getViewSize ());
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
	if (auto parent = getParentView ())
		parent->invalidRect (_rect);
}

//-----------------------------------------------------------------------------
void CViewContainer::invalidRect (const CRect& rect)
{
	if (!isVisible ())
		return;
	CRect _rect (rect);
	getTransform ().transform (_rect);
	_rect.offset (getViewSize ().left, getViewSize ().top);
	_rect.bound (getViewSize ());
	if (_rect.isEmpty ())
		return;
	if (auto parent = getParentView ())
		parent->invalidRect (_rect);
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
		drawClipped (pContext, _updateRect, [&] () {
			CRect tr (0, 0, getViewSize ().getWidth (), getViewSize ().getHeight ());
			getDrawBackground ()->draw (pContext, tr, getBackgroundOffset ());
		});
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
	auto frame = getFrame ();
	if (frame && frame->focusDrawingEnabled () && isChild (frame->getFocusView (), false) && frame->getFocusView ()->isVisible () && frame->getFocusView ()->wantsFocus ())
	{
		_focusView = frame->getFocusView ();
		_focusDrawing = dynamic_cast<IFocusDrawing*> (_focusView);
	}

	{
		CDrawContext::Transform tr (*pContext, getTransform ());
		getTransform ().inverse ().transform (newClip);
		getTransform ().inverse ().transform (clientRect);
		getTransform ().transform (oldClip2);
		
		// draw each view
		for (const auto& pV : pImpl->children)
		{
			if (pV->isVisible ())
			{
				if (frame && _focusDrawing && _focusView == pV && !_focusDrawing->drawFocusOnTop ())
				{
					SharedPointer<CGraphicsPath> focusPath = owned (pContext->createGraphicsPath ());
					if (focusPath)
					{
						if (_focusDrawing->getFocusPath (*focusPath))
						{
							auto lastDrawnFocus = focusPath->getBoundingBox ();
							if (!lastDrawnFocus.isEmpty ())
							{
								pContext->setClipRect (oldClip2);
								pContext->setDrawMode (kAntiAliasing|kNonIntegralMode);
								pContext->setFillColor (frame->getFocusColor ());
								pContext->drawGraphicsPath (focusPath, CDrawContext::kPathFilledEvenOdd);
								lastDrawnFocus.extend (1, 1);
								setLastDrawnFocus (lastDrawnFocus);
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

	if (frame && _focusView)
	{
		SharedPointer<CGraphicsPath> focusPath = owned (pContext->createGraphicsPath ());
		if (focusPath)
		{
			if (_focusDrawing)
				_focusDrawing->getFocusPath (*focusPath);
			else
			{
				CCoord focusWidth = frame->getFocusWidth ();
				CRect r (_focusView->getVisibleViewSize ());
				if (!r.isEmpty ())
				{
					focusPath->addRect (r);
					r.extend (focusWidth, focusWidth);
					focusPath->addRect (r);
				}
			}
			auto lastDrawnFocus = focusPath->getBoundingBox ();
			if (!lastDrawnFocus.isEmpty ())
			{
				pContext->setDrawMode (kAntiAliasing|kNonIntegralMode);
				pContext->setFillColor (frame->getFocusColor ());
				pContext->drawGraphicsPath (focusPath, CDrawContext::kPathFilledEvenOdd);
				lastDrawnFocus.extend (1, 1);
				setLastDrawnFocus (lastDrawnFocus);
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
 * @param event current event
 * @return true if any sub view accepts the hit
 */
bool CViewContainer::hitTestSubViews (const CPoint& where, const Event& event)
{
	CPoint where2 (where);
	where2.offset (-getViewSize ().left, -getViewSize ().top);
	getTransform ().inverse ().transform (where2);

	for (auto it = pImpl->children.rbegin (), end = pImpl->children.rend (); it != end; ++it)
	{
		const auto& pV = *it;
		if (pV && pV->isVisible () && pV->getMouseEnabled () && pV->hitTest (where2, event))
		{
			if (auto container = pV->asViewContainer ())
			{
				if (container->hitTestSubViews (where2, event))
					return true;
			}
			else
				return true;
		}
	}
	return false;
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
bool CViewContainer::onWheel (const CPoint& where, const CMouseWheelAxis& axis,
                              const float& distance, const CButtonState& buttons)
{
	return false;
}
#endif

//------------------------------------------------------------------------
void CViewContainer::dispatchEventToSubViews (Event& event)
{
	if (auto mouseEvent = asMousePositionEvent (event))
	{
		auto mousePos = mouseEvent->mousePosition;
		auto f = finally ([&] () { mouseEvent->mousePosition = mousePos; });
		mouseEvent->mousePosition.offset (-getViewSize ().left, -getViewSize ().top);
		getTransform ().inverse ().transform (mouseEvent->mousePosition);
		for (auto it = pImpl->children.rbegin (), end = pImpl->children.rend (); it != end;
			 ++it)
		{
			const auto& pV = *it;
			if (pV && pV->isVisible () && pV->getMouseEnabled () &&
				pV->getMouseableArea ().pointInside (mouseEvent->mousePosition))
			{
				pV->dispatchEvent (event);
				if (!pV->getTransparency () || event.consumed)
					return;
			}
		}
	}
}

//------------------------------------------------------------------------
void CViewContainer::onMouseWheelEvent (MouseWheelEvent& event)
{
	dispatchEventToSubViews (event);
}

//------------------------------------------------------------------------
void CViewContainer::onZoomGestureEvent (ZoomGestureEvent& event)
{
	dispatchEventToSubViews (event);
}

//------------------------------------------------------------------------
void CViewContainer::onMouseDownEvent (MouseDownEvent& event)
{
	auto buttonState = buttonStateFromMouseEvent (event);
	auto mouseResult = onMouseDown (event.mousePosition, buttonState);
	if (!(mouseResult == kMouseEventNotHandled || mouseResult == kMouseEventNotImplemented))
	{
		event.consumed = true;
		if (mouseResult == kMouseMoveEventHandledButDontNeedMoreEvents)
			event.ignoreFollowUpMoveAndUpEvents (true);
		return;
	}

	auto f = finally ([&, pos = event.mousePosition] () { event.mousePosition = pos; });
	event.mousePosition.offset (-getViewSize ().left, -getViewSize ().top);
	getTransform ().inverse ().transform (event.mousePosition);
	for (auto it = pImpl->children.rbegin (), end = pImpl->children.rend (); it != end; ++it)
	{
		const auto& pV = *it;
		if (pV && pV->isVisible () && pV->getMouseEnabled () &&
		    pV->hitTest (event.mousePosition, event))
		{
			if (!event.modifiers.empty ())
			{
				if (auto control = pV.cast<CControl> ())
				{
					if (auto listener = control->getListener ())
					{
						if (listener->controlModifierClicked (control, buttonState) != 0)
						{
							event.consumed = true;
							return;
						}
					}
				}
			}
			auto frame = getFrame ();
			auto previousFocusView = frame ? frame->getFocusView () : nullptr;
#if VSTGUI_ENABLE_DEPRECATED_METHODS
			mouseResult = pV->callMouseListener (MouseListenerCall::MouseDown, event.mousePosition, buttonState);
			if (!(mouseResult == kMouseEventNotHandled || mouseResult == kMouseEventNotImplemented))
			{
				event.consumed = true;
				if (mouseResult == kMouseMoveEventHandledButDontNeedMoreEvents)
					event.ignoreFollowUpMoveAndUpEvents (true);
				return;
			}
#endif
			pV->dispatchEvent (event);
			if (event.consumed)
			{
				if (pV->getNbReference () >1)
				{
					if (pV->wantsFocus () && frame && frame->getFocusView () == previousFocusView &&
					    dynamic_cast<CControl*> (pV.get ()))
					{
						getFrame ()->setFocusView (pV);
					}
					if (!event.ignoreFollowUpMoveAndUpEvents ())
						setMouseDownView (pV);
				}
				return;
			}
			if (!pV->getTransparency ())
				return;
		}
	}
}

//------------------------------------------------------------------------
void CViewContainer::onMouseMoveEvent (MouseMoveEvent& event)
{
	auto buttonState = buttonStateFromMouseEvent (event);
	auto mouseResult = onMouseMoved (event.mousePosition, buttonState);
	if (!(mouseResult == kMouseEventNotHandled || mouseResult == kMouseEventNotImplemented))
	{
		event.consumed = true;
		if (mouseResult == kMouseMoveEventHandledButDontNeedMoreEvents)
			event.ignoreFollowUpMoveAndUpEvents (true);
		return;
	}
	if (auto view = shared (getMouseDownView ()))
	{
		auto f = finally ([&, pos = event.mousePosition] () { event.mousePosition = pos; });
		event.mousePosition.offset (-getViewSize ().left, -getViewSize ().top);
		getTransform ().inverse ().transform (event.mousePosition);
#if VSTGUI_ENABLE_DEPRECATED_METHODS
		mouseResult = view->callMouseListener (MouseListenerCall::MouseMoved, event.mousePosition,
		                                       buttonState);
		if (!(mouseResult == kMouseEventNotHandled || mouseResult == kMouseEventNotImplemented))
		{
			event.consumed = true;
			if (mouseResult == kMouseMoveEventHandledButDontNeedMoreEvents)
				event.ignoreFollowUpMoveAndUpEvents (true);
			return;
		}
#endif
		view->dispatchEvent (event);
	}
}

//------------------------------------------------------------------------
void CViewContainer::onMouseUpEvent (MouseUpEvent& event)
{
	auto buttonState = buttonStateFromMouseEvent (event);
	auto mouseResult = onMouseUp (event.mousePosition, buttonState);
	if (!(mouseResult == kMouseEventNotHandled || mouseResult == kMouseEventNotImplemented))
	{
		event.consumed = true;
		return;
	}
	if (auto view = shared (getMouseDownView ()))
	{
		auto f = finally ([&, pos = event.mousePosition] () { event.mousePosition = pos; });
		event.mousePosition.offset (-getViewSize ().left, -getViewSize ().top);
		getTransform ().inverse ().transform (event.mousePosition);
#if VSTGUI_ENABLE_DEPRECATED_METHODS
		mouseResult =
		    view->callMouseListener (MouseListenerCall::MouseUp, event.mousePosition, buttonState);
		if (!(mouseResult == kMouseEventNotHandled || mouseResult == kMouseEventNotImplemented))
		{
			event.consumed = true;
			return;
		}
#endif
		view->dispatchEvent (event);
		clearMouseDownView ();
	}
}

//-----------------------------------------------------------------------------
void CViewContainer::onMouseCancelEvent (MouseCancelEvent& event)
{
	if (auto mouseDownView = getMouseDownView ())
	{
		CBaseObjectGuard crg (mouseDownView);
#if VSTGUI_ENABLE_DEPRECATED_METHODS
		mouseDownView->callMouseListener (MouseListenerCall::MouseCancel, {}, 0);
#endif
		mouseDownView->dispatchEvent (event);
		clearMouseDownView ();
	}
}

//-----------------------------------------------------------------------------
SharedPointer<IDropTarget> CViewContainer::getDropTarget ()
{
	if (getFrame () == this)
	{
		IDropTarget* dropTarget = nullptr;
		if (!getAttribute (kCViewContainerDropTargetAttribute, dropTarget))
		{
			dropTarget = new CViewContainerDropTarget (this);
			setAttribute (kCViewContainerDropTargetAttribute, dropTarget);
		}
		return dropTarget;
	}
	if (auto customDropTarget = CView::getDropTarget ())
		return customDropTarget;
	return makeOwned<CViewContainerDropTarget> (this);
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
					CPoint where (e.second.location);
					frameToLocal (where);
					MouseDownEvent downEvent (where, MouseButton::Left);
					downEvent.clickCount = e.second.tapCount;
					if (view->hitTest (where, downEvent))
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
bool CViewContainer::findSingleTouchEventTarget (ITouchEvent::Touch& event)
{
	vstgui_assert(event.target == 0);
	vstgui_assert(event.state == ITouchEvent::kBegan);

	CPoint where (event.location);
	frameToLocal (where);

	MouseDownEvent downEvent (where, MouseButton::Left);
	downEvent.clickCount = event.tapCount;

	ReverseViewIterator it (this);
	while (*it)
	{
		CView* view = *it;
		CBaseObjectGuard guard (view);
		if (view->getMouseEnabled () && view->isVisible () && view->hitTest (where, downEvent))
		{
			if (auto container = view->asViewContainer ())
			{
				if (container->findSingleTouchEventTarget (event))
					return true;
			}
			else
			{
				view->dispatchEvent (downEvent);
				if (downEvent.ignoreFollowUpMoveAndUpEvents ())
					return true;
				else if (downEvent.consumed)
				{
					event.target = view;
					event.targetIsSingleTouch = true;
					return true;
				}
			}
		}
		it++;
	}
	return false;
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

		auto func = [&] (CView* pV) {
			if (oldFocus && !foundOld)
			{
				if (oldFocus == pV)
				{
					foundOld = true;
					return false;
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
			return false;
		};

		if (reverse)
		{
			for (auto it = pImpl->children.rbegin (), end = pImpl->children.rend (); it != end; ++it)
			{
				if (func (*it))
					return true;
			}
		}
		else
		{
			for (const auto& view : pImpl->children)
			{
				if (func (view))
					return true;
			}
		}
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
	getTransform ().inverse ().transform (where);

	for (auto it = pImpl->children.rbegin (), end = pImpl->children.rend (); it != end; ++it)
	{
		const auto& pV = *it;
		if (pV && pV->getMouseableArea ().pointInside (where))
		{
			if (!options.getIncludeInvisible () && pV->isVisible () == false)
				continue;
			if (options.getMouseEnabled ())
			{
				if (pV->getMouseEnabled () == false)
					continue;
			}
			if (options.getDeep ())
			{
				if (auto container = pV->asViewContainer ())
				{
					CView* view = container->getViewAt (where, options);
					return options.getIncludeViewContainer () ? (view ? view : container) : view;
				}
			}
			if (!options.getIncludeViewContainer () && pV->asViewContainer ())
				continue;
			return pV;
		}
	}

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
	getTransform ().inverse ().transform (where);

	for (auto it = pImpl->children.rbegin (), end = pImpl->children.rend (); it != end; ++it)
	{
		const auto& pV = *it;
		if (pV && pV->getMouseableArea ().pointInside (where))
		{
			if (!options.getIncludeInvisible () && pV->isVisible () == false)
				continue;
			if (options.getMouseEnabled ())
			{
				if (pV->getMouseEnabled () == false)
					continue;
			}
			if (options.getDeep ())
			{
				if (CViewContainer* container = pV->asViewContainer ())
					result |= container->getViewsAt (where, views, options);
			}
			if (options.getIncludeViewContainer () == false)
			{
				if (pV->asViewContainer ())
					continue;
			}
			views.emplace_back (pV);
			result = true;
		}
	}

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
	getTransform ().inverse ().transform (where);

	for (auto it = pImpl->children.rbegin (), end = pImpl->children.rend (); it != end; ++it)
	{
		const auto& pV = *it;
		if (pV && pV->getMouseableArea ().pointInside (where))
		{
			if (!options.getIncludeInvisible () && pV->isVisible () == false)
				continue;
			if (options.getMouseEnabled ())
			{
				if (pV->getMouseEnabled() == false)
					continue;
			}
			if (options.getDeep ())
			{
				if (CViewContainer* container = pV->asViewContainer ())
					return container->getContainerAt (where, options);
			}
			break;
		}
	}

	return const_cast<CViewContainer*>(this);
}

//-----------------------------------------------------------------------------
CPoint& CViewContainer::frameToLocal (CPoint& point) const
{
	point.offset (-getViewSize ().left, -getViewSize ().top);
	if (auto parent = getParentView ())
		return parent->frameToLocal (point);
	return point;
}

//-----------------------------------------------------------------------------
CPoint& CViewContainer::localToFrame (CPoint& point) const
{
	point.offset (getViewSize ().left, getViewSize ().top);
	if (auto parent = getParentView ())
		return parent->localToFrame (point);
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

} // VSTGUI
