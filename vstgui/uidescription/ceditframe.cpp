//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#if VSTGUI_LIVE_EDITING

#include "ceditframe.h"
#include "cviewinspector.h"
#include "viewhierarchybrowser.h"
#include "viewfactory.h"
#include "viewcreator.h"
#include "editingcolordefs.h"
#include "cstream.h"
#include "../lib/coffscreencontext.h"
#include "../lib/vstkeycode.h"
#include "../lib/cfileselector.h"
#include "../lib/cvstguitimer.h"
#include "../lib/cdropsource.h"
#include <map>
#include <sstream>

#if MAC
#include "../lib/platform/mac/cgdrawcontext.h"
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
class UndoStackTop : public IActionOperation
{
public:
	const char* getName () { return 0; }
	void perform () {}
	void undo () {}
};

//-----------------------------------------------------------------------------
class SizeToFitOperation : public IActionOperation, protected std::list<CView*>
{
public:
	SizeToFitOperation (CSelection* selection)
	: selection (selection)
	{
		selection->remember ();
		FOREACH_IN_SELECTION(selection, view)
			push_back (view);
			view->remember ();
			sizes.push_back (view->getViewSize ());
		FOREACH_IN_SELECTION_END
	}
	
	~SizeToFitOperation ()
	{
		iterator it = begin ();
		while (it != end ())
		{
			(*it)->forget ();
			it++;
		}
		selection->forget ();
	}

	const char* getName () { return "size to fit"; }
	
	void perform ()
	{
		selection->empty ();
		const_iterator it = begin ();
		while (it != end ())
		{
			(*it)->invalid ();
			(*it)->sizeToFit ();
			(*it)->invalid ();
			selection->add (*it);
			it++;
		}
	}
	
	void undo ()
	{
		selection->empty ();
		const_iterator it = begin ();
		std::list<CRect>::const_iterator it2 = sizes.begin ();
		while (it != end ())
		{
			(*it)->invalid ();
			CRect r (*it2);
			(*it)->setViewSize (r);
			(*it)->setMouseableArea (r);
			(*it)->invalid ();
			selection->add (*it);
			it++;
			it2++;
		}
	}

protected:
	CSelection* selection;
	std::list<CRect> sizes;
};

//-----------------------------------------------------------------------------
class EmbedViewOperation : public IActionOperation, protected std::list<CView*>
{
public:
	EmbedViewOperation (CSelection* selection, CViewContainer* newContainer)
	: newContainer (newContainer)
	{
		parent = dynamic_cast<CViewContainer*> (selection->first ()->getParentView ());
		FOREACH_IN_SELECTION(selection, view)
			if (view->getParentView () == parent)
			{
				push_back (view);
				view->remember ();
			}
		FOREACH_IN_SELECTION_END

		CRect r = selection->first ()->getViewSize ();
		const_iterator it = begin ();
		while (it != end ())
		{
			CView* view = (*it);
			CRect viewSize = view->getViewSize ();
			if (viewSize.left < r.left)
				r.left = viewSize.left;
			if (viewSize.right > r.right)
				r.right = viewSize.right;
			if (viewSize.top < r.top)
				r.top = viewSize.top;
			if (viewSize.bottom > r.bottom)
				r.bottom = viewSize.bottom;
			it++;
		}
		r.inset (-10, -10);
		newContainer->setViewSize (r);
		newContainer->setMouseableArea (r);
	}
	~EmbedViewOperation ()
	{
		const_iterator it = begin ();
		while (it != end ())
		{
			(*it)->forget ();
			it++;
		}
		newContainer->forget ();
	}
	
	const char* getName () { return 0; }
	void perform ()
	{
		CRect parentRect = newContainer->getViewSize ();
		const_iterator it = begin ();
		while (it != end ())
		{
			CView* view = (*it);
			parent->removeView (view, false);
			CRect r = view->getViewSize ();
			r.offset (-parentRect.left, -parentRect.top);
			view->setViewSize (r);
			view->setMouseableArea (r);
			newContainer->addView (view);
			it++;
		}
		parent->addView (newContainer);
		newContainer->remember ();
	}
	void undo ()
	{
		CRect parentRect = newContainer->getViewSize ();
		const_iterator it = begin ();
		while (it != end ())
		{
			CView* view = (*it);
			newContainer->removeView (view, false);
			CRect r = view->getViewSize ();
			r.offset (parentRect.left, parentRect.top);
			view->setViewSize (r);
			view->setMouseableArea (r);
			parent->addView (view);
			it++;
		}
		parent->removeView (newContainer);
	}

protected:
	CViewContainer* newContainer;
	CViewContainer* parent;
};

