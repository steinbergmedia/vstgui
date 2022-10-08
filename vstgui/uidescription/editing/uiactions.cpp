// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uiactions.h"

#if VSTGUI_LIVE_EDITING

#include "uieditview.h"
#include "../uiviewfactory.h"
#include "../uidescription.h"
#include "../uiattributes.h"
#include "../../lib/cgraphicspath.h"
#include "../../lib/cbitmap.h"
#include "../detail/uiviewcreatorattributes.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
SizeToFitOperation::SizeToFitOperation (UISelection* selection)
: BaseSelectionOperation<std::pair<SharedPointer<CView>, CRect> > (selection)
{
	for (auto view : *selection)
		emplace_back (view, view->getViewSize ());
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr SizeToFitOperation::getName ()
{
	return "Size To Fit";
}

//----------------------------------------------------------------------------------------------------
void SizeToFitOperation::perform ()
{
	selection->viewsWillChange ();
	for (auto& element : *this)
	{
		element.first->invalid ();
		element.first->sizeToFit ();
		element.first->invalid ();
	}
	selection->viewsDidChange ();
}

//----------------------------------------------------------------------------------------------------
void SizeToFitOperation::undo ()
{
	selection->viewsWillChange ();
	for (auto& element : *this)
	{
		element.first->invalid ();
		element.first->setViewSize (element.second);
		element.first->setMouseableArea (element.second);
		element.first->invalid ();
	}
	selection->viewsDidChange ();
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UnembedViewOperation::UnembedViewOperation (UISelection* selection, const IViewFactory* factory)
: BaseSelectionOperation<SharedPointer<CView> > (selection)
, factory (static_cast<const UIViewFactory*> (factory))
{
	containerView = selection->first ()->asViewContainer ();
	collectSubviews (containerView, true);
	parent = containerView->getParentView ()->asViewContainer ();
}

//----------------------------------------------------------------------------------------------------
void UnembedViewOperation::collectSubviews (CViewContainer* container, bool deep)
{
	container->forEachChild ([&] (CView* view) {
		if (factory->getViewName (view))
		{
			emplace_back (view);
		}
		else if (deep)
		{
			if (auto c = view->asViewContainer ())
				collectSubviews (c, false);
		}
	});
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr UnembedViewOperation::getName ()
{
	return "Unembed Views";
}

//----------------------------------------------------------------------------------------------------
void UnembedViewOperation::perform ()
{
	UISelection::DeferChange dc (*selection);
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
	for (auto& view : *this)
	{
		parent->removeView (view, false);
		CRect viewSize = view->getViewSize ();
		CRect mouseSize = view->getMouseableArea ();
		viewSize.offset (-containerViewSize.left, -containerViewSize.top);
		mouseSize.offset (-containerViewSize.left, -containerViewSize.top);
		view->setViewSize (viewSize);
		view->setMouseableArea (mouseSize);
		containerView->addView (view);
	}
	parent->addView (containerView);
	selection->setExclusive (containerView);
}

//-----------------------------------------------------------------------------
EmbedViewOperation::EmbedViewOperation (UISelection* selection, CViewContainer* newContainer)
: BaseSelectionOperation<std::pair<SharedPointer<CView>, CRect> > (selection)
, newContainer (owned (newContainer))
{
	parent = selection->first ()->getParentView ()->asViewContainer ();
	for (auto view : *selection)
	{
		if (view->getParentView () == parent)
		{
			emplace_back (view, view->getViewSize ());
		}
	}

	CRect r = selection->first ()->getViewSize ();
	for (auto& element : *this)
	{
		CView* view = element.first;
		CRect viewSize = view->getViewSize ();
		if (viewSize.left < r.left)
			r.left = viewSize.left;
		if (viewSize.right > r.right)
			r.right = viewSize.right;
		if (viewSize.top < r.top)
			r.top = viewSize.top;
		if (viewSize.bottom > r.bottom)
			r.bottom = viewSize.bottom;
	}
	r.extend (10, 10);
	newContainer->setViewSize (r);
	newContainer->setMouseableArea (r);
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
	for (auto& element : *this)
	{
		CView* view = element.first;
		parent->removeView (view, false);
		CRect r = view->getViewSize ();
		r.offset (-parentRect.left, -parentRect.top);
		view->setViewSize (r);
		view->setMouseableArea (r);
		newContainer->addView (view);
	}
	parent->addView (newContainer);
	newContainer->remember ();
	selection->setExclusive (newContainer);
}

//-----------------------------------------------------------------------------
void EmbedViewOperation::undo ()
{
	selection->clear ();
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
ViewCopyOperation::ViewCopyOperation (UISelection* copySelection, UISelection* workingSelection,
                                      CViewContainer* parent, const CPoint& offset,
                                      IUIDescription* desc)
: parent (parent), copySelection (copySelection), workingSelection (workingSelection)
{
	CRect selectionBounds = copySelection->getBounds ();
	for (auto view : *copySelection)
	{
		if (!copySelection->containsParent (view))
		{
			CRect viewSize = UISelection::getGlobalViewCoordinates (view);
			CRect newSize (0, 0, view->getWidth (), view->getHeight ());
			newSize.offset (offset.x, offset.y);
			newSize.offset (viewSize.left - selectionBounds.left, viewSize.top - selectionBounds.top);

			view->setViewSize (newSize);
			view->setMouseableArea (newSize);
			emplace_back (view);
		}
	}

	for (auto view : *workingSelection)
		oldSelectedViews.emplace_back (view);
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
	workingSelection->clear ();
	for (auto& view : *this)
	{
		parent->addView (view);
		(view)->remember ();
		(view)->invalid ();
		workingSelection->add (view);
	}
}

//-----------------------------------------------------------------------------
void ViewCopyOperation::undo ()
{
	workingSelection->clear ();
	for (auto& view : *this)
	{
		view->invalid ();
		parent->removeView (view, true);
	}
	for (auto& view : oldSelectedViews)
	{
		workingSelection->add (view);
		view->invalid ();
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
	for (auto view : *selection)
		emplace_back (view, view->getViewSize ());
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
	selection->clear ();
	for (auto& element : *this)
	{
		CView* view = element.first;
		CRect size (element.second);
		view->invalid ();
		element.second = view->getViewSize ();
		CViewContainer* container = nullptr;
		bool oldAutosizing = false;
		if (!autosizing)
		{
			container = view->asViewContainer ();
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
	}
}

//-----------------------------------------------------------------------------
bool ViewSizeChangeOperation::didChange ()
{
	auto result = false;
	for (auto& element : *this)
	{
		if (element.second != element.first->getViewSize ())
			result = true;
	}
	return result;
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
DeleteOperation::DeleteOperation (UISelection* selection)
: selection (selection)
{
	for (auto view : *selection)
	{
		CViewContainer* container = view->getParentView ()->asViewContainer ();
		if (dynamic_cast<UIEditView*>(container) == nullptr)
		{
			CView* nextView = nullptr;
			ViewIterator it (container);
			while (*it)
			{
				if (*it == view)
				{
					while (*it && selection->contains (*it))
					{
						++it;
					}
					nextView = *it;
					break;
				}
				++it;
			}
			insert (std::make_pair (container, DeleteOperationViewAndNext (view, nextView)));
		}
	}
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
	selection->clear ();
	for (auto& element : *this)
		element.first->removeView (element.second.view);
}

//----------------------------------------------------------------------------------------------------
void DeleteOperation::undo ()
{
	selection->clear ();
	UISelection::DeferChange dc (*selection);
	for (auto& element : *this)
	{
		if (element.second.nextView)
			element.first->addView (element.second.view, element.second.nextView);
		else
			element.first->addView (element.second.view);
		element.second.view->remember ();
		selection->add (element.second.view);
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
TransformViewTypeOperation::TransformViewTypeOperation (UISelection* selection, CView* view,
                                                        IdStringPtr viewClassName,
                                                        UIDescription* desc,
                                                        const UIViewFactory* factory)
: view (view)
, newView (nullptr)
, insertIndex (-1)
, parent (view->getParentView ()->asViewContainer ())
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
			++insertIndex;
			if (*it == view)
				break;
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

		src->forEachChild ([&] (CView* childView) {
			if (factory->getViewName (childView))
			{
				temp.emplace_back (childView);
			}
			else if (auto container = childView->asViewContainer ())
			{
				exchangeSubViews (container, dst);
			}
		});
		for (auto& viewToMove : temp)
		{
			src->removeView (viewToMove, false);
			dst->addView (viewToMove);
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
		parent->addView (newView);
		if (insertIndex >= 0)
			parent->changeViewZOrder (newView, static_cast<uint32_t> (insertIndex));
		exchangeSubViews (view->asViewContainer (), newView->asViewContainer ());
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
		parent->addView (view);
		if (insertIndex >= 0)
			parent->changeViewZOrder (view, static_cast<uint32_t> (insertIndex));
		exchangeSubViews (newView->asViewContainer (), view->asViewContainer ());
		selection->setExclusive (view);
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
AttributeChangeAction::AttributeChangeAction (UIDescription* desc, UISelection* selection, const std::string& attrName, const std::string& attrValue)
: desc (desc)
, selection (selection)
, attrName (attrName)
, attrValue (attrValue)
{
	const UIViewFactory* viewFactory = dynamic_cast<const UIViewFactory*> (desc->getViewFactory ());
	std::string attrOldValue;
	for (auto view : *selection)
	{
		viewFactory->getAttributeValue (view, attrName, attrOldValue, desc);
		insert (std::make_pair (view, attrOldValue));
	}
	name = "'" + attrName + "' change";
}

//-----------------------------------------------------------------------------
UTF8StringPtr AttributeChangeAction::getName ()
{
	return name.c_str ();
}

//-----------------------------------------------------------------------------
void AttributeChangeAction::updateSelection ()
{
	for (auto& element : *this)
	{
		if (selection->contains (element.first) == false)
		{
			UISelection::DeferChange dc (*selection);
			selection->clear ();
			for (auto& it2 : *this)
				selection->add (it2.first);
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
	selection->viewsWillChange ();
	for (auto& element : *this)
	{
		element.first->invalid ();	// we need to invalid before changing anything as the size may change
		viewFactory->applyAttributeValues (element.first, attr, desc);
		element.first->invalid ();	// and afterwards also
	}
	selection->viewsDidChange ();
	updateSelection ();
}

//-----------------------------------------------------------------------------
void AttributeChangeAction::undo ()
{
	const IViewFactory* viewFactory = desc->getViewFactory ();
	selection->viewsWillChange ();
	for (auto& element : *this)
	{
		UIAttributes attr;
		attr.setAttribute (attrName, element.second);
		element.first->invalid ();	// we need to invalid before changing anything as the size may change
		viewFactory->applyAttributeValues (element.first, attr, desc);
		element.first->invalid ();	// and afterwards also
	}
	selection->viewsDidChange ();
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
	for (auto& view : views)
		collectViewsWithAttributeValue (viewFactory, description, view, attrType, oldValue);
}

//----------------------------------------------------------------------------------------------------
void MultipleAttributeChangeAction::collectViewsWithAttributeValue (const UIViewFactory* viewFactory, IUIDescription* desc, CView* startView, IViewCreator::AttrType type, const std::string& value)
{
	std::list<CView*> views;
	collectAllSubViews (startView, views);
	for (auto& view : views)
	{
		std::list<std::string> attrNames;
		if (viewFactory->getAttributeNamesForView (view, attrNames))
		{
			for (auto& attrName : attrNames)
			{
				if (viewFactory->getAttributeType (view, attrName) == type)
				{
					std::string typeValue;
					if (viewFactory->getAttributeValue (view, attrName, typeValue, desc))
					{
						if (typeValue == value)
						{
							emplace_back (view, attrName);
						}
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------
void MultipleAttributeChangeAction::collectAllSubViews (CView* view, std::list<CView*>& views)
{
	views.emplace_back (view);
	if (auto container = view->asViewContainer ())
	{
		container->forEachChild ([&] (CView* view) {
			collectAllSubViews (view, views);
		});
	}
}

//----------------------------------------------------------------------------------------------------
void MultipleAttributeChangeAction::setAttributeValue (UTF8StringPtr value)
{
	const IViewFactory* viewFactory = description->getViewFactory ();
	for (auto& element : *this)
	{
		CView* view = element.first;
		UIAttributes newAttr;
		newAttr.setAttribute (element.second, value);
		viewFactory->applyAttributeValues (view, newAttr, description);
		view->invalid ();
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
			description->changeControlTagString (name.c_str (), originalTag, remove);
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
, oldRect (nullptr)
, newRect (nullptr)
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
MultiFrameBitmapChangeAction::MultiFrameBitmapChangeAction (
	UIDescription* description, UTF8StringPtr name, const CMultiFrameBitmapDescription* desc,
	bool performOrUndo)
: description (description), name (name), performOrUndo (performOrUndo)
{
	if (desc)
		newDesc = std::make_unique<CMultiFrameBitmapDescription> (*desc);
	CBitmap* bitmap = description->getBitmap (name);
	if (bitmap)
	{
		if (auto mfb = dynamic_cast<CMultiFrameBitmap*> (bitmap))
		{
			oldDesc = std::make_unique<CMultiFrameBitmapDescription> ();
			oldDesc->frameSize = mfb->getFrameSize ();
			oldDesc->numFrames = mfb->getNumFrames ();
			oldDesc->framesPerRow = mfb->getNumFramesPerRow ();
		}
	}
}

//----------------------------------------------------------------------------------------------------
MultiFrameBitmapChangeAction::~MultiFrameBitmapChangeAction () {}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr MultiFrameBitmapChangeAction::getName () { return "Change MultiFrameBitmap"; }

//----------------------------------------------------------------------------------------------------
void MultiFrameBitmapChangeAction::perform ()
{
	if (performOrUndo)
	{
		CBitmap* bitmap = description->getBitmap (name.data ());
		if (bitmap)
			description->changeMultiFrameBitmap (
				name.data (), bitmap->getResourceDescription ().u.name, newDesc.get ());
	}
}

//----------------------------------------------------------------------------------------------------
void MultiFrameBitmapChangeAction::undo ()
{
	if (performOrUndo == false)
	{
		CBitmap* bitmap = description->getBitmap (name.data ());
		if (bitmap)
			description->changeMultiFrameBitmap (
				name.data (), bitmap->getResourceDescription ().u.name, oldDesc.get ());
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
HierarchyMoveViewOperation::HierarchyMoveViewOperation (CView* view, UISelection* selection, int32_t dir)
: view (view)
, parent (nullptr)
, selection (selection)
, dir (dir)
{
	parent = view->getParentView ()->asViewContainer ();
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
		++it;
		currentIndex++;
	}
	selection->willChange ();
	parent->changeViewZOrder (view, currentIndex + dir);
	selection->didChange ();
	parent->invalid ();
}

//----------------------------------------------------------------------------------------------------
void HierarchyMoveViewOperation::undo ()
{
	dir = -dir;
	perform ();
	dir = -dir;
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
	auto attr = makeOwned<UIAttributes> ();
	attr->setAttribute (UIViewCreator::kAttrClass, baseViewClassName);
	attr->setAttribute ("size", "400,400");
	description->addNewTemplate (name.c_str (), attr);
	if (view == nullptr)
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
	description->duplicateTemplate (name.c_str (), dupName.c_str ());
	if (view == nullptr)
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
	actionPerformer->onTemplateCreation (name.c_str (), view);
	description->addNewTemplate (name.c_str (), attributes);
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
ChangeFocusDrawingAction::ChangeFocusDrawingAction (UIDescription* description, const FocusDrawingSettings& newSettings)
: description (description)
, newSettings (newSettings)
{
	oldSettings = description->getFocusDrawingSettings ();
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr ChangeFocusDrawingAction::getName ()
{
	return "Change Focus Drawing Settings";
}

//----------------------------------------------------------------------------------------------------
void ChangeFocusDrawingAction::perform ()
{
	description->setFocusDrawingSettings (newSettings);
}

//----------------------------------------------------------------------------------------------------
void ChangeFocusDrawingAction::undo ()
{
	description->setFocusDrawingSettings (oldSettings);
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
ChangeTemplateMinMaxAction::ChangeTemplateMinMaxAction (UIDescription* description, UTF8StringPtr templateName, CPoint minSize, CPoint maxSize)
: description (description)
, templateName (templateName)
, minSize (minSize)
, maxSize (maxSize)
{
	if (auto attr = description->getViewAttributes (templateName))
	{
		CPoint p;
		if (attr->getPointAttribute (kTemplateAttributeMinSize, p))
			oldMinSize = p;
		else
			oldMinSize = {-1, -1};
		if (attr->getPointAttribute (kTemplateAttributeMaxSize, p))
			oldMaxSize = p;
		else
			oldMaxSize = {-1, -1};
	}
}

//----------------------------------------------------------------------------------------------------
void ChangeTemplateMinMaxAction::setMinMaxSize (CPoint minimum, CPoint maximum)
{
	if (auto attr = const_cast<UIAttributes*> (description->getViewAttributes (templateName.data ())))
	{
		if (minimum.x == -1. && minimum.y == -1.)
		{
			attr->removeAttribute (kTemplateAttributeMinSize);
		}
		else
		{
			attr->setPointAttribute (kTemplateAttributeMinSize, minimum);
		}
		if (maximum.x == -1. && maximum.y == -1.)
		{
			attr->removeAttribute (kTemplateAttributeMaxSize);
		}
		else
		{
			attr->setPointAttribute (kTemplateAttributeMaxSize, maximum);
		}
	}

}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr ChangeTemplateMinMaxAction::getName ()
{
	return "Change Template Min/Max Sizes";
}

//----------------------------------------------------------------------------------------------------
void ChangeTemplateMinMaxAction::perform ()
{
	setMinMaxSize (minSize, maxSize);
}

//----------------------------------------------------------------------------------------------------
void ChangeTemplateMinMaxAction::undo ()
{
	setMinMaxSize (oldMinSize, oldMaxSize);
}

//----------------------------------------------------------------------------------------------------
} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
