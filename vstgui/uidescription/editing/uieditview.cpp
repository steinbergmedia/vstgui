// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uieditview.h"

#if VSTGUI_LIVE_EDITING

#include "uiundomanager.h"
#include "uiactions.h"
#include "uicrosslines.h"
#include "igridprocessor.h"
#include "uiselection.h"
#include "uioverlayview.h"
#include "../icontroller.h"
#include "../uiattributes.h"
#include "../uidescription.h"
#include "../iviewfactory.h"
#include "../cstream.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../../lib/cvstguitimer.h"
#include "../../lib/cexternalview.h"
#include "../../lib/cframe.h"
#include "../../lib/cscrollview.h"
#include "../../lib/cdropsource.h"
#include "../../lib/coffscreencontext.h"
#include "../../lib/clayeredviewcontainer.h"
#include "../../lib/dragging.h"
#include "../../lib/events.h"
#include "../../lib/idatapackage.h"
#include "../../lib/controls/ctextedit.h"
#include <cassert>

namespace VSTGUI {

namespace UIEditViewInternal {

//----------------------------------------------------------------------------------------------------
class UISelectionView : public UIOverlayView, public UISelectionListenerAdapter
//----------------------------------------------------------------------------------------------------
{
public:
	UISelectionView (const SharedPointer<CViewContainer>& editView,
					 const SharedPointer<UISelection>& selection, const CColor& selectionColor,
					 CCoord handleSize);
	~UISelectionView () override;

private:
	void draw (CDrawContext& context) override;
	void drawResizeHandle (const CPoint& p, CDrawContext& pContext);

	void selectionWillChange (const UISelection&) override { onSelectionChanged (); }
	void selectionDidChange (const UISelection&) override { onSelectionChanged (); }
	void selectionViewsWillChange (const UISelection&) override { onSelectionChanged (); }
	void selectionViewsDidChange (const UISelection&) override { onSelectionChanged (); }

	void onSelectionChanged ();

	SharedPointer<UISelection> selection;
	CColor selectionColor;
	CCoord handleInset;
};

//----------------------------------------------------------------------------------------------------
UISelectionView::UISelectionView (const SharedPointer<CViewContainer>& editView,
								  const SharedPointer<UISelection>& selection,
								  const CColor& selectionColor, CCoord handleSize)
: UIOverlayView (editView)
, selection (selection)
, selectionColor (selectionColor)
, handleInset (handleSize / 2.)
{
	selection->registerListener (this);
}

//----------------------------------------------------------------------------------------------------
UISelectionView::~UISelectionView ()
{
	selection->unregisterListener (this);
}

//----------------------------------------------------------------------------------------------------
void UISelectionView::drawResizeHandle (const CPoint& p, CDrawContext& pContext)
{
	CRect r (p.x, p.y, p.x, p.y);
	r.inset (-handleInset, -handleInset);
	pContext.drawRect (r, kDrawFilledAndStroked);
}

//----------------------------------------------------------------------------------------------------
void UISelectionView::draw (CDrawContext& context)
{
	auto lineWidth = context.getHairlineSize ();
	CRect r (getVisibleViewSize ());
	ConcatClip cc (context, r);
	context.setDrawMode (kAliasing);
	context.setLineStyle (kLineSolid);
	context.setLineWidth (lineWidth);

	CColor lightColor (kWhiteCColor);
	lightColor.alpha = 140;
	context.setFillColor (lightColor);

	auto mainView = getTargetView ()->getView (0);
	CPoint p;
	frameToLocal (p);
	for (auto view : *selection.get ())
	{
		CRect vs = selection->getGlobalViewCoordinates (*view.get ());
		vs.offsetInverse (p);
		vs.extend (lineWidth, lineWidth);
		context.setFrameColor (lightColor);
		context.drawRect (vs);
		vs.inset (lineWidth, lineWidth);
		context.setFrameColor (selectionColor);
		context.drawRect (vs);
		if (vs.getWidth () > handleInset * 2. && vs.getHeight () > handleInset * 2.)
		{
			drawResizeHandle (vs.getBottomRight (), context);
			if (view != mainView)
			{
				drawResizeHandle (vs.getTopLeft (), context);
				drawResizeHandle (vs.getBottomLeft (), context);
				drawResizeHandle (vs.getTopRight (), context);
			}
			if (vs.getHeight () > handleInset * 4)
			{
				drawResizeHandle (vs.getRightCenter (), context);
				if (view != mainView)
					drawResizeHandle (vs.getLeftCenter (), context);
			}
			if (vs.getWidth () > handleInset * 4)
			{
				drawResizeHandle (vs.getBottomCenter (), context);
				if (view != mainView)
					drawResizeHandle (vs.getTopCenter (), context);
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UISelectionView::onSelectionChanged ()
{
	CPoint p;
	frameToLocal (p);
	for (auto view : *selection.get ())
	{
		CRect vs = selection->getGlobalViewCoordinates (*view.get ());
		vs.offsetInverse (p);
		vs.extend (handleInset + 2, handleInset + 2);
		invalidRect (vs);
	}
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
class UIHighlightView : public UIOverlayView
{
public:
	UIHighlightView (const SharedPointer<CViewContainer>& editView,
					 const CColor& viewHighlightColor);

	void setHighlightView (const SharedPointer<CView>& view);

private:
	void draw (CDrawContext& context) override;

	SharedPointer<CView> highlightView;
	CColor strokeColor;
	CColor fillColor;
};

//----------------------------------------------------------------------------------------------------
UIHighlightView::UIHighlightView (const SharedPointer<CViewContainer>& editView,
								  const CColor& viewHighlightColor)
: UIOverlayView (editView), highlightView (nullptr), strokeColor (viewHighlightColor)
{
	double h,s,l;
	strokeColor.toHSL (h, s, l);
	l /= 2.;
	fillColor.fromHSL (h, s, l);
	fillColor.alpha = strokeColor.alpha;
}

//----------------------------------------------------------------------------------------------------
void UIHighlightView::setHighlightView (const SharedPointer<CView>& view)
{
	if (highlightView != view)
	{
		highlightView = view;
		invalid ();
	}
}

//----------------------------------------------------------------------------------------------------
void UIHighlightView::draw (CDrawContext& context)
{
	if (highlightView == nullptr)
		return;
	CRect r = UISelection::getGlobalViewCoordinates (*highlightView.get ());
	CPoint p;
	frameToLocal (p);
	r.offsetInverse (p);
	r.inset (2, 2);
	context.setFillColor (fillColor);
	context.setFrameColor (strokeColor);
	context.setLineStyle (kLineSolid);
	context.setLineWidth (3);
	context.drawRect (r, kDrawFilledAndStroked);
}

//------------------------------------------------------------------------
template<typename T>
void collectExternalViewsOnInlineEditing (CViewContainer& container, T& array)
{
	container.forEachChild ([&] (auto&& view) {
		if (dynamic_cast<ExternalView::IViewEmbedder*> (view.get ()))
			array.emplace_back (view);
		else if (auto c = view->asViewContainer ())
			collectExternalViewsOnInlineEditing (*c, array);
	});
}

} // UIEditViewInternal

//------------------------------------------------------------------------
struct UIEditView::ViewAddedObserver : IViewAddedRemovedObserver,
									   ViewListenerAdapter
{
	ViewAddedObserver (const SharedPointer<CViewContainer>& root) : root (root) {}
	~ViewAddedObserver () override
	{
		for (auto view : views)
		{
			if (auto viewEmbedder = dynamic_cast<ExternalView::IViewEmbedder*> (view.get ()))
			{
				if (auto ev = viewEmbedder->getExternalView ())
					ev->setMouseEnabled (view->getMouseEnabled ());
			}
			view->unregisterViewListener (this);
		}
	}

	bool isViewSubViewOfRoot (CView& view)
	{
		auto parent = view.getParentView ();
		if (parent == nullptr || parent == root->getFrame ())
			return false;
		if (parent == root.get ())
			return true;
		return isViewSubViewOfRoot (*parent);
	}

	void onViewAdded (CFrame& frame, CView& view) override
	{
		if (auto viewEmbedder = dynamic_cast<ExternalView::IViewEmbedder*> (&view))
		{
			if (!isViewSubViewOfRoot (view))
				return;
			if (auto ev = viewEmbedder->getExternalView ())
				ev->setMouseEnabled (false);
			view.registerViewListener (this);
			views.emplace_back (shared (&view));
		}
	}
	void onViewRemoved (CFrame& frame, CView& view) override {}

	void viewWillDelete (CView& view) override
	{
		view.unregisterViewListener (this);
		auto it = std::find_if (views.begin (), views.end (),
								[&] (auto&& v) { return v.get () == &view; });
		if (it != views.end ())
			views.erase (it);
	}
	void viewOnMouseEnabled (CView& view, bool state) override
	{
		if (auto viewEmbedder = dynamic_cast<ExternalView::IViewEmbedder*> (&view))
		{
			if (auto ev = viewEmbedder->getExternalView ())
				ev->setMouseEnabled (false);
		}
	}

	SharedPointer<CViewContainer> root;
	std::vector<SharedPointer<CView>> views;
};

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
static constexpr auto kResizeHandleSize = 6.;
static constexpr auto UIEditViewMargin = 8.;

//----------------------------------------------------------------------------------------------------
UIEditView::UIEditView (const CRect& size, const SharedPointer<UIDescription>& uidescription)
: CViewContainer (size), description (uidescription), gridProcessor (nullptr)
{
	setScale (1.);
	setWantsFocus (true);
}

//----------------------------------------------------------------------------------------------------
UIEditView::~UIEditView ()
{
	editTimer = nullptr;
	setUndoManager (nullptr);
	setSelection (nullptr);
}

//------------------------------------------------------------------------
void UIEditView::updateSize ()
{
	if (auto view = getEditView ())
	{
		CRect r (getViewSize ());
		CPoint size (view->getWidth (), view->getHeight ());
		getTransform ().transform (size);
		r.setSize (size);
		r.right += UIEditViewMargin;
		r.bottom += UIEditViewMargin;
		if (r != getViewSize ())
		{
			setAutosizingEnabled (false);
			setViewSize (r);
			setMouseableArea (getViewSize ());
			setAutosizingEnabled (true);
			getParentView ()->invalid ();
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditView::setScale (double scale)
{
	scale = std::round (scale * 100) / 100.;
	setTransform (CGraphicsTransform ().scale (scale, scale));
	updateSize ();
}

//----------------------------------------------------------------------------------------------------
void UIEditView::enableEditing (bool state)
{
	if (editing != state)
	{
		editing = state;
		invalid ();
		auto parent = getFrame ();
		if (parent == nullptr)
			return;

		if (editing)
		{
			CRect r = parent->getViewSize ();
			r.originize ();
			vstgui_assert (overlayView == nullptr);
			overlayView = makeShared<CLayeredViewContainer> (r);
			overlayView->setAutosizeFlags (kAutosizeAll);
			overlayView->setMouseEnabled (false);
			overlayView->setTransparency (true);
			overlayView->setZIndex (1000);
			parent->addSubview (overlayView);

			highlightView =
				makeShared<UIEditViewInternal::UIHighlightView> (shared (this), viewHighlightColor);
			overlayView->addSubview (highlightView);
			auto selectionView = makeShared<UIEditViewInternal::UISelectionView> (
				shared (this), getSelection (), viewSelectionColor, kResizeHandleSize);
			overlayView->addSubview (selectionView);
		}
		else
		{
			parent->removeSubview (overlayView);
			overlayView.reset ();
			highlightView.reset ();
			lines.reset ();
		}
		disableExternalViewsOnInlineEditing (editing);
	}
}

//------------------------------------------------------------------------
void UIEditView::disableExternalViewsOnInlineEditing (bool state)
{
	auto parent = getFrame ();
	if (editingViewAddedObserver)
		parent->setViewAddedRemovedObserver (nullptr);
	editingViewAddedObserver.reset ();
	if (state)
	{
		editingViewAddedObserver = std::make_unique<ViewAddedObserver> (shared (this));
		std::vector<SharedPointer<CView>> views;
		UIEditViewInternal::collectExternalViewsOnInlineEditing (*this, views);
		for (auto& v : views)
			editingViewAddedObserver->onViewAdded (*parent, *v.get ());
		parent->setViewAddedRemovedObserver (editingViewAddedObserver.get ());
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditView::enableAutosizing (bool state)
{
	autosizing = state;
}

//----------------------------------------------------------------------------------------------------
void UIEditView::setUndoManager (const SharedPointer<UIUndoManager>& manager)
{
	undoManger = manager;
}

//----------------------------------------------------------------------------------------------------
SharedPointer<UIUndoManager> UIEditView::getUndoManager ()
{
	if (undoManger == nullptr)
		undoManger = makeShared<UIUndoManager> ();
	return undoManger;
}

//----------------------------------------------------------------------------------------------------
void UIEditView::setSelection (const SharedPointer<UISelection>& inSelection)
{
	selection = inSelection;
}

//----------------------------------------------------------------------------------------------------
SharedPointer<UISelection> UIEditView::getSelection ()
{
	if (selection == nullptr)
	{
		selection = makeShared<UISelection> ();
	}
	return selection;
}

//----------------------------------------------------------------------------------------------------
void UIEditView::setGridProcessor (const SharedPointer<IGridProcessor>& inGrid)
{
	gridProcessor = inGrid;
}

//----------------------------------------------------------------------------------------------------
void UIEditView::setEditView (const SharedPointer<CView>& view)
{
	if (view != getEditView ())
	{
		disableExternalViewsOnInlineEditing (false);
		invalid ();
		removeAll ();
		CRect vs (getViewSize ());
		if (view)
		{
			addSubview (view);
			updateSize ();
		}
		else
		{
			vs.setWidth (0);
			vs.setHeight (0);
			setMouseableArea (vs);
			setViewSize (vs);
		}
		disableExternalViewsOnInlineEditing (editing);
		invalid ();
	}
}

//----------------------------------------------------------------------------------------------------
SharedPointer<CView> UIEditView::getEditView () const
{
	return getChildren ().empty () ? nullptr : getChildren ().front ();
}

//----------------------------------------------------------------------------------------------------
CMessageResult UIEditView::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == kMsgViewSizeChanged)
	{
		CView* view = dynamic_cast<CView*> (sender);
		if (view && view == getEditView ().get ())
		{
			updateSize ();
		}
	}
	return CViewContainer::notify (sender, message);
}

//----------------------------------------------------------------------------------------------------
void UIEditView::draw (CDrawContext& context) { drawRect (context, getViewSize ()); }

//----------------------------------------------------------------------------------------------------
void UIEditView::drawRect (CDrawContext& context, const CRect& updateRect)
{
	// disable focus drawing
	bool focusDrawing = getFrame ()->focusDrawingEnabled ();
	if (!editing && focusDrawing)
		getFrame ()->setFocusDrawingEnabled (false);

	CViewContainer::drawRect (context, updateRect);

	if (!editing && focusDrawing)
		getFrame ()->setFocusDrawingEnabled (focusDrawing);

	context.setClipRect (updateRect);

	CDrawContext::Transform transform (
		context, CGraphicsTransform ().translate (getViewSize ().left, getViewSize ().top));

	const CCoord dashLength[] = {5, 5};
	const CLineStyle lineDash (CLineStyle::kLineCapButt, CLineStyle::kLineJoinMiter, 0, 2, dashLength);
	context.setLineStyle (lineDash);
	context.setLineWidth (1);
	context.setDrawMode (kAliasing);
	context.setFrameColor (kBlueCColor);
	context.drawRect (CRect (0, 0, getWidth () - UIEditViewMargin, getHeight () - UIEditViewMargin),
					  kDrawStroked);
}

//----------------------------------------------------------------------------------------------------
SharedPointer<CView> UIEditView::getViewAt (const CPoint& p, const GetViewOptions& options) const
{
	auto view = CViewContainer::getViewAt (p, options);
	if (editing)
	{
		while (view && IViewFactory::getViewName (*view.get ()) == nullptr)
		{
			view = shared (view->getParentView ());
		}
	}
	return view;
}

//----------------------------------------------------------------------------------------------------
SharedPointer<CViewContainer> UIEditView::getContainerAt (const CPoint& p,
														  const GetViewOptions& options) const
{
	auto view = CViewContainer::getContainerAt (p, options);
	if (editing)
	{
		while (view && IViewFactory::getViewName (*view.get ()) == nullptr)
		{
			view = shared (view->getParentView ());
		}
	}
	return view;
}

//----------------------------------------------------------------------------------------------------
bool UIEditView::advanceNextFocusView (const SharedPointer<CView>& oldFocus, bool reverse)
{
	if (editing)
		return false;
	return CViewContainer::advanceNextFocusView (oldFocus, reverse);
}

//----------------------------------------------------------------------------------------------------
void UIEditView::onMouseWheelEvent (MouseWheelEvent& event)
{
	if (!editing)
		CViewContainer::onMouseWheelEvent (event);
}

//------------------------------------------------------------------------
void UIEditView::onZoomGestureEvent (ZoomGestureEvent& event)
{
	if (editing)
	{
		auto scale = getTransform ().m11;
		auto newScale = scale + scale * event.zoom;
		setScale (newScale);
		event.consumed = true;
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditView::invalidSelection ()
{
	getSelection ()->invalidRects ();
}

namespace UIEditViewInternal {

//----------------------------------------------------------------------------------------------------
static bool pointInResizeHandleRect (const CPoint& where, const CPoint& handle)
{
	CRect r (handle.x, handle.y, handle.x, handle.y);
	r.extend (kResizeHandleSize/2., kResizeHandleSize/2.);
	return r.pointInside (where);
}

} // UIEditViewInternal

//----------------------------------------------------------------------------------------------------
UIEditView::MouseSizeMode UIEditView::selectionHitTest (const CPoint& _where,
														SharedPointer<CView>& resultView)
{
	CPoint where (_where);
	where.offset (-getViewSize ().left, -getViewSize ().top);
	CPoint p;
	frameToLocal (p);

	auto mainView = getEditView ();
	for (auto it = getSelection ()->rbegin (), end = getSelection ()->rend (); it != end; ++it)
	{
		auto view = (*it);
		CRect r = getSelection ()->getGlobalViewCoordinates (*view.get ());
		bool isMainView = (mainView == view) ? true : false;
		r.offset (p);
		r.extend (kResizeHandleSize, kResizeHandleSize);
		if (r.pointInside (where))
		{
			resultView = view;
			r.inset (kResizeHandleSize, kResizeHandleSize);
			if (UIEditViewInternal::pointInResizeHandleRect (where, r.getBottomRight ()))
				return MouseSizeMode::BottomRight;
			if (UIEditViewInternal::pointInResizeHandleRect (where, r.getRightCenter ()))
				return MouseSizeMode::Right;
			if (UIEditViewInternal::pointInResizeHandleRect (where, r.getBottomCenter ()))
				return MouseSizeMode::Bottom;
			if (!isMainView)
			{
				if (UIEditViewInternal::pointInResizeHandleRect (where, r.getLeftCenter ()))
					return MouseSizeMode::Left;
				if (UIEditViewInternal::pointInResizeHandleRect (where, r.getTopCenter ()))
					return MouseSizeMode::Top;
				if (UIEditViewInternal::pointInResizeHandleRect (where, r.getBottomLeft ()))
					return MouseSizeMode::BottomLeft;
				if (UIEditViewInternal::pointInResizeHandleRect (where, r.getTopLeft ()))
					return MouseSizeMode::TopLeft;
				if (UIEditViewInternal::pointInResizeHandleRect (where, r.getTopRight ()))
					return MouseSizeMode::TopRight;
				if (r.pointInside (where))
					return MouseSizeMode::None;
			}
			resultView.reset ();
		}
	}
	resultView.reset ();
	return MouseSizeMode::None;
}

//----------------------------------------------------------------------------------------------------
bool UIEditView::hitTestSubViews (const CPoint& where, const Event& event)
{
	return hitTest (where, event);
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIEditView::onMouseDown (CPoint &where, const CButtonState& buttons)
{
	if (!editing)
		return CViewContainer::onMouseDown (where, buttons);
	if (!buttons.isLeftButton ())
		return kMouseEventHandled;

	getFrame ()->setFocusView (shared (this));

	CPoint where2 (where);
	where2.offset (-getViewSize ().left, -getViewSize ().top);
	getTransform ().inverse ().transform (where2);

	if (buttons.isShiftSet ())
	{
		if (!buttons.isControlSet ())
			getSelection ()->clear ();

		mouseEditMode = MouseEditMode::WaitLasso;
		dragStartMouseObserver.init (where);
		mouseStartPoint = where2;
		return kMouseEventHandled;
	}

	SharedPointer<CView> selectionHitView;
	MouseSizeMode sizeMode = selectionHitTest (where, selectionHitView);
	auto mouseHitView =
		getViewAt (where, GetViewOptions ().deep ().includeViewContainer ().includeInvisible ());
	if (selectionHitView == nullptr && mouseHitView == nullptr)
	{
		getSelection ()->clear ();
		return kMouseEventHandled;
	}
	if (getSelection ()->contains (*mouseHitView.get ()))
	{
		if (buttons.isControlSet ())
		{
			getSelection ()->remove (mouseHitView);
			onMouseMoved (where, CButtonState (buttons.getModifierState ()));
			return kMouseEventHandled;
		}
	}
	else if (mouseHitView && sizeMode == MouseSizeMode::None)
	{
		if (buttons.isControlSet ())
		{
			getSelection ()->add (mouseHitView);
			selectionHitView = mouseHitView;
			onMouseMoved (where, CButtonState (buttons.getModifierState ()));
		}
		else if (selectionHitView == nullptr || selectionHitView == getEditView ())
		{
			getSelection ()->setExclusive (mouseHitView);
			selectionHitView = mouseHitView;
			onMouseMoved (where, CButtonState (buttons.getModifierState ()));
		}
	}
	if (selectionHitView)
	{
		if (buttons.isDoubleClick ())
		{
			onDoubleClickEditing (*selectionHitView.get ());
			return kMouseEventHandled;
		}
		if (buttons.isAltSet () && !getSelection ()->contains (*getEditView ().get ()))
		{
			mouseEditMode = MouseEditMode::WaitDrag;
			dragStartMouseObserver.init (where);
			return kMouseEventHandled;
		}
		if (sizeMode == MouseSizeMode::None)
		{
			if (getSelection ()->contains (*getEditView ().get ()))
			{
				return kMouseEventHandled;
			}
			mouseEditMode = MouseEditMode::DragEditing;
			mouseStartPoint = where2;
			if (gridProcessor)
				gridProcessor->process (mouseStartPoint);
			editTimer = makeShared<CVSTGUITimer> (
				[this] (auto&&) {
					if (lines == nullptr)
					{
						lines = makeShared<UICrossLines> (
							shared (this), UICrossLines::kSelectionStyle, crosslineBackgroundColor,
							crosslineForegroundColor);
						overlayView->addSubview (lines);
						lines->update (*selection.get ());
						getFrame ()->setCursor (kCursorMoveObject);
					}
					editTimer.reset ();
				},
				500);
			return kMouseEventHandled;
		}
		else
		{
			mouseEditMode = MouseEditMode::SizeEditing;
			mouseStartPoint = where2;
			if (gridProcessor)
				gridProcessor->process (mouseStartPoint);
			mouseSizeMode = sizeMode;
			if (true)
			{
				int32_t crossLineMode = 0;
				switch (sizeMode)
				{
					case MouseSizeMode::Left:
					case MouseSizeMode::Right:
					case MouseSizeMode::Top:
					case MouseSizeMode::Bottom: crossLineMode = UICrossLines::kSelectionStyle; break;
					default : crossLineMode = UICrossLines::kDragStyle; break;
				}
				lines =
					makeShared<UICrossLines> (shared (this), crossLineMode,
											  crosslineBackgroundColor, crosslineForegroundColor);
				overlayView->addSubview (lines);
				if (crossLineMode == UICrossLines::kSelectionStyle)
					lines->update (*selection.get ());
				else
					lines->update (CPoint (mouseStartPoint.x, mouseStartPoint.y));
			}
			return kMouseEventHandled;
		}
	}
	return kMouseEventHandled;
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIEditView::onMouseUp (CPoint &where, const CButtonState& buttons)
{
	if (!editing)
		return CViewContainer::onMouseUp (where, buttons);

	editTimer = nullptr;
	if (mouseEditMode == MouseEditMode::LassoSelection)
	{
		CPoint where2 (where);
		where2.offset (-getViewSize ().left, -getViewSize ().top);
		getTransform ().inverse ().transform (where2);

		CRect area;
		area.setTopLeft (mouseStartPoint);
		area.setBottomRight (where2);
		area.normalize ();
		auto result = findChildsInArea (*getEditView ()->asViewContainer (), area);
		for (auto& view : result)
		{
			if (IViewFactory::getViewName (*view.get ()) &&
				!getSelection ()->contains (*view.get ()))
				getSelection ()->add (view);
		}
	}
	else if (mouseEditMode != MouseEditMode::NoEditing && !moveSizeOperation && buttons == kLButton && !lines)
	{
		auto view = getViewAt (
			where, GetViewOptions ().deep ().includeViewContainer ().includeInvisible ());
		if (view.get () == this)
			view = nullptr;
		if (view)
		{
			getSelection ()->setExclusive (view);
		}
	}
	if (lines)
	{
		overlayView->removeSubview (lines);
		lines.reset ();
	}
	mouseEditMode = MouseEditMode::NoEditing;
	if (moveSizeOperation)
	{
		if (moveSizeOperation->didChange ())
			getUndoManager ()->pushAndPerform (moveSizeOperation);
		moveSizeOperation.reset ();
	}
	onMouseMoved (where, CButtonState (buttons.getModifierState ()));
	return kMouseEventHandled;
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIEditView::onMouseMoved (CPoint &where, const CButtonState& buttons)
{
	if (!editing)
		return CViewContainer::onMouseMoved (where, buttons);
	if (inlineAttrTextEditOpen)
		return kMouseEventHandled;

	CPoint where2 (where);
	where2.offset (-getViewSize ().left, -getViewSize ().top);
	getTransform ().inverse ().transform (where2);
	if (buttons & kLButton)
	{
		if (mouseEditMode == MouseEditMode::LassoSelection)
		{
			if (lines)
			{
				CRect r;
				r.setTopLeft (mouseStartPoint);
				r.setBottomRight (where2);
				r.normalize ();
				lines->update (r);
			}
			getFrame ()->setCursor (kCursorDefault);
		}
		else if (mouseEditMode == MouseEditMode::WaitLasso && buttons.isShiftSet ())
		{
			if (dragStartMouseObserver.shouldStartDrag (where))
			{
				mouseEditMode = MouseEditMode::LassoSelection;
				lines = makeShared<UICrossLines> (shared (this), UICrossLines::kLassoStyle,
												  lassoFrameColor, lassoFillColor);
				overlayView->addSubview (lines);
				getFrame ()->setCursor (kCursorDefault);
				CViewContainer::onMouseMoved (where, buttons);
			}
		}
		else if (getSelection ()->total () > 0)
		{
			if (mouseEditMode == MouseEditMode::DragEditing)
			{
				doDragEditingMove (where2);
			}
			else if (mouseEditMode == MouseEditMode::SizeEditing)
			{
				doSizeEditingMove (where2);
			}
			else if (mouseEditMode == MouseEditMode::WaitDrag)
			{
				if (dragStartMouseObserver.shouldStartDrag (where))
				{
					mouseEditMode = MouseEditMode::DragEditing;
					invalidSelection ();
					startDrag (where);
					mouseEditMode = MouseEditMode::NoEditing;
					invalidSelection ();
				}
			}
		}
		if (auto scrollView = dynamic_cast<CScrollView*> (getParentView ()->getParentView ()))
		{
			scrollView->makeRectVisible (CRect (where, CPoint (1, 1)));
		}
		return kMouseEventHandled;
	}
	else if (buttons.getButtonState () == 0 && !buttons.isShiftSet ())
	{
		SharedPointer<CView> view;
		CCursorType ctype = kCursorDefault;
		auto mode = selectionHitTest (where, view);
		if (view)
		{
			switch (mode)
			{
				case MouseSizeMode::Right:
				case MouseSizeMode::Left: ctype = kCursorHSize; break;
				case MouseSizeMode::Top:
				case MouseSizeMode::Bottom: ctype = kCursorVSize; break;
				case MouseSizeMode::TopLeft:
				case MouseSizeMode::BottomRight: ctype = kCursorNWSESize; break;
				case MouseSizeMode::TopRight:
				case MouseSizeMode::BottomLeft: ctype = kCursorNESWSize; break;
				case MouseSizeMode::None:
				{
					if (getSelection ()->contains (*getEditView ().get ()) == false)
						ctype = kCursorMovableObject;
					break;
				}
				default: ctype = kCursorDefault; break;
			}
		}
		getFrame ()->setCursor (ctype);
	}
	else
		getFrame ()->setCursor (kCursorDefault);
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
void UIEditView::onKeyboardEvent (KeyboardEvent& event)
{
	if (mouseEditMode != MouseEditMode::NoEditing && event.virt == VirtualKey::Escape &&
	    event.type == EventType::KeyDown)
	{
		if (lines)
		{
			overlayView->removeSubview (lines);
			lines.reset ();
		}
		if (moveSizeOperation)
		{
			moveSizeOperation->undo ();
			moveSizeOperation.reset ();
		}
		mouseEditMode = MouseEditMode::NoEditing;
		getFrame ()->setCursor (kCursorDefault);
		event.consumed = true;
		return;
	}
	CViewContainer::onKeyboardEvent (event);
}

//-----------------------------------------------------------------------------
void UIEditView::doKeyMove (const CPoint& delta)
{
	if (delta.x != 0. || delta.y != 0.)
	{
		if (getSelection ()->contains (*getEditView ().get ()))
			return;
		if (!moveSizeOperation)
			moveSizeOperation = makeShared<ViewSizeChangeOperation> (selection, false, autosizing);
		getSelection ()->moveBy (delta);
		if (moveSizeOperation)
		{
			getUndoManager ()->pushAndPerform (moveSizeOperation);
			moveSizeOperation.reset ();
		}
	}
}

//-----------------------------------------------------------------------------
void UIEditView::doKeySize (const CPoint& delta)
{
	if (delta.x != 0. || delta.y != 0.)
	{
		if (!moveSizeOperation)
			moveSizeOperation = makeShared<ViewSizeChangeOperation> (selection, true, autosizing);
		getSelection ()->viewsWillChange ();
		for (auto view : *selection.get ())
		{
			CRect viewSize = view->getViewSize ();
			CPoint bottomRight = viewSize.getBottomRight ();
			bottomRight += delta;
			viewSize.setBottomRight (bottomRight);
			view->setViewSize (viewSize);
			view->setMouseableArea (viewSize);
		}
		getSelection ()->viewsDidChange ();
		getUndoManager ()->pushAndPerform (moveSizeOperation);
		moveSizeOperation.reset ();
	}
}

//----------------------------------------------------------------------------------------------------
std::vector<SharedPointer<CView>> UIEditView::findChildsInArea (CViewContainer& view, CRect r) const
{
	std::vector<SharedPointer<CView>> views;
	view.forEachChild ([&] (auto child) {
		if (r.rectOverlap (child->getViewSize ()))
		{
			if (auto container = child->asViewContainer ())
			{
				auto r2 = r;
				auto viewSize = container->getViewSize ();
				r2.bound (viewSize);
				if (!r2.isEmpty ())
				{
					r2.offsetInverse (viewSize.getTopLeft ());
					auto res2 = findChildsInArea (*container, r2);
					std::move (res2.begin (), res2.end (), std::back_inserter (views));
				}
			}
			else
			{
				views.push_back (child);
			}
		}
	});

	return views;
}

//----------------------------------------------------------------------------------------------------
void UIEditView::doDragEditingMove (CPoint& where)
{
	if (gridProcessor)
		gridProcessor->process (where);
	CPoint diff (where.x - mouseStartPoint.x, where.y - mouseStartPoint.y);
	if (diff.x != 0. || diff.y != 0.)
	{
		if (!moveSizeOperation)
			moveSizeOperation = makeShared<ViewSizeChangeOperation> (selection, false, autosizing);
		getSelection ()->moveBy (diff);
		mouseStartPoint = where;
		if (editTimer)
		{
			editTimer = nullptr;
			if (!lines)
			{
				lines =
					makeShared<UICrossLines> (shared (this), UICrossLines::kSelectionStyle,
											  crosslineBackgroundColor, crosslineForegroundColor);
				overlayView->addSubview (lines);
				lines->update (*selection.get ());
			}
			getFrame ()->setCursor (kCursorMoveObject);
		}
		if (lines)
			lines->update (*selection.get ());
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditView::doSizeEditingMove (CPoint& where)
{
	if (!moveSizeOperation)
		moveSizeOperation = makeShared<ViewSizeChangeOperation> (selection, true, autosizing);
	if (gridProcessor)
		gridProcessor->process (where);
	if (mouseStartPoint == where)
		return;

	CRect diff;
	switch (mouseSizeMode)
	{
		case MouseSizeMode::Left:
		{
			diff.left = where.x - mouseStartPoint.x;
			break;
		}
		case MouseSizeMode::Right:
		{
			diff.right = where.x - mouseStartPoint.x;
			break;
		}
		case MouseSizeMode::Top:
		{
			diff.top = where.y - mouseStartPoint.y;
			break;
		}
		case MouseSizeMode::Bottom:
		{
			diff.bottom = where.y - mouseStartPoint.y;
			break;
		}
		case MouseSizeMode::TopLeft:
		{
			diff.left = where.x - mouseStartPoint.x;
			diff.top = where.y - mouseStartPoint.y;
			break;
		}
		case MouseSizeMode::TopRight:
		{
			diff.right = where.x - mouseStartPoint.x;
			diff.top = where.y - mouseStartPoint.y;
			break;
		}
		case MouseSizeMode::BottomRight:
		{
			diff.right = where.x - mouseStartPoint.x;
			diff.bottom = where.y - mouseStartPoint.y;
			break;
		}
		case MouseSizeMode::BottomLeft:
		{
			diff.left = where.x - mouseStartPoint.x;
			diff.bottom = where.y - mouseStartPoint.y;
			break;
		}
		default: break;
	}
	std::vector<bool> oldAutosizeState;
	if (!autosizing)
	{
		for (auto& view : *selection.get ())
		{
			if (auto container = view->asViewContainer ())
			{
				oldAutosizeState.emplace_back (container->getAutosizingEnabled ());
				container->setAutosizingEnabled (false);
			}
		}
	}
	selection->sizeBy (diff);
	if (!autosizing)
	{
		size_t index = 0;
		for (auto& view : *selection.get ())
		{
			if (auto container = view->asViewContainer ())
			{
				container->setAutosizingEnabled (oldAutosizeState[index]);
				++index;
			}
		}
	}
	mouseStartPoint = where;
	if (lines)
	{
		if (lines->getStyle () == UICrossLines::kSelectionStyle)
			lines->update (*selection.get ());
		else
			lines->update (mouseStartPoint);
	}
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIEditView::onMouseExited (CPoint& where, const CButtonState& buttons)
{
	getFrame()->setCursor (kCursorDefault);
	return kMouseEventHandled;
}

//----------------------------------------------------------------------------------------------------
void UIEditView::startDrag (CPoint& where)
{
	auto bitmap = createBitmapFromSelection (*selection.get (), getFrame ()->getScaleFactor (),
											 shared (this));
	if (bitmap == nullptr)
		return;
	where.makeIntegral ();

	CRect selectionBounds = getSelection ()->getBounds ();
	CPoint p;
	getParentView ()->localToFrame (p);
	selectionBounds.offsetInverse (p);

	CPoint offset;
	offset.x = (selectionBounds.left - where.x);
	offset.y = (selectionBounds.top - where.y);

	getSelection ()->setDragOffset (CPoint (offset.x, offset.y));

	std::string templateName;
	if (description->getTemplateNameFromView (*getEditView ().get (), templateName))
		description->updateViewDescription (templateName.c_str (), getEditView ());

	CMemoryStream stream (1024, 1024, false);
	if (!getSelection ()->store (stream, description))
		return;
	stream.end ();

	auto callback = makeShared<DragCallbackFunctions> ();
	callback->endedFunc = [this] (const auto&, CPoint pos, auto) {
		frameToLocal (pos);
		onMouseMoved (pos, 0);
	};

	auto dropSource = CDropSource::create (stream.getBuffer (), static_cast<uint32_t>(stream.tell ()), CDropSource::kText);
	doDrag (DragDescription (dropSource, offset, bitmap), callback);
}

//----------------------------------------------------------------------------------------------------
SharedPointer<UISelection> UIEditView::getSelectionOutOfDrag (const IDataPackage& drag) const
{
	IDataPackage::Type type;
	const void* dragData;
	uint32_t size;
	if ((size = drag.getData (0, dragData, type)) > 0 && type == IDataPackage::kText)
	{
		auto oldController = description->getController ();
		if (auto* controller =
				getFrame () ? dynamic_cast<IController*> (getFrame ()->getEditor ()) : nullptr)
		{
			description->setController (shared (controller));
		}
		CMemoryStream stream (static_cast<const int8_t*> (dragData), size, false);
		auto newSelection = makeShared<UISelection> ();
		if (newSelection->restore (stream, description))
		{
			description->setController (oldController);
			return newSelection;
		}
		description->setController (oldController);
	}
	return nullptr;
}

//----------------------------------------------------------------------------------------------------
SharedPointer<IDropTarget> UIEditView::getDropTarget ()
{
	if (editing)
		return shared (this);
	return CViewContainer::getDropTarget ();
}

//----------------------------------------------------------------------------------------------------
bool UIEditView::onDrop (DragEventData data)
{
	if (lines)
	{
		overlayView->removeSubview (lines);
		lines.reset ();
	}
	if (dragSelection)
	{
		if (highlightView)
		{
			highlightView->setHighlightView (nullptr);
		}
		CPoint where2 (data.pos);
		where2.offset (dragSelection->getDragOffset ().x, dragSelection->getDragOffset ().y);
		if (gridProcessor)
		{
			getTransform ().inverse ().transform (where2);
			gridProcessor->process (where2);
			getTransform ().transform (where2);
		}
		auto viewContainer = getContainerAt (where2, GetViewOptions ().deep ());
		if (viewContainer && viewContainer.get () != this)
		{
			where2.offset (-getViewSize ().left, -getViewSize ().top);
			getTransform ().inverse ().transform (where2);
			CPoint containerOffset;
			viewContainer->localToFrame (containerOffset);
			frameToLocal (containerOffset);
			where2.offset (-containerOffset.x, -containerOffset.y);

			where2.makeIntegral ();
			auto action = makeShared<ViewCopyOperation> (dragSelection, getSelection (),
														 viewContainer, where2, description);
			getUndoManager()->pushAndPerform (action);
		}
		dragSelection = nullptr;
	}
	return true;
}

//----------------------------------------------------------------------------------------------------
DragOperation UIEditView::onDragEnter (DragEventData data)
{
	if (data.drag)
	{
		dragSelection = getSelectionOutOfDrag (*data.drag.get ());
		if (dragSelection)
		{
			if (!lines)
			{
				lines =
					makeShared<UICrossLines> (shared (this), UICrossLines::kDragStyle,
											  crosslineBackgroundColor, crosslineForegroundColor);
				overlayView->addSubview (lines);
			}
			return onDragMove (data);
		}
	}
	return DragOperation::None;
}

//----------------------------------------------------------------------------------------------------
void UIEditView::onDragLeave (DragEventData data)
{
	dragSelection = nullptr;
	if (highlightView)
	{
		highlightView->setHighlightView (nullptr);
	}
	if (lines)
	{
		overlayView->removeSubview (lines);
		lines.reset ();
	}
}

//----------------------------------------------------------------------------------------------------
DragOperation UIEditView::onDragMove (DragEventData data)
{
	if (editing)
	{
		if (lines)
		{
			if (dragSelection)
			{
				CPoint where2 (data.pos);
				where2.offset (dragSelection->getDragOffset ().x, dragSelection->getDragOffset ().y);
				where2.offset (-getViewSize ().left, -getViewSize ().top);
				if (gridProcessor)
				{
					getTransform ().inverse ().transform (where2);
					gridProcessor->process (where2);
					getTransform ().transform (where2);
				}
				CPoint where3 (where2);
				getTransform ().inverse ().transform (where3);
				lines->update (where3);
				if (highlightView)
				{
					CRect visibleRect = getVisibleViewSize ();
					where2.offset (getViewSize ().left, getViewSize ().top);
					where2.offset (-visibleRect.left, -visibleRect.top);
					auto container = getContainerAt (where2, GetViewOptions ().deep ());
					if (container.get () == this)
					{
						container = nullptr;
						highlightView->setHighlightView (nullptr);
						return DragOperation::None;
					}
					highlightView->setHighlightView (container);
				}
				return DragOperation::Copy;
			}
		}
	}
	return DragOperation::None;
}

//-----------------------------------------------------------------------------
void UIEditView::onDoubleClickEditing (CView& view)
{
	struct AttributeInlineEditorController : ViewListenerAdapter
	{
		using Callback = std::function<void ()>;
		AttributeInlineEditorController (const SharedPointer<CTextEdit>& edit, Callback&& callback)
		: edit (edit), callback (std::move (callback))
		{
			edit->registerViewListener (this);
		}
		void viewWillDelete (CView&) override
		{
			edit->unregisterViewListener (this);
			delete this;
		}
		void viewLostFocus (CView&) override
		{
			callback ();
		}

	private:
		SharedPointer<CTextEdit> edit;
		Callback callback;
	};

	const auto& factory = description->getViewFactory ();
	std::string attrValue;
	if (!factory.getAttributeValue (view, UIViewCreator::kAttrTitle, attrValue,
									*description.get ()))
		return;

	auto frame = getFrame ();
	frame->setCursor (kCursorDefault);

	auto r = selection->getGlobalViewCoordinates (view);
	r.offsetInverse (getViewSize ().getTopLeft ());
	translateToLocal (r, true);
	auto textEdit = makeShared<CTextEdit> (r, nullptr, 0);
	textEdit->setText (attrValue.data ());
	addSubview (textEdit);
	new AttributeInlineEditorController (textEdit, [this, textEdit, attrValue] () {
		const auto& text = textEdit->getText ();
		if (text != attrValue)
		{
			auto action = makeShared<AttributeChangeAction> (
				description, selection, UIViewCreator::kAttrTitle, text.getString ());
			getUndoManager ()->pushAndPerform (action);
		}
		textEdit->getParentView ()->removeSubview (textEdit);
		inlineAttrTextEditOpen = false;
	});
	frame->setFocusView (textEdit);
	inlineAttrTextEditOpen = true;
}

//-----------------------------------------------------------------------------
void UIEditView::looseFocus ()
{
	if (editing == false)
		CViewContainer::looseFocus ();
}

//-----------------------------------------------------------------------------
void UIEditView::takeFocus ()
{
	if (editing == false)
		CViewContainer::takeFocus ();
}

//-----------------------------------------------------------------------------
bool UIEditView::attached (CViewContainer& parent)
{
	if (CViewContainer::attached (parent))
	{
		editing = !editing;
		enableEditing (!editing);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool UIEditView::removed (CViewContainer& parent)
{
	auto frame = getFrame ();
	if (editingViewAddedObserver)
	{
		frame->setViewAddedRemovedObserver (nullptr);
		editingViewAddedObserver.reset ();
	}
	if (overlayView)
	{
		frame->removeSubview (overlayView);
		overlayView.reset ();
	}
	frame->setCursor (kCursorDefault);
	return CViewContainer::removed (parent);
}

//-----------------------------------------------------------------------------
void UIEditView::setupColors (const IUIDescription& desc)
{
	desc.getColor ("editView.crosslines.background", crosslineBackgroundColor);
	desc.getColor ("editView.crosslines.foreground", crosslineForegroundColor);
	desc.getColor ("editView.lasso.fill", lassoFillColor);
	desc.getColor ("editView.lasso.frame", lassoFrameColor);
	desc.getColor ("editView.view.highlight", viewHighlightColor);
	desc.getColor ("editView.view.selection", viewSelectionColor);
}

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
