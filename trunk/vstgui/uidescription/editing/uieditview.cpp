//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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

#include "uieditview.h"

#if VSTGUI_LIVE_EDITING

#include "uiundomanager.h"
#include "uiactions.h"
#include "uicrosslines.h"
#include "uigrid.h"
#include "uiselection.h"
#include "../uidescription.h"
#include "../uiviewfactory.h"
#include "../../lib/cvstguitimer.h"
#include "../../lib/cframe.h"
#include "../../lib/cscrollview.h"
#include "../../lib/cdropsource.h"
#include "../../lib/coffscreencontext.h"
#include "../../lib/idatapackage.h"

namespace VSTGUI {

IdStringPtr UIEditView::kMsgAttached = "UIEditView::kMsgAttached";
IdStringPtr UIEditView::kMsgRemoved = "UIEditView::kMsgRemoved";

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
UIUndoManager* UIEditView::getUndoManager ()
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
CView* UIEditView::getViewAt (const CPoint& p, bool deep, bool mustbeMouseEnabled) const
{
	CView* view = CViewContainer::getViewAt (p, deep, mustbeMouseEnabled);
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

#define NEW_MOUSE_HANDLING 1

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
		#if !NEW_MOUSE_HANDLING
			if (resultView)
				*resultView = 0;
		#endif
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
		#if NEW_MOUSE_HANDLING
			CView* selectionHitView = 0;
			CPoint where2 (where);
			where2.offset (-getViewSize ().left, -getViewSize ().top);
			MouseSizeMode sizeMode = selectionHitTest (where2, &selectionHitView);
			CView* mouseHitView = getViewAt (where, true);
			if (mouseHitView == 0)
				mouseHitView = getContainerAt (where, true);
			if (selectionHitView || mouseHitView)
			{
				if (getSelection ()->contains (mouseHitView))
				{
					if (buttons & kControl)
					{
						getSelection ()->remove (mouseHitView);
						onMouseMoved (where, CButtonState (buttons.getModifierState ()));
						return kMouseEventHandled;
					}
					selectionHitView = mouseHitView;
				}
				else if (mouseHitView && sizeMode == kSizeModeNone)
				{
					if (buttons & kControl || buttons & kShift)
					{
						getSelection ()->add (mouseHitView);
						selectionHitView = mouseHitView;
						onMouseMoved (where, CButtonState (buttons.getModifierState ()));
					}
					else if (selectionHitView == 0 || selectionHitView == getView (0))
					{
						getSelection ()->setExclusive (mouseHitView);
						selectionHitView = mouseHitView;
						onMouseMoved (where, CButtonState (buttons.getModifierState ()));
					}
				}
				if (selectionHitView)
				{
					if (buttons & kAlt && !getSelection ()->contains (getView (0)))
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
						if (getSelection ()->contains (getView (0)))
						{
							return kMouseEventHandled;
						}
						mouseEditMode = kDragEditing;
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
						// TODO: multiple view resizing (width or height only)
						getSelection ()->setExclusive (selectionHitView);
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
			
		#else
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
		#endif
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
		mouseEditMode = kNoEditing;
		invalidSelection ();
		if (moveSizeOperation)
		{
			getUndoManager ()->pushAndPerform (moveSizeOperation);
			moveSizeOperation = 0;
		}
		onMouseMoved (where, CButtonState (buttons.getModifierState ()));
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
						case kSizeModeTop: if (where2.y <= viewSize.bottom) viewSize.top = where2.y; break;
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
			CScrollView* scrollView = dynamic_cast<CScrollView*>(getParentView ()->getParentView ());
			if (scrollView)
			{
				scrollView->makeRectVisible (CRect (where, CPoint (1, 1)));
			}
			return kMouseEventHandled;
		}
		else if (buttons.getButtonState() == 0)
		{
			CView* view = 0;
			CCursorType ctype = kCursorDefault;
			int32_t mode = selectionHitTest (where2, &view);
			if (view)
			{
				switch (mode)
				{
					case kSizeModeRight:
					case kSizeModeLeft: ctype = kCursorHSize; break;
					case kSizeModeTop:
					case kSizeModeBottom: ctype = kCursorVSize; break;
					case kSizeModeTopLeft:
					case kSizeModeBottomRight: ctype = kCursorNWSESize; break;
					case kSizeModeTopRight:
					case kSizeModeBottomLeft: ctype = kCursorNESWSize; break;
					case kSizeModeNone:
					{
						if (getSelection ()->contains (getView (0)) == false)
							ctype = kCursorHand;
						break;	
					}
					default: getFrame ()->setCursor (kCursorDefault); break;
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
	CMemoryStream stream (1024, 1024, false);
	if (!getSelection ()->store (stream, viewFactory, description))
		return;
	stream.end ();

	offset.offset (getViewSize ().left, getViewSize ().top);
	CDropSource dropSource (stream.getBuffer (), (int32_t)stream.tell (), CDropSource::kText);
	doDrag (&dropSource, offset, bitmap);
	if (bitmap)
		bitmap->forget ();
}

//----------------------------------------------------------------------------------------------------
UISelection* UIEditView::getSelectionOutOfDrag (IDataPackage* drag)
{
	
	IDataPackage::Type type;
	const void* dragData;
	int32_t size;
	if ((size = drag->getData (0, dragData, type)) > 0 && type == IDataPackage::kText)
	{
		IController* controller = getEditor () ? dynamic_cast<IController*> (getEditor ()) : 0;
		if (controller)
			description->setController (controller);
		UIViewFactory* viewFactory = dynamic_cast<UIViewFactory*> (description->getViewFactory ());
		CMemoryStream stream (static_cast<const int8_t*> (dragData), size, false);
		UISelection* newSelection = new UISelection;
		if (newSelection->restore (stream, viewFactory, description))
		{
			description->setController (0);
			return newSelection;
		}
		description->setController (0);
		newSelection->forget ();
	}
	return 0;
}

//----------------------------------------------------------------------------------------------------
bool UIEditView::onDrop (IDataPackage* drag, const CPoint& where)
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
				IAction* action = new ViewCopyOperation (dragSelection, getSelection (), viewContainer, where2, viewFactory, description);
				getUndoManager()->pushAndPerform (action);
			}
			dragSelection->forget ();
			dragSelection = 0;
		}
		return true;
	}
	return CViewContainer::onDrop (drag, where);
}

//----------------------------------------------------------------------------------------------------
void UIEditView::onDragEnter (IDataPackage* drag, const CPoint& where)
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
void UIEditView::onDragLeave (IDataPackage* drag, const CPoint& where)
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
void UIEditView::onDragMove (IDataPackage* drag, const CPoint& where)
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
	return CViewContainer::removed (parent);
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

#endif // VSTGUI_LIVE_EDITING