//-----------------------------------------------------------------------------
class ViewCopyOperation : public IActionOperation, protected std::list<CView*>
{
public:
	ViewCopyOperation (CSelection* copySelection, CSelection* workingSelection, CViewContainer* parent, const CPoint& offset, ViewFactory* viewFactory, IUIDescription* desc)
	: parent (parent)
	, copySelection (copySelection)
	, workingSelection (workingSelection)
	{
		parent->remember ();
		copySelection->remember ();
		workingSelection->remember ();
		CRect selectionBounds = copySelection->getBounds ();
		FOREACH_IN_SELECTION(copySelection, view)
			if (!copySelection->containsParent (view))
			{
				CRect viewSize = CSelection::getGlobalViewCoordinates (view);
				CRect newSize (0, 0, viewSize.getWidth (), viewSize.getHeight ());
				newSize.offset (offset.x, offset.y);
				newSize.offset (viewSize.left - selectionBounds.left, viewSize.top - selectionBounds.top);

				view->setViewSize (newSize);
				view->setMouseableArea (newSize);
				push_back (view);
				view->remember ();
			}
		FOREACH_IN_SELECTION_END

		FOREACH_IN_SELECTION(workingSelection, view)
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
		copySelection->forget ();
		workingSelection->forget ();
	}
	
	const char* getName () 
	{
		if (size () > 0)
			return "copy views";
		return "copy view";
	}

	void perform ()
	{
		workingSelection->empty ();
		const_iterator it = begin ();
		while (it != end ())
		{
			parent->addView (*it);
			(*it)->remember ();
			(*it)->invalid ();
			workingSelection->add (*it);
			it++;
		}
	}
	
	void undo ()
	{
		workingSelection->empty ();
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
			workingSelection->add (*it);
			(*it)->invalid ();
			it++;
		}
	}
protected:
	CViewContainer* parent;
	CSelection* copySelection;
	CSelection* workingSelection;
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
			return sizing ? "resize views" : "move views";
		return sizing ? "resize view" : "move view";
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
			return "delete views";
		return "delete view";
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
		return "create new subview";
	}
	
	void perform ()
	{
		if (parent->addView (view))
			selection->setExclusive (view);
	}
	
	void undo ()
	{
		selection->remove (view);
		view->remember ();
		if (!parent->removeView (view))
			view->forget ();
	}
protected:
	CViewContainer* parent;
	CView* view;
	CSelection* selection;
};

//-----------------------------------------------------------------------------
class TransformViewTypeOperation : public IActionOperation
{
public:
	TransformViewTypeOperation (CSelection* selection, const char* viewClassName, IUIDescription* desc, ViewFactory* factory)
	: view (selection->first ())
	, newView (0)
	, beforeView (0)
	, parent (dynamic_cast<CViewContainer*> (view->getParentView ()))
	, selection (selection)
	{
		UIAttributes attr;
		if (factory->getAttributesForView (view, desc, attr))
		{
			attr.setAttribute ("class", viewClassName);
			newView = factory->createView (attr, desc);
			for (long i = 0; i < parent->getNbViews (); i++)
			{
				if (parent->getView (i) == view)
					beforeView = parent->getView (i+1);
			}
		}
		view->remember ();
	}
	
	~TransformViewTypeOperation ()
	{
		view->forget ();
		if (newView)
			newView->forget ();
	}
	
	const char* getName () { return "transform view type"; }

	void exchangeSubViews (CViewContainer* src, CViewContainer* dst)
	{
		if (src && dst)
		{
			for (long i = src->getNbViews ()-1; i >= 0; i--)
			{
				CView* view = src->getView (i);
				src->removeView (view, false);
				dst->addView (view, dst->getView (0));
			}
		}
	}

	void perform ()
	{
		if (newView)
		{
			newView->remember ();
			parent->removeView (view);
			if (beforeView)
				parent->addView (newView, beforeView);
			else
				parent->addView (newView);
			exchangeSubViews (dynamic_cast<CViewContainer*> (view), dynamic_cast<CViewContainer*> (newView));
			selection->setExclusive (newView);
		}
	}
	
