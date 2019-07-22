// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uiviewfactory.h"
#include "uiattributes.h"
#include "../lib/cview.h"
#include "../lib/cstring.h"
#include "detail/uiviewcreatorattributes.h"
#include "../lib/platform/std_unorderedmap.h"

namespace VSTGUI {

/** @class IViewCreator

	You can register your own custom views with the UIViewFactory by inheriting from this interface and register it
	with UIViewFactory::registerViewCreator().
	
	Example for an imaginary view class called MyView which directly inherites from CView:
	@code
	class MyViewCreator : public ViewCreatorAdapter
	{
	public:
		// register this class with the view factory
		MyViewCreator () { UIViewFactory::registerViewCreator (*this); }
		
		// return an unique name here
		IdStringPtr getViewName () const { return "MyView"; }

		// return the name here from where your custom view inherites.
		// Your view automatically supports the attributes from it.
		IdStringPtr getBaseViewName () const { return "CView"; }

		// create your view here.
		// Note you don't need to apply attributes here as the apply method will be called with this new view
		CView* create (const UIAttributes& attributes, const IUIDescription* description) const { return new MyView (); }

		// apply custom attributes to your view		
		bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const
		{
			MyView* myView = dynamic_cast<MyView*> (view);
			if (myView == 0)
				return false;
			int32_t value;
			if (attributes.getIntegerAttribute ("my-custom-attribute", value))
				myView->setCustomAttribute (value);
			return true;
		}

		// add your custom attributes to the list		
		bool getAttributeNames (StringList& attributeNames) const
		{
			attributeNames.emplace_back ("my-custom-attribute");
			return true;
		}
		
		// return the type of your custom attributes
		AttrType getAttributeType (const std::string& attributeName) const
		{
			if (attributeName == "my-custom-attribute")
				return kIntegerType;
			return kUnknownType;
		}
		
		// return the string value of the custom attributes of the view
		bool getAttributeValue (CView* view, const string& attributeName, string& stringValue, const IUIDescription* desc) const
		{
			MyView* myView = dynamic_cast<MyView*> (view);
			if (myView == 0)
				return false;
			if (attributeName == "my-custom-attribute")
			{
				stringValue = UIAttributes::integerToString (myView->getCustomAttribute ());
				return true;
			}
			return false;
		}
	};
	// create a static instance so that it registers itself with the view factory
	MyViewCreator __gMyViewCreator;
	
	@endcode
*/

using ViewCreatorRegistryMap = std::unordered_map<std::string, const IViewCreator*>;

//-----------------------------------------------------------------------------
class ViewCreatorRegistry : private ViewCreatorRegistryMap
{
public:
	using const_iterator = ViewCreatorRegistryMap::const_iterator;

	const_iterator begin () { return ViewCreatorRegistryMap::begin (); }
	const_iterator end () { return ViewCreatorRegistryMap::end (); }

	const_iterator find (IdStringPtr name)
	{
		if (name)
			return ViewCreatorRegistryMap::find (name);
		return end ();
	}

	void add (IdStringPtr name, const IViewCreator* viewCreator)
	{
#if DEBUG
		if (find (viewCreator->getViewName ()) != end ())
		{
			DebugPrint ("ViewCreateFunction for '%s' already registered\n", viewCreator->getViewName ());
		}
#endif
		insert (std::make_pair (viewCreator->getViewName (), viewCreator));
		
	}

