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
#include "viewlayouter/autosizeviewlayouter.h"
#include "iviewlistener.h"
#include "controls/icontrollistener.h"
#include "cgraphicspath.h"
#include "controls/ccontrol.h"
#include "dragging.h"
#include "dispatchlist.h"
#include "events.h"
#include "finally.h"
#include "algorithm.h"

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
	SharedPointer<IViewLayouter> layouter {AutoSizeViewLayouter::get ()};

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
		auto view = container->getViewAt (
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
CViewContainer::~CViewContainer () noexcept
{
	vstgui_assert (pImpl->children.empty ());
	vstgui_assert (pImpl->viewContainerListeners.empty ());
}

//-----------------------------------------------------------------------------
void CViewContainer::beforeDelete ()
{
	removeAttribute (kCViewContainerDropTargetAttribute);

	// remove all views
	CViewContainer::removeAll ();
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
void CViewContainer::setMouseDownView (const SharedPointer<CView>& view)
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
SharedPointer<CView> CViewContainer::getMouseDownView () const
{
	SharedPointer<CView> view;
	if (getAttribute (kCViewContainerMouseDownViewAttribute, view))
		return view;
	return {};
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
			listener->viewContainerTransformChanged (*this);
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
void CViewContainer::setViewLayouter (const SharedPointer<IViewLayouter>& layouter)
{
	if (layouter == nullptr)
		pImpl->layouter = AutoSizeViewLayouter::get ();
	else
		pImpl->layouter = layouter;
}

//-----------------------------------------------------------------------------
SharedPointer<IViewLayouter> CViewContainer::getViewLayouter () const { return pImpl->layouter; }

//-----------------------------------------------------------------------------
std::optional<ViewLayout> CViewContainer::calculateViewLayout (const CRect& newSize) const
{
	if (pImpl->layouter)
	{
		return pImpl->layouter->calculateLayout (*this, pImpl->children, newSize);
	}
	return {};
}

//-----------------------------------------------------------------------------
bool CViewContainer::applyViewLayout (const ViewLayout& layout)
{
	bool result = true;
	if (pImpl->layouter)
	{
		setViewFlag (kInApplyLayout, true);
		result = pImpl->layouter->applyLayout (*this, pImpl->children, layout);
		setViewFlag (kInApplyLayout, false);
	}
	else
	{
		setViewSize (layout.size);
		setMouseableArea (layout.size);
	}
	return result;
}

//-----------------------------------------------------------------------------
bool CViewContainer::inApplyViewLayout () const { return hasViewFlag (kInApplyLayout); }

//-----------------------------------------------------------------------------
/**
 * @param rect the new size of the container
 * @param invalid the views to dirty
 */
void CViewContainer::setViewSize (const CRect &rect, bool invalid)
{
	if (rect == getViewSize ())
		return;

	if (!inApplyViewLayout () && pImpl->layouter)
	{
		if (auto layout = pImpl->layouter->calculateLayout (*this, pImpl->children, rect))
		{
			if (applyViewLayout (*layout))
			{
				parentSizeChanged ();
				return;
			}
		}
	}
	CView::setViewSize (rect, invalid);

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
		result = parent->getVisibleSize (result);
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
		invalid ();
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
		invalid ();
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
		auto view = dynamic_cast<CView*> (sender);
		if (view && isChild (*view, false))
		{
			if (auto frame = getFrame ())
			{
				if (frame->focusDrawingEnabled ())
				{
					CCoord width = frame->getFocusWidth ();
					CRect viewSize (view->getViewSize ());
					viewSize.extend (width, width);
					invalidRect (viewSize);
				}
			}
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

//------------------------------------------------------------------------
bool CViewContainer::doInsertSubview (const SharedPointer<CView>& view,
									  ViewList::const_iterator pos)
{
	if (!view)
		return false;

	vstgui_assert (!view->isSubview (), "view is already added to a container view");

	pImpl->children.insert (pos, view);

	view->setSubviewState (true);

	pImpl->viewContainerListeners.forEach ([&] (IViewContainerListener* listener) {
		listener->viewContainerViewAdded (*this, *view.get ());
	});

	if (isAttached ())
	{
		view->attached (*this);
		view->invalid ();
	}
	return true;
}

//------------------------------------------------------------------------
void CViewContainer::doRemoveSubview (ViewList::const_iterator pos)
{
	vstgui_assert (pos != pImpl->children.end (), "The iterator must point to a valid object");

	if (*pos == getInitialFocusView ())
		setInitialFocusView (nullptr);
	(*pos)->invalid ();
	if ((*pos) == getMouseDownView ())
		clearMouseDownView ();
	if (isAttached ())
		(*pos)->removed (*this);
	(*pos)->setSubviewState (false);
	pImpl->viewContainerListeners.forEach ([&] (IViewContainerListener* listener) {
		listener->viewContainerViewRemoved (*this, *(*pos).get ());
	});
	pImpl->children.erase (pos);
}

//------------------------------------------------------------------------
bool CViewContainer::addSubview (const SharedPointer<CView>& view) { return insertSubview (view); }

//------------------------------------------------------------------------
bool CViewContainer::insertSubview (const SharedPointer<CView>& view,
									const Optional<size_t>& position)
{
	ViewList::const_iterator it;
	if (position)
	{
		it = pImpl->children.begin ();
		std::advance (it, *position);
	}
	else
	{
		it = pImpl->children.end ();
	}
	return doInsertSubview (view, it);
}

//------------------------------------------------------------------------
bool CViewContainer::removeSubview (const SharedPointer<CView>& view)
{
	if (auto pos = CViewContainer::indexOfSubview (view))
	{
		auto it = pImpl->children.begin ();
		std::advance (it, *pos);
		doRemoveSubview (it);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
Optional<size_t> CViewContainer::indexOfSubview (const SharedPointer<CView>& view) const
{
	return indexOf<size_t> (pImpl->children.begin (), pImpl->children.end (), view);
}

//-----------------------------------------------------------------------------
/**
 * @return true on success
 */
bool CViewContainer::removeAll ()
{
	clearMouseDownView ();
	setInitialFocusView (nullptr);

	auto it = pImpl->children.begin ();
	while (it != pImpl->children.end ())
	{
		auto view = *it;
		if (isAttached ())
			view->removed (*this);
		pImpl->children.erase (it);
		view->setSubviewState (false);
		pImpl->viewContainerListeners.forEach ([&] (IViewContainerListener* listener) {
			listener->viewContainerViewRemoved (*this, *view.get ());
		});
		it = pImpl->children.begin ();
	}
	return true;
}

//-----------------------------------------------------------------------------
/**
 * @param pView the view which should be checked if it is a child of this container
 * @return true on success
 */
bool CViewContainer::isChild (const CView& pView) const { return isChild (pView, false); }

//-----------------------------------------------------------------------------
bool CViewContainer::isChild (const CView& pView, bool deep) const
{
	bool found = false;

	if (deep)
	{
		auto it = pImpl->children.begin ();
		while (!found && it != pImpl->children.end ())
		{
			auto v = (*it);
			if (&pView == v.get ())
			{
				found = true;
				break;
			}
			if (auto container = v->asViewContainer ())
				found = container->isChild (pView, true);
			++it;
		}
	}
	else
	{
		found = std::find_if (pImpl->children.begin (), pImpl->children.end (), [&] (auto&& child) {
					return child.get () == &pView;
				}) != pImpl->children.end ();
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
SharedPointer<CView> CViewContainer::getView (uint32_t index) const
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
bool CViewContainer::changeViewZOrder (const SharedPointer<CView>& view, uint32_t newIndex)
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
				listener->viewContainerViewZOrderChanged (*this, *view.get ());
			});
			return true;
		}
	}
	return false;
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
void CViewContainer::draw (CDrawContext& context)
{
	CViewContainer::drawRect (context, getViewSize ());
}

//-----------------------------------------------------------------------------
/**
 * @param context the context which to use to draw the background
 * @param _updateRect the area which to draw
 */
void CViewContainer::drawBackgroundRect (CDrawContext& context, const CRect& _updateRect)
{
	if (getDrawBackground ())
	{
		drawClipped (context, _updateRect, [&] () {
			CRect tr (0, 0, getViewSize ().getWidth (), getViewSize ().getHeight ());
			getDrawBackground ()->draw (context, tr, getBackgroundOffset ());
		});
	}
	else if ((pImpl->backgroundColor.alpha != 255 && getTransparency ()) || !getTransparency ())
	{
		context.setDrawMode (kAliasing);
		context.setLineWidth (1);
		context.setFillColor (pImpl->backgroundColor);
		context.setFrameColor (pImpl->backgroundColor);
		context.setLineStyle (kLineSolid);
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
		context.drawRect (r, pImpl->backgroundColorDrawStyle);
	}
}

//-----------------------------------------------------------------------------
/**
 * @param pContext the context which to use to draw
 * @param updateRect the area which to draw
 */
void CViewContainer::drawRect (CDrawContext& context, const CRect& updateRect)
{
	CPoint offset (getViewSize ().left, getViewSize ().top);
	CDrawContext::Transform offsetTransform (context,
											 CGraphicsTransform ().translate (offset.x, offset.y));

	CRect _updateRect (updateRect);
	_updateRect.bound (getViewSize ());

	CRect clientRect (_updateRect);
	clientRect.offset (-getViewSize ().left, -getViewSize ().top);

	CRect oldClip;
	context.getClipRect (oldClip);
	CRect oldClip2 (oldClip);

	CRect newClip (clientRect);
	newClip.bound (oldClip);
	context.setClipRect (newClip);

	// draw the background
	drawBackgroundRect (context, clientRect);

	SharedPointer<CView> _focusView;
	IFocusDrawing* _focusDrawing = nullptr;
	auto frame = getFrame ();
	if (frame && frame->focusDrawingEnabled ())
	{
		auto fV = frame->getFocusView ();
		if (fV && isChild (*fV.get (), false) && fV->isVisible () && fV->wantsFocus ())
		{
			_focusView = fV;
			_focusDrawing = dynamic_cast<IFocusDrawing*> (_focusView.get ());
		}
	}

	{
		CDrawContext::Transform tr (context, getTransform ());
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
					auto focusPath = context.createGraphicsPath ();
					if (focusPath)
					{
						if (_focusDrawing->getFocusPath (*focusPath.get (),
														 frame->getFocusWidth ()))
						{
							auto lastDrawnFocus = focusPath->getBoundingBox ();
							if (!lastDrawnFocus.isEmpty ())
							{
								context.setClipRect (oldClip2);
								context.setDrawMode (kAntiAliasing | kNonIntegralMode);
								context.setFillColor (frame->getFocusColor ());
								context.drawGraphicsPath (focusPath,
														  CDrawContext::kPathFilledEvenOdd);
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
					context.setClipRect (viewSize);
					float globalContextAlpha = context.getGlobalAlpha ();
					context.setGlobalAlpha (globalContextAlpha * pV->getAlphaValue ());
					pV->drawRect (context, viewSize);
					context.setGlobalAlpha (globalContextAlpha);
#if DEBUG
					static bool drawViewWireFrames = false;
					if (drawViewWireFrames)
					{
						context.setFrameColor (kRedCColor);
						context.setLineWidth (context.getHairlineSize ());
						context.setLineStyle (kLineSolid);
						context.drawRect (pV->getViewSize ());
					}
#endif
				}
			}
		}
	}

	context.setClipRect (oldClip2);

	if (frame && _focusView)
	{
		auto focusPath = context.createGraphicsPath ();
		if (focusPath)
		{
			if (_focusDrawing)
				_focusDrawing->getFocusPath (*focusPath.get (), frame->getFocusWidth ());
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
				context.setDrawMode (kAntiAliasing | kNonIntegralMode);
				context.setFillColor (frame->getFocusColor ());
				context.drawGraphicsPath (focusPath, CDrawContext::kPathFilledEvenOdd);
				lastDrawnFocus.extend (1, 1);
				setLastDrawnFocus (lastDrawnFocus);
			}
		}
	}
}

//-----------------------------------------------------------------------------
/**
 * check if view needs to be updated for rect
 * @param view view to check
 * @param rect update rect
 * @return true if view needs update
 */
bool CViewContainer::checkUpdateRect (const SharedPointer<CView>& view, const CRect& rect)
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
						if (listener->controlModifierClicked (*control.get (), buttonState) != 0)
						{
							event.consumed = true;
							return;
						}
					}
				}
			}
			auto frame = getFrame ();
			auto previousFocusView = frame ? frame->getFocusView () : nullptr;
			pV->dispatchEvent (event);
			if (event.consumed)
			{
				if (pV->wantsFocus () && frame && frame->getFocusView () == previousFocusView &&
					dynamic_cast<CControl*> (pV.get ()))
				{
					frame->setFocusView (pV);
				}
				if (!event.ignoreFollowUpMoveAndUpEvents ())
					setMouseDownView (pV);
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
	if (auto view = getMouseDownView ())
	{
		auto f = finally ([&, pos = event.mousePosition] () { event.mousePosition = pos; });
		event.mousePosition.offset (-getViewSize ().left, -getViewSize ().top);
		getTransform ().inverse ().transform (event.mousePosition);
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
	if (auto view = getMouseDownView ())
	{
		auto f = finally ([&, pos = event.mousePosition] () { event.mousePosition = pos; });
		event.mousePosition.offset (-getViewSize ().left, -getViewSize ().top);
		getTransform ().inverse ().transform (event.mousePosition);
		view->dispatchEvent (event);
		clearMouseDownView ();
	}
}

//-----------------------------------------------------------------------------
void CViewContainer::onMouseCancelEvent (MouseCancelEvent& event)
{
	if (auto mouseDownView = getMouseDownView ())
	{
		mouseDownView->dispatchEvent (event);
		clearMouseDownView ();
	}
}

//-----------------------------------------------------------------------------
SharedPointer<IDropTarget> CViewContainer::getDropTarget ()
{
	if (getFrame () == this)
	{
		SharedPointer<IDropTarget> dropTarget;
		if (!getAttribute (kCViewContainerDropTargetAttribute, dropTarget))
		{
			dropTarget = makeShared<CViewContainerDropTarget> (this);
			setAttribute (kCViewContainerDropTargetAttribute, dropTarget);
		}
		return dropTarget;
	}
	if (auto customDropTarget = CView::getDropTarget ())
		return customDropTarget;
	return makeShared<CViewContainerDropTarget> (this);
}

#if VSTGUI_TOUCH_EVENT_HANDLING
//-----------------------------------------------------------------------------
void CViewContainer::onTouchEvent (ITouchEvent& event)
{
	ReverseViewIterator it (*this);
	while (*it)
	{
		auto view = *it;
		auto lifeGuard = makeLifeGuard (view);
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

	ReverseViewIterator it (*this);
	while (*it)
	{
		auto view = *it;
		auto lifeGuard = makeLifeGuard (view);
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

//------------------------------------------------------------------------
void CViewContainer::setInitialFocusView (const SharedPointer<CView>& view)
{
	if (view)
	{
		setAttribute (kInitialFocusViewAttribute, view);
	}
	else
	{
		removeAttribute (kInitialFocusViewAttribute);
	}
}

//------------------------------------------------------------------------
SharedPointer<CView> CViewContainer::getInitialFocusView () const
{
	SharedPointer<CView> initialFocusView;
	getAttribute (kInitialFocusViewAttribute, initialFocusView);
	return initialFocusView;
}

//-----------------------------------------------------------------------------
/**
 * @param oldFocus old focus view
 * @param reverse search order
 * @return true on success
 */
bool CViewContainer::advanceNextFocusView (const SharedPointer<CView>& oldFocus, bool reverse)
{
	if (auto frame = getFrame ())
	{
		if (!oldFocus)
		{
			if (auto initialFocusView = getInitialFocusView ())
			{
				frame->setFocusView (initialFocusView);
				return true;
			}
		}

		bool foundOld = false;

		auto func = [&] (auto pV) {
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
					frame->setFocusView (pV);
					return true;
				}
				else if (auto container = pV->asViewContainer ())
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
/**
 * @param p location
 * @param options search options
 * @return view at position p or null
 */
SharedPointer<CView> CViewContainer::getViewAt (const CPoint& p,
												const GetViewOptions& options) const
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
					auto view = container->getViewAt (where, options);
					if (options.getIncludeViewContainer ())
					{
						if (view)
							return view;
						return shared (container);
					}
					return view;
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
				if (auto container = pV->asViewContainer ())
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
SharedPointer<CViewContainer> CViewContainer::getContainerAt (const CPoint& p,
															  const GetViewOptions& options) const
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
				if (auto container = pV->asViewContainer ())
					return container->getContainerAt (where, options);
			}
			break;
		}
	}

	return shared (const_cast<CViewContainer*> (this));
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
bool CViewContainer::removed (CViewContainer& parent)
{
	if (!isAttached ())
		return false;

	for (const auto& pV : pImpl->children)
		pV->removed (*this);

	return CView::removed (parent);
}

//-----------------------------------------------------------------------------
bool CViewContainer::attached (CViewContainer& parent)
{
	if (isAttached ())
		return false;

	setParentFrame (parent.getFrame ());

	bool result = CView::attached (parent);
	if (result)
	{
		for (const auto& pV : pImpl->children)
			pV->attached (*this);
	}
	if (auto layout = calculateViewLayout (getViewSize ()))
		applyViewLayout (*layout);
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
		if (auto container = pV->asViewContainer ())
			container->dumpHierarchy ();
	}
	_debugDumpLevel--;
}

#endif

} // VSTGUI
