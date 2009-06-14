/*
 *  ceditframe.cpp
 *
 *  Created by Arne Scheffler on 12/8/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#if VSTGUI_LIVE_EDITING

#include "ceditframe.h"
#include "cviewinspector.h"
#include "viewhierarchybrowser.h"
#include "viewfactory.h"
#include "viewcreator.h"
#include "../vstkeycode.h"
#include "../cfileselector.h"
#include <map>
#include <sstream>

BEGIN_NAMESPACE_VSTGUI

//-----------------------------------------------------------------------------
static bool stringCompare (const std::string* lhs, const std::string* rhs)
{
  return *lhs < *rhs;
}

//-----------------------------------------------------------------------------
class UndoStackTop : public IActionOperation
{
public:
	const char* getName () { return 0; }
	void perform () {}
	void undo () {}
};

//-----------------------------------------------------------------------------
class ViewCopyOperation : public IActionOperation, protected std::list<CView*>
{
public:
	ViewCopyOperation (CSelection* selection, CViewContainer* parent, const CPoint& offset, ViewFactory* viewFactory, IUIDescription* desc)
	: parent (parent)
	, selection (selection)
	{
		parent->remember ();
		selection->remember ();
		CRect selectionBounds = selection->getBounds ();
		FOREACH_IN_SELECTION(selection, view)
			if (!selection->containsParent (view))
			{
				CView* viewCopy = duplicateView (view, viewFactory, desc);
				if (viewCopy)
				{
					CRect viewSize = CSelection::getGlobalViewCoordinates (view);
					CRect newSize (0, 0, viewSize.getWidth (), viewSize.getHeight ());
					newSize.offset (offset.x, offset.y);
					newSize.offset (viewSize.left - selectionBounds.left, viewSize.top - selectionBounds.top);

					viewCopy->setViewSize (newSize);
					viewCopy->setMouseableArea (newSize);
					push_back (viewCopy);
				}
			}
			oldSelectedViews.push_back (view);
		FOREACH_IN_SELECTION_END
	}
	
	~ViewCopyOperation ()
	{
		const_iterator it = begin ();
		while (it != end ())
		{
			(*it)->forget ();
			it++;
		}
		parent->forget ();
		selection->forget ();
	}
	
	const char* getName () { return "Copy"; }

	void perform ()
	{
		selection->empty ();
		const_iterator it = begin ();
		while (it != end ())
		{
			parent->addView (*it);
			(*it)->remember ();
			(*it)->invalid ();
			selection->add (*it);
			it++;
		}
	}
	
	void undo ()
	{
		selection->empty ();
		const_iterator it = begin ();
		while (it != end ())
		{
			(*it)->invalid ();
			parent->removeView (*it, true);
			it++;
		}
		it = oldSelectedViews.begin ();
		while (it != oldSelectedViews.end ())
		{
			selection->add (*it);
			(*it)->invalid ();
			it++;
		}
	}
protected:
	//----------------------------------------------------------------------------------------------------
	static CView* duplicateView (CView* view, ViewFactory* viewFactory, IUIDescription* desc)
	{
		UIAttributes attr;
		if (viewFactory->getAttributesForView (view, desc, attr))
		{
			CView* viewCopy = viewFactory->createView (attr, desc);
			if (viewCopy)
			{
				viewFactory->applyAttributeValues (viewCopy, attr, desc);
				CViewContainer* container = dynamic_cast<CViewContainer*> (view);
				if (container)
				{
					for (long i = 0; i < container->getNbViews (); i++)
					{
						CView* subview = container->getView (i);
						if (!subview)
							continue;
						CView* subviewCopy = duplicateView (subview, viewFactory, desc);
						if (subviewCopy)
						{
							dynamic_cast<CViewContainer*> (viewCopy)->addView (subviewCopy);
						}
					}
				}
			}
			return viewCopy;
		}
		return 0;
	}

	CViewContainer* parent;
	CSelection* selection;
	std::list<CView*> oldSelectedViews;
};

//-----------------------------------------------------------------------------
class ViewSizeChangeOperation : public IActionOperation, protected std::map<CView*, CRect>
{
public:
	ViewSizeChangeOperation (CSelection* selection, bool sizing)
	: first (true)
	, sizing (sizing)
	{
		FOREACH_IN_SELECTION(selection, view)
			insert (std::make_pair (view, view->getViewSize ()));
			view->remember ();
		FOREACH_IN_SELECTION_END
	}

	~ViewSizeChangeOperation ()
	{
		const_iterator it = begin ();
		while (it != end ())
		{
			(*it).first->forget ();
			it++;
		}
	}
	
	const char* getName ()
	{
		if (size () > 1)
			return sizing ? "Resize Views" : "Move Views";
		return sizing ? "Resize View" : "Move View";
	}

	void perform ()
	{
		if (first)
		{
			first = false;
			return;
		}
		undo ();
	}
	
	void undo ()
	{
		iterator it = begin ();
		while (it != end ())
		{
			CRect size ((*it).second);
			(*it).first->invalid ();
			(*it).second = (*it).first->getViewSize ();
			(*it).first->setViewSize (size);
			(*it).first->setMouseableArea (size);
			(*it).first->invalid ();
			it++;
		}
	}
protected:
	bool first;
	bool sizing;
};

struct ViewAndNext
{
	ViewAndNext (CView* view, CView* nextView) : view (view), nextView (nextView) {}
	ViewAndNext (const ViewAndNext& copy) : view (copy.view), nextView (copy.nextView) {}
	CView* view;
	CView* nextView;
};
//----------------------------------------------------------------------------------------------------
class DeleteOperation : public IActionOperation, protected std::multimap<CViewContainer*, ViewAndNext*>
{
public:
	DeleteOperation (CSelection* selection)
	: selection (selection)
	{
		selection->remember ();
		FOREACH_IN_SELECTION(selection, view)
			CViewContainer* container = dynamic_cast<CViewContainer*> (view->getParentView ());
			CView* nextView = 0;
			for (long i = 0; i < container->getNbViews (); i++)
			{
				if (container->getView (i) == view)
				{
					nextView = container->getView (i+1);
					break;
				}
			}
			insert (std::make_pair (container, new ViewAndNext (view, nextView)));
			container->remember ();
			view->remember ();
			if (nextView)
				nextView->remember ();
		FOREACH_IN_SELECTION_END
	}

	~DeleteOperation ()
	{
		const_iterator it = begin ();
		while (it != end ())
		{
			(*it).first->forget ();
			(*it).second->view->forget ();
			if ((*it).second->nextView)
				(*it).second->nextView->forget ();
			delete (*it).second;
			it++;
		}
		selection->forget ();
	}
	
	const char* getName ()
	{
		if (size () > 1)
			return "Delete Views";
		return "Delete View";
	}

	void perform ()
	{
		const_iterator it = begin ();
		while (it != end ())
		{
			(*it).first->removeView ((*it).second->view);
			it++;
		}
		selection->empty ();
	}
	
	void undo ()
	{
		selection->empty ();
		const_iterator it = begin ();
		while (it != end ())
		{
			if ((*it).second->nextView)
				(*it).first->addView ((*it).second->view, (*it).second->nextView);
			else
				(*it).first->addView ((*it).second->view);
			(*it).second->view->remember ();
			selection->add ((*it).second->view);
			it++;
		}
	}
protected:
	CSelection* selection;
};

//-----------------------------------------------------------------------------
class InsertViewOperation : public IActionOperation
{
public:
	InsertViewOperation (CViewContainer* parent, CView* view, CSelection* selection)
	: parent (parent)
	, view (view)
	, selection (selection)
	{
		parent->remember ();
		view->remember ();
		selection->remember ();
	}

	~InsertViewOperation ()
	{
		parent->forget ();
		view->forget ();
		selection->forget ();
	}
	
	const char* getName ()
	{
		return "create new Subview";
	}
	
	void perform ()
	{
		parent->addView (view);
		selection->setExclusive (view);
	}
	
	void undo ()
	{
		selection->remove (view);
		view->remember ();
		parent->removeView (view);
	}
protected:
	CViewContainer* parent;
	CView* view;
	CSelection* selection;
};

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
class CrossLines
{
public:
	enum {
		kSelectionStyle,
		kDragStyle
	};
	
	CrossLines (CFrame* frame, int style)
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
		currentRect.left = point.x-1;
		currentRect.top = point.y-1;
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

		CColor c = MakeCColor (255, 255, 255, 200);
		pContext->setFrameColor (c);
		pContext->setLineWidth (1);
		pContext->setDrawMode (kCopyMode);
		pContext->moveTo (CPoint (size.left, selectionSize.top+1));
		pContext->lineTo (CPoint (size.right, selectionSize.top+1));
		pContext->moveTo (CPoint (selectionSize.left, size.top+1));
		pContext->lineTo (CPoint (selectionSize.left, size.bottom));
		if (style == kSelectionStyle)
		{
			pContext->moveTo (CPoint (size.left, selectionSize.bottom));
			pContext->lineTo (CPoint (size.right, selectionSize.bottom));
			pContext->moveTo (CPoint (selectionSize.right-1, size.top));
			pContext->lineTo (CPoint (selectionSize.right-1, size.bottom));
		}
		c = MakeCColor (0, 0, 0, 255);
		pContext->setFrameColor (c);
		pContext->setLineWidth (1);
		pContext->setLineStyle (kLineOnOffDash);
		pContext->moveTo (CPoint (size.left, selectionSize.top+1));
		pContext->lineTo (CPoint (size.right, selectionSize.top+1));
		pContext->moveTo (CPoint (selectionSize.left, size.top));
		pContext->lineTo (CPoint (selectionSize.left, size.bottom));
		if (style == kSelectionStyle)
		{
			pContext->moveTo (CPoint (size.left, selectionSize.bottom));
			pContext->lineTo (CPoint (size.right, selectionSize.bottom));
			pContext->moveTo (CPoint (selectionSize.right-1, size.top));
			pContext->lineTo (CPoint (selectionSize.right-1, size.bottom));
		}
	}
protected:
	CFrame* frame;
	CRect currentRect;
	int style;
};

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
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
const char* CEditFrame::kMsgPerformOptionsMenuAction = "CEditFrame PerformOptionsMenuAction";
const char* CEditFrame::kMsgShowOptionsMenu = "CEditFrame ShowOptionsMenu";
const char* CEditFrame::kMsgEditEnding = "CEditFrame Edit Ending";

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
CEditFrame::CEditFrame (const CRect& size, void* windowPtr, VSTGUIEditorInterface* editor, EditMode _editMode, CSelection* _selection, UIDescription* description, const char* uiDescViewName)
: CFrame (size, windowPtr, editor)
, lines (0)
, grid (0)
, selection (_selection)
, uiDescription (0)
, hierarchyBrowser (0)
, inspector (0)
, moveSizeOperation (0)
, highlightView (0)
, editMode (kNoEditMode)
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

	if (uiDescViewName)
		templateName = uiDescViewName;

	inspector = new CViewInspector (selection, this);
	setUIDescription (description);
	setEditMode (_editMode);
	undoStackList.push_back (new UndoStackTop);
	undoStack = undoStackList.end ();
	
	if (editMode == kPaletteMode)
		selection->setStyle (CSelection::kSingleSelectionStyle);
}

//----------------------------------------------------------------------------------------------------
CEditFrame::~CEditFrame ()
{
	emptyUndoStack ();
	undoStack--;
	delete (*undoStack);
	setUIDescription (0);
	if (hierarchyBrowser)
		hierarchyBrowser->forget ();
	if (inspector)
		inspector->forget ();
	if (selection)
		selection->forget ();
	if (timer)
		timer->forget ();
	if (grid)
		delete grid;
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::emptyUndoStack ()
{
	std::list<IActionOperation*>::reverse_iterator it = undoStackList.rbegin ();
	while (it != undoStackList.rend ())
	{
		delete (*it);
		it++;
	}
	undoStackList.clear ();
	undoStackList.push_back (new UndoStackTop);
	undoStack = undoStackList.end ();
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
void CEditFrame::setUIDescription (UIDescription* description)
{
	if (selection)
		selection->empty ();
	if (uiDescription)
		uiDescription->forget ();
	uiDescription = description;
	if (uiDescription)
		uiDescription->remember ();
	inspector->setUIDescription (uiDescription);
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::setEditMode (EditMode mode)
{
	editMode = mode;
	if (editMode == kEditMode)
		inspector->show ();
	else
	{
		if (editTimer)
		{
			editTimer->forget ();
			editTimer = 0;
		}
		if (uiDescription && templateName.size () > 0)
		{
			uiDescription->updateViewDescription (templateName.c_str (), getView (0));
		}
		if (hierarchyBrowser)
		{
			hierarchyBrowser->forget ();
			hierarchyBrowser = 0;
		}
		inspector->hide ();
		selection->empty ();
		CBaseObject* editorObj = dynamic_cast<CBaseObject*> (pEditor);
		if (editorObj)
			editorObj->notify (this, kMsgEditEnding);
	}
	invalid ();
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::onViewAdded (CView* pView)
{
	if (pView == getView (0))
	{
		if (uiDescription && uiDescription->getTemplateNameFromView (pView, templateName) && hierarchyBrowser)
			hierarchyBrowser->changeBaseView (dynamic_cast<CViewContainer*> (pView));
	}
	else if (hierarchyBrowser)
		hierarchyBrowser->notifyHierarchyChange (pView, false);

	CFrame::onViewAdded (pView);
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::onViewRemoved (CView* pView)
{
	if (pView == getView (0))
	{
		if (editMode == kEditMode && uiDescription && templateName.size () > 0)
			uiDescription->updateViewDescription (templateName.c_str (), getView (0));
		if (selection)
			selection->empty ();
		emptyUndoStack ();
	}
	else if (hierarchyBrowser)
		hierarchyBrowser->notifyHierarchyChange (pView, true);
	CFrame::onViewRemoved (pView);
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::showOptionsMenu (const CPoint& where)
{
	enum {
		kEnableEditing = 1,
		kHierarchyBrowserTag,
		kGridSize1,
		kGridSize2,
		kGridSize5,
		kGridSize10,
		kGridSize15,
		kCreateNewViewTag,
		kInsertTemplateTag,
		kDeleteSelectionTag,
		kUndoTag,
		kRedoTag,
		kSaveTag,
	};
	
	COptionMenu* menu = new COptionMenu ();
	menu->setStyle (kMultipleCheckStyle|kPopupStyle);
	if (editMode == kEditMode)
	{
		std::stringstream undoName;
		undoName << "Undo";
		if (canUndo ())
		{
			undoName << " ";
			undoName << getUndoName ();
		}
		CMenuItem* item = menu->addEntry (new CMenuItem (undoName.str ().c_str (), kUndoTag));
		item->setEnabled (canUndo ());
		item->setKey ("z", kControl);
		
		std::stringstream redoName;
		redoName << "Redo";
		if (canRedo ())
		{
			redoName << " ";
			redoName << getRedoName ();
		}
		item = menu->addEntry (new CMenuItem (redoName.str ().c_str (), kRedoTag));
		item->setKey ("z", kControl|kShift);
		item->setEnabled (canRedo ());
		
		menu->addSeparator ();
		item = menu->addEntry (new CMenuItem ("Delete", kDeleteSelectionTag));
		if (selection->total () <= 0 || selection->contains (getView (0)))
			item->setEnabled (false);
		if (uiDescription)
		{
			ViewFactory* viewFactory = dynamic_cast<ViewFactory*> (uiDescription->getViewFactory ());
			if (viewFactory)
			{
				menu->addSeparator ();
				std::list<const std::string*> viewNames;
				viewFactory->collectRegisteredViewNames (viewNames);
				COptionMenu* viewMenu = new COptionMenu ();
				std::list<const std::string*>::const_iterator it = viewNames.begin ();
				while (it != viewNames.end ())
				{
					viewMenu->addEntry (new CMenuItem ((*it)->c_str (), kCreateNewViewTag));
					it++;
				}
				menu->addEntry (viewMenu, "Insert Subview");
				std::list<const std::string*> templateNames;
				uiDescription->collectTemplateViewNames (templateNames);
				if (templateNames.size () > 1)
				{
					templateNames.sort (stringCompare);
					COptionMenu* templateNameMenu = new COptionMenu ();
					it = templateNames.begin ();
					while (it != templateNames.end ())
					{
						if (*(*it) != templateName)
							templateNameMenu->addEntry (new CMenuItem ((*it)->c_str (), kInsertTemplateTag));
						it++;
					}
					menu->addEntry (templateNameMenu, "Insert Template");
					templateNameMenu->forget ();
				}
				viewMenu->forget ();
			}
		}

		menu->addSeparator ();
		item = menu->addEntry ("Grid");
		COptionMenu* gridMenu = new COptionMenu ();
		gridMenu->setStyle (kMultipleCheckStyle|kPopupStyle);
		item->setSubmenu (gridMenu);
		gridMenu->forget (); // was remembered by the item
		item = gridMenu->addEntry (new CMenuItem ("off", kGridSize1));
		if (getGrid () == 1)
			item->setChecked (true);
		gridMenu->addSeparator ();
		item = gridMenu->addEntry (new CMenuItem ("2x2", kGridSize2));
		if (getGrid () == 2)
			item->setChecked (true);
		item = gridMenu->addEntry (new CMenuItem ("5x5", kGridSize5));
		if (getGrid () == 5)
			item->setChecked (true);
		item = gridMenu->addEntry (new CMenuItem ("10x10", kGridSize10));
		if (getGrid () == 10)
			item->setChecked (true);
		item = gridMenu->addEntry (new CMenuItem ("15x15", kGridSize15));
		if (getGrid () == 15)
			item->setChecked (true);
		menu->addSeparator ();
		menu->addEntry (new CMenuItem ("Save...", kSaveTag));
		menu->addSeparator ();
		menu->addEntry (new CMenuItem (hierarchyBrowser ? "Hide Hierarchy Browser" : "Show Hierarchy Browser", kHierarchyBrowserTag));
		menu->addSeparator ();
	}
	CMenuItem* item = menu->addEntry (new CMenuItem (editMode == kEditMode ? "Disable Editing" : "Enable Editing", kEnableEditing));
	item->setKey ("e", kControl);
	CBaseObject* editorObj = dynamic_cast<CBaseObject*> (pEditor);
	if (editorObj)
	{
		editorObj->notify (menu, kMsgShowOptionsMenu);
	}
	if (menu->popup (this, where))
	{
		long index = 0;
		COptionMenu* resMenu = menu->getLastItemMenu (index);
		if (resMenu)
		{
			item = resMenu->getEntry (index);
			switch (item->getTag ())
			{
				case kEnableEditing: setEditMode (editMode == kEditMode ? kNoEditMode : kEditMode); break;
				case kGridSize1: setGrid (1); break;
				case kGridSize2: setGrid (2); break;
				case kGridSize5: setGrid (5); break;
				case kGridSize10: setGrid (10); break;
				case kGridSize15: setGrid (15); break;
				case kCreateNewViewTag: createNewSubview (where, item->getTitle ()); break;
				case kInsertTemplateTag: insertTemplate (where, item->getTitle ()); break;
				case kDeleteSelectionTag: deleteSelectedViews (); break;
				case kUndoTag: performUndo (); break;
				case kRedoTag: performRedo (); break;
				case kSaveTag:
				{
					CNewFileSelector* fileSelector = CNewFileSelector::create (this, CNewFileSelector::kSelectSaveFile);
					fileSelector->setTitle ("Save VSTGUI UI Description File");
					fileSelector->setDefaultExtension (CFileExtension ("VSTGUI UI Description", "uidesc"));
					fileSelector->setDefaultSaveName (uiDescription->getXmFileName ());
					if (fileSelector->runModal ())
					{
						const char* filename = fileSelector->getSelectedFile (0);
						uiDescription->updateViewDescription (templateName.c_str (), getView (0));
						uiDescription->save (filename);
					}
					fileSelector->forget ();
					break;
				}
				case kHierarchyBrowserTag:
				{
					if (hierarchyBrowser)
					{
						hierarchyBrowser->forget ();
						hierarchyBrowser = 0;
					}
					else
					{
						hierarchyBrowser = new ViewHierarchyBrowserWindow (dynamic_cast<CViewContainer*> (getView (0)), this, uiDescription);
					}
					break;
				}
				default:
				{
					if (editorObj)
					{
						editorObj->notify (item, kMsgPerformOptionsMenuAction);
					}
					break;
				}
			}
		}
	}
	menu->forget ();
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::insertTemplate (const CPoint& where, const char* templateName)
{
	CViewContainer* parent = dynamic_cast<CViewContainer*> (selection->first ());
	if (parent == 0)
		parent = getContainerAt (where);
	if (parent == 0)
		return;

	CPoint origin (where);
	grid->process (origin);
	parent->frameToLocal (origin);
	
	IController* controller = pEditor ? dynamic_cast<IController*> (pEditor) : 0;
	CView* view = uiDescription->createView (templateName, controller);
	if (view)
	{
		CRect r = view->getViewSize ();
		r.offset (origin.x, origin.y);
		view->setViewSize (r);
		view->setMouseableArea (r);
		performAction (new InsertViewOperation (parent, view, selection));
	}
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::createNewSubview (const CPoint& where, const char* viewName)
{
	CViewContainer* parent = dynamic_cast<CViewContainer*> (selection->first ());
	if (parent == 0)
		parent = getContainerAt (where);
	if (parent == 0)
		return;

	CPoint origin (where);
	grid->process (origin);
	parent->frameToLocal (origin);
	ViewFactory* viewFactory = dynamic_cast<ViewFactory*> (uiDescription->getViewFactory ());
	UIAttributes viewAttr;
	viewAttr.setAttribute ("class", viewName);
	CView* view = viewFactory->createView (viewAttr, uiDescription);
	if (view)
	{
		CRect size (origin, CPoint (20, 20));
		view->setViewSize (size);
		view->setMouseableArea (size);
		performAction (new InsertViewOperation (parent, view, selection));
	}
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
	else if (message == ViewHierarchyBrowserWindow::kMsgWindowClosed)
	{
		if (hierarchyBrowser)
		{
			hierarchyBrowser->forget ();
			hierarchyBrowser = 0;
		}
		return kMessageNotified;
	}
	return CFrame::notify (sender, message);
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::invalidSelection ()
{
	invalidRect (selection->getBounds ());
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
	if (lines)
		lines->draw (pContext);
	if (editMode == kEditMode)
	{
		pContext->setDrawMode (kAntialias);
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
			pContext->setDrawMode (kAntialias);
			pContext->setFrameColor (c);
			pContext->setFillColor (c2);
			pContext->setLineStyle (kLineSolid);
			pContext->setLineWidth (1);

			FOREACH_IN_SELECTION(selection, view)
				CRect vs = selection->getGlobalViewCoordinates (view);
				CRect sizeRect (-kSizingRectSize, -kSizingRectSize, 0, 0);
				sizeRect.offset (vs.right, vs.bottom);
				sizeRect.bound (vs);
				if (mouseEditMode == kNoEditing)
					pContext->drawRect (sizeRect, kDrawFilled/*AndStroked*/);
				pContext->drawRect (vs);
			FOREACH_IN_SELECTION_END

		}
	}
	pContext->setDrawMode (kCopyMode);
	pContext->setClipRect (oldClip);
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult CEditFrame::onMouseDown (CPoint &where, const long& buttons)
{
	if (editMode != kNoEditMode)
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
				// first alter selection
				if (selection->contains (view))
				{
					if (buttons & kControl)
					{
						view->invalid ();
						selection->remove (view);
						return kMouseEventHandled;
					}
				}
				else
				{
					if (buttons & kControl)
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
				if (selection->total () > 0 && !selection->contains (getView (0)))
				{
					if (buttons == kLButton && editMode == kEditMode) // only if there is a simple left button click we allow sizing
					{
						CRect viewSize = selection->getGlobalViewCoordinates (view);
						CRect sizeRect (-kSizingRectSize, -kSizingRectSize, 0, 0);
						sizeRect.offset (viewSize.right, viewSize.bottom);
						if (sizeRect.pointInside (where))
						{
							if (selection->total () > 0)
							{
								CRect r = selection->getBounds ();
								invalidRect (r);
							}
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
						if (buttons & kAlt)
						{
							mouseEditMode = kDragEditing;
							invalidSelection ();
							startDrag (where);
							mouseEditMode = kNoEditing;
							invalidSelection ();
						}
						else
						{
							mouseEditMode = kDragEditing;
							mouseStartPoint = where;
							if (grid)
								grid->process (mouseStartPoint);
							if (editTimer)
								editTimer->forget ();
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
				invalidSelection ();
				if (editMode == kEditMode)
					selection->empty ();
				invalid ();
			}
		}
		else if (buttons & kRButton)
		{
			showOptionsMenu (where);
		}
		return kMouseEventHandled;
	}
	CMouseEventResult result = CFrame::onMouseDown (where, buttons);
	if (result == kMouseEventNotHandled && buttons & kRButton)
	{
		showOptionsMenu (where);
		return kMouseEventHandled;
	}
	return result;
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult CEditFrame::onMouseUp (CPoint &where, const long& buttons)
{
	if (editMode != kNoEditMode)
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
				invalidSelection ();
				selection->setExclusive (view);
			}
		}
		if (lines)
		{
			delete lines;
			lines = 0;
		}
		setCursor (kCursorDefault);
		mouseEditMode = kNoEditing;
		invalidSelection ();
		if (moveSizeOperation)
		{
			performAction (moveSizeOperation);
			moveSizeOperation = 0;
		}
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
						CPoint diff (where.x - mouseStartPoint.x, where.y - mouseStartPoint.y);
						if (diff.x || diff.y)
						{
							invalidSelection ();
							if (!moveSizeOperation)
								moveSizeOperation = new ViewSizeChangeOperation (selection, false);
							selection->moveBy (diff);
							mouseStartPoint = where;
							invalidSelection ();
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
					}
					else if (mouseEditMode == kSizeEditing)
					{
						if (!moveSizeOperation)
							moveSizeOperation = new ViewSizeChangeOperation (selection, true);
						if (grid)
						{
							where.offset (grid->getSize ()/2, grid->getSize ()/2);
							grid->process (where);
						}
						mouseStartPoint = where;
						CView* view = selection->first ();
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
						selection->changed (CSelection::kMsgSelectionViewChanged);
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
bool CEditFrame::onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const long &buttons)
{
	if (editMode == kNoEditMode)
		return CFrame::onWheel (where, axis, distance, buttons);
	return true;
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
	FOREACH_IN_SELECTION(selection, view)
		CRect gvs = CSelection::getGlobalViewCoordinates (view);
		cgRects[i].origin.x = gvs.left + context.offset.x;
		cgRects[i].origin.y = gvs.top + context.offset.y;
		cgRects[i].size.width = gvs.getWidth ();
		cgRects[i].size.height = gvs.getHeight ();
		i++;
	FOREACH_IN_SELECTION_END
	CGContextClipToRects (context.getCGContext (), cgRects, selection->total ());
	delete [] cgRects;
	CGContextSetAlpha (context.getCGContext (), 0.5);
	#endif

	CFrame::drawRect (&context, viewSize);

	drawContext->forget ();
	
	return bitmap;
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::startDrag (CPoint& where)
{
	CBitmap* bitmap = createBitmapFromSelection (selection);
	// this is really bad, should be done with some kind of storing the CSelection object as string and later on recreating it from the string
	char dragString[40] = {0};
	sprintf (dragString, "CSelection: %d", selection);
	PlatformUtilities::startDrag (this, where, dragString, bitmap);
	if (bitmap)
		bitmap->forget ();
}

//----------------------------------------------------------------------------------------------------
static CSelection* getSelectionOutOfDrag (CDragContainer* drag)
{
	CSelection* selection = 0;
	long size, type;
	const char* dragData = (const char*)drag->first (size, type);
	if (type == CDragContainer::kUnicodeText)
	{
		if (strncmp (dragData, "CSelection: ", 12) == 0)
		{
			// as said above, really not a good practice
			long ptr = strtol (dragData+12, 0, 10);
			selection = (CSelection*)ptr;
		}
	}
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

			ViewFactory* viewFactory = dynamic_cast<ViewFactory*> (uiDescription->getViewFactory ());
			
			#if 1
			performAction (new ViewCopyOperation (dragSelection, viewContainer, where2, viewFactory, uiDescription));
			#else
			CView* view = dragSelection->getFirst ();
			while (view)
			{
				if (!dragSelection->containsParent (view))
				{
					CView* viewCopy = duplicateView (view, viewFactory, uiDescription);
					if (viewCopy)
					{
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
				}
				view = dragSelection->getNext (view);
			}
			invalidSelection ();
			selection->empty ();
			view = newSelection.getFirst ();
			while (view)
			{
				selection->add (view);
				view = newSelection.getNext (view);
			}
			#endif
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
static void collectAllSubViews (CView* view, std::list<CView*>& views)
{
	views.push_back (view);
	CViewContainer* container = dynamic_cast<CViewContainer*> (view);
	if (container)
	{
		for (long i = 0; i < container->getNbViews (); i++)
		{
			CView* subview = container->getView (i);
			collectAllSubViews (subview, views);
		}
	}
}

//----------------------------------------------------------------------------------------------------
static void changeAttributeValueForType (ViewFactory* viewFactory, IUIDescription* desc, CView* startView, IViewCreator::AttrType type, const std::string& oldValue, const std::string& newValue)
{
	std::list<CView*> views;
	collectAllSubViews (startView, views);
	std::list<CView*>::iterator it = views.begin ();
	while (it != views.end ())
	{
		CView* view = (*it);
		std::list<std::string> attrNames;
		if (viewFactory->getAttributeNamesForView (view, attrNames))
		{
			std::list<std::string>::iterator namesIt = attrNames.begin ();
			while (namesIt != attrNames.end ())
			{
				if (viewFactory->getAttributeType (view, (*namesIt)) == type)
				{
					std::string typeValue;
					if (viewFactory->getAttributeValue (view, (*namesIt), typeValue, desc))
					{
						if (typeValue == oldValue)
						{
							UIAttributes newAttr;
							newAttr.setAttribute ((*namesIt).c_str (), newValue.c_str ());
							viewFactory->applyAttributeValues (view, newAttr, desc);
							view->invalid ();
						}
					}
				}
				namesIt++;
			}
		}
		it++;
	}
}

//----------------------------------------------------------------------------------------------------
static void collectViewsWithAttributeValue (ViewFactory* viewFactory, IUIDescription* desc, CView* startView, IViewCreator::AttrType type, const std::string& value, std::map<CView*, std::string>& result)
{
	std::list<CView*> views;
	collectAllSubViews (startView, views);
	std::list<CView*>::iterator it = views.begin ();
	while (it != views.end ())
	{
		CView* view = (*it);
		std::list<std::string> attrNames;
		if (viewFactory->getAttributeNamesForView (view, attrNames))
		{
			std::list<std::string>::iterator namesIt = attrNames.begin ();
			while (namesIt != attrNames.end ())
			{
				if (viewFactory->getAttributeType (view, (*namesIt)) == type)
				{
					std::string typeValue;
					if (viewFactory->getAttributeValue (view, (*namesIt), typeValue, desc))
					{
						if (typeValue == value)
						{
							result.insert (std::make_pair (view, (*namesIt)));
						}
					}
				}
				namesIt++;
			}
		}
		it++;
	}
}

//----------------------------------------------------------------------------------------------------
static void performAttributeChange (ViewFactory* viewFactory, IUIDescription* desc, const std::string& newValue, const std::map<CView*, std::string>& m)
{
	std::map<CView*, std::string>::const_iterator it = m.begin ();
	while (it != m.end ())
	{
		CView* view = (*it).first;
		UIAttributes newAttr;
		newAttr.setAttribute ((*it).second.c_str (), newValue.c_str ());
		viewFactory->applyAttributeValues (view, newAttr, desc);
		view->invalid ();
		it++;
	}
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::performColorChange (const char* colorName, const CColor& newColor, bool remove)
{
	if (remove)
	{
		uiDescription->removeColor (colorName);
	}
	else
	{
		ViewFactory* viewFactory = dynamic_cast<ViewFactory*> (uiDescription->getViewFactory ());
		std::map<CView*, std::string> m;
		collectViewsWithAttributeValue (viewFactory, uiDescription, getView (0), IViewCreator::kColorType, colorName, m);
		uiDescription->changeColor (colorName, newColor);
		performAttributeChange (viewFactory, uiDescription, colorName, m);
	}
	selection->changed (CSelection::kMsgSelectionViewChanged);
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::performTagChange (const char* tagName, long tag, bool remove)
{
	ViewFactory* viewFactory = dynamic_cast<ViewFactory*> (uiDescription->getViewFactory ());
	std::map<CView*, std::string> m;
	collectViewsWithAttributeValue (viewFactory, uiDescription, getView (0), IViewCreator::kTagType, tagName, m);

	if (remove)
	{
		performAttributeChange (viewFactory, uiDescription, "", m);
		uiDescription->removeTag (tagName);
	}
	else
	{
		std::stringstream str;
		str << tag;
		uiDescription->changeTag (tagName, tag);
		performAttributeChange (viewFactory, uiDescription, tagName, m);
	}
	selection->changed (CSelection::kMsgSelectionViewChanged);
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::performBitmapChange (const char* bitmapName, const char* bitmapPath, bool remove)
{
	ViewFactory* viewFactory = dynamic_cast<ViewFactory*> (uiDescription->getViewFactory ());
	std::map<CView*, std::string> m;
	collectViewsWithAttributeValue (viewFactory, uiDescription, getView (0), IViewCreator::kBitmapType, bitmapName, m);

	if (remove)
	{
		performAttributeChange (viewFactory, uiDescription, "", m);
		uiDescription->removeBitmap (bitmapName);
	}
	else
	{
		uiDescription->changeBitmap (bitmapName, bitmapPath);
		performAttributeChange (viewFactory, uiDescription, bitmapName, m);
	}
	selection->changed (CSelection::kMsgSelectionViewChanged);
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::performFontChange (const char* fontName, CFontRef newFont, bool remove)
{
	ViewFactory* viewFactory = dynamic_cast<ViewFactory*> (uiDescription->getViewFactory ());
	std::map<CView*, std::string> m;
	collectViewsWithAttributeValue (viewFactory, uiDescription, getView (0), IViewCreator::kFontType, fontName, m);

	if (remove)
	{
		performAttributeChange (viewFactory, uiDescription, "", m);
		uiDescription->removeFont (fontName);
	}
	else
	{
		uiDescription->changeFont (fontName, newFont);
		performAttributeChange (viewFactory, uiDescription, fontName, m);
	}
	selection->changed (CSelection::kMsgSelectionViewChanged);
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::performColorNameChange (const char* oldName, const char* newName)
{
	ViewFactory* viewFactory = dynamic_cast<ViewFactory*> (uiDescription->getViewFactory ());
	std::map<CView*, std::string> m;
	collectViewsWithAttributeValue (viewFactory, uiDescription, getView (0), IViewCreator::kColorType, oldName, m);

	uiDescription->changeColorName (oldName, newName);

	performAttributeChange (viewFactory, uiDescription, newName, m);
	selection->changed (CSelection::kMsgSelectionViewChanged);
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::performTagNameChange (const char* oldName, const char* newName)
{
	uiDescription->changeTagName (oldName, newName);
	selection->changed (CSelection::kMsgSelectionViewChanged);
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::performFontNameChange (const char* oldName, const char* newName)
{
	uiDescription->changeFontName (oldName, newName);
	selection->changed (CSelection::kMsgSelectionViewChanged);
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::performBitmapNameChange (const char* oldName, const char* newName)
{
	uiDescription->changeBitmapName (oldName, newName);
	selection->changed (CSelection::kMsgSelectionViewChanged);
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::makeSelection (CView* view)
{
	invalidSelection ();
	selection->setExclusive (view);
	invalidSelection ();
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::performAction (IActionOperation* action)
{
	if (undoStack != undoStackList.end ())
	{
		undoStack++;
		std::list<IActionOperation*>::iterator oldStack = undoStack;
		while (undoStack != undoStackList.end ())
		{
			delete (*undoStack);
			undoStack++;
		}
		undoStackList.erase (oldStack, undoStackList.end ());
	}
	undoStackList.push_back (action);
	undoStack = undoStackList.end ();
	undoStack--;
	invalidSelection ();
	(*undoStack)->perform ();
	invalidSelection ();
}

//----------------------------------------------------------------------------------------------------
bool CEditFrame::canUndo ()
{
	return (undoStack != undoStackList.end () && undoStack != undoStackList.begin ());
}

//----------------------------------------------------------------------------------------------------
bool CEditFrame::canRedo ()
{
	if (undoStack == undoStackList.end () && undoStack != undoStackList.begin ())
		return false;
	undoStack++;
	bool result = (undoStack != undoStackList.end ());
	undoStack--;
	return result;
}

//----------------------------------------------------------------------------------------------------
const char* CEditFrame::getUndoName ()
{
	if (undoStack != undoStackList.end () && undoStack != undoStackList.begin ())
		return (*undoStack)->getName ();
	return 0;
}

//----------------------------------------------------------------------------------------------------
const char* CEditFrame::getRedoName ()
{
	const char* redoName = 0;
	if (undoStack != undoStackList.end ())
	{
		undoStack++;
		if (undoStack != undoStackList.end ())
			redoName = (*undoStack)->getName ();
		undoStack--;
	}
	return redoName;
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::performUndo ()
{
	if (undoStack != undoStackList.end () && undoStack != undoStackList.begin ())
	{
		invalidSelection ();
		(*undoStack)->undo ();
		undoStack--;
		invalidSelection ();
	}
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::performRedo ()
{
	if (undoStack != undoStackList.end ())
	{
		undoStack++;
		if (undoStack != undoStackList.end ())
		{
			invalidSelection ();
			(*undoStack)->perform ();
			invalidSelection ();
		}
	}
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::deleteSelectedViews ()
{
	performAction (new DeleteOperation (selection));
}

//----------------------------------------------------------------------------------------------------
long CEditFrame::onKeyDown (VstKeyCode& keycode)
{
	if (keycode.character == 'e' && keycode.modifier == MODIFIER_CONTROL)
	{
		setEditMode (editMode == kEditMode ? kNoEditMode : kEditMode);
		return 1;
	}
	if (editMode == kEditMode)
	{
		if (keycode.character == 0 && keycode.virt == VKEY_BACK && keycode.modifier == MODIFIER_CONTROL)
		{
			if (!selection->contains (getView (0)))
			{
				deleteSelectedViews ();
				return 1;
			}
		}
		if (keycode.character == 'z' && keycode.modifier == MODIFIER_CONTROL)
		{
			if (canUndo ())
			{
				performUndo ();
				return 1;
			}
		}
		if (keycode.character == 'z' && keycode.modifier == (MODIFIER_CONTROL|MODIFIER_SHIFT))
		{
			if (canRedo ())
			{
				performRedo ();
				return 1;
			}
		}
		return -1;
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

#endif // VSTGUI_LIVE_EDITING
