// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uiactions.h"

#if VSTGUI_LIVE_EDITING

#include "uieditview.h"
#include "../uidescription.h"
#include "../uiattributes.h"
#include "../../lib/cgraphicspath.h"
#include "../../lib/cbitmap.h"
#include "../detail/uiviewcreatorattributes.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
SizeToFitOperation::SizeToFitOperation (const SharedPointer<UISelection>& selection)
: BaseSelectionOperation<std::pair<SharedPointer<CView>, CRect>> (selection)
{
	for (auto view : *selection.get ())
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
UnembedViewOperation::UnembedViewOperation (const SharedPointer<UISelection>& selection,
											const IViewFactory& factory)
: BaseSelectionOperation<SharedPointer<CView>> (selection), factory (factory)
{
	containerView = shared (selection->first ()->asViewContainer ());
	collectSubviews (*containerView.get (), true);
	parent = shared (containerView->getParentView ()->asViewContainer ());
}

//----------------------------------------------------------------------------------------------------
void UnembedViewOperation::collectSubviews (CViewContainer& container, bool deep)
{
	container.forEachChild ([&] (auto& view) {
		if (factory.getViewName (*view.get ()))
		{
			emplace_back (view);
		}
		else if (deep)
		{
			if (auto c = view->asViewContainer ())
				collectSubviews (*c, false);
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
	UISelection::DeferChange dc (*selection.get ());
	selection->remove (containerView);
	CRect containerViewSize = containerView->getViewSize ();
	const_reverse_iterator it = rbegin ();
	while (it != rend ())
	{
		auto view = (*it);
		CRect viewSize = view->getViewSize ();
		CRect mouseSize = view->getMouseableArea ();
		containerView->removeSubview (view);
		viewSize.offset (containerViewSize.left, containerViewSize.top);
		mouseSize.offset (containerViewSize.left, containerViewSize.top);
		view->setViewSize (viewSize);
		view->setMouseableArea (mouseSize);
		if (parent->addSubview (view))
			selection->add (view);
		it++;
	}
	parent->removeSubview (containerView);
}

//----------------------------------------------------------------------------------------------------
void UnembedViewOperation::undo ()
{
	CRect containerViewSize = containerView->getViewSize ();
	for (auto& view : *this)
	{
		parent->removeSubview (view);
		CRect viewSize = view->getViewSize ();
		CRect mouseSize = view->getMouseableArea ();
		viewSize.offset (-containerViewSize.left, -containerViewSize.top);
		mouseSize.offset (-containerViewSize.left, -containerViewSize.top);
		view->setViewSize (viewSize);
		view->setMouseableArea (mouseSize);
		containerView->addSubview (view);
	}
	parent->addSubview (containerView);
	selection->setExclusive (containerView);
}

//-----------------------------------------------------------------------------
EmbedViewOperation::EmbedViewOperation (const SharedPointer<UISelection>& selection,
										const SharedPointer<CViewContainer>& newContainer)
: BaseSelectionOperation<std::pair<SharedPointer<CView>, CRect>> (selection)
, newContainer (newContainer)
{
	parent = shared (selection->first ()->getParentView ()->asViewContainer ());
	for (auto view : *selection.get ())
	{
		if (view->getParentView () == parent.get ())
		{
			emplace_back (view, view->getViewSize ());
		}
	}

	CRect r = selection->first ()->getViewSize ();
	for (auto& element : *this)
	{
		auto view = element.first;
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
		auto view = element.first;
		parent->removeSubview (view);
		CRect r = view->getViewSize ();
		r.offset (-parentRect.left, -parentRect.top);
		view->setViewSize (r);
		view->setMouseableArea (r);
		newContainer->addSubview (view);
	}
	parent->addSubview (newContainer);
	selection->setExclusive (newContainer);
}

//-----------------------------------------------------------------------------
void EmbedViewOperation::undo ()
{
	selection->clear ();
	const_reverse_iterator it = rbegin ();
	while (it != rend ())
	{
		auto view = (*it).first;
		newContainer->removeSubview (view);
		CRect r = (*it).second;
		view->setViewSize (r);
		view->setMouseableArea (r);
		parent->addSubview (view);
		selection->add (view);
		it++;
	}
	parent->removeSubview (newContainer);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ViewCopyOperation::ViewCopyOperation (const SharedPointer<UISelection>& copySelection,
									  const SharedPointer<UISelection>& workingSelection,
									  const SharedPointer<CViewContainer>& parent,
									  const CPoint& offset,
									  const SharedPointer<IUIDescription>& desc)
: parent (parent), copySelection (copySelection), workingSelection (workingSelection)
{
	CRect selectionBounds = copySelection->getBounds ();
	for (auto view : *copySelection.get ())
	{
		if (!copySelection->containsParent (*view.get ()))
		{
			CRect viewSize = UISelection::getGlobalViewCoordinates (*view.get ());
			CRect newSize (0, 0, view->getWidth (), view->getHeight ());
			newSize.offset (offset.x, offset.y);
			newSize.offset (viewSize.left - selectionBounds.left, viewSize.top - selectionBounds.top);

			view->setViewSize (newSize);
			view->setMouseableArea (newSize);
			emplace_back (view);
		}
	}

	for (auto view : *workingSelection.get ())
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
		parent->addSubview (view);
		view->invalid ();
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
		parent->removeSubview (view);
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
ViewSizeChangeOperation::ViewSizeChangeOperation (const SharedPointer<UISelection>& selection,
												  bool sizing, bool autosizingEnabled)
: BaseSelectionOperation<std::pair<SharedPointer<CView>, CRect>> (selection)
, first (true)
, sizing (sizing)
, autosizing (autosizingEnabled)
{
	for (auto view : *selection.get ())
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
		auto view = element.first;
		CRect size (element.second);
		view->invalid ();
		element.second = view->getViewSize ();
		SharedPointer<CViewContainer> container;
		bool oldAutosizing = false;
		if (!autosizing)
		{
			container = shared (view->asViewContainer ());
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
DeleteOperation::DeleteOperation (const SharedPointer<UISelection>& sel) : selection (sel)
{
	for (auto view : *selection.get ())
	{
		auto container = view->getParentView ();
		if (dynamic_cast<UIEditView*> (container) == nullptr)
		{
			SharedPointer<CView> nextView;
			ViewIterator it (*container);
			while (*it)
			{
				if (*it == view)
				{
					while (*it && selection->contains (*(*it).get ()))
					{
						++it;
					}
					nextView = *it;
					break;
				}
				++it;
			}
			map.emplace (shared (container), ViewAndNext {view, nextView});
		}
	}
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr DeleteOperation::getName ()
{
	if (map.size () > 1)
		return "Delete Views";
	return "Delete View";
}

//----------------------------------------------------------------------------------------------------
void DeleteOperation::perform ()
{
	selection->clear ();
	for (auto& element : map)
		element.first->removeSubview (element.second.view);
}

//----------------------------------------------------------------------------------------------------
void DeleteOperation::undo ()
{
	selection->clear ();
	UISelection::DeferChange dc (*selection.get ());
	for (auto& element : map)
	{
		if (element.second.nextView)
		{
			auto pos = element.first->indexOfSubview (element.second.nextView);
			element.first->insertSubview (element.second.view, pos);
		}
		else
			element.first->addSubview (element.second.view);
		selection->add (element.second.view);
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
InsertViewOperation::InsertViewOperation (const SharedPointer<CViewContainer>& parent,
										  const SharedPointer<CView>& view,
										  const SharedPointer<UISelection>& selection)
: parent (parent), view (view), selection (selection)
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
	if (parent->addSubview (view))
		selection->setExclusive (view);
}

//-----------------------------------------------------------------------------
void InsertViewOperation::undo ()
{
	selection->remove (view);
	parent->removeSubview (view);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
TransformViewTypeOperation::TransformViewTypeOperation (const SharedPointer<UISelection>& selection,
														const SharedPointer<CView>& view,
														IdStringPtr viewClassName,
														const SharedPointer<UIDescription>& desc,
														const IViewFactory& factory)
: view (view)
, insertIndex (-1)
, parent (shared (view->getParentView ()->asViewContainer ()))
, selection (selection)
, factory (factory)
, description (desc)
{
	if (const auto* vfEditingSupport = dynamic_cast<const IViewFactoryEditingSupport*> (&factory))
	{
		UIAttributes attr;
		if (vfEditingSupport->getAttributesForView (*view.get (), *desc.get (), attr))
		{
			attr.setAttribute (UIViewCreator::kAttrClass, viewClassName);
			newView = factory.createView (attr, *desc.get ());
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
}

//-----------------------------------------------------------------------------
TransformViewTypeOperation::~TransformViewTypeOperation () {}

//-----------------------------------------------------------------------------
UTF8StringPtr TransformViewTypeOperation::getName ()
{
	return "Transform View Type";
}

//-----------------------------------------------------------------------------
void TransformViewTypeOperation::exchangeSubViews (const SharedPointer<CViewContainer>& src,
												   const SharedPointer<CViewContainer>& dst)
{
	if (dynamic_cast<const IViewFactoryEditingSupport*> (&factory))
	{
		if (src && dst)
		{
			std::list<SharedPointer<CView>> temp;

			src->forEachChild ([&] (auto& childView) {
				if (IViewFactory::getViewName (*childView.get ()))
				{
					temp.emplace_back (childView);
				}
				else if (auto container = childView->asViewContainer ())
				{
					exchangeSubViews (shared (container), dst);
				}
			});
			for (auto& viewToMove : temp)
			{
				src->removeSubview (viewToMove);
				dst->addSubview (viewToMove);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void TransformViewTypeOperation::perform ()
{
	if (newView)
	{
		parent->removeSubview (view);
		parent->addSubview (newView);
		if (insertIndex >= 0)
			parent->changeViewZOrder (newView, static_cast<uint32_t> (insertIndex));
		exchangeSubViews (shared (view->asViewContainer ()), shared (newView->asViewContainer ()));
		selection->setExclusive (newView);
	}
}

//-----------------------------------------------------------------------------
void TransformViewTypeOperation::undo ()
{
	if (newView)
	{
		parent->removeSubview (newView);
		parent->addSubview (view);
		if (insertIndex >= 0)
			parent->changeViewZOrder (view, static_cast<uint32_t> (insertIndex));
		exchangeSubViews (shared (newView->asViewContainer ()), shared (view->asViewContainer ()));
		selection->setExclusive (view);
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
AttributeChangeAction::AttributeChangeAction (const SharedPointer<UIDescription>& desc,
											  const SharedPointer<UISelection>& selection,
											  const std::string& attrName,
											  const std::string& attrValue)
: desc (desc), selection (selection), attrName (attrName), attrValue (attrValue)
{
	const auto& viewFactory = desc->getViewFactory ();
	std::string attrOldValue;
	for (auto view : *selection.get ())
	{
		viewFactory.getAttributeValue (*view.get (), attrName, attrOldValue, *desc.get ());
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
		if (selection->contains (*element.first.get ()) == false)
		{
			UISelection::DeferChange dc (*selection.get ());
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
	const IViewFactory& viewFactory = desc->getViewFactory ();
	UIAttributes attr;
	attr.setAttribute (attrName, attrValue);
	selection->viewsWillChange ();
	for (auto& element : *this)
	{
		element.first->invalid ();	// we need to invalid before changing anything as the size may change
		viewFactory.applyAttributeValues (*element.first.get (), attr, *desc.get ());
		element.first->invalid ();	// and afterwards also
	}
	selection->viewsDidChange ();
	updateSelection ();
}

//-----------------------------------------------------------------------------
void AttributeChangeAction::undo ()
{
	const IViewFactory& viewFactory = desc->getViewFactory ();
	selection->viewsWillChange ();
	for (auto& element : *this)
	{
		UIAttributes attr;
		attr.setAttribute (attrName, element.second);
		element.first->invalid ();	// we need to invalid before changing anything as the size may change
		viewFactory.applyAttributeValues (*element.first.get (), attr, *desc.get ());
		element.first->invalid ();	// and afterwards also
	}
	selection->viewsDidChange ();
	updateSelection ();
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
MultipleAttributeChangeAction::MultipleAttributeChangeAction (
	const SharedPointer<UIDescription>& description, const std::list<SharedPointer<CView>>& views,
	IViewCreator::AttrType attrType, UTF8StringPtr oldValue, UTF8StringPtr newValue)
: description (description), oldValue (oldValue), newValue (newValue)
{
	for (auto& view : views)
		collectViewsWithAttributeValue (description->getViewFactory (), description, view, attrType,
										oldValue);
}

//----------------------------------------------------------------------------------------------------
void MultipleAttributeChangeAction::collectViewsWithAttributeValue (
	const IViewFactory& viewFactory, const SharedPointer<IUIDescription>& desc,
	const SharedPointer<CView>& startView, IViewCreator::AttrType type, const std::string& value)
{
	const auto* viewFactoryEditing = dynamic_cast<const IViewFactoryEditingSupport*> (&viewFactory);
	if (!viewFactoryEditing)
		return;
	std::list<SharedPointer<CView>> views;
	collectAllSubViews (startView, views);
	for (auto& view : views)
	{
		std::list<std::string> attrNames;
		if (viewFactoryEditing->getAttributeNamesForView (*view.get (), attrNames))
		{
			for (auto& attrName : attrNames)
			{
				if (viewFactoryEditing->getAttributeType (*view.get (), attrName) == type)
				{
					std::string typeValue;
					if (viewFactory.getAttributeValue (*view.get (), attrName, typeValue,
													   *desc.get ()))
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
void MultipleAttributeChangeAction::collectAllSubViews (const SharedPointer<CView>& view,
														std::list<SharedPointer<CView>>& views)
{
	views.emplace_back (view);
	if (auto container = view->asViewContainer ())
	{
		container->forEachChild ([&] (auto view) { collectAllSubViews (view, views); });
	}
}

//----------------------------------------------------------------------------------------------------
void MultipleAttributeChangeAction::setAttributeValue (UTF8StringPtr value)
{
	const IViewFactory& viewFactory = description->getViewFactory ();
	for (auto& element : *this)
	{
		auto view = element.first;
		UIAttributes newAttr;
		newAttr.setAttribute (element.second, value);
		viewFactory.applyAttributeValues (*view.get (), newAttr, *description.get ());
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
TagChangeAction::TagChangeAction (const SharedPointer<UIDescription>& description,
								  UTF8StringPtr name, UTF8StringPtr newTagString, bool remove,
								  bool performOrUndo)
: description (description)
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
TagNameChangeAction::TagNameChangeAction (const SharedPointer<UIDescription>& description,
										  UTF8StringPtr oldName, UTF8StringPtr newName,
										  bool performOrUndo)
: description (description), oldName (oldName), newName (newName), performOrUndo (performOrUndo)
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
ColorNameChangeAction::ColorNameChangeAction (const SharedPointer<UIDescription>& description,
											  UTF8StringPtr oldName, UTF8StringPtr newName,
											  bool performOrUndo)
: description (description), oldName (oldName), newName (newName), performOrUndo (performOrUndo)
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
ColorChangeAction::ColorChangeAction (const SharedPointer<UIDescription>& description,
									  UTF8StringPtr name, const CColor& color, bool remove,
									  bool performOrUndo)
: description (description)
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
BitmapChangeAction::BitmapChangeAction (const SharedPointer<UIDescription>& description,
										UTF8StringPtr name, UTF8StringPtr path, bool remove,
										bool performOrUndo)
: description (description)
, name (name)
, path (path ? path : "")
, remove (remove)
, performOrUndo (performOrUndo)
, isNewBitmap (!description->hasBitmapName (name))
{
	auto bitmap = description->getBitmap (name);
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
BitmapNameChangeAction::BitmapNameChangeAction (const SharedPointer<UIDescription>& description,
												UTF8StringPtr oldName, UTF8StringPtr newName,
												bool performOrUndo)
: description (description), oldName (oldName), newName (newName), performOrUndo (performOrUndo)
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
NinePartTiledBitmapChangeAction::NinePartTiledBitmapChangeAction (
	const SharedPointer<UIDescription>& description, UTF8StringPtr name, const CRect* rect,
	bool performOrUndo)
: description (description)
, name (name)
, oldRect (nullptr)
, newRect (nullptr)
, performOrUndo (performOrUndo)
{
	if (rect)
		newRect = std::make_unique<CRect> (*rect);
	auto bitmap = description->getBitmap (name);
	if (bitmap)
	{
		if (auto tiledBitmap = bitmap.cast<CNinePartTiledBitmap> ())
		{
			const CNinePartTiledDescription& offset = tiledBitmap->getPartOffsets ();
			oldRect = std::make_unique<CRect> ();
			oldRect->left = offset.left;
			oldRect->top = offset.top;
			oldRect->right = offset.right;
			oldRect->bottom = offset.bottom;
		}
	}
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
		if (auto bitmap = description->getBitmap (name.data ()))
		{
			description->changeBitmap (name.data (), bitmap->getResourceDescription ().u.name,
									   newRect.get ());
		}
	}
}

//----------------------------------------------------------------------------------------------------
void NinePartTiledBitmapChangeAction::undo ()
{
	if (performOrUndo == false)
	{
		if (auto bitmap = description->getBitmap (name.data ()))
		{
			description->changeBitmap (name.data (), bitmap->getResourceDescription ().u.name,
									   oldRect.get ());
		}
	}
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
MultiFrameBitmapChangeAction::MultiFrameBitmapChangeAction (
	const SharedPointer<UIDescription>& description, UTF8StringPtr name,
	const CMultiFrameBitmapDescription* desc, bool performOrUndo)
: description (description), name (name), performOrUndo (performOrUndo)
{
	if (desc)
		newDesc = std::make_unique<CMultiFrameBitmapDescription> (*desc);
	auto bitmap = description->getBitmap (name);
	if (bitmap)
	{
		if (auto mfb = bitmap.cast<CMultiFrameBitmap> ())
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
		auto bitmap = description->getBitmap (name.data ());
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
		auto bitmap = description->getBitmap (name.data ());
		if (bitmap)
			description->changeMultiFrameBitmap (
				name.data (), bitmap->getResourceDescription ().u.name, oldDesc.get ());
	}
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
BitmapFilterChangeAction::BitmapFilterChangeAction (
	const SharedPointer<UIDescription>& description, UTF8StringPtr bitmapName,
	const std::list<SharedPointer<UIAttributes>>& attributes, bool performOrUndo)
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
GradientChangeAction::GradientChangeAction (const SharedPointer<UIDescription>& description,
											UTF8StringPtr name,
											const SharedPointer<CGradient>& gradient, bool remove,
											bool performOrUndo)
: description (description)
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
GradientNameChangeAction::GradientNameChangeAction (const SharedPointer<UIDescription>& description,
													UTF8StringPtr oldName, UTF8StringPtr newName,
													bool performOrUndo)
: description (description), oldName (oldName), newName (newName), performOrUndo (performOrUndo)
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
FontChangeAction::FontChangeAction (const SharedPointer<UIDescription>& description,
									UTF8StringPtr name, const SharedPointer<CFontDesc>& font,
									bool remove, bool performOrUndo)
: description (description)
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
FontNameChangeAction::FontNameChangeAction (const SharedPointer<UIDescription>& description,
											UTF8StringPtr oldName, UTF8StringPtr newName,
											bool performOrUndo)
: description (description), oldName (oldName), newName (newName), performOrUndo (performOrUndo)
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
AlternateFontChangeAction::AlternateFontChangeAction (
	const SharedPointer<UIDescription>& description, UTF8StringPtr fontName,
	UTF8StringPtr newAlternateFontNames)
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
HierarchyMoveViewOperation::HierarchyMoveViewOperation (const SharedPointer<CView>& view,
														const SharedPointer<UISelection>& selection,
														int32_t dir)
: view (view), selection (selection), dir (dir)
{
	parent = shared (view->getParentView ());
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
TemplateNameChangeAction::TemplateNameChangeAction (const SharedPointer<UIDescription>& description,
													WeakPointer<IActionPerformer> actionPerformer,
													UTF8StringPtr oldName, UTF8StringPtr newName)
: description (description), actionPerformer (actionPerformer), oldName (oldName), newName (newName)
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
	if (auto ap = actionPerformer.lock ())
		ap->onTemplateNameChange (oldName.c_str (), newName.c_str ());
	description->changeTemplateName (oldName.c_str (), newName.c_str ());
}

//----------------------------------------------------------------------------------------------------
void TemplateNameChangeAction::undo ()
{
	if (auto ap = actionPerformer.lock ())
		ap->onTemplateNameChange (newName.c_str (), oldName.c_str ());
	description->changeTemplateName (newName.c_str (), oldName.c_str ());
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
CreateNewTemplateAction::CreateNewTemplateAction (const SharedPointer<UIDescription>& description,
												  WeakPointer<IActionPerformer> actionPerformer,
												  UTF8StringPtr name,
												  UTF8StringPtr baseViewClassName)
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
	auto attr = makeShared<UIAttributes> ();
	attr->setAttribute (UIViewCreator::kAttrClass, baseViewClassName);
	attr->setAttribute ("size", "400,400");
	description->addNewTemplate (name.c_str (), attr);
	if (view == nullptr)
		view = description->createView (name.c_str (), description->getController ());
	if (auto ap = actionPerformer.lock ())
		ap->onTemplateCreation (name.c_str (), view);
}

//----------------------------------------------------------------------------------------------------
void CreateNewTemplateAction::undo ()
{
	description->removeTemplate (name.c_str ());
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
DuplicateTemplateAction::DuplicateTemplateAction (const SharedPointer<UIDescription>& description,
												  WeakPointer<IActionPerformer> actionPerformer,
												  UTF8StringPtr name, UTF8StringPtr dupName)
: description (description), actionPerformer (actionPerformer), name (name), dupName (dupName)
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
	if (auto ap = actionPerformer.lock ())
		ap->onTemplateCreation (dupName.c_str (), view);
}

//----------------------------------------------------------------------------------------------------
void DuplicateTemplateAction::undo ()
{
	description->removeTemplate (dupName.c_str ());
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
DeleteTemplateAction::DeleteTemplateAction (const SharedPointer<UIDescription>& description,
											WeakPointer<IActionPerformer> actionPerformer,
											const SharedPointer<CView>& view, UTF8StringPtr name)
: description (description), actionPerformer (actionPerformer), view (view), name (name)
{
	attributes = description->getViewAttributes (name);
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr DeleteTemplateAction::getName ()
{
	return "Delete Template";
}

//----------------------------------------------------------------------------------------------------
void DeleteTemplateAction::perform () { description->removeTemplate (name.c_str ()); }

//----------------------------------------------------------------------------------------------------
void DeleteTemplateAction::undo ()
{
	if (auto ap = actionPerformer.lock ())
		ap->onTemplateCreation (name.c_str (), view);
	description->addNewTemplate (name.c_str (), attributes);
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
ChangeFocusDrawingAction::ChangeFocusDrawingAction (const SharedPointer<UIDescription>& description,
													const FocusDrawingSettings& newSettings)
: description (description), newSettings (newSettings)
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
ChangeTemplateMinMaxAction::ChangeTemplateMinMaxAction (
	const SharedPointer<UIDescription>& description, UTF8StringPtr templateName, CPoint minSize,
	CPoint maxSize)
: description (description), templateName (templateName), minSize (minSize), maxSize (maxSize)
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
	if (auto attr = description->getViewAttributes (templateName.data ()))
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
