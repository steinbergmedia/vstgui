//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins :
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

#include "uiactions.h"

#if VSTGUI_LIVE_EDITING

#include "uieditview.h"
#include "../uiviewfactory.h"
#include "../uidescription.h"
#include "../uiattributes.h"
#include "../../lib/cgraphicspath.h"
#include "../detail/uiviewcreatorattributes.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
SizeToFitOperation::SizeToFitOperation (UISelection* selection)
: BaseSelectionOperation<std::pair<SharedPointer<CView>, CRect> > (selection)
{
	FOREACH_IN_SELECTION(selection, view)
		push_back (std::make_pair (view, view->getViewSize ()));
	FOREACH_IN_SELECTION_END
}

//----------------------------------------------------------------------------------------------------
SizeToFitOperation::~SizeToFitOperation ()
{
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr SizeToFitOperation::getName ()
{
	return "Size To Fit";
}

//----------------------------------------------------------------------------------------------------
void SizeToFitOperation::perform ()
{
	selection->changed (UISelection::kMsgSelectionViewWillChange);
	const_iterator it = begin ();
	while (it != end ())
	{
		(*it).first->invalid ();
		(*it).first->sizeToFit ();
		(*it).first->invalid ();
		it++;
	}
	selection->changed (UISelection::kMsgSelectionViewChanged);
}

//----------------------------------------------------------------------------------------------------
void SizeToFitOperation::undo ()
{
	selection->changed (UISelection::kMsgSelectionViewWillChange);
	const_iterator it = begin ();
	while (it != end ())
	{
		(*it).first->invalid ();
		(*it).first->setViewSize ((*it).second);
		(*it).first->setMouseableArea ((*it).second);
		(*it).first->invalid ();
		it++;
	}
	selection->changed (UISelection::kMsgSelectionViewChanged);
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UnembedViewOperation::UnembedViewOperation (UISelection* selection, const IViewFactory* factory)
: BaseSelectionOperation<SharedPointer<CView> > (selection)
, factory (factory)
{
	containerView = dynamic_cast<CViewContainer*> (selection->first ());
	collectSubviews (containerView, true);
	parent = dynamic_cast<CViewContainer*> (containerView->getParentView ());
}

//----------------------------------------------------------------------------------------------------
UnembedViewOperation::~UnembedViewOperation ()
{
}

//----------------------------------------------------------------------------------------------------
void UnembedViewOperation::collectSubviews (CViewContainer* container, bool deep)
{
	ViewIterator it (container);
	while (*it)
	{
		if (factory->getViewName (*it))
		{
			push_back (*it);
		}
		else if (deep)
		{
			CViewContainer* c = dynamic_cast<CViewContainer*>(*it);
			if (c)
				collectSubviews (c, false);
		}
		++it;
	}
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr UnembedViewOperation::getName ()
{
	return "Unembed Views";
}

//----------------------------------------------------------------------------------------------------
void UnembedViewOperation::perform ()
{
	IDependency::DeferChanges dc (selection);
	selection->remove (containerView);
	CRect containerViewSize = containerView->getViewSize ();
	const_reverse_iterator it = rbegin ();
	while (it != rend ())
	{
		CView* view = (*it);
		CRect viewSize = view->getViewSize ();
		CRect mouseSize = view->getMouseableArea ();
		containerView->removeView (view, false);
		viewSize.offset (containerViewSize.left, containerViewSize.top);
		mouseSize.offset (containerViewSize.left, containerViewSize.top);
		view->setViewSize (viewSize);
		view->setMouseableArea (mouseSize);
		if (parent->addView (view))
			selection->add (view);
		it++;
	}
	parent->removeView (containerView, false);
}

//----------------------------------------------------------------------------------------------------
void UnembedViewOperation::undo ()
{
	CRect containerViewSize = containerView->getViewSize ();
	const_iterator it = begin ();
	while (it != end ())
	{
		CView* view = (*it);
		parent->removeView (view, false);
		CRect viewSize = view->getViewSize ();
		CRect mouseSize = view->getMouseableArea ();
		viewSize.offset (-containerViewSize.left, -containerViewSize.top);
		mouseSize.offset (-containerViewSize.left, -containerViewSize.top);
		view->setViewSize (viewSize);
		view->setMouseableArea (mouseSize);
		containerView->addView (view);
		it++;
	}
	parent->addView (containerView);
	selection->setExclusive (containerView);
}

//-----------------------------------------------------------------------------
EmbedViewOperation::EmbedViewOperation (UISelection* selection, CViewContainer* newContainer)
: BaseSelectionOperation<std::pair<SharedPointer<CView>, CRect> > (selection)
, newContainer (newContainer)
{
	parent = dynamic_cast<CViewContainer*> (selection->first ()->getParentView ());
	FOREACH_IN_SELECTION(selection, view)
		if (view->getParentView () == parent)
		{
			push_back (std::make_pair (view, view->getViewSize ()));
		}
	FOREACH_IN_SELECTION_END

	CRect r = selection->first ()->getViewSize ();
	const_iterator it = begin ();
	while (it != end ())
	{
		CView* view = (*it).first;
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
	r.extend (10, 10);
	newContainer->setViewSize (r);
	newContainer->setMouseableArea (r);
}

//-----------------------------------------------------------------------------
EmbedViewOperation::~EmbedViewOperation ()
{
}

//-----------------------------------------------------------------------------
UTF8StringPtr EmbedViewOperation::getName ()
{
	return "Embed Views";
}

//-----------------------------------------------------------------------------
void EmbedViewOperation::perform ()
{
	CRect parentRect = newContainer->getViewSize ();
	const_iterator it = begin ();
	while (it != end ())
	{
		CView* view = (*it).first;
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
	selection->setExclusive (newContainer);
}

//-----------------------------------------------------------------------------
void EmbedViewOperation::undo ()
{
	selection->empty ();
	CRect parentRect = newContainer->getViewSize ();
	const_reverse_iterator it = rbegin ();
	while (it != rend ())
	{
		CView* view = (*it).first;
		newContainer->removeView (view, false);
		CRect r = (*it).second;
		view->setViewSize (r);
		view->setMouseableArea (r);
		parent->addView (view);
		selection->add (view);
		it++;
	}
	parent->removeView (newContainer);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ViewCopyOperation::ViewCopyOperation (UISelection* copySelection, UISelection* workingSelection, CViewContainer* parent, const CPoint& offset, IUIDescription* desc)
: parent (parent)
, copySelection (copySelection)
, workingSelection (workingSelection)
{
	CRect selectionBounds = copySelection->getBounds ();
	FOREACH_IN_SELECTION(copySelection, view)
		if (!copySelection->containsParent (view))
		{
			CRect viewSize = UISelection::getGlobalViewCoordinates (view);
			CRect newSize (0, 0, view->getWidth (), view->getHeight ());
			newSize.offset (offset.x, offset.y);
			newSize.offset (viewSize.left - selectionBounds.left, viewSize.top - selectionBounds.top);

			view->setViewSize (newSize);
			view->setMouseableArea (newSize);
			push_back (view);
		}
	FOREACH_IN_SELECTION_END

	FOREACH_IN_SELECTION(workingSelection, view)
		oldSelectedViews.push_back (view);
	FOREACH_IN_SELECTION_END
}

//-----------------------------------------------------------------------------
ViewCopyOperation::~ViewCopyOperation ()
{
}

//-----------------------------------------------------------------------------
UTF8StringPtr ViewCopyOperation::getName () 
{
	if (size () > 0)
		return "Copy Views";
	return "Copy View";
}

//-----------------------------------------------------------------------------
void ViewCopyOperation::perform ()
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

//-----------------------------------------------------------------------------
void ViewCopyOperation::undo ()
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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ViewSizeChangeOperation::ViewSizeChangeOperation (UISelection* selection, bool sizing, bool autosizingEnabled)
: BaseSelectionOperation<std::pair<SharedPointer<CView>, CRect> > (selection)
, first (true)
, sizing (sizing)
, autosizing (autosizingEnabled)
{
	FOREACH_IN_SELECTION(selection, view)
		push_back (std::make_pair (view, view->getViewSize ()));
	FOREACH_IN_SELECTION_END
}

//-----------------------------------------------------------------------------
ViewSizeChangeOperation::~ViewSizeChangeOperation ()
{
}

//-----------------------------------------------------------------------------
UTF8StringPtr ViewSizeChangeOperation::getName ()
{
	if (size () > 1)
		return sizing ? "Resize Views" : "Move Views";
	return sizing ? "Resize View" : "Move View";
}

//-----------------------------------------------------------------------------
void ViewSizeChangeOperation::perform ()
{
	if (first)
	{
		first = false;
		return;
	}
	undo ();
}

//-----------------------------------------------------------------------------
void ViewSizeChangeOperation::undo ()
{
	selection->empty ();
	iterator it = begin ();
	while (it != end ())
	{
		CView* view = (*it).first;
		CRect size ((*it).second);
		view->invalid ();
		(*it).second = view->getViewSize ();
		CViewContainer* container = 0;
		bool oldAutosizing = false;
		if (!autosizing)
		{
			container = dynamic_cast<CViewContainer*> (view);
			if (container)
			{
				oldAutosizing = container->getAutosizingEnabled ();
				container->setAutosizingEnabled (false);
			}
		}
		view->setViewSize (size);
		view->setMouseableArea (size);
		view->invalid ();
		selection->add (view);
		if (!autosizing && container)
		{
			container->setAutosizingEnabled (oldAutosizing);
		}
		it++;
	}
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
DeleteOperation::DeleteOperation (UISelection* selection)
: selection (selection)
{
	FOREACH_IN_SELECTION(selection, view)
		CViewContainer* container = dynamic_cast<CViewContainer*> (view->getParentView ());
		if (dynamic_cast<UIEditView*>(container) == 0)
		{
			CView* nextView = 0;
			ViewIterator it (container);
			while (*it)
			{
				if (*it == view)
				{
					nextView = *++it;
					break;
				}
				++it;
			}
			insert (std::make_pair (container, new DeleteOperationViewAndNext (view, nextView)));
		}
	FOREACH_IN_SELECTION_END
}

//----------------------------------------------------------------------------------------------------
DeleteOperation::~DeleteOperation ()
{
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr DeleteOperation::getName ()
{
	if (size () > 1)
		return "Delete Views";
	return "Delete View";
}

//----------------------------------------------------------------------------------------------------
void DeleteOperation::perform ()
{
	selection->empty ();
	const_iterator it = begin ();
	while (it != end ())
	{
		(*it).first->removeView ((*it).second->view);
		it++;
	}
}

//----------------------------------------------------------------------------------------------------
void DeleteOperation::undo ()
{
	selection->empty ();
	IDependency::DeferChanges dc (selection);
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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
InsertViewOperation::InsertViewOperation (CViewContainer* parent, CView* view, UISelection* selection)
: parent (parent)
, view (view)
, selection (selection)
{
}

//-----------------------------------------------------------------------------
InsertViewOperation::~InsertViewOperation ()
{
}

//-----------------------------------------------------------------------------
UTF8StringPtr InsertViewOperation::getName ()
{
	return "Insert New Subview";
}

//-----------------------------------------------------------------------------
void InsertViewOperation::perform ()
{
	if (parent->addView (view))
		selection->setExclusive (view);
}

//-----------------------------------------------------------------------------
void InsertViewOperation::undo ()
{
	selection->remove (view);
	view->remember ();
	if (!parent->removeView (view))
		view->forget ();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
TransformViewTypeOperation::TransformViewTypeOperation (UISelection* selection, IdStringPtr viewClassName, UIDescription* desc, const UIViewFactory* factory)
: view (selection->first ())
, newView (0)
, beforeView (0)
, parent (dynamic_cast<CViewContainer*> (view->getParentView ()))
, selection (selection)
, factory (factory)
, description (desc)
{
	UIAttributes attr;
	if (factory->getAttributesForView (view, desc, attr))
	{
		attr.setAttribute (UIViewCreator::kAttrClass, viewClassName);
		newView = factory->createView (attr, desc);
		ViewIterator it (parent);
		while (*it)
		{
			if (*it == view)
			{
				beforeView = *++it;
				break;
			}
			++it;
		}
	}
}

//-----------------------------------------------------------------------------
TransformViewTypeOperation::~TransformViewTypeOperation ()
{
	if (newView)
		newView->forget ();
}

//-----------------------------------------------------------------------------
UTF8StringPtr TransformViewTypeOperation::getName ()
{
	return "Transform View Type";
}

//-----------------------------------------------------------------------------
void TransformViewTypeOperation::exchangeSubViews (CViewContainer* src, CViewContainer* dst)
{
	if (src && dst)
	{
		std::list<CView*> temp;

		ViewIterator it (src);
		while (*it)
		{
			CView* childView = *it;
			if (factory->getViewName (childView))
			{
				temp.push_back (childView);
			}
			else if (CViewContainer* container = dynamic_cast<CViewContainer*>(childView))
			{
				exchangeSubViews (container, dst);
			}
			++it;
		}
		for (std::list<CView*>::const_iterator it = temp.begin (); it != temp.end (); it++)
		{
			src->removeView (*it, false);
			dst->addView (*it);
		}
	}
}

//-----------------------------------------------------------------------------
void TransformViewTypeOperation::perform ()
{
	if (newView)
	{
		newView->remember ();
		parent->removeView (view);
		if (beforeView)
			parent->addView (newView, beforeView);
		else
			parent->addView (newView);
		exchangeSubViews (view.cast<CViewContainer> (), dynamic_cast<CViewContainer*> (newView));
		selection->setExclusive (newView);
	}
}

//-----------------------------------------------------------------------------
void TransformViewTypeOperation::undo ()
{
	if (newView)
	{
		view->remember ();
		parent->removeView (newView);
		if (beforeView)
			parent->addView (view, beforeView);
		else
			parent->addView (view);
		exchangeSubViews (dynamic_cast<CViewContainer*> (newView), view.cast<CViewContainer> ());
		selection->setExclusive (view);
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
AttributeChangeAction::AttributeChangeAction (UIDescription* desc, UISelection* selection, const std::string& attrName, const std::string& attrValue)
: desc (desc)
, attrName (attrName)
, attrValue (attrValue)
, selection (selection)
{
	const UIViewFactory* viewFactory = dynamic_cast<const UIViewFactory*> (desc->getViewFactory ());
	std::string attrOldValue;
	FOREACH_IN_SELECTION(selection, view)
		viewFactory->getAttributeValue (view, attrName, attrOldValue, desc);
		insert (std::make_pair (view, attrOldValue));
	FOREACH_IN_SELECTION_END
	name = "'" + attrName + "' change";
}

//-----------------------------------------------------------------------------
AttributeChangeAction::~AttributeChangeAction ()
{
}

//-----------------------------------------------------------------------------
UTF8StringPtr AttributeChangeAction::getName ()
{
	return name.c_str ();
}

//-----------------------------------------------------------------------------
void AttributeChangeAction::updateSelection ()
{
	for (const_iterator it = begin (); it != end (); it++)
	{
		if (selection->contains ((*it).first) == false)
		{
			IDependency::DeferChanges dc (selection);
			selection->empty ();
			for (const_iterator it2 = begin (); it2 != end (); it2++)
				selection->add ((*it2).first);
			break;
		}
	}
}

//-----------------------------------------------------------------------------
void AttributeChangeAction::perform ()
{
	const IViewFactory* viewFactory = desc->getViewFactory ();
	UIAttributes attr;
	attr.setAttribute (attrName, attrValue);
	selection->changed (UISelection::kMsgSelectionViewWillChange);
	const_iterator it = begin ();
	while (it != end ())
	{
		(*it).first->invalid ();	// we need to invalid before changing anything as the size may change
		viewFactory->applyAttributeValues ((*it).first, attr, desc);
		(*it).first->invalid ();	// and afterwards also
		it++;
	}
	selection->changed (UISelection::kMsgSelectionViewChanged);
	updateSelection ();
}

//-----------------------------------------------------------------------------
void AttributeChangeAction::undo ()
{
	const IViewFactory* viewFactory = desc->getViewFactory ();
	selection->changed (UISelection::kMsgSelectionViewWillChange);
	const_iterator it = begin ();
	while (it != end ())
	{
		UIAttributes attr;
		attr.setAttribute (attrName, (*it).second);
		(*it).first->invalid ();	// we need to invalid before changing anything as the size may change
		viewFactory->applyAttributeValues ((*it).first, attr, desc);
		(*it).first->invalid ();	// and afterwards also
		it++;
	}
	selection->changed (UISelection::kMsgSelectionViewChanged);
	updateSelection ();
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
MultipleAttributeChangeAction::MultipleAttributeChangeAction (UIDescription* description, const std::list<CView*>& views, IViewCreator::AttrType attrType, UTF8StringPtr oldValue, UTF8StringPtr newValue)
: description (description)
, oldValue (oldValue)
, newValue (newValue)
{
	const UIViewFactory* viewFactory = dynamic_cast<const UIViewFactory*>(description->getViewFactory ());
	for (std::list<CView*>::const_iterator it = views.begin (); it != views.end (); it++)
		collectViewsWithAttributeValue (viewFactory, description, *it, attrType, oldValue);
}

//----------------------------------------------------------------------------------------------------
void MultipleAttributeChangeAction::collectViewsWithAttributeValue (const UIViewFactory* viewFactory, IUIDescription* desc, CView* startView, IViewCreator::AttrType type, const std::string& value)
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
							push_back (std::make_pair (view, (*namesIt)));
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
void MultipleAttributeChangeAction::collectAllSubViews (CView* view, std::list<CView*>& views)
{
	views.push_back (view);
	CViewContainer* container = dynamic_cast<CViewContainer*> (view);
	if (container)
	{
		ViewIterator it (container);
		while (*it)
		{
			collectAllSubViews (*it, views);
			++it;
		}
	}
}

//----------------------------------------------------------------------------------------------------
void MultipleAttributeChangeAction::setAttributeValue (UTF8StringPtr value)
{
	const IViewFactory* viewFactory = description->getViewFactory ();
	const_iterator it = begin ();
	while (it != end ())
	{
		CView* view = (*it).first;
		UIAttributes newAttr;
		newAttr.setAttribute ((*it).second, value);
		viewFactory->applyAttributeValues (view, newAttr, description);
		view->invalid ();
		it++;
	}
}

//----------------------------------------------------------------------------------------------------
void MultipleAttributeChangeAction::perform ()
{
	setAttributeValue (newValue.c_str ());
}

//----------------------------------------------------------------------------------------------------
void MultipleAttributeChangeAction::undo ()
{
	setAttributeValue (oldValue.c_str ());
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
TagChangeAction::TagChangeAction (UIDescription* description, UTF8StringPtr name, UTF8StringPtr newTagString, bool remove, bool performOrUndo)
: description(description)
, name (name)
, newTag (newTagString ? newTagString : "")
, remove (remove)
, performOrUndo (performOrUndo)
, isNewTag (!description->hasTagName (name))
{
	description->getControlTagString (name, originalTag);
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr TagChangeAction::getName ()
{
	return isNewTag ? "Add Tag" : "Change Tag";
}

//----------------------------------------------------------------------------------------------------
void TagChangeAction::perform ()
{
	if (performOrUndo)
	{
		if (remove)
		{
			description->removeTag (name.c_str ());
		}
		else
		{
			description->changeControlTagString (name.c_str (), newTag, isNewTag);
		}
	}
}

//----------------------------------------------------------------------------------------------------
void TagChangeAction::undo ()
{
	if (performOrUndo == false)
	{
		if (isNewTag)
			description->removeTag (name.c_str ());
		else
			description->changeControlTagString (name.c_str (), originalTag);
	}
}

//----------------------------------------------------------------------------------------------------
TagNameChangeAction::TagNameChangeAction (UIDescription* description, UTF8StringPtr oldName, UTF8StringPtr newName, bool performOrUndo)
: description(description)
, oldName (oldName)
, newName (newName)
, performOrUndo (performOrUndo)
{
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr TagNameChangeAction::getName ()
{
	return "Change Tag Name";
}

//----------------------------------------------------------------------------------------------------
void TagNameChangeAction::perform ()
{
	if (performOrUndo)
		description->changeTagName (oldName.c_str(), newName.c_str());
}

//----------------------------------------------------------------------------------------------------
void TagNameChangeAction::undo ()
{
	if (performOrUndo == false)
		description->changeTagName (newName.c_str(), oldName.c_str());
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
ColorNameChangeAction::ColorNameChangeAction (UIDescription* description, UTF8StringPtr oldName, UTF8StringPtr newName, bool performOrUndo)
: description(description)
, oldName (oldName)
, newName (newName)
, performOrUndo (performOrUndo)
{
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr ColorNameChangeAction::getName ()
{
	return "Change Color Name";
}

//----------------------------------------------------------------------------------------------------
void ColorNameChangeAction::perform ()
{
	if (performOrUndo)
		description->changeColorName (oldName.c_str(), newName.c_str());
}

//----------------------------------------------------------------------------------------------------
void ColorNameChangeAction::undo ()
{
	if (performOrUndo == false)
		description->changeColorName (newName.c_str(), oldName.c_str());
}

//----------------------------------------------------------------------------------------------------
ColorChangeAction::ColorChangeAction (UIDescription* description, UTF8StringPtr name, const CColor& color, bool remove, bool performOrUndo)
: description(description)
, name (name)
, newColor (color)
, remove (remove)
, performOrUndo (performOrUndo)
, isNewColor (!description->hasColorName (name))
{
	if (!isNewColor)
		description->getColor (name, oldColor);
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr ColorChangeAction::getName ()
{
	return isNewColor ? "Add Color" : "Change Color";
}

//----------------------------------------------------------------------------------------------------
void ColorChangeAction::perform ()
{
	if (performOrUndo)
	{
		if (remove)
		{
			description->removeColor (name.c_str ());
		}
		else
		{
			description->changeColor (name.c_str (), newColor);
		}
	}
}

//----------------------------------------------------------------------------------------------------
void ColorChangeAction::undo ()
{
	if (performOrUndo == false)
	{
		if (isNewColor)
			description->removeColor (name.c_str ());
		else
			description->changeColor (name.c_str (), oldColor);
	}
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
BitmapChangeAction::BitmapChangeAction (UIDescription* description, UTF8StringPtr name, UTF8StringPtr path, bool remove, bool performOrUndo)
: description(description)
, name (name)
, path (path ? path : "")
, remove (remove)
, performOrUndo (performOrUndo)
, isNewBitmap (!description->hasBitmapName (name))
{
	CBitmap* bitmap = description->getBitmap (name);
	if (bitmap)
		originalPath = bitmap->getResourceDescription().u.name;
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr BitmapChangeAction::getName ()
{
	return isNewBitmap ? "Add New Bitmap" : "Change Bitmap";
}

//----------------------------------------------------------------------------------------------------
void BitmapChangeAction::perform ()
{
	if (performOrUndo)
	{
		if (remove)
		{
			description->removeBitmap (name.c_str ());
		}
		else
		{
			description->changeBitmap (name.c_str (), path.c_str ());
		}
	}
}

//----------------------------------------------------------------------------------------------------
void BitmapChangeAction::undo ()
{
	if (performOrUndo == false)
	{
		if (isNewBitmap)
			description->removeBitmap (name.c_str ());
		else
			description->changeBitmap (name.c_str (), originalPath.c_str ());
	}
}

//----------------------------------------------------------------------------------------------------
BitmapNameChangeAction::BitmapNameChangeAction (UIDescription* description, UTF8StringPtr oldName, UTF8StringPtr newName, bool performOrUndo)
: description(description)
, oldName (oldName)
, newName (newName)
, performOrUndo (performOrUndo)
{
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr BitmapNameChangeAction::getName ()
{
	return "Change Bitmap Name";
}

//----------------------------------------------------------------------------------------------------
void BitmapNameChangeAction::perform ()
{
	if (performOrUndo)
		description->changeBitmapName (oldName.c_str(), newName.c_str());
}

//----------------------------------------------------------------------------------------------------
void BitmapNameChangeAction::undo ()
{
	if (performOrUndo == false)
		description->changeBitmapName (newName.c_str(), oldName.c_str());
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
NinePartTiledBitmapChangeAction::NinePartTiledBitmapChangeAction (UIDescription* description, UTF8StringPtr name, const CRect* rect, bool performOrUndo)
: description (description)
, name (name)
, oldRect (0)
, newRect (0)
, performOrUndo (performOrUndo)
{
	if (rect)
		newRect = new CRect (*rect);
	CBitmap* bitmap = description->getBitmap (name);
	if (bitmap)
	{
		CNinePartTiledBitmap* tiledBitmap = dynamic_cast<CNinePartTiledBitmap*>(bitmap);
		if (tiledBitmap)
		{
			const CNinePartTiledDescription& offset = tiledBitmap->getPartOffsets ();
			oldRect = new CRect;
			oldRect->left = offset.left;
			oldRect->top = offset.top;
			oldRect->right = offset.right;
			oldRect->bottom = offset.bottom;
		}
	}
}

//----------------------------------------------------------------------------------------------------
NinePartTiledBitmapChangeAction::~NinePartTiledBitmapChangeAction ()
{
	if (newRect)
		delete newRect;
	if (oldRect)
		delete oldRect;
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr NinePartTiledBitmapChangeAction::getName ()
{
	return "Change NinePartTiledBitmap";
}

//----------------------------------------------------------------------------------------------------
void NinePartTiledBitmapChangeAction::perform ()
{
	if (performOrUndo)
	{
		CBitmap* bitmap = description->getBitmap (name.c_str ());
		if (bitmap)
			description->changeBitmap (name.c_str (), bitmap->getResourceDescription ().u.name, newRect);
	}
}

//----------------------------------------------------------------------------------------------------
void NinePartTiledBitmapChangeAction::undo ()
{
	if (performOrUndo == false)
	{
		CBitmap* bitmap = description->getBitmap (name.c_str ());
		if (bitmap)
			description->changeBitmap (name.c_str (), bitmap->getResourceDescription ().u.name, oldRect);
	}
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
BitmapFilterChangeAction::BitmapFilterChangeAction (UIDescription* description, UTF8StringPtr bitmapName, const std::list<SharedPointer<UIAttributes> >& attributes, bool performOrUndo)
: description (description)
, bitmapName (bitmapName)
, newAttributes (attributes)
, performOrUndo (performOrUndo)
{
	description->collectBitmapFilters (bitmapName, oldAttributes);
}

//----------------------------------------------------------------------------------------------------
BitmapFilterChangeAction::~BitmapFilterChangeAction ()
{
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr BitmapFilterChangeAction::getName ()
{
	return "Change Bitmap Filter";
}

//----------------------------------------------------------------------------------------------------
void BitmapFilterChangeAction::perform ()
{
	if (performOrUndo)
	{
		description->changeBitmapFilters (bitmapName.c_str (), newAttributes);
	}
}

//----------------------------------------------------------------------------------------------------
void BitmapFilterChangeAction::undo ()
{
	if (performOrUndo == false)
	{
		description->changeBitmapFilters (bitmapName.c_str (), oldAttributes);
	}
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
GradientChangeAction::GradientChangeAction (UIDescription* description, UTF8StringPtr name, CGradient* gradient, bool remove, bool performOrUndo)
: description(description)
, name (name)
, gradient (gradient)
, remove (remove)
, performOrUndo (performOrUndo)
{
	originalGradient = description->getGradient (name);
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr GradientChangeAction::getName ()
{
	return originalGradient ? "Change Gradient" : "Add New Gradient";
}

//----------------------------------------------------------------------------------------------------
void GradientChangeAction::perform ()
{
	if (performOrUndo)
	{
		if (remove)
		{
			description->removeGradient (name.c_str ());
		}
		else
		{
			description->changeGradient (name.c_str (), gradient);
		}
	}
}

//----------------------------------------------------------------------------------------------------
void GradientChangeAction::undo ()
{
	if (performOrUndo == false)
	{
		if (originalGradient)
		{
			description->changeGradient (name.c_str (), originalGradient);
		}
		else
		{
			description->removeGradient (name.c_str ());
		}
	}
}

//----------------------------------------------------------------------------------------------------
GradientNameChangeAction::GradientNameChangeAction (UIDescription* description, UTF8StringPtr oldName, UTF8StringPtr newName, bool performOrUndo)
: description(description)
, oldName (oldName)
, newName (newName)
, performOrUndo (performOrUndo)
{
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr GradientNameChangeAction::getName ()
{
	return "Change Gradient Name";
}

//----------------------------------------------------------------------------------------------------
void GradientNameChangeAction::perform ()
{
	if (performOrUndo)
		description->changeGradientName (oldName.c_str(), newName.c_str());
}

//----------------------------------------------------------------------------------------------------
void GradientNameChangeAction::undo ()
{
	if (performOrUndo == false)
		description->changeGradientName (newName.c_str(), oldName.c_str());
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
FontChangeAction::FontChangeAction (UIDescription* description, UTF8StringPtr name, CFontRef font, bool remove, bool performOrUndo)
: description(description)
, name (name)
, font (font)
, remove (remove)
, performOrUndo (performOrUndo)
{
	originalFont = description->getFont (name);
	if (remove)
		description->getAlternativeFontNames (name, alternativeNames);
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr FontChangeAction::getName ()
{
	return originalFont ? "Change Font" : "Add New Font";
}

//----------------------------------------------------------------------------------------------------
void FontChangeAction::perform ()
{
	if (performOrUndo)
	{
		if (remove)
		{
			description->removeFont (name.c_str ());
		}
		else
		{
			description->changeFont (name.c_str (), font);
		}
	}
}

//----------------------------------------------------------------------------------------------------
void FontChangeAction::undo ()
{
	if (performOrUndo == false)
	{
		if (originalFont)
		{
			description->changeFont (name.c_str (), originalFont);
			description->changeAlternativeFontNames (name.c_str (), alternativeNames.c_str ());
		}
		else
		{
			description->removeFont (name.c_str ());
		}
	}
}

//----------------------------------------------------------------------------------------------------
FontNameChangeAction::FontNameChangeAction (UIDescription* description, UTF8StringPtr oldName, UTF8StringPtr newName, bool performOrUndo)
: description(description)
, oldName (oldName)
, newName (newName)
, performOrUndo (performOrUndo)
{
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr FontNameChangeAction::getName ()
{
	return "Change Font Name";
}

//----------------------------------------------------------------------------------------------------
void FontNameChangeAction::perform ()
{
	if (performOrUndo)
		description->changeFontName (oldName.c_str(), newName.c_str());
}

//----------------------------------------------------------------------------------------------------
void FontNameChangeAction::undo ()
{
	if (performOrUndo == false)
		description->changeFontName (newName.c_str(), oldName.c_str());
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
AlternateFontChangeAction::AlternateFontChangeAction (UIDescription* description, UTF8StringPtr fontName, UTF8StringPtr newAlternateFontNames)
: description (description)
, fontName (fontName)
, newAlternateFontNames (newAlternateFontNames ? newAlternateFontNames : "")
{
	description->getAlternativeFontNames (fontName, oldAlternateFontNames);
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr AlternateFontChangeAction::getName ()
{
	return "Change Alternative Font Names";
}

//----------------------------------------------------------------------------------------------------
void AlternateFontChangeAction::perform ()
{
	description->changeAlternativeFontNames (fontName.c_str (), newAlternateFontNames.c_str ());
}

//----------------------------------------------------------------------------------------------------
void AlternateFontChangeAction::undo ()
{
	description->changeAlternativeFontNames (fontName.c_str (), oldAlternateFontNames.c_str ());
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
HierarchyMoveViewOperation::HierarchyMoveViewOperation (CView* view, UISelection* selection, bool up)
: view (view)
, parent (0)
, selection (selection)
, up (up)
{
	parent = dynamic_cast<CViewContainer*> (view->getParentView ());
}

//----------------------------------------------------------------------------------------------------
HierarchyMoveViewOperation::~HierarchyMoveViewOperation ()
{
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr HierarchyMoveViewOperation::getName ()
{
	return "Change View Hierarchy";
}

//----------------------------------------------------------------------------------------------------
void HierarchyMoveViewOperation::perform ()
{
	if (!parent)
		return;
	uint32_t currentIndex = 0;
	ViewIterator it (parent);
	while (*it && *it != view)
	{
		it++;
		currentIndex++;
	}
	parent->changeViewZOrder (view, up ? currentIndex - 1 : currentIndex + 1);
	selection->changed (UISelection::kMsgSelectionChanged);
	parent->invalid ();
}

//----------------------------------------------------------------------------------------------------
void HierarchyMoveViewOperation::undo ()
{
	up = !up;
	perform ();
	up = !up;
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
TemplateNameChangeAction::TemplateNameChangeAction (UIDescription* description, IActionPerformer* actionPerformer, UTF8StringPtr oldName, UTF8StringPtr newName)
: description (description)
, actionPerformer (actionPerformer)
, oldName (oldName)
, newName (newName)
{
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr TemplateNameChangeAction::getName ()
{
	return "Change Template Name";
}

//----------------------------------------------------------------------------------------------------
void TemplateNameChangeAction::perform ()
{
	actionPerformer->onTemplateNameChange (oldName.c_str (), newName.c_str ());
	description->changeTemplateName (oldName.c_str (), newName.c_str ());
}

//----------------------------------------------------------------------------------------------------
void TemplateNameChangeAction::undo ()
{
	actionPerformer->onTemplateNameChange (newName.c_str (), oldName.c_str ());
	description->changeTemplateName (newName.c_str (), oldName.c_str ());
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
CreateNewTemplateAction::CreateNewTemplateAction (UIDescription* description, IActionPerformer* actionPerformer, UTF8StringPtr name, UTF8StringPtr baseViewClassName)
: description (description)
, actionPerformer (actionPerformer)
, name (name)
, baseViewClassName (baseViewClassName)
{
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr CreateNewTemplateAction::getName ()
{
	return "Create New Template";
}

//----------------------------------------------------------------------------------------------------
void CreateNewTemplateAction::perform ()
{
	IDependency::DeferChanges dc (description);
	UIAttributes* attr = new UIAttributes ();
	attr->setAttribute (UIViewCreator::kAttrClass, baseViewClassName);
	attr->setAttribute ("size", "400,400");
	description->addNewTemplate (name.c_str (), attr);
	if (view == 0)
		view = description->createView (name.c_str (), description->getController ());
	actionPerformer->onTemplateCreation (name.c_str (), view);
}

//----------------------------------------------------------------------------------------------------
void CreateNewTemplateAction::undo ()
{
	description->removeTemplate (name.c_str ());
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
DuplicateTemplateAction::DuplicateTemplateAction (UIDescription* description, IActionPerformer* actionPerformer, UTF8StringPtr name, UTF8StringPtr dupName)
: description (description)
, actionPerformer (actionPerformer)
, name (name)
, dupName (dupName)
{
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr DuplicateTemplateAction::getName ()
{
	return "Duplicate Template";
}

//----------------------------------------------------------------------------------------------------
void DuplicateTemplateAction::perform ()
{
	IDependency::DeferChanges dc (description);
	description->duplicateTemplate (name.c_str (), dupName.c_str ());
	if (view == 0)
		view = description->createView (dupName.c_str (), description->getController ());
	actionPerformer->onTemplateCreation (dupName.c_str (), view);
}

//----------------------------------------------------------------------------------------------------
void DuplicateTemplateAction::undo ()
{
	description->removeTemplate (dupName.c_str ());
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
DeleteTemplateAction::DeleteTemplateAction (UIDescription* description, IActionPerformer* actionPerformer, CView* view, UTF8StringPtr name)
: description (description)
, actionPerformer (actionPerformer)
, view (view)
, name (name)
{
	attributes = const_cast<UIAttributes*> (description->getViewAttributes (name));
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr DeleteTemplateAction::getName ()
{
	return "Delete Template";
}

//----------------------------------------------------------------------------------------------------
void DeleteTemplateAction::perform ()
{
	attributes->remember ();
	description->removeTemplate (name.c_str ());
}

//----------------------------------------------------------------------------------------------------
void DeleteTemplateAction::undo ()
{
	IDependency::DeferChanges dc (description);
	description->addNewTemplate (name.c_str (), attributes);
	actionPerformer->onTemplateCreation (name.c_str (), view);
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