	void remove (const IViewCreator* viewCreator)
	{
		auto it = find (viewCreator->getViewName ());
		if (it == end ())
			return;
		erase (it);
	}
};

//-----------------------------------------------------------------------------
static ViewCreatorRegistry& getCreatorRegistry ()
{
	static ViewCreatorRegistry creatorRegistry;
	return creatorRegistry;
}

//-----------------------------------------------------------------------------
static CViewAttributeID kViewNameAttribute = 'cvcr';

//-----------------------------------------------------------------------------
UIViewFactory::UIViewFactory ()
{
}

//-----------------------------------------------------------------------------
CView* UIViewFactory::createViewByName (const std::string* className, const UIAttributes& attributes, const IUIDescription* description) const
{
	auto& registry = getCreatorRegistry ();
	auto iter = registry.find (className->c_str ());
	if (iter != registry.end ())
	{
		CView* view = (*iter).second->create (attributes, description);
		if (view)
		{
			IdStringPtr viewName = (*iter).second->getViewName ();
			view->setAttribute (kViewNameAttribute, viewName);
			UIAttributes evaluatedAttributes;
			evaluateAttributesAndRemember (view, attributes, evaluatedAttributes, description);
			while (iter != registry.end () && (*iter).second->apply (view, evaluatedAttributes, description))
			{
				if ((*iter).second->getBaseViewName () == nullptr)
					break;
				iter = registry.find ((*iter).second->getBaseViewName ());
			}
			return view;
		}
	}
	else
	{
	#if DEBUG
		DebugPrint ("UIViewFactory::createView(..): Could not find view of class: %s\n", className->c_str ());
	#endif
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
CView* UIViewFactory::createView (const UIAttributes& attributes, const IUIDescription* description) const
{
	const std::string* className = attributes.getAttributeValue (UIViewCreator::kAttrClass);
	if (className)
		return createViewByName (className, attributes, description);
	std::string viewContainerName ("CViewContainer");
	return createViewByName (&viewContainerName, attributes, description);
}

//-----------------------------------------------------------------------------
bool UIViewFactory::applyAttributeValues (CView* view, const UIAttributes& attributes, const IUIDescription* desc) const
{
	bool result = false;
	auto& registry = getCreatorRegistry ();
	auto iter = registry.find (getViewName (view));

	UIAttributes evaluatedAttributes;
	evaluateAttributesAndRemember (view, attributes, evaluatedAttributes, desc);
	
	while (iter != registry.end () && (result = (*iter).second->apply (view, evaluatedAttributes, desc)) && (*iter).second->getBaseViewName ())
	{
		iter = registry.find ((*iter).second->getBaseViewName ());
	}
	return result;
}

//-----------------------------------------------------------------------------
bool UIViewFactory::applyCustomViewAttributeValues (CView* customView, IdStringPtr baseViewName, const UIAttributes& attributes, const IUIDescription* desc) const
{
	bool result = false;
	auto& registry = getCreatorRegistry ();
	auto iter = registry.find (baseViewName);
	if (iter != registry.end ())
	{
		IdStringPtr viewName = (*iter).second->getViewName ();
		customView->setAttribute (kViewNameAttribute, viewName);
	}
	UIAttributes evaluatedAttributes;
	evaluateAttributesAndRemember (customView, attributes, evaluatedAttributes, desc);
	while (iter != registry.end () && (result = (*iter).second->apply (customView, evaluatedAttributes, desc)) && (*iter).second->getBaseViewName ())
	{
		iter = registry.find ((*iter).second->getBaseViewName ());
	}
	return result;
}

//-----------------------------------------------------------------------------
IdStringPtr UIViewFactory::getViewName (CView* view)
{
	IdStringPtr viewName = nullptr;
	uint32_t size = sizeof (IdStringPtr);
	view->getAttribute (kViewNameAttribute, size, &viewName, size);
	return viewName;
}

//-----------------------------------------------------------------------------
void UIViewFactory::evaluateAttributesAndRemember (CView* view, const UIAttributes& attributes, UIAttributes& evaluatedAttributes, const IUIDescription* description) const
{
	std::string evaluatedValue;
	for (const auto& attr : attributes)
	{
		const std::string& value = attr.second;
		if (description && description->getVariable (value.c_str (), evaluatedValue))
		{
		#if VSTGUI_LIVE_EDITING
			rememberAttribute (view, attr.first.c_str (), value.c_str ());
		#endif
			evaluatedAttributes.setAttribute (attr.first, evaluatedValue);
		}
		else
		{
		#if VSTGUI_LIVE_EDITING
			auto type = getAttributeType (view, attr.first);
			switch (type)
			{
				case IViewCreator::kColorType:
				case IViewCreator::kTagType:
				case IViewCreator::kFontType:
				case IViewCreator::kGradientType:
					rememberAttribute (view, attr.first.c_str (), value.c_str ());
					break;
				default:
					break;
			}
		#endif
			evaluatedAttributes.setAttribute (attr.first, value);
		}
	}
}

#if VSTGUI_LIVE_EDITING
//-----------------------------------------------------------------------------
bool UIViewFactory::getAttributeNamesForView (CView* view, StringList& attributeNames) const
{
	bool result = false;
	auto& registry = getCreatorRegistry ();
	auto iter = registry.find (getViewName (view));
	while (iter != registry.end () && (result = (*iter).second->getAttributeNames (attributeNames)) && (*iter).second->getBaseViewName ())
	{
		iter = registry.find ((*iter).second->getBaseViewName ());
	}
	return result;
}

//-----------------------------------------------------------------------------
bool UIViewFactory::getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const
{
	bool result = getRememberedAttribute (view, attributeName.c_str (), stringValue);
	if (result == false)
	{
		auto& registry = getCreatorRegistry ();
		auto iter = registry.find (getViewName (view));
		while (iter != registry.end () && !(result = (*iter).second->getAttributeValue (view, attributeName, stringValue, desc)) && (*iter).second->getBaseViewName ())
		{
			iter = registry.find ((*iter).second->getBaseViewName ());
		}
		
	}
	return result;
}

//-----------------------------------------------------------------------------
IViewCreator::AttrType UIViewFactory::getAttributeType (CView* view, const std::string& attributeName) const
{
	auto& registry = getCreatorRegistry ();
	auto type = IViewCreator::kUnknownType;
	auto iter = registry.find (getViewName (view));
	while (iter != registry.end () && (type = (*iter).second->getAttributeType (attributeName)) == IViewCreator::kUnknownType && (*iter).second->getBaseViewName ())
	{
		iter = registry.find ((*iter).second->getBaseViewName ());
	}
	return type;
}

//-----------------------------------------------------------------------------
bool UIViewFactory::getPossibleAttributeListValues (CView* view, const std::string& attributeName, StringPtrList& values) const
{
	auto& registry = getCreatorRegistry ();
	auto iter = registry.find (getViewName (view));
	while (iter != registry.end () && (*iter).second->getPossibleListValues (attributeName, values) == false && (*iter).second->getBaseViewName ())
	{
		iter = registry.find ((*iter).second->getBaseViewName ());
	}
	return !values.empty ();
}

//-----------------------------------------------------------------------------
bool UIViewFactory::getAttributeValueRange (CView* view, const std::string& attributeName, double& minValue, double& maxValue) const
{
	minValue = maxValue = -1.;
	auto& registry = getCreatorRegistry ();
	auto iter = registry.find (getViewName (view));
	while (iter != registry.end () && (*iter).second->getAttributeValueRange (attributeName, minValue, maxValue) == false && (*iter).second->getBaseViewName ())
	{
		iter = registry.find ((*iter).second->getBaseViewName ());
	}
	return minValue != maxValue && minValue != -1.;
}

//-----------------------------------------------------------------------------
bool UIViewFactory::getAttributesForView (CView* view, const IUIDescription* desc, UIAttributes& attr) const
{
	bool result = false;
	StringList attrNames;
	if (getAttributeNamesForView (view, attrNames))
	{
		auto it = attrNames.begin ();
		while (it != attrNames.end ())
		{
			std::string value;
			if (getAttributeValue (view, *it, value, desc))
				attr.setAttribute (*it, value);
			it++;
		}
		attr.setAttribute (UIViewCreator::kAttrClass, getViewName (view));
		result = true;
	}
	return result;
}

//-----------------------------------------------------------------------------
void UIViewFactory::collectRegisteredViewNames (StringPtrList& viewNames, IdStringPtr _baseClassNameFilter) const
{
	UTF8StringView baseClassNameFilter (_baseClassNameFilter);
	auto& registry = getCreatorRegistry ();
	auto iter = registry.begin ();
	while (iter != registry.end ())
	{
		if (baseClassNameFilter)
		{
			bool found = false;
			auto iter2 = iter;
			while (iter2 != registry.end () && (*iter2).second->getBaseViewName ())
			{
				if (baseClassNameFilter == (*iter2).second->getViewName () || baseClassNameFilter == (*iter2).second->getBaseViewName ())
				{
					found = true;
					break;
				}
				iter2 = registry.find ((*iter2).second->getBaseViewName ());
			}
			if (!found)
			{
				iter++;
				continue;
			}
		}
		viewNames.emplace_back (&(*iter).first);
		iter++;
	}
	viewNames.sort ([] (const std::string* lhs, const std::string* rhs) { return *lhs < *rhs; });
}

//-----------------------------------------------------------------------------
auto UIViewFactory::collectRegisteredViewAndDisplayNames (IdStringPtr baseClassNameFilter) const -> ViewAndDisplayNameList
{
	ViewAndDisplayNameList list;
	auto& registry = getCreatorRegistry ();
	auto iter = registry.begin ();
	while (iter != registry.end ())
	{
		if (baseClassNameFilter)
		{
			UTF8StringView baseClassNameStr (baseClassNameFilter);
			bool found = false;
			auto iter2 = iter;
			while (iter2 != registry.end () && (*iter2).second->getBaseViewName ())
			{
				if (baseClassNameStr == (*iter2).second->getViewName () || baseClassNameStr == (*iter2).second->getBaseViewName ())
				{
					found = true;
					break;
				}
				iter2 = registry.find ((*iter2).second->getBaseViewName ());
			}
			if (!found)
			{
				iter++;
				continue;
			}
		}
		list.emplace_back (&(*iter).first, (*iter).second->getDisplayName ());
		iter++;
	}
	return list;
}

//------------------------------------------------------------------------
UTF8StringPtr UIViewFactory::getViewDisplayName (CView* view) const
{
	if (auto viewName = getViewName (view))
	{
		UTF8StringView viewNameStr (viewName);
		auto& registry = getCreatorRegistry ();
		for (auto& entry : registry)
		{
			if (viewNameStr == entry.second->getViewName ())
				return entry.second->getDisplayName ();
		}
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
size_t UIViewFactory::createHash (const std::string& str)
{
	static std::hash<std::string> hashFunc;
	return hashFunc (str);
}

//-----------------------------------------------------------------------------
void UIViewFactory::rememberAttribute (CView* view, IdStringPtr attrName, const std::string& value) const
{
#if ENABLE_UNIT_TESTS
	if (disableRememberAttributes)
		return;
#endif
	auto hash = createHash (attrName);
	view->setAttribute (hash, static_cast<uint32_t> (value.size () + 1), value.c_str ());
}

//-----------------------------------------------------------------------------
bool UIViewFactory::getRememberedAttribute (CView* view, IdStringPtr attrName, std::string& value) const
{
	bool result = false;
	size_t hash = createHash (attrName);
	uint32_t attrSize = 0;
	if (view->getAttributeSize (hash, attrSize))
	{
		char* temp = new char[attrSize];
		if (view->getAttribute (hash, attrSize, temp, attrSize))
		{
			value = temp;
			result = true;
		}
		delete [] temp;
	}
	return result;
}

#endif

//-----------------------------------------------------------------------------
void UIViewFactory::registerViewCreator (const IViewCreator& viewCreator)
{
	auto& registry = getCreatorRegistry ();
	registry.add (viewCreator.getViewName (), &viewCreator);
}

//-----------------------------------------------------------------------------
void UIViewFactory::unregisterViewCreator (const IViewCreator& viewCreator)
{
	auto& registry = getCreatorRegistry ();
	registry.remove (&viewCreator);
}

} // VSTGUI
