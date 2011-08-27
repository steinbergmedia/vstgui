//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins :
//
// Version 4.1
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2011, Steinberg Media Technologies, All Rights Reserved
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

#include "uiactions.h"

#if VSTGUI_LIVE_EDITING

#include "uieditview.h"
#include "../uiviewfactory.h"
#include "../uidescription.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
SizeToFitOperation::SizeToFitOperation (UISelection* selection)
: selection (selection)
{
	selection->remember ();
	FOREACH_IN_SELECTION(selection, view)
		push_back (view);
		view->remember ();
		sizes.push_back (view->getViewSize ());
	FOREACH_IN_SELECTION_END
}

//----------------------------------------------------------------------------------------------------
SizeToFitOperation::~SizeToFitOperation ()
{
	iterator it = begin ();
	while (it != end ())
	{
		(*it)->forget ();
		it++;
	}
	selection->forget ();
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr SizeToFitOperation::getName ()
{
	return "size to fit";
}

//----------------------------------------------------------------------------------------------------
void SizeToFitOperation::perform ()
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

//----------------------------------------------------------------------------------------------------
void SizeToFitOperation::undo ()
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

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UnembedViewOperation::UnembedViewOperation (UISelection* selection, UIViewFactory* factory)
: selection (selection)
, factory (factory)
{
	containerView = dynamic_cast<CViewContainer*> (selection->first ());
	ViewIterator it (containerView);
	while (*it)
	{
		if (factory->getViewName (*it))
		{
			push_back (*it);
			(*it)->remember ();
		}
		++it;
	}
	containerView->remember ();
	parent = dynamic_cast<CViewContainer*> (containerView->getParentView ());
}

//----------------------------------------------------------------------------------------------------
UnembedViewOperation::~UnembedViewOperation ()
{
	const_iterator it = begin ();
	while (it != end ())
	{
		(*it)->forget ();
		it++;
	}
	containerView->forget ();
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr UnembedViewOperation::getName ()
{
	return "unembed views";
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
: selection (selection)
, newContainer (newContainer)
{
	selection->remember ();
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

//-----------------------------------------------------------------------------
EmbedViewOperation::~EmbedViewOperation ()
{
	const_iterator it = begin ();
	while (it != end ())
	{
		(*it)->forget ();
		it++;
	}
	newContainer->forget ();
	selection->forget ();
}

//-----------------------------------------------------------------------------
UTF8StringPtr EmbedViewOperation::getName ()
{
	return "embed views";
}

//-----------------------------------------------------------------------------
void EmbedViewOperation::perform ()
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
	selection->changed (UISelection::kMsgSelectionViewChanged);
	selection->changed (UISelection::kMsgSelectionChanged);
}

//-----------------------------------------------------------------------------
void EmbedViewOperation::undo ()
{
	CRect parentRect = newContainer->getViewSize ();
	const_reverse_iterator it = rbegin ();
	while (it != rend ())
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
	selection->changed (UISelection::kMsgSelectionViewChanged);
	selection->changed (UISelection::kMsgSelectionChanged);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ViewCopyOperation::ViewCopyOperation (UISelection* copySelection, UISelection* workingSelection, CViewContainer* parent, const CPoint& offset, UIViewFactory* viewFactory, IUIDescription* desc)
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
			CRect viewSize = UISelection::getGlobalViewCoordinates (view);
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

//-----------------------------------------------------------------------------
ViewCopyOperation::~ViewCopyOperation ()
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

//-----------------------------------------------------------------------------
UTF8StringPtr ViewCopyOperation::getName () 
{
	if (size () > 0)
		return "copy views";
	return "copy view";
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
	const_iterator it = begin ();
	while (it != end ())
	{
		(*it)->invalid ();
		parent->removeView (*it, true);
		it++;
	}
	workingSelection->empty ();
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
ViewSizeChangeOperation::ViewSizeChangeOperation (UISelection* selection, bool sizing)
: first (true)
, sizing (sizing)
, selection (selection)
{
	selection->remember ();
	FOREACH_IN_SELECTION(selection, view)
		insert (std::make_pair (view, view->getViewSize ()));
		view->remember ();
	FOREACH_IN_SELECTION_END
}

//-----------------------------------------------------------------------------
ViewSizeChangeOperation::~ViewSizeChangeOperation ()
{
	const_iterator it = begin ();
	while (it != end ())
	{
		(*it).first->forget ();
		it++;
	}
	selection->forget ();
}

//-----------------------------------------------------------------------------
UTF8StringPtr ViewSizeChangeOperation::getName ()
{
	if (size () > 1)
		return sizing ? "resize views" : "move views";
	return sizing ? "resize view" : "move view";
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
	selection->changed (UISelection::kMsgSelectionWillChange);
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
	selection->changed (UISelection::kMsgSelectionChanged);
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
DeleteOperation::DeleteOperation (UISelection* selection)
: selection (selection)
{
	selection->remember ();
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
			container->remember ();
			view->remember ();
			if (nextView)
				nextView->remember ();
		}
	FOREACH_IN_SELECTION_END
}

//----------------------------------------------------------------------------------------------------
DeleteOperation::~DeleteOperation ()
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

//----------------------------------------------------------------------------------------------------
UTF8StringPtr DeleteOperation::getName ()
{
	if (size () > 1)
		return "delete views";
	return "delete view";
}

//----------------------------------------------------------------------------------------------------
void DeleteOperation::perform ()
{
	const_iterator it = begin ();
	while (it != end ())
	{
		(*it).first->removeView ((*it).second->view);
		it++;
	}
	selection->empty ();
}

//----------------------------------------------------------------------------------------------------
void DeleteOperation::undo ()
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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
InsertViewOperation::InsertViewOperation (CViewContainer* parent, CView* view, UISelection* selection)
: parent (parent)
, view (view)
, selection (selection)
{
	parent->remember ();
	view->remember ();
	selection->remember ();
}

//-----------------------------------------------------------------------------
InsertViewOperation::~InsertViewOperation ()
{
	parent->forget ();
	view->forget ();
	selection->forget ();
}

//-----------------------------------------------------------------------------
UTF8StringPtr InsertViewOperation::getName ()
{
	return "insert new subview";
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
TransformViewTypeOperation::TransformViewTypeOperation (UISelection* selection, IdStringPtr viewClassName, IUIDescription* desc, UIViewFactory* factory)
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
	view->remember ();
}

//-----------------------------------------------------------------------------
TransformViewTypeOperation::~TransformViewTypeOperation ()
{
	view->forget ();
	if (newView)
		newView->forget ();
}

//-----------------------------------------------------------------------------
UTF8StringPtr TransformViewTypeOperation::getName ()
{
	return "transform view type";
}

//-----------------------------------------------------------------------------
void TransformViewTypeOperation::exchangeSubViews (CViewContainer* src, CViewContainer* dst)
{
	if (src && dst)
	{
		ReverseViewIterator it (src);
		while (*it)
		{
			CView* view = *it;
			++it;
			src->removeView (view, false);
			dst->addView (view, dst->getView (0));
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
		exchangeSubViews (dynamic_cast<CViewContainer*> (view), dynamic_cast<CViewContainer*> (newView));
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
		exchangeSubViews (dynamic_cast<CViewContainer*> (newView), dynamic_cast<CViewContainer*> (view));
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
	UIViewFactory* viewFactory = dynamic_cast<UIViewFactory*> (desc->getViewFactory ());
	std::string attrOldValue;
	FOREACH_IN_SELECTION(selection, view)
		viewFactory->getAttributeValue (view, attrName, attrOldValue, desc);
		insert (std::make_pair (view, attrOldValue));
		view->remember ();
	FOREACH_IN_SELECTION_END
	name = "'" + attrName + "' change";
	selection->remember ();
}

//-----------------------------------------------------------------------------
AttributeChangeAction::~AttributeChangeAction ()
{
	const_iterator it = begin ();
	while (it != end ())
	{
		(*it).first->forget ();
		it++;
	}
	selection->forget ();
}

//-----------------------------------------------------------------------------
UTF8StringPtr AttributeChangeAction::getName ()
{
	return name.c_str ();
}

//-----------------------------------------------------------------------------
void AttributeChangeAction::perform ()
{
	UIViewFactory* viewFactory = dynamic_cast<UIViewFactory*> (desc->getViewFactory ());
	UIAttributes attr;
	attr.setAttribute (attrName.c_str (), attrValue.c_str ());
	const_iterator it = begin ();
	while (it != end ())
	{
		(*it).first->invalid ();	// we need to invalid before changing anything as the size may change
		viewFactory->applyAttributeValues ((*it).first, attr, desc);
		(*it).first->invalid ();	// and afterwards also
		it++;
	}
	selection->changed (UISelection::kMsgSelectionViewChanged);
}

//-----------------------------------------------------------------------------
void AttributeChangeAction::undo ()
{
	UIViewFactory* viewFactory = dynamic_cast<UIViewFactory*> (desc->getViewFactory ());
	const_iterator it = begin ();
	while (it != end ())
	{
		UIAttributes attr;
		attr.setAttribute (attrName.c_str (), (*it).second.c_str ());
		(*it).first->invalid ();	// we need to invalid before changing anything as the size may change
		viewFactory->applyAttributeValues ((*it).first, attr, desc);
		(*it).first->invalid ();	// and afterwards also
		it++;
	}
	selection->changed (UISelection::kMsgSelectionViewChanged);
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
MultipleAttributeChangeAction::MultipleAttributeChangeAction (UIDescription* description, CView* baseView, IViewCreator::AttrType attrType, UTF8StringPtr oldValue, UTF8StringPtr newValue)
: description (description)
, baseView (baseView)
, oldValue (oldValue)
, newValue (newValue)
{
	UIViewFactory* viewFactory = dynamic_cast<UIViewFactory*>(description->getViewFactory ());
	collectViewsWithAttributeValue (viewFactory, description, baseView, attrType, oldValue);
}

//----------------------------------------------------------------------------------------------------
void MultipleAttributeChangeAction::collectViewsWithAttributeValue (UIViewFactory* viewFactory, IUIDescription* desc, CView* startView, IViewCreator::AttrType type, const std::string& value)
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
							insert (std::make_pair (view, (*namesIt)));
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
	UIViewFactory* viewFactory = dynamic_cast<UIViewFactory*>(description->getViewFactory ());
	const_iterator it = begin ();
	while (it != end ())
	{
		CView* view = (*it).first;
		UIAttributes newAttr;
		newAttr.setAttribute ((*it).second.c_str (), value);
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
TagChangeAction::TagChangeAction (UIDescription* description, UTF8StringPtr name, int32_t tag, bool remove, bool performOrUndo)
: description(description)
, name (name)
, tag (tag)
, remove (remove)
, performOrUndo (performOrUndo)
, isNewTag (!description->hasTagName (name))
{
	originalTag = description->getTagForName (name);
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr TagChangeAction::getName ()
{
	return "";
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
			description->changeTag (name.c_str (), tag);
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
			description->changeTag (name.c_str (), originalTag);
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
	return "Change Tag Name";
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
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
BitmapChangeAction::BitmapChangeAction (UIDescription* description, UTF8StringPtr name, UTF8StringPtr path, bool remove, bool performOrUndo)
: description(description)
, name (name)
, path (path ? path : "")
, remove (remove)
, performOrUndo (performOrUndo)
{
	CBitmap* bitmap = description->getBitmap (name);
	if (bitmap)
		originalPath = bitmap->getResourceDescription().u.name;
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr BitmapChangeAction::getName ()
{
	return "";
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
		description->changeBitmap (name.c_str (), originalPath.c_str ());
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
FontChangeAction::FontChangeAction (UIDescription* description, UTF8StringPtr name, CFontRef font, bool remove, bool performOrUndo)
: description(description)
, name (name)
, font (font)
, remove (remove)
, performOrUndo (performOrUndo)
{
	originalFont = description->getFont (name);
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr FontChangeAction::getName ()
{
	return "";
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
			description->changeFont (name.c_str (), originalFont);
		else
			description->removeFont(name.c_str ());
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
	return "change view hierarchy";
}

//----------------------------------------------------------------------------------------------------
void HierarchyMoveViewOperation::perform ()
{
	if (!parent)
		return;
	int32_t currentIndex = 0;
	ViewIterator it (parent);
	while (*it && *it != view)
	{
		it++;
		currentIndex++;
	}
	parent->changeViewZOrder (view, up ? currentIndex - 1 : currentIndex + 1);
	if (selection)
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

} // namespace

#endif // VSTGUI_LIVE_EDITING