	void undo ()
	{
		if (newView)
		{
			view->remember ();
			parent->removeView (newView);
			if (beforeView)
				parent->addView (view, beforeView);
			else
				parent->addView (view);
			exchangeSubViews (dynamic_cast<CViewContainer*> (newView), dynamic_cast<CViewContainer*> (view));
			selection->setExclusive (view);
		}
	}
protected:
	CView* view;
	CView* newView;
	CView* beforeView;
	CViewContainer* parent;
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
	
	int getStyle () const { return style; }

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

		pContext->setFrameColor (uidCrossLinesBackground);
		pContext->setLineWidth (3);
		pContext->setLineStyle (kLineSolid);
		pContext->setDrawMode (kAliasing);
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
		pContext->setFrameColor (uidCrossLinesForeground);
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
		int x = (int) (p.x / size);
		p.x = x * size;
		int y = (int) (p.y / size);
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
, dragSelection (0)
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

	inspector = new CViewInspector (selection, this, windowPtr);
	setUIDescription (description);
	setEditMode (_editMode);
	undoStackList.push_back (new UndoStackTop);
	undoStack = undoStackList.end ();
	
	restoreAttributes ();
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
	{
		setFocusView (0);
		updateResourceBitmaps ();
		inspector->show ();
		inspector->getFrame ()->setKeyboardHook (this);
	}
	else if (getView (0))
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
		kEmbedViewTag,
		kTransformViewTag,
		kInsertTemplateTag,
		kDeleteSelectionTag,
		kUndoTag,
		kRedoTag,
		kSaveTag,
		kSizeToFitTag,
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
		int selectionCount = selection->total ();
		item = menu->addEntry (new CMenuItem ("Size To Fit", kSizeToFitTag));
		if (selectionCount <= 0 || selection->contains (getView (0)))
			item->setEnabled (false);
		item = menu->addEntry (new CMenuItem ("Delete", kDeleteSelectionTag));
		if (selectionCount <= 0 || selection->contains (getView (0)))
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
				COptionMenu* transformViewMenu = new COptionMenu ();
				std::list<const std::string*>::const_iterator it = viewNames.begin ();
				while (it != viewNames.end ())
				{
					viewMenu->addEntry (new CMenuItem ((*it)->c_str (), kCreateNewViewTag));
					transformViewMenu->addEntry (new CMenuItem ((*it)->c_str (), kTransformViewTag));
					it++;
				}
				menu->addEntry (viewMenu, "Insert Subview");
				if (selectionCount > 0 && !selection->contains (getView (0)))
				{
					COptionMenu* embedViewMenu = new COptionMenu ();
					viewNames.clear ();
					viewFactory->collectRegisteredViewNames (viewNames, "CViewContainer");
					it = viewNames.begin ();
					while (it != viewNames.end ())
					{
						embedViewMenu->addEntry (new CMenuItem ((*it)->c_str (), kEmbedViewTag));
						it++;
					}
					menu->addEntry (embedViewMenu, "Embed Into ...");
					embedViewMenu->forget ();
				}

