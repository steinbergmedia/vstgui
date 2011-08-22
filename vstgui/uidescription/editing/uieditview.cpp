#include "uieditview.h"
#include "uiundomanager.h"
#include "uiactions.h"
#include "uicrosslines.h"
#include "uigrid.h"
#include "uiselection.h"
//#include "../editingcolordefs.h"
#include "../uidescription.h"
#include "../uiviewfactory.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
UIEditView::UIEditView (const CRect& size, UIDescription* uidescription)
: CViewContainer (size)
, undoManger (0)
, selection (0)
, dragSelection (0)
, description (uidescription)
, highlightView (0)
, lines (0)
, moveSizeOperation (0)
, editTimer (0)
, mouseEditMode (kNoEditing)
, editing (true)
, grid (0)
{
	setWantsFocus (true);
}

//----------------------------------------------------------------------------------------------------
UIEditView::~UIEditView ()
{
	if (editTimer)
		editTimer->forget ();
	setUndoManager (0);
	setSelection (0);
}

//----------------------------------------------------------------------------------------------------
void UIEditView::enableEditing (bool state)
{
	if (editing != state)
	{
		editing = state;
		invalid ();
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditView::setUndoManager (UIUndoManager* manager)
{
	undoManger = manager;
}

//----------------------------------------------------------------------------------------------------
UIUndoManager* UIEditView::getUndoManger ()
{
	if (undoManger == 0)
		undoManger = OwningPointer<UIUndoManager> (new UIUndoManager ());
	return undoManger;
}

//----------------------------------------------------------------------------------------------------
void UIEditView::setSelection (UISelection* inSelection)
{
	if (selection)
	{
		selection->removeDependency (this);
	}
	selection = inSelection;
	if (selection)
	{
		selection->addDependency (this);
	}
}

//----------------------------------------------------------------------------------------------------
UISelection* UIEditView::getSelection ()
{
	if (selection == 0)
	{
		selection = OwningPointer<UISelection> (new UISelection ());
		selection->addDependency (this);
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
		getUndoManger ()->clear ();
		getSelection ()->empty ();
		invalid ();
		removeAll ();
		CRect vs (getViewSize ());
		if (view)
		{
			vs.setWidth (view->getWidth ());
			vs.setHeight(view->getHeight ());
			setViewSize (vs);
			setMouseableArea (vs);
			addView (view);
		}
		else
		{
			vs.setWidth (0);
			vs.setHeight (0);
			setViewSize (vs);
			setMouseableArea (vs);
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
			if (lines == 0)
			{
				lines = new UICrossLines (this, UICrossLines::kSelectionStyle, crosslineBackgroundColor, crosslineForegroundColor);
				lines->update (selection);
				getFrame ()->setCursor (kCursorHand);
			}
			editTimer->forget ();
			editTimer = 0;
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
	else if (message == UISelection::kMsgSelectionChanged || message == UISelection::kMsgSelectionWillChange)
	{
		invalidSelection ();
		return kMessageNotified;
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
	CViewContainer::drawRect (pContext, updateRect);

	CRect oldClip = pContext->getClipRect (oldClip);
	CRect newClip (updateRect);
	newClip.offset (-getViewSize ().left, -getViewSize ().top);
	pContext->setClipRect (updateRect);

	CCoord save[4];
	modifyDrawContext (save, pContext);

	const CCoord dashLength[] = {5, 5};
	const CLineStyle lineDash (CLineStyle::kLineCapButt, CLineStyle::kLineJoinMiter, 0, 2, dashLength);
	pContext->setLineStyle (lineDash);
	pContext->setLineWidth (1);
	pContext->setDrawMode (kAliasing);
	pContext->setFrameColor (kBlueCColor);
	pContext->drawRect (CRect (0, 0, getWidth(), getHeight()), kDrawStroked);

	if (editing)
	{
		if (lines)
			lines->draw (pContext);
		pContext->setDrawMode (kAntiAliasing);
		if (highlightView)
		{
			CRect r = UISelection::getGlobalViewCoordinates (highlightView);
			CPoint p;
			frameToLocal (p);
			r.offset (p.x, p.y);
			r.inset (2, 2);
			pContext->setFrameColor (viewHighlightColor);
			pContext->setLineStyle (kLineSolid);
			pContext->setLineWidth (3);
			pContext->drawRect (r);
		}
		if (getSelection ()->total () > 0)
		{
			pContext->setDrawMode (kAliasing);
			pContext->setFrameColor (viewSelectionColor);
			pContext->setLineStyle (kLineSolid);
			pContext->setLineWidth (1);

			FOREACH_IN_SELECTION(getSelection (), view)
				CRect vs = getSelection ()->getGlobalViewCoordinates (view);
				CPoint p;
				frameToLocal (p);
				vs.offset (p.x, p.y);
				pContext->drawRect (vs);
			FOREACH_IN_SELECTION_END

		}
	}

	restoreDrawContext (pContext, save);
	pContext->setClipRect (oldClip);
	pContext->setDrawMode (kAliasing);
}

//----------------------------------------------------------------------------------------------------
CView* UIEditView::getViewAt (const CPoint& p, bool deep) const
{
	CView* view = CViewContainer::getViewAt (p, deep);
	if (editing)
	{
		UIViewFactory* factory = dynamic_cast<UIViewFactory*> (description->getViewFactory ());
		if (factory)
		{
			while (view && factory->getViewName (view) == 0)
			{
				view = view->getParentView ();
			}
		}
	}
	return view;
}

//----------------------------------------------------------------------------------------------------
CViewContainer* UIEditView::getContainerAt (const CPoint& p, bool deep) const
{
	CViewContainer* view = CViewContainer::getContainerAt (p, deep);
	if (editing)
	{
		UIViewFactory* factory = dynamic_cast<UIViewFactory*> (description->getViewFactory ());
		if (factory)
		{
			while (view && factory->getViewName (view) == 0)
			{
				view = dynamic_cast<CViewContainer*> (view->getParentView ());
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

#define kSizingRectSize 5

//----------------------------------------------------------------------------------------------------
void UIEditView::invalidSelection ()
{
	CRect r (getSelection ()->getBounds ());
	CPoint p;
	frameToLocal (p);
	r.offset (p.x, p.y);
	invalidRect (r);
}

//----------------------------------------------------------------------------------------------------
UIEditView::MouseSizeMode UIEditView::selectionHitTest (const CPoint& where, CView** resultView)
{
	CView* mainView = getView (0);
	FOREACH_IN_SELECTION(getSelection (), view)
		CRect r = getSelection ()->getGlobalViewCoordinates (view);
		bool isMainView = (mainView == view) ? true : false;
		CPoint p;
		frameToLocal (p);
		r.offset (p.x, p.y);
		if (r.pointInside (where))
		{
			if (resultView)
				*resultView = view;
			CRect r2 (-kSizingRectSize, -kSizingRectSize, 0, 0);
			CRect r3 (r2);
			r3.offset (r.right, r.bottom);
			if (r3.pointInside (where))
				return kSizeModeBottomRight;
			r3 = r2;
			r3.offset (r.left + kSizingRectSize, r.top + kSizingRectSize);
			if (r3.pointInside (where) && !isMainView)
				return kSizeModeTopLeft;
			r3 = r2;
			r3.offset (r.right, r.top + kSizingRectSize);
			if (r3.pointInside (where) && !isMainView)
				return kSizeModeTopRight;
			r3 = r2;
			r3.offset (r.left + kSizingRectSize, r.bottom);
			if (r3.pointInside (where) && !isMainView)
				return kSizeModeBottomLeft;
			r3 = r;
			r3.bottom = r3.top + kSizingRectSize;
			if (r3.pointInside (where) && !isMainView)
				return kSizeModeTop;
			r3 = r;
			r3.top = r3.bottom - kSizingRectSize;
			if (r3.pointInside (where))
				return kSizeModeBottom;
			r3 = r;
			r3.right = r3.left + kSizingRectSize;
			if (r3.pointInside (where) && !isMainView)
				return kSizeModeLeft;
			r3 = r;
			r3.left = r3.right - kSizingRectSize;
			if (r3.pointInside (where))
				return kSizeModeRight;
			if (resultView)
				*resultView = 0;
			return kSizeModeNone;
		}
	FOREACH_IN_SELECTION_END
	if (resultView)
		*resultView = 0;
	return kSizeModeNone;
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIEditView::onMouseDown (CPoint &where, const CButtonState& buttons)
{
	if (editing)
	{
		if (buttons & kLButton)
		{
			CView* view = getViewAt (where, true);
			if (!view)
			{
				view = getContainerAt (where, true);
				if (view == this)
					view = 0;
			}
			if (view)
			{
				CPoint where2 (where);
				where2.offset (-getViewSize ().left, -getViewSize ().top);
				CView* mouseHitView = 0;
				MouseSizeMode sizeMode = selectionHitTest (where2, &mouseHitView);
				// first check copying
				if (buttons & kAlt && mouseHitView && getSelection ()->contains (mouseHitView) && !getSelection ()->contains(getView (0)))
				{
					mouseEditMode = kDragEditing;
					invalidSelection ();
					startDrag (where);
					mouseEditMode = kNoEditing;
					invalidSelection ();
					return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
				}
				if (sizeMode == kSizeModeNone)
				{
					// first alter selection
					if (getSelection ()->contains (view))
					{
						if (buttons & kControl)
						{
							getSelection ()->remove (view);
							return kMouseEventHandled;
						}
					}
					else
					{
						if (buttons & kControl || buttons & kShift)
						{
							getSelection ()->add (view);
						}
						else
						{
							getSelection ()->setExclusive (view);
						}
					}
					mouseHitView = view;
				}
				if (mouseHitView && getSelection ()->total () > 0)
				{
					if (!(buttons & kLButton) || !editing)
						return kMouseEventHandled;
					if (mouseHitView == 0)
						return kMouseEventHandled;
					if (sizeMode == kSizeModeNone)
					{
						if (getSelection ()->contains (getView (0)))
							return kMouseEventNotHandled;
						if (buttons & kAlt)
						{
							mouseEditMode = kDragEditing;
							invalidSelection ();
							startDrag (where);
							mouseEditMode = kNoEditing;
							invalidSelection ();
							return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
						}
						else
						{
							mouseEditMode = kDragEditing;
							mouseStartPoint = where2;
							if (grid)
								grid->process (mouseStartPoint);
							if (editTimer)
								editTimer->forget ();
							editTimer = new CVSTGUITimer (this, 500);
							editTimer->start ();
						}
						return kMouseEventHandled;
					}
					else
					{
						getSelection ()->setExclusive (mouseHitView);
						mouseEditMode = kSizeEditing;
						mouseStartPoint = where2;
						if (grid)
							grid->process (mouseStartPoint);
						mouseSizeMode = sizeMode;
						if (true)
						{
							int32_t crossLineMode = 0;
							switch (sizeMode)
							{
								case kSizeModeLeft:
								case kSizeModeRight:
								case kSizeModeTop:
								case kSizeModeBottom: crossLineMode = UICrossLines::kSelectionStyle; break;
								default : crossLineMode = UICrossLines::kDragStyle; break;
							}
							lines = new UICrossLines (this, crossLineMode, crosslineBackgroundColor, crosslineForegroundColor);
							if (crossLineMode == UICrossLines::kSelectionStyle)
								lines->update (selection);
							else
								lines->update (CPoint (mouseStartPoint.x, mouseStartPoint.y));
						}
						return kMouseEventHandled;
					}
				}
			}
			else
			{
				getSelection ()->empty ();
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
			editTimer = 0;
		}
		if (mouseEditMode != kNoEditing && !moveSizeOperation && buttons == kLButton && !lines)
		{
			CView* view = getViewAt (where, true);
			if (!view)
			{
				view = getContainerAt (where, true);
				if (view == this)
					view = 0;
			}
			if (view)
			{
				getSelection ()->setExclusive (view);
			}
		}
		if (lines)
		{
			delete lines;
			lines = 0;
		}
		getFrame ()->setCursor (kCursorDefault);
		mouseEditMode = kNoEditing;
		invalidSelection ();
		if (moveSizeOperation)
		{
			getUndoManger ()->pushAndPerform (moveSizeOperation);
			moveSizeOperation = 0;
		}
		return kMouseEventHandled;
	}
	return CViewContainer::onMouseUp (where, buttons);
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIEditView::onMouseMoved (CPoint &where, const CButtonState& buttons)
{
	if (editing)
	{
		CPoint where2 (where);
		where2.offset (-getViewSize ().left, -getViewSize ().top);
		if (buttons & kLButton)
		{
			if (getSelection ()->total () > 0)
			{
				if (mouseEditMode == kDragEditing)
				{
					if (grid)
						grid->process (where2);
					CPoint diff (where2.x - mouseStartPoint.x, where2.y - mouseStartPoint.y);
					if (diff.x || diff.y)
					{
						invalidSelection ();
						if (!moveSizeOperation)
							moveSizeOperation = new ViewSizeChangeOperation (selection, false);
						getSelection ()->moveBy (diff);
						mouseStartPoint = where2;
						invalidSelection ();
						if (editTimer)
						{
							editTimer->forget ();
							editTimer = 0;
							if (true)
							{
								lines = new UICrossLines (this, UICrossLines::kSelectionStyle, crosslineBackgroundColor, crosslineForegroundColor);
								lines->update (selection);
							}
							getFrame ()->setCursor (kCursorHand);
						}
						if (lines)
							lines->update (selection);
					}
				}
				else if (mouseEditMode == kSizeEditing)
				{
					if (!moveSizeOperation)
						moveSizeOperation = new ViewSizeChangeOperation (selection, true);
					if (grid)
					{
						where2.offset (grid->getSize ().x / 2., grid->getSize ().y / 2.);
						grid->process (where2);
					}
					mouseStartPoint = where2;
					CView* view = getSelection ()->first ();
					CRect viewSize (view->getViewSize ());
					localToFrame (where2);
					view->getParentView ()->frameToLocal (where2);
					switch (mouseSizeMode)
					{
						case kSizeModeLeft: viewSize.left = where2.x; break;
						case kSizeModeRight: viewSize.right = where2.x; break;
						case kSizeModeTop: viewSize.top = where2.y; break;
						case kSizeModeBottom: viewSize.bottom = where2.y; break;
						case kSizeModeTopLeft: viewSize.left = where2.x; viewSize.top = where2.y; break;
						case kSizeModeTopRight: viewSize.right = where2.x; viewSize.top = where2.y; break;
						case kSizeModeBottomRight: viewSize.right = where2.x; viewSize.bottom = where2.y; break;
						case kSizeModeBottomLeft: viewSize.left = where2.x; viewSize.bottom = where2.y; break;
						default: break;
					}
					if (viewSize.left > viewSize.right)
						viewSize.right = viewSize.left;
					if (viewSize.top > viewSize.bottom)
						viewSize.bottom = viewSize.top;
					if (viewSize != view->getViewSize ())
					{
						invalidSelection ();
						view->setViewSize (viewSize);
						view->setMouseableArea (viewSize);
						invalidSelection ();
					}
					if (lines)
					{
						if (lines->getStyle () == UICrossLines::kSelectionStyle)
							lines->update (selection);
						else
							lines->update (mouseStartPoint);
					}
					getSelection ()->changed (UISelection::kMsgSelectionViewChanged);
				}
			}
			return kMouseEventHandled;
		}
		else if (buttons.getButtonState() == 0)
		{
			int32_t mode = selectionHitTest (where2, 0);
			switch (mode)
			{
				case kSizeModeRight:
				case kSizeModeLeft: getFrame ()->setCursor (kCursorHSize); break;
				case kSizeModeTop:
				case kSizeModeBottom: getFrame ()->setCursor (kCursorVSize); break;
				case kSizeModeTopLeft:
				case kSizeModeBottomRight: getFrame ()->setCursor (kCursorNWSESize); break;
				case kSizeModeTopRight:
				case kSizeModeBottomLeft: getFrame ()->setCursor (kCursorNESWSize); break;
				default: getFrame ()->setCursor (kCursorDefault); break;
			}
		}
		else
			getFrame ()->setCursor (kCursorDefault);
		return kMouseEventHandled;
	}
	return CViewContainer::onMouseMoved (where, buttons);
}

//----------------------------------------------------------------------------------------------------
CBitmap* UIEditView::createBitmapFromSelection (UISelection* selection)
{
	CRect viewSize = getSelection ()->getBounds ();
	
	COffscreenContext* context = COffscreenContext::create (getFrame (), viewSize.getWidth (), viewSize.getHeight ());
	context->beginDraw ();
	context->setFillColor (CColor (0, 0, 0, 40));
	context->setFrameColor (CColor (255, 255, 255, 40));
	context->drawRect (CRect (0, 0, viewSize.getWidth (), viewSize.getHeight ()), kDrawFilledAndStroked);

	FOREACH_IN_SELECTION(getSelection (), view)
		if (!getSelection ()->containsParent (view))
		{
			CPoint p;
			view->getParentView ()->localToFrame (p);
			context->setOffset (CPoint (-viewSize.left + p.x, -viewSize.top + p.y));
			context->setClipRect (view->getViewSize ());
			view->drawRect (context, view->getViewSize ());
		}
	FOREACH_IN_SELECTION_END

	context->endDraw ();
	CBitmap* bitmap = context->getBitmap ();
	bitmap->remember ();
	context->forget ();
	return bitmap;
}

//----------------------------------------------------------------------------------------------------
void UIEditView::startDrag (CPoint& where)
{
	CBitmap* bitmap = createBitmapFromSelection (selection);
	if (bitmap == 0)
		return;

	CRect selectionBounds = getSelection ()->getBounds ();
	CPoint p;
	frameToLocal (p);
	selectionBounds.offset (p.x, p.y);

	CPoint offset;
	offset.x = (selectionBounds.left - where.x);
	offset.y = (selectionBounds.top - where.y);

	getSelection ()->setDragOffset (CPoint (offset.x, offset.y));

	UIViewFactory* viewFactory = dynamic_cast<UIViewFactory*> (description->getViewFactory ());
	CMemoryStream stream;
	if (!getSelection ()->store (stream, viewFactory, description))
		return;
	
	offset.offset (getViewSize ().left, getViewSize ().top);
	CDropSource dropSource (stream.getBuffer (), (int32_t)stream.tell (), CDropSource::kBinary);
	doDrag (&dropSource, offset, bitmap);
	if (bitmap)
		bitmap->forget ();
}

//----------------------------------------------------------------------------------------------------
UISelection* UIEditView::getSelectionOutOfDrag (CDragContainer* drag)
{
	int32_t size, type;
	const int8_t* dragData = (const int8_t*)drag->first (size, type);

	IController* controller = getEditor () ? dynamic_cast<IController*> (getEditor ()) : 0;
	if (controller)
		description->setController (controller);
	UIViewFactory* viewFactory = dynamic_cast<UIViewFactory*> (description->getViewFactory ());
	CMemoryStream stream (dragData, size);
	UISelection* newSelection = new UISelection;
	if (newSelection->restore (stream, viewFactory, description))
	{
		description->setController (0);
		return newSelection;
	}
	description->setController (0);
	newSelection->forget ();

	return 0;
}

//----------------------------------------------------------------------------------------------------
bool UIEditView::onDrop (CDragContainer* drag, const CPoint& where)
{
	if (editing)
	{
		if (lines)
		{
			delete lines;
			lines = 0;
		}
		if (dragSelection)
		{
			if (highlightView)
			{
				highlightView->invalid ();
				highlightView = 0;
			}
			CPoint where2 (where);
			where2.offset (dragSelection->getDragOffset ().x, dragSelection->getDragOffset ().y);
			where2.offset (-getViewSize ().left, -getViewSize ().top);
			if (grid)
			{
				where2.offset (grid->getSize ().x / 2., grid->getSize ().y / 2.);
				grid->process (where2);
			}
			CViewContainer* viewContainer = getContainerAt (where2, true);
			if (viewContainer)
			{
				CPoint containerOffset;
				viewContainer->localToFrame (containerOffset);
				frameToLocal (containerOffset);
				where2.offset (-containerOffset.x, -containerOffset.y);

				UIViewFactory* viewFactory = dynamic_cast<UIViewFactory*> (description->getViewFactory ());
				IActionOperation* action = new ViewCopyOperation (dragSelection, getSelection (), viewContainer, where2, viewFactory, description);
				getUndoManger()->pushAndPerform (action);
			}
			dragSelection->forget ();
			dragSelection = 0;
		}
		return true;
	}
	return CViewContainer::onDrop (drag, where);
}

//----------------------------------------------------------------------------------------------------
void UIEditView::onDragEnter (CDragContainer* drag, const CPoint& where)
{
	if (editing)
	{
		dragSelection = getSelectionOutOfDrag (drag);
		if (dragSelection)
		{
			CPoint where2 (where);
			where2.offset (dragSelection->getDragOffset ().x, dragSelection->getDragOffset ().y);
			where2.offset (-getViewSize ().left, -getViewSize ().top);
			if (grid)
			{
				where2.offset (grid->getSize ().x / 2., grid->getSize ().y / 2.);
				grid->process (where2);
			}
			lines = new UICrossLines (this, UICrossLines::kDragStyle, crosslineBackgroundColor, crosslineForegroundColor);
			lines->update (where2);
			highlightView = getContainerAt (where2, true);
			if (highlightView)
				highlightView->invalid ();
			getFrame ()->setCursor (kCursorCopy);
		}
		else
		{
			getFrame ()->setCursor (kCursorNotAllowed);
		}
	}
	else
		CViewContainer::onDragEnter (drag, where);
}

//----------------------------------------------------------------------------------------------------
void UIEditView::onDragLeave (CDragContainer* drag, const CPoint& where)
{
	if (dragSelection)
	{
		dragSelection->forget ();
		dragSelection = 0;
	}
	if (editing)
	{
		if (highlightView)
		{
			highlightView->invalid ();
			highlightView = 0;
		}
		if (lines)
		{
			delete lines;
			lines = 0;
		}
		getFrame ()->setCursor (kCursorNotAllowed);
	}
	else
		CViewContainer::onDragLeave (drag, where);
}

//----------------------------------------------------------------------------------------------------
void UIEditView::onDragMove (CDragContainer* drag, const CPoint& where)
{
	if (editing)
	{
		if (lines)
		{
			if (dragSelection)
			{
				CPoint where2 (where);
				where2.offset (dragSelection->getDragOffset ().x, dragSelection->getDragOffset ().y);
				where2.offset (-getViewSize ().left, -getViewSize ().top);
				if (grid)
				{
					where2.offset (grid->getSize ().x / 2., grid->getSize ().y / 2.);
					grid->process (where2);
				}
				lines->update (where2);
				CView* v = getContainerAt (where2, true);
				if (v != highlightView)
				{
					if (highlightView)
						highlightView->invalid ();
					highlightView = v;
					if (highlightView)
						highlightView->invalid ();
				}
			}
		}
	}
	else
		CViewContainer::onDragMove (drag, where);
}

//-----------------------------------------------------------------------------
void UIEditView::looseFocus ()
{
}

//-----------------------------------------------------------------------------
void UIEditView::takeFocus ()
{
}

//-----------------------------------------------------------------------------
void UIEditView::setupColors (IUIDescription* description)
{
	description->getColor ("editView.crosslines.background", crosslineBackgroundColor);
	description->getColor ("editView.crosslines.foreground", crosslineForegroundColor);
	description->getColor ("editView.view.highlight", viewHighlightColor);
	description->getColor ("editView.view.selection", viewSelectionColor);
}

} // namespace
