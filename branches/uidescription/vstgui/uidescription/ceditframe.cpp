/*
 *  ceditframe.cpp
 *
 *  Created by Arne Scheffler on 12/8/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "ceditframe.h"
#include "../vstkeycode.h"

BEGIN_NAMESPACE_VSTGUI

class CrossLines
{
public:
	enum {
		kSelectionStyle,
		kDragStyle
	};
	
	CrossLines (CEditFrame* frame, int style)
	: frame (frame)
	, style (style)
	{
	}
	
	~CrossLines ()
	{
		invalid ();
	}
	
	void update (CSelection* selection)
	{
		invalid ();
		currentRect = selection->getBounds ();
		invalid ();
	}

	void update (const CPoint& point)
	{
		invalid ();
		currentRect.left = point.x;
		currentRect.top = point.y;
		currentRect.setWidth (1);
		currentRect.setHeight (1);
		invalid ();
	}

	void invalid ()
	{
		if (!currentRect.isEmpty ())
		{
			CRect frameRect = frame->getViewSize (frameRect);
			frame->invalidRect (CRect (currentRect.left-2, frameRect.top, currentRect.left+2, frameRect.bottom));
			frame->invalidRect (CRect (frameRect.left, currentRect.top-2, frameRect.right, currentRect.top+2));
			if (style == kSelectionStyle)
			{
				frame->invalidRect (CRect (currentRect.right-2, frameRect.top, currentRect.right+2, frameRect.bottom));
				frame->invalidRect (CRect (frameRect.left, currentRect.bottom-2, frameRect.right, currentRect.bottom+2));
			}
		}
	}
	
	void draw (CDrawContext* pContext)
	{
		CRect size = frame->getViewSize (size);
		CRect selectionSize (currentRect);

		CColor c = MakeCColor (255, 255, 255, 100);
		pContext->setFrameColor (c);
		pContext->setLineWidth (3);
		pContext->moveTo (CPoint (size.left, selectionSize.top));
		pContext->lineTo (CPoint (size.right, selectionSize.top));
		pContext->moveTo (CPoint (selectionSize.left, size.top));
		pContext->lineTo (CPoint (selectionSize.left, size.bottom));
		if (style == kSelectionStyle)
		{
			pContext->moveTo (CPoint (size.left, selectionSize.bottom));
			pContext->lineTo (CPoint (size.right, selectionSize.bottom));
			pContext->moveTo (CPoint (selectionSize.right, size.top));
			pContext->lineTo (CPoint (selectionSize.right, size.bottom));
		}
		c = MakeCColor (0, 0, 0, 150);
		pContext->setFrameColor (c);
		pContext->setLineWidth (1);
		pContext->setLineStyle (kLineSolid);
		pContext->moveTo (CPoint (size.left, selectionSize.top));
		pContext->lineTo (CPoint (size.right, selectionSize.top));
		pContext->moveTo (CPoint (selectionSize.left, size.top));
		pContext->lineTo (CPoint (selectionSize.left, size.bottom));
		if (style == kSelectionStyle)
		{
			pContext->moveTo (CPoint (size.left, selectionSize.bottom));
			pContext->lineTo (CPoint (size.right, selectionSize.bottom));
			pContext->moveTo (CPoint (selectionSize.right, size.top));
			pContext->lineTo (CPoint (selectionSize.right, size.bottom));
		}
	}
protected:
	CEditFrame* frame;
	CRect currentRect;
	int style;
};

class Grid
{
public:
	Grid (int size) : size (size) {}
	
	void process (CPoint& p)
	{
		int x = p.x / size;
		p.x = x * size;
		int y = p.y / size;
		p.y = y * size;
	}

	void setSize (int s) { size = s; }
	int getSize () const { return size; }

protected:
	int size;
};

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
CEditFrame::CEditFrame (const CRect& size, void* windowPtr, VSTGUIEditorInterface* editor, EditMode editMode, CSelection* _selection)
: CFrame (size, windowPtr, editor)
, lines (0)
, grid (0)
, selection (_selection)
, highlightView (0)
, editMode (editMode)
, mouseEditMode (kNoEditing)
, timer (0)
, editTimer (0)
, showLines (true)
{
	timer = new CVSTGUITimer (this, 100);
	timer->start ();
	
	grid = new Grid (10);

	if (selection)
		selection->remember ();
	else
		selection = new CSelection;

	if (editMode == kPaletteMode)
		selection->setStyle (CSelection::kSingleSelectionStyle);
}

//----------------------------------------------------------------------------------------------------
CEditFrame::~CEditFrame ()
{
	if (selection)
		selection->forget ();
	if (timer)
		timer->forget ();
	if (grid)
		delete grid;
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::setGrid (int size)
{
	if (grid)
		grid->setSize (size);
}

//----------------------------------------------------------------------------------------------------
int CEditFrame::getGrid () const
{
	if (grid)
		return grid->getSize ();
	return 0;
}

//----------------------------------------------------------------------------------------------------
CMessageResult CEditFrame::notify (CBaseObject* sender, const char* message)
{
	if (message == CVSTGUITimer::kMsgTimer)
	{
		if (sender == timer)
			idle ();
		if (sender == editTimer)
		{
			if (lines == 0 && showLines)
			{
				lines = new CrossLines (this, CrossLines::kSelectionStyle);
				lines->update (selection);
				setCursor (kCursorHand);
			}
			editTimer->forget ();
			editTimer = 0;
		}
		return kMessageNotified;
	}
	return CFrame::notify (sender, message);
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::invalidRect (const CRect rect)
{
	CRect r (rect);
	r.inset (-2, -2);
	CFrame::invalidRect (r);
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::draw (CDrawContext *pContext)
{
	drawRect (pContext, size);
}

#define kSizingRectSize 10

//----------------------------------------------------------------------------------------------------
void CEditFrame::drawRect (CDrawContext *pContext, const CRect& updateRect)
{
	CFrame::drawRect (pContext, updateRect);
	CRect oldClip = pContext->getClipRect (oldClip);
	pContext->setClipRect (updateRect);
	pContext->setDrawMode (kAntialias);
	if (lines)
		lines->draw (pContext);
	if (editMode == kEditMode)
	{
		if (highlightView)
		{
			CRect r = CSelection::getGlobalViewCoordinates (highlightView);
			r.inset (2, 2);
			CColor c = MakeCColor (255, 255, 255, 150);
			pContext->setFrameColor (c);
			pContext->setLineStyle (kLineSolid);
			pContext->setLineWidth (3);
			pContext->drawRect (r);
		}
		if (selection->total () > 0)
		{
			CColor c = MakeCColor (255, 0, 0, 255);
			CColor c2 = MakeCColor (255, 0, 0, 150);
			pContext->setFrameColor (c);
			pContext->setFillColor (c2);
			pContext->setLineStyle (kLineSolid);
			pContext->setLineWidth (1);

			CView* view = selection->getFirst ();
			while (view)
			{
				CRect vs = selection->getGlobalViewCoordinates (view);
				CRect sizeRect (-kSizingRectSize, -kSizingRectSize, 0, 0);
				sizeRect.offset (vs.right, vs.bottom);
				sizeRect.bound (vs);
				pContext->drawRect (sizeRect, kDrawFilled/*AndStroked*/);
				pContext->drawRect (vs);
				view = selection->getNext (view);
			}

		}
	}
	pContext->setDrawMode (kCopyMode);
	pContext->setClipRect (oldClip);
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult CEditFrame::onMouseDown (CPoint &where, const long& buttons)
{
	if (editMode != kNoEditMode && buttons & kLButton)
	{
		mouseDownView = this;
		CView* view = getViewAt (where, true);
		if (!view)
		{
			view = getContainerAt (where, true);
			if (view == this)
				view = 0;
		}
		if (view)
		{
			// first alter selection
			if (selection->contains (view))
			{
				if (buttons & kShift)
				{
					view->invalid ();
					selection->remove (view);
					return kMouseEventHandled;
				}
			}
			else
			{
				if (buttons & kShift)
				{
					selection->add (view);
				}
				else
				{
					if (selection->total () > 0)
					{
						CRect r = selection->getBounds ();
						invalidRect (r);
					}
					selection->setExclusive (view);
				}
			}
			if (selection->total () > 0)
			{
				if (buttons == kLButton && editMode == kEditMode) // only if there is a simple left button click we allow sizing
				{
					CRect viewSize = selection->getGlobalViewCoordinates (view);
					CRect sizeRect (-kSizingRectSize, -kSizingRectSize, 0, 0);
					sizeRect.offset (viewSize.right, viewSize.bottom);
					if (sizeRect.pointInside (where))
					{
						selection->setExclusive (view);
						mouseEditMode = kSizeEditing;
						mouseStartPoint = where;
						if (showLines)
						{
							lines = new CrossLines (this, CrossLines::kDragStyle);
							lines->update (CPoint (viewSize.right, viewSize.bottom));
						}
						setCursor (kCursorSizeAll);
						return kMouseEventHandled;
					}
				}
				CRect r = selection->getBounds ();
				invalidRect (r);
				if (editMode == kEditMode)
				{
					if (buttons & kControl)
					{
						mouseEditMode = kDragEditing;
						startDrag (where);
						mouseEditMode = kNoEditing;
					}
					else
					{
						mouseEditMode = kDragEditing;
						mouseStartPoint = where;
						if (grid)
							grid->process (mouseStartPoint);
						editTimer = new CVSTGUITimer (this, 500);
						editTimer->start ();
					}
				}
				else if (editMode == kPaletteMode)
					mouseEditMode = kDragEditing;
			}
		}
		else
		{
			CRect r = selection->getBounds ();
			invalidRect (r);
			if (editMode == kEditMode)
				selection->setExclusive (this);
			invalid ();
		}
		return kMouseEventHandled;
	}
	else
		return CFrame::onMouseDown (where, buttons);
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult CEditFrame::onMouseUp (CPoint &where, const long& buttons)
{
	if (editMode != kNoEditMode)
	{
		mouseDownView = 0;
		if (editTimer)
		{
			editTimer->forget ();
			editTimer = 0;
		}
		if (lines)
		{
			delete lines;
			lines = 0;
		}
		setCursor (kCursorDefault);
		mouseEditMode = kNoEditing;
		return kMouseEventHandled;
	}
	else
		return CFrame::onMouseUp (where, buttons);
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult CEditFrame::onMouseMoved (CPoint &where, const long& buttons)
{
	if (editMode != kNoEditMode)
	{
		if (buttons & kLButton)
		{
			if (selection->total () > 0)
			{
				if (editMode == kEditMode)
				{
					if (mouseEditMode == kDragEditing)
					{
						if (grid)
							grid->process (where);
						CRect r = selection->getBounds ();
						invalidRect (r);
						CPoint diff (where.x - mouseStartPoint.x, where.y - mouseStartPoint.y);
						selection->moveBy (diff);
						mouseStartPoint = where;
						r = selection->getBounds ();
						invalidRect (r);
						if (editTimer)
						{
							editTimer->forget ();
							editTimer = 0;
							if (showLines)
							{
								lines = new CrossLines (this, CrossLines::kSelectionStyle);
								lines->update (selection);
							}
							setCursor (kCursorHand);
						}
						if (lines)
							lines->update (selection);
					}
					else if (mouseEditMode == kSizeEditing)
					{
						if (grid)
						{
							where.offset (grid->getSize ()/2, grid->getSize ()/2);
							grid->process (where);
						}
						mouseStartPoint = where;
						CView* view = selection->getFirst ();
						view->invalid ();
						CRect globalRect = selection->getGlobalViewCoordinates (view);
						globalRect.right = mouseStartPoint.x;
						globalRect.bottom = mouseStartPoint.y;
						if (globalRect.getWidth () < 0)
							globalRect.setWidth (0);
						if (globalRect.getHeight () < 0)
							globalRect.setHeight (0);
						CRect viewSize = view->getViewSize (viewSize);
						viewSize.setWidth (globalRect.getWidth ());
						viewSize.setHeight (globalRect.getHeight ());
						view->setViewSize (viewSize);
						view->setMouseableArea (viewSize);
						view->invalid ();
						if (lines)
							lines->update (mouseStartPoint);
						selection->changed ();
					}
				}
				else if (mouseEditMode == kDragEditing)
				{
					startDrag (where);
				}
			}
		}
		return kMouseEventHandled;
	}
	else
		return CFrame::onMouseMoved (where, buttons);
}

//----------------------------------------------------------------------------------------------------
CBitmap* CEditFrame::createBitmapFromSelection (CSelection* selection)
{
	CRect viewSize = selection->getBounds ();
	CBitmap* bitmap = new CBitmap (*this, viewSize.getWidth (), viewSize.getHeight ());
	CDrawContext* drawContext = createDrawContext ();
	COffscreenContext context (drawContext, bitmap, true);
	context.offset.x = -viewSize.left;
	context.offset.y = -viewSize.top;

	// platform dependent code
	// clip to selected views
	#if MAC
	CGRect* cgRects = new CGRect [selection->total ()];
	int i = 0;
	CView * view = selection->getFirst ();
	while (view)
	{
		CRect gvs = CSelection::getGlobalViewCoordinates (view);
		cgRects[i].origin.x = gvs.left + context.offset.x;
		cgRects[i].origin.y = gvs.top + context.offset.y;
		cgRects[i].size.width = gvs.getWidth ();
		cgRects[i].size.height = gvs.getHeight ();
		i++;
		view = selection->getNext (view);
	}
	CGContextClipToRects (context.getCGContext (), cgRects, selection->total ());
	delete [] cgRects;
	#endif

	CFrame::drawRect (&context, viewSize);

	#if 0
	CView* view = selection->getFirst ();
	while (view)
	{
		CPoint off;
		view->getParentView ()->localToFrame (off);
		view->getViewSize (viewSize);
		off.offset (viewSize.left, viewSize.top);
		context.offset.x = -off.x;
		context.offset.y = -off.y;
		view->draw (&context);
		view = selection->getNext (view);
	}
	#endif
	
	drawContext->forget ();
	
	return bitmap;
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::startDrag (CPoint& where)
{
	// platform dependent code
	#if 0 // MAC
	OSStatus status = noErr;
	PasteboardRef pasteboard;
	if ((status = PasteboardCreate (CFSTR("net.sourceforge.vstguieditor"), &pasteboard)) == noErr)
	{
		status = PasteboardClear (pasteboard);
		CFDataRef data = CFDataCreate (0, (const UInt8*)selection, sizeof (void*));
		status = PasteboardPutItemFlavor (pasteboard, selection, CFSTR("net.sourceforge.vstgui.CSelection"), data, kPasteboardFlavorSenderOnly);
		DragRef drag;
		if ((status = NewDragWithPasteboard (pasteboard, &drag)) == noErr)
		{
			CBitmap* bitmap = createBitmapFromSelection (selection);
			if (bitmap)
			{
				CGImageRef image = bitmap->createCGImage ();
				HIPoint offset = { -bitmap->getWidth () / 2, -bitmap->getHeight () / 2 };
				SetDragImageWithCGImage (drag, image, &offset, kDragDarkerTranslucency);
				CGImageRelease (image);
			}
			
			EventRecord event;
			ConvertEventRefToEventRecord (GetCurrentEvent (), &event);
			
			RgnHandle rgn = NewRgn ();
			status = TrackDrag (drag, &event, rgn);
			DisposeRgn (rgn);

			DisposeDrag (drag);
			
			if (bitmap)
				bitmap->forget ();
		}
		CFRelease (data);
		CFRelease (pasteboard);
	}
	#endif
}

//----------------------------------------------------------------------------------------------------
static CSelection* getSelectionOutOfDrag (CDragContainer* drag)
{
	CSelection* selection = 0;
	#if 0 // MAC
	// platform dependent code
	OSStatus status;
	DragRef dragRef = (DragRef)drag->getPlatformDrag ();
	if (dragRef)
	{
		PasteboardRef pasteboard;
		if ((status = GetDragPasteboard ((DragRef) dragRef, &pasteboard)) == noErr)
		{
			ItemCount itemCount;
			status = PasteboardGetItemCount (pasteboard, &itemCount);
			CFStringRef name;
			if ((status = PasteboardCopyName (pasteboard, &name)) == noErr)
			{
				if (CFStringCompare (name, CFSTR("net.sourceforge.vstguieditor"), 0) == kCFCompareEqualTo)
				{
					PasteboardItemID itemID;
					if ((status = PasteboardGetItemIdentifier (pasteboard, 1, &itemID)) == noErr)
					{
						selection = (CSelection*)itemID;
					}
				}
				CFRelease (name);
			}
		}
	}
	#endif
	return selection;
}

//----------------------------------------------------------------------------------------------------
bool CEditFrame::onDrop (CDragContainer* drag, const CPoint& where)
{
	if (editMode == kEditMode)
	{
		if (lines)
		{
			delete lines;
			lines = 0;
		}
		if (drag->getType (0) == CDragContainer::kFile)
		{
			long size;
			long type;
			const char* filePath = (const char*) drag->first (size, type);
			if (filePath)
			{
				CFileBitmap* bitmap = new CFileBitmap (filePath);
				if (bitmap->isLoaded ())
				{
					CView* view = getViewAt (where);
					if (view)
					{
						view->setBackground (bitmap);
					}
					else
					{
						setBackground (bitmap);
					}
					selection->changed ();
				}
				bitmap->forget ();
			}
		}
		else
		{
			CSelection* dragSelection = getSelectionOutOfDrag (drag);
			if (dragSelection)
			{
				if (highlightView)
				{
					highlightView->invalid ();
					highlightView = 0;
				}
				CSelection newSelection;
				CRect selectionBounds = dragSelection->getBounds ();

				CPoint where2 (where);
				where2.offset (-selectionBounds.getWidth () / 2, -selectionBounds.getHeight () / 2);
				if (grid)
				{
					where2.offset (grid->getSize ()/2, grid->getSize ()/2);
					grid->process (where2);
				}
				CViewContainer* viewContainer = getContainerAt (where2, true);
				CRect containerSize = viewContainer->getViewSize (containerSize);
				CPoint containerOffset;
				viewContainer->localToFrame (containerOffset);
				where2.offset (-containerOffset.x, -containerOffset.y);
				
				CView* view = dragSelection->getFirst ();
				while (view)
				{
					if (!dragSelection->containsParent (view))
					{
						CView* viewCopy = view->newCopy ();
						CRect viewSize = CSelection::getGlobalViewCoordinates (view);
						CRect newSize (0, 0, viewSize.getWidth (), viewSize.getHeight ());
						newSize.offset (where2.x, where2.y);
						newSize.offset (viewSize.left - selectionBounds.left, viewSize.top - selectionBounds.top);

						viewCopy->setViewSize (newSize);
						viewCopy->setMouseableArea (newSize);
						viewContainer->addView (viewCopy);
						viewCopy->invalid ();
						newSelection.add (viewCopy);
					}
					view = dragSelection->getNext (view);
				}
				selectionBounds = selection->getBounds ();
				invalidRect (selectionBounds);
				selection->empty ();
				view = newSelection.getFirst ();
				while (view)
				{
					selection->add (view);
					view = newSelection.getNext (view);
				}
			}
		}
		return true;
	}
	else
		return CFrame::onDrop (drag, where);
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::onDragEnter (CDragContainer* drag, const CPoint& where)
{
	if (editMode == kEditMode)
	{
		CSelection* selection = getSelectionOutOfDrag (drag);
		if (selection)
		{
			CRect vr = selection->getBounds ();
			CPoint where2 (where);
			where2.offset (-vr.getWidth () / 2, -vr.getHeight () / 2);
			if (grid)
			{
				where2.offset (grid->getSize ()/2, grid->getSize ()/2);
				grid->process (where2);
			}
			if (showLines)
			{
				lines = new CrossLines (this, CrossLines::kDragStyle);
				lines->update (where2);
			}
			highlightView = getContainerAt (where2, true);
			if (highlightView)
				highlightView->invalid ();
			setCursor (kCursorCopy);
		}
	}
	else
		CFrame::onDragEnter (drag, where);
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::onDragLeave (CDragContainer* drag, const CPoint& where)
{
	if (editMode == kEditMode)
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
		setCursor (kCursorDefault);
	}
	else
		CFrame::onDragLeave (drag, where);
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::onDragMove (CDragContainer* drag, const CPoint& where)
{
	if (editMode == kEditMode)
	{
		if (lines)
		{
			CSelection* selection = getSelectionOutOfDrag (drag);
			if (selection)
			{
				CRect vr = selection->getBounds ();
				CPoint where2 (where);
				where2.offset (-vr.getWidth () / 2, -vr.getHeight () / 2);
				if (grid)
				{
					where2.offset (grid->getSize ()/2, grid->getSize ()/2);
					grid->process (where2);
				}
				lines->update (where2);
				CView* v = getContainerAt (where2, true);
				if (v != highlightView)
				{
					highlightView->invalid ();
					highlightView = v;
					highlightView->invalid ();
				}
			}
		}
	}
	else
		CFrame::onDragMove (drag, where);
}

//----------------------------------------------------------------------------------------------------
long CEditFrame::onKeyDown (VstKeyCode& keycode)
{
	if (editMode == kEditMode)
	{
		if (keycode.character == 0)
		{
			if (keycode.virt == VKEY_BACK && keycode.modifier == MODIFIER_CONTROL)
			{
				CView* view = selection->getFirst ();
				while (view)
				{
					view->invalid ();
					selection->remove (view);
					removeView (view);
					view = selection->getFirst ();
				}
				return 1;
			}
		}
		return 0;
	}
	else
		return CFrame::onKeyDown (keycode);
}

//----------------------------------------------------------------------------------------------------
long CEditFrame::onKeyUp (VstKeyCode& keyCode)
{
	if (editMode == kEditMode)
	{
		return 0;
	}
	else
		return CFrame::onKeyUp (keyCode);
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
CFileBitmap::CFileBitmap (const char* path)
: path (0)
{
	if (path)
		load (path);
}

//----------------------------------------------------------------------------------------------------
CFileBitmap::~CFileBitmap ()
{
	setPath (0);
}

//----------------------------------------------------------------------------------------------------
void CFileBitmap::setPath (const char* _path)
{
	if (path)
	{
		free (path);
		path = 0;
	}
	if (_path)
	{
		path = (char*)malloc (strlen (_path) + 1);
		strcpy (path, _path);
		resourceDesc.type = CResourceDescription::kStringType;
		resourceDesc.u.name = path;
	}
}

//----------------------------------------------------------------------------------------------------
bool CFileBitmap::load (const char* _path)
{
	bool result = false;
	// platform dependent code
	#if MAC
	CFURLRef url = CFURLCreateFromFileSystemRepresentation (0, (const UInt8*)_path, strlen (_path), false);
	if (url)
	{
		result = loadFromPath (url);
		CFRelease (url);
	}
	#endif
	if (result)
		setPath (_path);
	return result;
}

END_NAMESPACE_VSTGUI

