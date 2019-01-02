// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uieditview.h"

#if VSTGUI_LIVE_EDITING

#include "uiundomanager.h"
#include "uiactions.h"
#include "uicrosslines.h"
#include "uigrid.h"
#include "uiselection.h"
#include "../icontroller.h"
#include "../uiattributes.h"
#include "../uidescription.h"
#include "../uiviewfactory.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../../lib/cvstguitimer.h"
#include "../../lib/cframe.h"
#include "../../lib/cscrollview.h"
#include "../../lib/cdropsource.h"
#include "../../lib/coffscreencontext.h"
#include "../../lib/clayeredviewcontainer.h"
#include "../../lib/dragging.h"
#include "../../lib/idatapackage.h"
#include "../../lib/controls/ctextedit.h"
#include <cassert>

namespace VSTGUI {

namespace UIEditViewInternal {
	
//----------------------------------------------------------------------------------------------------
class UIEditViewOverlay : public CView, public ViewListenerAdapter
//----------------------------------------------------------------------------------------------------
{
public:
	UIEditViewOverlay (CViewContainer* editView);
	~UIEditViewOverlay () override;
	
	bool attached (CView* parent) override;
	void viewSizeChanged (CView* view, const CRect& oldSize) override;
protected:
	CViewContainer* editView;
};

//----------------------------------------------------------------------------------------------------
UIEditViewOverlay::UIEditViewOverlay (CViewContainer* editView)
: CView (CRect (0, 0, 0, 0))
, editView (editView)
{
	setMouseEnabled (false);
	editView->getParentView ()->registerViewListener (this);
	editView->registerViewListener (this);
}

//----------------------------------------------------------------------------------------------------
UIEditViewOverlay::~UIEditViewOverlay ()
{
	editView->getParentView ()->unregisterViewListener (this);
	editView->unregisterViewListener (this);
}

//----------------------------------------------------------------------------------------------------
bool UIEditViewOverlay::attached (CView* parent)
{
	auto result = CView::attached (parent);
	viewSizeChanged (editView->getParentView (), CRect (0, 0, 0, 0));
	return result;
}

//----------------------------------------------------------------------------------------------------
void UIEditViewOverlay::viewSizeChanged (CView* view, const CRect& oldSize)
{
	invalid ();
	CRect r = editView->getVisibleViewSize ();
	r.originize ();
	CPoint p;
	editView->getParentView ()->localToFrame (p);
	r.offset (p.x, p.y);
	setViewSize (r);
	invalid ();
}

//----------------------------------------------------------------------------------------------------
class UISelectionView : public UIEditViewOverlay, public UISelectionListenerAdapter
//----------------------------------------------------------------------------------------------------
{
public:
	UISelectionView (CViewContainer* editView, UISelection* selection, const CColor& selectionColor, CCoord handleSize);
	~UISelectionView () override;

private:
	void draw (CDrawContext* pContext) override;
	void drawResizeHandle (const CPoint& p, CDrawContext* pContext);

	void selectionWillChange (UISelection* selection) override { onSelectionChanged (); }
	void selectionDidChange (UISelection* selection) override { onSelectionChanged (); }
	void selectionViewsWillChange (UISelection* selection) override { onSelectionChanged (); }
	void selectionViewsDidChange (UISelection* selection) override { onSelectionChanged (); }

	void onSelectionChanged ();

	SharedPointer<UISelection> selection;
	CColor selectionColor;
	CCoord handleInset;
};

//----------------------------------------------------------------------------------------------------
UISelectionView::UISelectionView (CViewContainer* editView, UISelection* selection, const CColor& selectionColor, CCoord handleSize)
: UIEditViewOverlay (editView)
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
void UISelectionView::drawResizeHandle (const CPoint& p, CDrawContext* pContext)
{
	CRect r (p.x, p.y, p.x, p.y);
	r.inset (-handleInset, -handleInset);
	pContext->drawRect (r, kDrawFilledAndStroked);
}

//----------------------------------------------------------------------------------------------------
void UISelectionView::draw (CDrawContext* pContext)
{
	auto lineWidth = pContext->getHairlineSize ();
	CRect r (getVisibleViewSize ());
	ConcatClip cc (*pContext, r);
	pContext->setDrawMode (kAliasing);
	pContext->setLineStyle (kLineSolid);
	pContext->setLineWidth (lineWidth);

	CColor lightColor (kWhiteCColor);
	lightColor.alpha = 140;
	pContext->setFillColor (lightColor);

	CView* mainView = editView->getView (0);
	CPoint p;
	frameToLocal (p);
	for (auto view : *selection)
	{
		CRect vs = selection->getGlobalViewCoordinates (view);
		vs.offsetInverse (p);
		vs.extend (lineWidth, lineWidth);
		pContext->setFrameColor (lightColor);
		pContext->drawRect (vs);
		vs.inset (lineWidth, lineWidth);
		pContext->setFrameColor (selectionColor);
		pContext->drawRect (vs);
		if (vs.getWidth () > handleInset * 2. && vs.getHeight () > handleInset * 2.)
		{
			drawResizeHandle (vs.getBottomRight (), pContext);
			if (view != mainView)
			{
				drawResizeHandle (vs.getTopLeft (), pContext);
				drawResizeHandle (vs.getBottomLeft (), pContext);
				drawResizeHandle (vs.getTopRight (), pContext);
				if (vs.getHeight () > handleInset * 4)
				{
					CPoint hp (vs.getTopLeft ());
					hp.y += vs.getHeight ()/ 2.;
					drawResizeHandle (hp, pContext);
					hp = vs.getTopRight ();
					hp.y += vs.getHeight ()/ 2.;
					drawResizeHandle (hp, pContext);
				}
				if (vs.getWidth () > handleInset * 4)
				{
					CPoint hp (vs.getTopLeft ());
					hp.x += vs.getWidth ()/ 2.;
					drawResizeHandle (hp, pContext);
					hp = vs.getBottomLeft ();
					hp.x += vs.getWidth ()/ 2.;
					drawResizeHandle (hp, pContext);
				}
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UISelectionView::onSelectionChanged ()
{
	CPoint p;
	frameToLocal (p);
	for (auto view : *selection)
	{
		CRect vs = selection->getGlobalViewCoordinates (view);
		vs.offsetInverse (p);
		vs.extend (handleInset + 2, handleInset + 2);
		invalidRect (vs);
	}
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
class UIHighlightView : public UIEditViewOverlay
{
public:
	UIHighlightView (CViewContainer* editView, const CColor& viewHighlightColor);

	void setHighlightView (CView* view);
private:
	void draw (CDrawContext* pContext) override;

	CView* highlightView;
	CColor strokeColor;
	CColor fillColor;
};

//----------------------------------------------------------------------------------------------------
UIHighlightView::UIHighlightView (CViewContainer* editView, const CColor& viewHighlightColor)
: UIEditViewOverlay (editView)
, highlightView (nullptr)
, strokeColor (viewHighlightColor)
{
	double h,s,l;
	strokeColor.toHSL (h, s, l);
	l /= 2.;
	fillColor.fromHSL (h, s, l);
	fillColor.alpha = strokeColor.alpha;
}

//----------------------------------------------------------------------------------------------------
void UIHighlightView::setHighlightView (CView* view)
{
	if (highlightView != view)
	{
		highlightView = view;
		invalid ();
	}
}

//----------------------------------------------------------------------------------------------------
void UIHighlightView::draw (CDrawContext* pContext)
{
	if (highlightView == nullptr)
		return;
	CRect r = UISelection::getGlobalViewCoordinates (highlightView);
	CPoint p;
	frameToLocal (p);
	r.offsetInverse (p);
	r.inset (2, 2);
	pContext->setFillColor (fillColor);
	pContext->setFrameColor (strokeColor);
	pContext->setLineStyle (kLineSolid);
	pContext->setLineWidth (3);
	pContext->drawRect (r, kDrawFilledAndStroked);
}

} // UIEditViewInternal

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
IdStringPtr UIEditView::kMsgAttached = "UIEditView::kMsgAttached";
IdStringPtr UIEditView::kMsgRemoved = "UIEditView::kMsgRemoved";

static const CCoord kResizeHandleSize = 6.;

//----------------------------------------------------------------------------------------------------
UIEditView::UIEditView (const CRect& size, UIDescription* uidescription)
: CViewContainer (size)
, description (uidescription)
, grid (nullptr)
{
	setScale (1.);
	setWantsFocus (true);
}

//----------------------------------------------------------------------------------------------------
UIEditView::~UIEditView ()
{
	if (editTimer)
		editTimer->forget ();
	setUndoManager (nullptr);
	setSelection (nullptr);
}

//----------------------------------------------------------------------------------------------------
void UIEditView::setScale (double scale)
{
	scale = std::round (scale * 100) / 100.;
	setTransform (CGraphicsTransform ().scale (scale, scale));
	if (CView* view = getEditView ())
	{
		CRect r (getViewSize ());
		CPoint size (view->getWidth (), view->getHeight ());
		getTransform ().transform (size);
		r.setSize (size);
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
void UIEditView::enableEditing (bool state)
{
	if (editing != state)
	{
		editing = state;
		invalid ();
		CFrame* parent = getFrame ();
		if (parent == nullptr)
			return;
		
		if (editing)
		{
			CRect r = parent->getViewSize ();
			r.originize ();
			vstgui_assert (overlayView == nullptr);
			overlayView = new CLayeredViewContainer (r);
			overlayView->setAutosizeFlags (kAutosizeAll);
			overlayView->setMouseEnabled (false);
			overlayView->setTransparency (true);
			overlayView->setZIndex (1);
			parent->addView (overlayView);
			
			highlightView = new UIEditViewInternal::UIHighlightView (this, viewHighlightColor);
			overlayView->addView (highlightView);
			auto selectionView = new UIEditViewInternal::UISelectionView (this, getSelection (), viewSelectionColor, kResizeHandleSize);
			overlayView->addView (selectionView);
		}
		else
		{
			parent->removeView (overlayView);
			overlayView = nullptr;
			highlightView = nullptr;
			lines = nullptr;
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditView::enableAutosizing (bool state)
{
	autosizing = state;
}

//----------------------------------------------------------------------------------------------------
void UIEditView::setUndoManager (UIUndoManager* manager)
{
	undoManger = manager;
}

//----------------------------------------------------------------------------------------------------
UIUndoManager* UIEditView::getUndoManager ()
{
	if (undoManger == nullptr)
		undoManger = makeOwned<UIUndoManager> ();
	return undoManger;
}

//----------------------------------------------------------------------------------------------------
void UIEditView::setSelection (UISelection* inSelection)
{
	selection = inSelection;
}

//----------------------------------------------------------------------------------------------------
UISelection* UIEditView::getSelection ()
{
	if (selection == nullptr)
	{
		selection = makeOwned<UISelection> ();
	}
	return selection;
}

//----------------------------------------------------------------------------------------------------
void UIEditView::setGrid (UIGrid* grid)
{
	this->grid = grid;
}

//----------------------------------------------------------------------------------------------------
void UIEditView::setEditView (CView* view)
{
	if (view != getView (0))
	{
		invalid ();
		removeAll ();
		CRect vs (getViewSize ());
		if (view)
		{
			vs.setWidth (view->getWidth ());
			vs.setHeight(view->getHeight ());
			vs.offset (-getViewSize ().left, -getViewSize ().top);
			getTransform ().transform (vs);
			vs.offset (getViewSize ().left, getViewSize ().top);
			setViewSize (vs);
			setMouseableArea (getViewSize ());
			addView (view);
		}
		else
		{
			vs.setWidth (0);
			vs.setHeight (0);
			setMouseableArea (vs);
			setViewSize (vs);
		}
		invalid ();
	}
}

//----------------------------------------------------------------------------------------------------
CView* UIEditView::getEditView () const
{
	return getView (0);
}

//----------------------------------------------------------------------------------------------------
CMessageResult UIEditView::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == CVSTGUITimer::kMsgTimer)
	{
		if (sender == editTimer)
		{
			if (lines == nullptr)
			{
				lines = new UICrossLines (this, UICrossLines::kSelectionStyle, crosslineBackgroundColor, crosslineForegroundColor);
				overlayView->addView (lines);
				lines->update (selection);
				getFrame ()->setCursor (kCursorHand);
			}
			editTimer->forget ();
			editTimer = nullptr;
		}
		return kMessageNotified;
	}
	else if (message == kMsgViewSizeChanged)
	{
		CView* view = dynamic_cast<CView*> (sender);
		if (view && view == getView (0))
		{
			CRect r = getViewSize ();
			r.setWidth (view->getWidth ());
			r.setHeight (view->getHeight ());
			getTransform ().transform (r);
			if (r != getViewSize ())
			{
				setAutosizingEnabled (false);
				setViewSize (r);
				setMouseableArea (r);
				setAutosizingEnabled (true);
				getParentView ()->invalid ();
			}
		}
	}
	return CViewContainer::notify (sender, message);
}

//----------------------------------------------------------------------------------------------------
void UIEditView::draw (CDrawContext *pContext)
{
	drawRect (pContext, getViewSize ());
}

//----------------------------------------------------------------------------------------------------
void UIEditView::drawRect (CDrawContext *pContext, const CRect& updateRect)
{
	// disable focus drawing
	bool focusDrawing = getFrame ()->focusDrawingEnabled ();
	if (!editing && focusDrawing)
		getFrame ()->setFocusDrawingEnabled (false);

	CViewContainer::drawRect (pContext, updateRect);

	if (!editing && focusDrawing)
		getFrame ()->setFocusDrawingEnabled (focusDrawing);

	CRect oldClip = pContext->getClipRect (oldClip);
	CRect newClip (updateRect);
	newClip.offset (-getViewSize ().left, -getViewSize ().top);
	pContext->setClipRect (updateRect);

	CDrawContext::Transform transform (*pContext, CGraphicsTransform ().translate (getViewSize ().left, getViewSize ().top));

	const CCoord dashLength[] = {5, 5};
	const CLineStyle lineDash (CLineStyle::kLineCapButt, CLineStyle::kLineJoinMiter, 0, 2, dashLength);
	pContext->setLineStyle (lineDash);
	pContext->setLineWidth (1);
	pContext->setDrawMode (kAliasing);
	pContext->setFrameColor (kBlueCColor);
	pContext->drawRect (CRect (0, 0, getWidth (), getHeight ()), kDrawStroked);
}

//----------------------------------------------------------------------------------------------------
CView* UIEditView::getViewAt (const CPoint& p, const GetViewOptions& options) const
{
	CView* view = CViewContainer::getViewAt (p, options);
	if (editing)
	{
		auto factory = static_cast<const UIViewFactory*> (description->getViewFactory ());
		if (factory)
		{
			while (view && factory->getViewName (view) == nullptr)
			{
				view = view->getParentView ();
			}
		}
	}
	return view;
}

//----------------------------------------------------------------------------------------------------
CViewContainer* UIEditView::getContainerAt (const CPoint& p, const GetViewOptions& options) const
{
	CViewContainer* view = CViewContainer::getContainerAt (p, options);
	if (editing)
	{
		auto factory = static_cast<const UIViewFactory*> (description->getViewFactory ());
		if (factory)
		{
			while (view && factory->getViewName (view) == nullptr)
			{
				view = view->getParentView ()->asViewContainer ();
			}
		}
	}
	return view;
}

//----------------------------------------------------------------------------------------------------
bool UIEditView::advanceNextFocusView (CView* oldFocus, bool reverse)
{
	if (editing)
		return false;
	return CViewContainer::advanceNextFocusView (oldFocus, reverse);
}

//----------------------------------------------------------------------------------------------------
bool UIEditView::onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const CButtonState &buttons)
{
	if (editing == false)
		return CViewContainer::onWheel (where, axis, distance, buttons);
	return false;
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
UIEditView::MouseSizeMode UIEditView::selectionHitTest (const CPoint& _where, CView** resultView)
{
	CPoint where (_where);
	where.offset (-getViewSize ().left, -getViewSize ().top);
	CPoint p;
	frameToLocal (p);

	CView* mainView = getView (0);
	for (auto it = getSelection ()->rbegin (), end = getSelection ()->rend (); it != end; ++it)
	{
		auto view = (*it);
		CRect r = getSelection ()->getGlobalViewCoordinates (view);
		bool isMainView = (mainView == view) ? true : false;
		r.offset (p);
		r.extend (kResizeHandleSize, kResizeHandleSize);
		if (r.pointInside (where))
		{
			if (resultView)
				*resultView = view;
			r.inset (kResizeHandleSize, kResizeHandleSize);
			if (UIEditViewInternal::pointInResizeHandleRect (where, r.getBottomRight ()))
				return MouseSizeMode::BottomRight;
			if (!isMainView)
			{
				if (UIEditViewInternal::pointInResizeHandleRect (where, r.getBottomLeft ()))
					return MouseSizeMode::BottomLeft;
				if (UIEditViewInternal::pointInResizeHandleRect (where, r.getTopLeft ()))
					return MouseSizeMode::TopLeft;
				if (UIEditViewInternal::pointInResizeHandleRect (where, r.getTopRight ()))
					return MouseSizeMode::TopRight;
				CPoint hp (r.getTopLeft ());
				hp.y += r.getHeight () /2.;
				if (UIEditViewInternal::pointInResizeHandleRect (where, hp))
					return MouseSizeMode::Left;
				hp.x += r.getWidth ();
				if (UIEditViewInternal::pointInResizeHandleRect (where, hp))
					return MouseSizeMode::Right;
				hp = r.getTopLeft ();
				hp.x += r.getWidth () / 2.;
				if (UIEditViewInternal::pointInResizeHandleRect (where, hp))
					return MouseSizeMode::Top;
				hp.y += r.getHeight ();
				if (UIEditViewInternal::pointInResizeHandleRect (where, hp))
					return MouseSizeMode::Bottom;
				if (r.pointInside (where))
					return MouseSizeMode::None;
			}
			if (resultView)
				*resultView = nullptr;
		}
	}
	if (resultView)
		*resultView = nullptr;
	return MouseSizeMode::None;
}

//----------------------------------------------------------------------------------------------------
bool UIEditView::hitTestSubViews (const CPoint& where, const CButtonState& buttons)
{
	return hitTest (where, buttons);
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIEditView::onMouseDown (CPoint &where, const CButtonState& buttons)
{
	if (editing)
	{
		if (buttons.isLeftButton ())
		{
			getFrame ()->setFocusView (this);

			CView* selectionHitView = nullptr;
			CPoint where2 (where);
			where2.offset (-getViewSize ().left, -getViewSize ().top);
			getTransform ().inverse ().transform (where2);
			MouseSizeMode sizeMode = selectionHitTest (where, &selectionHitView);
			CView* mouseHitView = getViewAt (where, GetViewOptions ().deep ().includeViewContainer ().includeInvisible ());
			if (selectionHitView == nullptr && mouseHitView == nullptr)
			{
				getSelection ()->clear ();
				return kMouseEventHandled;
			}
			if (getSelection ()->contains (mouseHitView))
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
				if (buttons.isControlSet () || buttons.isShiftSet ())
				{
					getSelection ()->add (mouseHitView);
					selectionHitView = mouseHitView;
					onMouseMoved (where, CButtonState (buttons.getModifierState ()));
				}
				else if (selectionHitView == nullptr || selectionHitView == getView (0))
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
					onDoubleClickEditing (selectionHitView);
					return kMouseEventHandled;
				}
				if (buttons.isAltSet () && !getSelection ()->contains (getView (0)))
				{
					mouseEditMode = MouseEditMode::DragEditing;
					invalidSelection ();
					startDrag (where);
					mouseEditMode = MouseEditMode::NoEditing;
					invalidSelection ();
					return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
				}
				if (sizeMode == MouseSizeMode::None)
				{
					if (getSelection ()->contains (getView (0)))
					{
						return kMouseEventHandled;
					}
					mouseEditMode = MouseEditMode::DragEditing;
					mouseStartPoint = where2;
					if (grid)
						grid->process (mouseStartPoint);
					if (editTimer)
						editTimer->forget ();
					editTimer = new CVSTGUITimer (this, 500);
					editTimer->start ();
					return kMouseEventHandled;
				}
				else
				{
					mouseEditMode = MouseEditMode::SizeEditing;
					mouseStartPoint = where2;
					if (grid)
						grid->process (mouseStartPoint);
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
						lines = new UICrossLines (this, crossLineMode, crosslineBackgroundColor, crosslineForegroundColor);
						overlayView->addView (lines);
						if (crossLineMode == UICrossLines::kSelectionStyle)
							lines->update (selection);
						else
							lines->update (CPoint (mouseStartPoint.x, mouseStartPoint.y));
					}
					return kMouseEventHandled;
				}
			}
		}
		return kMouseEventHandled;
	}
	return CViewContainer::onMouseDown (where, buttons);
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIEditView::onMouseUp (CPoint &where, const CButtonState& buttons)
{
	if (editing)
	{
		if (editTimer)
		{
			editTimer->forget ();
			editTimer = nullptr;
		}
		if (mouseEditMode != MouseEditMode::NoEditing && !moveSizeOperation && buttons == kLButton && !lines)
		{
			CView* view = getViewAt (where, GetViewOptions ().deep ().includeViewContainer ().includeInvisible ());
			if (view == this)
				view = nullptr;
			if (view)
			{
				getSelection ()->setExclusive (view);
			}
		}
		if (lines)
		{
			overlayView->removeView (lines);
			lines = nullptr;
		}
		mouseEditMode = MouseEditMode::NoEditing;
		if (moveSizeOperation)
		{
			if (moveSizeOperation->didChange ())
				getUndoManager ()->pushAndPerform (moveSizeOperation);
			else
				delete moveSizeOperation;
			moveSizeOperation = nullptr;
		}
		onMouseMoved (where, CButtonState (buttons.getModifierState ()));
		return kMouseEventHandled;
	}
	return CViewContainer::onMouseUp (where, buttons);
}

//------------------------------------------------------------------------
int32_t UIEditView::onKeyDown (VstKeyCode& keyCode)
{
	if (mouseEditMode != MouseEditMode::NoEditing && keyCode.virt == VKEY_ESCAPE)
	{
		if (lines)
		{
			overlayView->removeView (lines);
			lines = nullptr;
		}
		if (moveSizeOperation)
		{
			moveSizeOperation->undo ();
			delete moveSizeOperation;
			moveSizeOperation = nullptr;
		}
		mouseEditMode = MouseEditMode::NoEditing;
		getFrame ()->setCursor (kCursorDefault);
		return 1;
	}
	return CViewContainer::onKeyDown (keyCode);
}

//-----------------------------------------------------------------------------
void UIEditView::doKeyMove (const CPoint& delta)
{
	if (delta.x != 0. || delta.y != 0.)
	{
		if (getSelection ()->contains (getEditView ()))
			return;
		if (!moveSizeOperation)
			moveSizeOperation = new ViewSizeChangeOperation (selection, false, autosizing);
		getSelection ()->moveBy (delta);
		if (moveSizeOperation)
		{
			getUndoManager ()->pushAndPerform (moveSizeOperation);
			moveSizeOperation = nullptr;
		}
	}
}

//-----------------------------------------------------------------------------
void UIEditView::doKeySize (const CPoint& delta)
{
	if (delta.x != 0. || delta.y != 0.)
	{
		if (!moveSizeOperation)
			moveSizeOperation = new ViewSizeChangeOperation (selection, true, autosizing);
		getSelection ()->viewsWillChange ();
		for (auto view : *selection)
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
		moveSizeOperation = nullptr;
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditView::doDragEditingMove (CPoint& where)
{
	if (grid)
		grid->process (where);
	CPoint diff (where.x - mouseStartPoint.x, where.y - mouseStartPoint.y);
	if (diff.x != 0. || diff.y != 0.)
	{
		if (!moveSizeOperation)
			moveSizeOperation = new ViewSizeChangeOperation (selection, false, autosizing);
		getSelection ()->moveBy (diff);
		mouseStartPoint = where;
		if (editTimer)
		{
			editTimer->forget ();
			editTimer = nullptr;
			if (!lines)
			{
				lines = new UICrossLines (this, UICrossLines::kSelectionStyle, crosslineBackgroundColor, crosslineForegroundColor);
				overlayView->addView (lines);
				lines->update (selection);
			}
			getFrame ()->setCursor (kCursorHand);
		}
		if (lines)
			lines->update (selection);
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditView::doSizeEditingMove (CPoint& where)
{
	if (!moveSizeOperation)
		moveSizeOperation = new ViewSizeChangeOperation (selection, true, autosizing);
	if (grid)
	{
		where.offset (grid->getSize ().x / 2., grid->getSize ().y / 2.);
		grid->process (where);
	}
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
		for (auto& view : *selection)
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
		for (auto& view : *selection)
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
			lines->update (selection);
		else
			lines->update (mouseStartPoint);
	}
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIEditView::onMouseMoved (CPoint &where, const CButtonState& buttons)
{
	if (editing)
	{
		if (inlineAttrTextEditOpen)
			return kMouseEventHandled;
		CPoint where2 (where);
		where2.offset (-getViewSize ().left, -getViewSize ().top);
		getTransform ().inverse ().transform (where2);
		if (buttons & kLButton)
		{
			if (getSelection ()->total () > 0)
			{
				if (mouseEditMode == MouseEditMode::DragEditing)
				{
					doDragEditingMove (where2);
				}
				else if (mouseEditMode == MouseEditMode::SizeEditing)
				{
					doSizeEditingMove (where2);
				}
			}
			CScrollView* scrollView = dynamic_cast<CScrollView*>(getParentView ()->getParentView ());
			if (scrollView)
			{
				scrollView->makeRectVisible (CRect (where, CPoint (1, 1)));
			}
			return kMouseEventHandled;
		}
		else if (buttons.getButtonState () == 0)
		{
			CView* view = nullptr;
			CCursorType ctype = kCursorDefault;
			auto mode = selectionHitTest (where, &view);
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
						if (getSelection ()->contains (getView (0)) == false)
							ctype = kCursorHand;
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
	return CViewContainer::onMouseMoved (where, buttons);
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
	auto bitmap = createBitmapFromSelection (selection, getFrame (), this);
	if (bitmap == nullptr)
		return;

	CRect selectionBounds = getSelection ()->getBounds ();
	CPoint p;
	getParentView ()->localToFrame (p);
	selectionBounds.offsetInverse (p);

	CPoint offset;
	offset.x = (selectionBounds.left - where.x);
	offset.y = (selectionBounds.top - where.y);

	getSelection ()->setDragOffset (CPoint (offset.x, offset.y));

	std::string templateName;
	if (description->getTemplateNameFromView (getEditView (), templateName))
		description->updateViewDescription (templateName.c_str (), getEditView ());

	CMemoryStream stream (1024, 1024, false);
	if (!getSelection ()->store (stream, description))
		return;
	stream.end ();

	auto callback = makeOwned<DragCallbackFunctions> ();
	callback->endedFunc = [this] (IDraggingSession*, CPoint pos, DragOperation) {
		frameToLocal (pos);
		onMouseMoved (pos, 0);
	};

	auto dropSource = CDropSource::create (stream.getBuffer (), static_cast<uint32_t>(stream.tell ()), CDropSource::kText);
	doDrag (DragDescription (dropSource, offset, bitmap), callback);
}

//----------------------------------------------------------------------------------------------------
UISelection* UIEditView::getSelectionOutOfDrag (IDataPackage* drag)
{
	
	IDataPackage::Type type;
	const void* dragData;
	uint32_t size;
	if ((size = drag->getData (0, dragData, type)) > 0 && type == IDataPackage::kText)
	{
		IController* controller = getEditor () ? dynamic_cast<IController*> (getEditor ()) : nullptr;
		if (controller)
			description->setController (controller);
		CMemoryStream stream (static_cast<const int8_t*> (dragData), size, false);
		UISelection* newSelection = new UISelection;
		if (newSelection->restore (stream, description))
		{
			description->setController (nullptr);
			return newSelection;
		}
		description->setController (nullptr);
		newSelection->forget ();
	}
	return nullptr;
}

//----------------------------------------------------------------------------------------------------
SharedPointer<IDropTarget> UIEditView::getDropTarget ()
{
	if (editing)
		return this;
	return CViewContainer::getDropTarget ();
}

//----------------------------------------------------------------------------------------------------
bool UIEditView::onDrop (DragEventData data)
{
	if (lines)
	{
		overlayView->removeView (lines);
		lines = nullptr;
	}
	if (dragSelection)
	{
		if (highlightView)
		{
			highlightView->setHighlightView (nullptr);
		}
		CPoint where2 (data.pos);
		where2.offset (dragSelection->getDragOffset ().x, dragSelection->getDragOffset ().y);
		if (grid)
		{
			where2.offset (grid->getSize ().x / 2., grid->getSize ().y / 2.);
			getTransform ().inverse ().transform (where2);
			grid->process (where2);
			getTransform ().transform (where2);
		}
		CViewContainer* viewContainer = getContainerAt (where2, GetViewOptions ().deep ());
		if (viewContainer)
		{
			where2.offset (-getViewSize ().left, -getViewSize ().top);
			getTransform ().inverse ().transform (where2);
			CPoint containerOffset;
			viewContainer->localToFrame (containerOffset);
			frameToLocal (containerOffset);
			where2.offset (-containerOffset.x, -containerOffset.y);

			IAction* action = new ViewCopyOperation (dragSelection, getSelection (), viewContainer, where2, description);
			getUndoManager()->pushAndPerform (action);
		}
		dragSelection->forget ();
		dragSelection = nullptr;
	}
	return true;
}

//----------------------------------------------------------------------------------------------------
DragOperation UIEditView::onDragEnter (DragEventData data)
{
	dragSelection = getSelectionOutOfDrag (data.drag);
	if (dragSelection)
	{
		if (!lines)
		{
			lines = new UICrossLines (this, UICrossLines::kDragStyle, crosslineBackgroundColor, crosslineForegroundColor);
			overlayView->addView (lines);
		}
		return onDragMove (data);
	}
	return DragOperation::None;
}

//----------------------------------------------------------------------------------------------------
void UIEditView::onDragLeave (DragEventData data)
{
	if (dragSelection)
	{
		dragSelection->forget ();
		dragSelection = nullptr;
	}
	if (highlightView)
	{
		highlightView->setHighlightView (nullptr);
	}
	if (lines)
	{
		overlayView->removeView (lines);
		lines = nullptr;
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
				if (grid)
				{
					where2.offset (grid->getSize ().x / 2., grid->getSize ().y / 2.);
					getTransform ().inverse ().transform (where2);
					grid->process (where2);
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
					highlightView->setHighlightView (getContainerAt (where2, GetViewOptions ().deep ()));
				}
				return DragOperation::Copy;
			}
		}
	}
	return DragOperation::None;
}

//-----------------------------------------------------------------------------
void UIEditView::onDoubleClickEditing (CView* view)
{
	struct AttributeInlineEditorController : ViewListenerAdapter
	{
		using Callback = std::function<void ()>;
		AttributeInlineEditorController (CTextEdit* edit, Callback&& callback)
		: edit (edit), callback (std::move (callback))
		{
			edit->registerViewListener (this);
		}
		void viewWillDelete (CView*) override
		{
			edit->unregisterViewListener (this);
			delete this;
		}
		void viewLostFocus (CView*) override
		{
			callback ();
		}

	private:
		CTextEdit* edit;
		Callback callback;
	};

	auto factory = static_cast<const UIViewFactory*> (description->getViewFactory ());
	vstgui_assert (factory);
	std::string attrValue;
	if (!factory->getAttributeValue (view, UIViewCreator::kAttrTitle, attrValue, description))
		return;

	auto frame = getFrame ();
	frame->setCursor (kCursorDefault);

	auto r = selection->getGlobalViewCoordinates (view);
	r.offsetInverse (getViewSize ().getTopLeft ());
	translateToLocal (r);
	auto textEdit = new CTextEdit (r, nullptr, 0);
	textEdit->setText (attrValue.data ());
	addView (textEdit);
	new AttributeInlineEditorController (textEdit, [this, textEdit, attrValue] () {
		const auto& text = textEdit->getText ();
		if (text != attrValue)
		{
			auto action = new AttributeChangeAction (description, selection,
			                                         UIViewCreator::kAttrTitle, text.getString ());
			getUndoManager ()->pushAndPerform (action);
		}
		textEdit->getParentView ()->asViewContainer ()->removeView (textEdit);
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
bool UIEditView::attached (CView* parent)
{
	if (CViewContainer::attached (parent))
	{
		editing = !editing;
		enableEditing (!editing);
		IController* controller = getViewController (this, true);
		if (controller)
		{
			CBaseObject* obj = dynamic_cast<CBaseObject*>(controller);
			if (obj)
				obj->notify (this, kMsgAttached);
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool UIEditView::removed (CView* parent)
{
	IController* controller = getViewController (this, true);
	if (controller)
	{
		CBaseObject* obj = dynamic_cast<CBaseObject*>(controller);
		if (obj)
			obj->notify (this, kMsgRemoved);
	}
	if (overlayView)
	{
		getFrame()->removeView (overlayView);
		overlayView = nullptr;
	}
	return CViewContainer::removed (parent);
}

//-----------------------------------------------------------------------------
void UIEditView::setupColors (const IUIDescription* description)
{
	description->getColor ("editView.crosslines.background", crosslineBackgroundColor);
	description->getColor ("editView.crosslines.foreground", crosslineForegroundColor);
	description->getColor ("editView.view.highlight", viewHighlightColor);
	description->getColor ("editView.view.selection", viewSelectionColor);
}

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