				std::list<const std::string*> templateNames;
				uiDescription->collectTemplateViewNames (templateNames);
				if (templateNames.size () > 1)
				{
					templateNames.sort (std__stringCompare);
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
				if (selectionCount == 1 && selection->first () != getView (0))
					menu->addEntry (transformViewMenu, "Transform View Type");
				viewMenu->forget ();
				transformViewMenu->forget ();
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
		item = menu->addEntry (new CMenuItem ("Save...", kSaveTag));
		item->setKey ("s", kControl);
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
				case kTransformViewTag: performAction (new TransformViewTypeOperation (selection, item->getTitle (), uiDescription, dynamic_cast<ViewFactory*> (uiDescription->getViewFactory ()))); break;
				case kInsertTemplateTag: insertTemplate (where, item->getTitle ()); break;
				case kEmbedViewTag: embedSelectedViewsInto (item->getTitle ()); break;
				case kDeleteSelectionTag: deleteSelectedViews (); break;
				case kUndoTag: performUndo (); break;
				case kRedoTag: performRedo (); break;
				case kSaveTag:
				{
					CNewFileSelector* fileSelector = CNewFileSelector::create (this, CNewFileSelector::kSelectSaveFile);
					fileSelector->setTitle ("Save VSTGUI UI Description File");
					fileSelector->setDefaultExtension (CFileExtension ("VSTGUI UI Description", "uidesc"));
					fileSelector->setDefaultSaveName (uiDescription->getXmFileName ());
					if (savePath.length () > 0)
						fileSelector->setInitialDirectory (savePath.c_str ());
					if (fileSelector->runModal ())
					{
						const char* filename = fileSelector->getSelectedFile (0);
						savePath = filename;
						uiDescription->updateViewDescription (templateName.c_str (), getView (0));
						if (inspector)
							inspector->beforeSave ();
						storeAttributes ();
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
						void* parentPlatformWindow = getPlatformFrame ()->getPlatformRepresentation ();
						hierarchyBrowser = new ViewHierarchyBrowserWindow (dynamic_cast<CViewContainer*> (getView (0)), this, uiDescription, parentPlatformWindow);
						hierarchyBrowser->getFrame ()->setKeyboardHook (this);
					}
					break;
				}
				case kSizeToFitTag:
				{
					performAction (new SizeToFitOperation (selection));
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
void CEditFrame::updateResourceBitmaps ()
{
	std::list<std::string> resBitmapPaths;
	PlatformUtilities::gatherResourceBitmaps (resBitmapPaths);
	std::list<const std::string*> uiDescBitmapNames;
	uiDescription->collectBitmapNames (uiDescBitmapNames);
	std::list<std::string>::const_iterator it = resBitmapPaths.begin ();
	while (it != resBitmapPaths.end ())
	{
		bool found = false;
		std::list<const std::string*>::const_iterator it2 = uiDescBitmapNames.begin ();
		while (!found && it2 != uiDescBitmapNames.end ())
		{
			CBitmap* bitmap = uiDescription->getBitmap ((*it2)->c_str ());
			if (bitmap && bitmap->getResourceDescription ().type == CResourceDescription::kStringType)
			{
				if ((*it) == bitmap->getResourceDescription ().u.name)
					found = true;
			}
			it2++;
		}
		if (!found)
		{
			std::string name (*it);
			size_t dotPos = name.find (".");
			if (dotPos != std::string::npos)
				name.erase (dotPos, name.length () - dotPos);
			uiDescription->changeBitmap (name.c_str (), (*it).c_str (), 0);
		}
		it++;
	}
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::storeAttributes ()
{
	UIAttributes* attr = uiDescription->getCustomAttributes ("CEditFrame");
	if (!attr)
		attr = new UIAttributes;
	if (attr)
	{
		if (savePath.length () > 0)
			attr->setAttribute ("uidescPath", savePath.c_str ());
		if (grid)
		{
			std::stringstream stream;
			stream << grid->getSize ();
			attr->setAttribute ("gridsize", stream.str ().c_str ());
		}

		uiDescription->setCustomAttributes ("CEditFrame", attr);
	}
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::restoreAttributes ()
{
	UIAttributes* attr = uiDescription->getCustomAttributes ("CEditFrame");
	if (attr)
	{
		const std::string* value = attr->getAttributeValue ("uidescPath");
		if (value)
			savePath = *value;
		value = attr->getAttributeValue ("gridsize");
		if (value)
		{
			long gridSize = strtol (value->c_str (), 0, 10);
			grid->setSize (gridSize);
		}
	}
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
		if (view->getViewSize ().isEmpty ())
		{
			CRect size (origin, CPoint (20, 20));
			view->setViewSize (size);
			view->setMouseableArea (size);
		}
		else
		{
			CRect size (view->getViewSize ());
			size.offset (origin.x, origin.y);
			view->setViewSize (size);
			view->setMouseableArea (size);
		}
		performAction (new InsertViewOperation (parent, view, selection));
	}
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::embedSelectedViewsInto (const char* containerViewName)
{
	ViewFactory* viewFactory = dynamic_cast<ViewFactory*> (uiDescription->getViewFactory ());
	UIAttributes viewAttr;
	viewAttr.setAttribute ("class", containerViewName);
	CViewContainer* newContainer = dynamic_cast<CViewContainer*> (viewFactory->createView (viewAttr, uiDescription));
	if (newContainer)
	{
		performAction (new EmbedViewOperation (selection, newContainer));
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
	else if (message == kMsgViewSizeChanged)
	{
		static bool recursiveGuard = false;
		if (!recursiveGuard && sender == getView (0))
		{
			recursiveGuard = true;
			CView* view = getView (0);
			CRect viewSize = view->getViewSize ();
			setSize (viewSize.getWidth (), viewSize.getHeight ());
			CBaseObject* editorObj = dynamic_cast<CBaseObject*> (pEditor);
			if (editorObj)
			{
				editorObj->notify (this, kMsgViewSizeChanged);
			}
			recursiveGuard = false;
		}
	}
	return CFrame::notify (sender, message);
}

#define kSizingRectSize 10

//----------------------------------------------------------------------------------------------------
void CEditFrame::invalidSelection ()
{
	CRect r (selection->getBounds ());
	r.inset (-kSizingRectSize, -kSizingRectSize);
	invalidRect (r);
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::invalidRect (const CRect rect)
{
	CRect r (rect);
//	r.inset (-kSizingRectSize, -kSizingRectSize);
	CFrame::invalidRect (r);
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::draw (CDrawContext *pContext)
{
	drawRect (pContext, size);
}

//----------------------------------------------------------------------------------------------------
enum {
	kSizeModeNone = 0,
	kSizeModeBottomRight,
	kSizeModeBottomLeft,
	kSizeModeTopRight,
	kSizeModeTopLeft,
	kSizeModeLeft,
	kSizeModeRight,
	kSizeModeTop,
	kSizeModeBottom
};

//----------------------------------------------------------------------------------------------------
long CEditFrame::selectionHitTest (const CPoint& where, CView** resultView)
{
	FOREACH_IN_SELECTION(selection, view)
		CRect r = selection->getGlobalViewCoordinates (view);
		if (r.pointInside (where))
		{
			if (resultView)
				*resultView = view;
			CRect r2 (-kSizingRectSize, -kSizingRectSize, 0, 0);
			r2.offset (r.right, r.bottom);
			if (r2.pointInside (where))
				return kSizeModeBottomRight;
			if (r.getWidth () < kSizingRectSize*3 || r.getHeight () < kSizingRectSize*3)
				return kSizeModeNone;
			r2 (-kSizingRectSize, 0, 0, kSizingRectSize);
			r2.offset (r.right, r.top);
			if (r2.pointInside (where))
				return kSizeModeTopRight;
			r2 (kSizingRectSize, -kSizingRectSize, 0, 0);
			r2.offset (r.left, r.bottom);
			if (r2.pointInside (where))
				return kSizeModeBottomLeft;
			r2 (0, 0, kSizingRectSize, kSizingRectSize);
			r2.offset (r.left, r.top);
			if (r2.pointInside (where))
				return kSizeModeTopLeft;
			r2 (0, 0, kSizingRectSize, kSizingRectSize*2./3.);
			r2.offset (r.left+r.getWidth () / 2 - kSizingRectSize / 2, r.bottom - r2.bottom);
			if (r2.pointInside (where))
				return kSizeModeBottom;
			r2 (0, 0, kSizingRectSize, kSizingRectSize*2./3.);
			r2.offset (r.left+r.getWidth () / 2 - kSizingRectSize / 2, r.top);
			if (r2.pointInside (where))
				return kSizeModeTop;
			r2 (0, 0, kSizingRectSize*2./3., kSizingRectSize);
			r2.offset (r.left, r.top + r.getHeight () / 2 - kSizingRectSize / 2);
			if (r2.pointInside (where))
				return kSizeModeLeft;
			r2 (0, 0, kSizingRectSize*2./3., kSizingRectSize);
			r2.offset (r.right - r2.right, r.top + r.getHeight () / 2 - kSizingRectSize / 2);
			if (r2.pointInside (where))
				return kSizeModeRight;

			return kSizeModeNone;
		}
	FOREACH_IN_SELECTION_END
	if (resultView)
		*resultView = 0;
	return kSizeModeNone;
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::drawSizingHandles (CDrawContext* context, const CRect& r)
{
	if (r.getHeight () >= kSizingRectSize && r.getWidth () >= kSizingRectSize)
	{
		CPoint polygon[4];

		polygon[0] = CPoint (r.right, r.bottom);
		polygon[1] = CPoint (r.right-kSizingRectSize, r.bottom);
		polygon[2] = CPoint (r.right, r.bottom-kSizingRectSize);
		polygon[3] = polygon[0];
		context->drawPolygon (polygon, 4, kDrawFilled);

		if (r.getWidth () < kSizingRectSize*3 || r.getHeight () < kSizingRectSize*3)
			return;

		if (r.getWidth () >= kSizingRectSize * 3)
		{
			CRect r2 (0, 0, kSizingRectSize, kSizingRectSize*2./3.);
			r2.offset (r.left+r.getWidth () / 2 - kSizingRectSize / 2, r.top);
			context->drawRect (r2, kDrawFilled);

			r2 (0, 0, kSizingRectSize, kSizingRectSize*2./3.);
			r2.offset (r.left+r.getWidth () / 2 - kSizingRectSize / 2, r.bottom - r2.bottom);
			context->drawRect (r2, kDrawFilled);
		}

		if (r.getHeight () >= kSizingRectSize * 3)
		{
			CRect r2 (0, 0, kSizingRectSize*2./3., kSizingRectSize);
			r2.offset (r.left, r.top + r.getHeight () / 2 - kSizingRectSize / 2);
			context->drawRect (r2, kDrawFilled);

			r2 (0, 0, kSizingRectSize*2./3., kSizingRectSize);
			r2.offset (r.right - r2.right, r.top + r.getHeight () / 2 - kSizingRectSize / 2);
			context->drawRect (r2, kDrawFilled);
		}

		polygon[0] = CPoint (r.left, r.top);
		polygon[1] = CPoint (r.left+kSizingRectSize, r.top);
		polygon[2] = CPoint (r.left, r.top+kSizingRectSize);
		polygon[3] = polygon[0];
		context->drawPolygon (polygon, 4, kDrawFilled);

		polygon[0] = CPoint (r.right, r.top);
		polygon[1] = CPoint (r.right-kSizingRectSize, r.top);
		polygon[2] = CPoint (r.right, r.top+kSizingRectSize);
		polygon[3] = polygon[0];
		context->drawPolygon (polygon, 4, kDrawFilled);

		polygon[0] = CPoint (r.left, r.bottom);
		polygon[1] = CPoint (r.left+kSizingRectSize, r.bottom);
		polygon[2] = CPoint (r.left, r.bottom-kSizingRectSize);
		polygon[3] = polygon[0];
		context->drawPolygon (polygon, 4, kDrawFilled);
	}
}

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
		pContext->setDrawMode (kAntiAliasing);
		if (highlightView)
		{
			CRect r = CSelection::getGlobalViewCoordinates (highlightView);
			r.inset (2, 2);
			pContext->setFrameColor (uidHilightColor);
			pContext->setLineStyle (kLineSolid);
			pContext->setLineWidth (3);
			pContext->drawRect (r);
		}
		if (selection->total () > 0)
		{
			pContext->setDrawMode (kAntiAliasing);
			pContext->setFrameColor (uidSelectionColor);
			pContext->setFillColor (uidSelectionHandleColor);
			pContext->setLineStyle (kLineSolid);
			pContext->setLineWidth (1);

			FOREACH_IN_SELECTION(selection, view)
				CRect vs = selection->getGlobalViewCoordinates (view);
				pContext->drawRect (vs);
				if (mouseEditMode == kNoEditing && view != getView (0))
					drawSizingHandles (pContext, vs);
			FOREACH_IN_SELECTION_END

		}
	}
	pContext->setDrawMode (kAliasing);
	pContext->setClipRect (oldClip);
}

//----------------------------------------------------------------------------------------------------
CView* CEditFrame::getViewAt (const CPoint& p, bool deep) const
{
	CView* view = CViewContainer::getViewAt (p, deep);
	if (editMode != kNoEditMode)
	{
		ViewFactory* factory = dynamic_cast<ViewFactory*> (uiDescription->getViewFactory ());
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
CViewContainer* CEditFrame::getContainerAt (const CPoint& p, bool deep) const
{
	CViewContainer* view = CViewContainer::getContainerAt (p, deep);
	if (editMode != kNoEditMode)
	{
		ViewFactory* factory = dynamic_cast<ViewFactory*> (uiDescription->getViewFactory ());
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
				CView* mouseHitView = 0;
				long sizeMode = selectionHitTest (where, &mouseHitView);
				if (sizeMode == kSizeModeNone)
				{
					// first alter selection
					if (selection->contains (view))
					{
						if (buttons & kControl)
						{
							invalidSelection ();
							selection->remove (view);
							return kMouseEventHandled;
						}
					}
					else
					{
						if (buttons & kControl || buttons & kShift)
						{
							selection->add (view);
							invalidSelection ();
						}
						else
						{
							if (selection->total () > 0)
								invalidSelection ();
							selection->setExclusive (view);
							invalidSelection ();
						}
					}
					mouseHitView = view;
				}
				if (mouseHitView && selection->total () > 0 && !selection->contains (getView (0)))
				{
					if (!(buttons & kLButton) || editMode != kEditMode)
						return kMouseEventHandled;
					if (mouseHitView == 0)
						return kMouseEventHandled;
					if (sizeMode == kSizeModeNone)
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
						return kMouseEventHandled;
					}
					else
					{
						invalidSelection ();
						selection->setExclusive (mouseHitView);
						mouseEditMode = kSizeEditing;
						mouseStartPoint = where;
						if (grid)
							grid->process (mouseStartPoint);
						mouseSizeMode = sizeMode;
						if (showLines)
						{
							int crossLineMode = 0;
							switch (sizeMode)
							{
								case kSizeModeLeft:
								case kSizeModeRight:
								case kSizeModeTop:
								case kSizeModeBottom: crossLineMode = CrossLines::kSelectionStyle; break;
								default : crossLineMode = CrossLines::kDragStyle; break;
							}
							lines = new CrossLines (this, crossLineMode);
							if (crossLineMode == CrossLines::kSelectionStyle)
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
						CRect viewSize (view->getViewSize ());
						view->getParentView ()->frameToLocal (where);
						switch (mouseSizeMode)
						{
							case kSizeModeLeft: viewSize.left = where.x; break;
							case kSizeModeRight: viewSize.right = where.x; break;
							case kSizeModeTop: viewSize.top = where.y; break;
							case kSizeModeBottom: viewSize.bottom = where.y; break;
							case kSizeModeTopLeft: viewSize.left = where.x; viewSize.top = where.y; break;
							case kSizeModeTopRight: viewSize.right = where.x; viewSize.top = where.y; break;
							case kSizeModeBottomRight: viewSize.right = where.x; viewSize.bottom = where.y; break;
							case kSizeModeBottomLeft: viewSize.left = where.x; viewSize.bottom = where.y; break;
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
							if (lines->getStyle () == CrossLines::kSelectionStyle)
								lines->update (selection);
							else
								lines->update (mouseStartPoint);
						}
						selection->changed (CSelection::kMsgSelectionViewChanged);
					}
				}
				else if (mouseEditMode == kDragEditing)
				{
					startDrag (where);
				}
			}
			return kMouseEventHandled;
		}
		else if (buttons == 0)
		{
			long mode = selectionHitTest (where, 0);
			switch (mode)
			{
				case kSizeModeRight:
				case kSizeModeLeft: setCursor (kCursorHSize); break;
				case kSizeModeTop:
				case kSizeModeBottom: setCursor (kCursorVSize); break;
				case kSizeModeTopLeft:
				case kSizeModeBottomRight: setCursor (kCursorNWSESize); break;
				case kSizeModeTopRight:
				case kSizeModeBottomLeft: setCursor (kCursorNESWSize); break;
				default: setCursor (kCursorDefault); break;
			}
		}
		else
			setCursor (kCursorDefault);
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
	
	COffscreenContext* context = COffscreenContext::create (this, viewSize.getWidth (), viewSize.getHeight ());
	context->beginDraw ();
//	context->setGlobalAlpha (0.9f);

	FOREACH_IN_SELECTION(selection, view)
		if (!selection->containsParent (view))
		{
			CPoint p;
			view->getParentView ()->localToFrame (p);
			context->setOffset (CPoint (-viewSize.left + p.x, -viewSize.top + p.y));
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
void CEditFrame::startDrag (CPoint& where)
{
	CBitmap* bitmap = createBitmapFromSelection (selection);
	if (bitmap == 0)
		return;

	CRect selectionBounds = selection->getBounds ();

	CPoint offset;
	offset.x = (selectionBounds.left - where.x);
	offset.y = (selectionBounds.top - where.y);

	selection->setDragOffset (CPoint (offset.x, offset.y));

	ViewFactory* viewFactory = dynamic_cast<ViewFactory*> (uiDescription->getViewFactory ());
	CMemoryStream stream;
	if (!selection->store (stream, viewFactory, uiDescription))
		return;
		
	CDropSource dropSource (stream.getBuffer (), stream.tell (), CDropSource::kBinary);
	doDrag (&dropSource, offset, bitmap);
	if (bitmap)
		bitmap->forget ();
}

//----------------------------------------------------------------------------------------------------
CSelection* CEditFrame::getSelectionOutOfDrag (CDragContainer* drag)
{
	long size, type;
	const char* dragData = (const char*)drag->first (size, type);

	IController* controller = getEditor () ? dynamic_cast<IController*> (getEditor ()) : 0;
	if (controller)
		uiDescription->setController (controller);
	ViewFactory* viewFactory = dynamic_cast<ViewFactory*> (uiDescription->getViewFactory ());
	CMemoryStream stream (dragData, size);
	CSelection* selection = new CSelection;
	if (selection->restore (stream, viewFactory, uiDescription))
	{
		uiDescription->setController (0);
		return selection;
	}
	uiDescription->setController (0);
	selection->forget ();

	return 0;
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
			where2.offset (dragSelection->getDragOffset ().x, dragSelection->getDragOffset ().y);
			if (grid)
			{
				where2.offset (grid->getSize ()/2, grid->getSize ()/2);
				grid->process (where2);
			}
			CViewContainer* viewContainer = getContainerAt (where2, true);
			if (viewContainer)
			{
				CRect containerSize = viewContainer->getViewSize (containerSize);
				CPoint containerOffset;
				viewContainer->localToFrame (containerOffset);
				where2.offset (-containerOffset.x, -containerOffset.y);

				ViewFactory* viewFactory = dynamic_cast<ViewFactory*> (uiDescription->getViewFactory ());
				performAction (new ViewCopyOperation (dragSelection, selection, viewContainer, where2, viewFactory, uiDescription));
			}
			dragSelection->forget ();
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
		dragSelection = getSelectionOutOfDrag (drag);
		if (dragSelection)
		{
			CRect vr = dragSelection->getBounds ();
			CPoint where2 (where);
			where2.offset (dragSelection->getDragOffset ().x, dragSelection->getDragOffset ().y);
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
		else
		{
			setCursor (kCursorNotAllowed);
		}
	}
	else
		CFrame::onDragEnter (drag, where);
}

//----------------------------------------------------------------------------------------------------
void CEditFrame::onDragLeave (CDragContainer* drag, const CPoint& where)
{
	if (dragSelection)
	{
		dragSelection->forget ();
		dragSelection = 0;
	}
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
			if (dragSelection)
			{
				CRect vr = dragSelection->getBounds ();
				CPoint where2 (where);
				where2.offset (dragSelection->getDragOffset ().x, dragSelection->getDragOffset ().y);
				if (grid)
				{
					where2.offset (grid->getSize ()/2, grid->getSize ()/2);
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
void CEditFrame::performBitmapNinePartTiledChange (const char* bitmapName, const CRect* offsets)
{
	ViewFactory* viewFactory = dynamic_cast<ViewFactory*> (uiDescription->getViewFactory ());
	std::map<CView*, std::string> m;
	collectViewsWithAttributeValue (viewFactory, uiDescription, getView (0), IViewCreator::kBitmapType, bitmapName, m);

	CBitmap* bitmap = uiDescription->getBitmap (bitmapName);
	if (bitmap == 0)
		return;

	uiDescription->changeBitmap (bitmapName, bitmap->getResourceDescription ().u.name, offsets);
	performAttributeChange (viewFactory, uiDescription, bitmapName, m);

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
long CEditFrame::onKeyDown (const VstKeyCode& code, CFrame* frame)
{
	VstKeyCode vc (code);
	if (onKeyDown (vc) == 1)
		return 1;
	return -1;
}

//----------------------------------------------------------------------------------------------------
long CEditFrame::onKeyUp (const VstKeyCode& code, CFrame* frame)
{
	return -1;
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

} // namespace

#endif // VSTGUI_LIVE_EDITING
