//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
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
	class MyViewCreator : public IViewCreator
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
			const std::string* attr = attributes.getAttributeValue ("my-custom-attribute");
			if (attr)
			{
				int32_t value = (int32_t)strtol (attr->c_str (), 0, 10);
				myView->setCustomAttribute (value);
			}
			return true;
		}

		// add your custom attributes to the list		
		bool getAttributeNames (std::list<std::string>& attributeNames) const
		{
			attributeNames.push_back ("my-custom-attribute");
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
		bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const
		{
			MyView* myView = dynamic_cast<MyView*> (view);
			if (myView == 0)
				return false;
			if (attributeName == "my-custom-attribute")
			{
				std::stringstream stream;
				stream << (int32_t)myView->getCustomAttribute ();
				stringValue = stream.str ();
				return true;
			}
			return false;
		}
	};
	// create a static instance so that it registers itself with the view factory
	MyViewCreator __gMyViewCreator;
	
	@endcode
*/

typedef std::unordered_map<std::string, const IViewCreator*> ViewCreatorRegistryMap;

//-----------------------------------------------------------------------------
class ViewCreatorRegistry : private ViewCreatorRegistryMap
{
public:
	typedef ViewCreatorRegistryMap::const_iterator const_iterator;

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
		ViewCreatorRegistry::const_iterator it = find (viewCreator->getViewName ());
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
UIViewFactory::~UIViewFactory ()
{
}

//-----------------------------------------------------------------------------
CView* UIViewFactory::createViewByName (const std::string* className, const UIAttributes& attributes, const IUIDescription* description) const
{
	ViewCreatorRegistry& registry = getCreatorRegistry ();
	ViewCreatorRegistry::const_iterator iter = registry.find (className->c_str ());
	if (iter != registry.end ())
	{
		CView* view = (*iter).second->create (attributes, description);
		if (view)
		{
			IdStringPtr viewName = (*iter).second->getViewName ();
			view->setAttribute (kViewNameAttribute, sizeof (IdStringPtr), &viewName);
			UIAttributes evaluatedAttributes;
			evaluateAttributesAndRemember (view, attributes, evaluatedAttributes, description);
			while (iter != registry.end () && (*iter).second->apply (view, evaluatedAttributes, description))
			{
				if ((*iter).second->getBaseViewName () == 0)
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
	return 0;
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
	ViewCreatorRegistry& registry = getCreatorRegistry ();
	ViewCreatorRegistry::const_iterator iter = registry.find (getViewName (view));

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
	ViewCreatorRegistry& registry = getCreatorRegistry ();
	ViewCreatorRegistry::const_iterator iter = registry.find (baseViewName);
	if (iter != registry.end ())
	{
		IdStringPtr viewName = (*iter).second->getViewName ();
		customView->setAttribute (kViewNameAttribute, sizeof (IdStringPtr), &viewName);
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
IdStringPtr UIViewFactory::getViewName (CView* view) const
{
	IdStringPtr viewName = 0;
	uint32_t size = sizeof (IdStringPtr);
	view->getAttribute (kViewNameAttribute, size, &viewName, size);
	return viewName;
}

//-----------------------------------------------------------------------------
void UIViewFactory::evaluateAttributesAndRemember (CView* view, const UIAttributes& attributes, UIAttributes& evaluatedAttributes, const IUIDescription* description) const
{
	std::string evaluatedValue;
	typedef std::pair<std::string, std::string> StringPair;
	VSTGUI_RANGE_BASED_FOR_LOOP (UIAttributesMap, attributes, StringPair, attr)
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
			IViewCreator::AttrType type = getAttributeType (view, attr.first);
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
	VSTGUI_RANGE_BASED_FOR_LOOP_END
}

#if VSTGUI_LIVE_EDITING
//-----------------------------------------------------------------------------
bool UIViewFactory::getAttributeNamesForView (CView* view, StringList& attributeNames) const
{
	bool result = false;
	ViewCreatorRegistry& registry = getCreatorRegistry ();
	ViewCreatorRegistry::const_iterator iter = registry.find (getViewName (view));
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
		ViewCreatorRegistry& registry = getCreatorRegistry ();
		ViewCreatorRegistry::const_iterator iter = registry.find (getViewName (view));
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
	ViewCreatorRegistry& registry = getCreatorRegistry ();
	IViewCreator::AttrType type = IViewCreator::kUnknownType;
	ViewCreatorRegistry::const_iterator iter = registry.find (getViewName (view));
	while (iter != registry.end () && (type = (*iter).second->getAttributeType (attributeName)) == IViewCreator::kUnknownType && (*iter).second->getBaseViewName ())
	{
		iter = registry.find ((*iter).second->getBaseViewName ());
	}
	return type;
}

//-----------------------------------------------------------------------------
bool UIViewFactory::getPossibleAttributeListValues (CView* view, const std::string& attributeName, StringPtrList& values) const
{
	ViewCreatorRegistry& registry = getCreatorRegistry ();
	ViewCreatorRegistry::const_iterator iter = registry.find (getViewName (view));
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
	ViewCreatorRegistry& registry = getCreatorRegistry ();
	ViewCreatorRegistry::const_iterator iter = registry.find (getViewName (view));
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
		StringList::const_iterator it = attrNames.begin ();
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
static bool viewNamesSortFunc (const std::string* lhs, const std::string* rhs)
{
	return *lhs < *rhs;
}

//-----------------------------------------------------------------------------
void UIViewFactory::collectRegisteredViewNames (StringPtrList& viewNames, IdStringPtr _baseClassNameFilter) const
{
	UTF8StringView baseClassNameFilter (_baseClassNameFilter);
	ViewCreatorRegistry& registry = getCreatorRegistry ();
	ViewCreatorRegistry::const_iterator iter = registry.begin ();
	while (iter != registry.end ())
	{
		if (baseClassNameFilter)
		{
			bool found = false;
			ViewCreatorRegistry::const_iterator iter2 (iter);
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
		viewNames.push_back (&(*iter).first);
		iter++;
	}
	viewNames.sort (viewNamesSortFunc);
}

//-----------------------------------------------------------------------------
size_t UIViewFactory::createHash (const std::string& str)
{
#if VSTGUI_HAS_FUNCTIONAL
	static std::hash<std::string> hashFunc;
	return hashFunc (str);
#else
	size_t hash = 5381;
	for (std::size_t i = 0; i < str.length (); i++)
	{
		hash = ((hash << 5) + hash) + static_cast<size_t> (str[i]);
	}
	return hash;
#endif
}

//-----------------------------------------------------------------------------
void UIViewFactory::rememberAttribute (CView* view, IdStringPtr attrName, const std::string& value) const
{
#if ENABLE_UNIT_TESTS
	if (disableRememberAttributes)
		return;
#endif
	size_t hash = createHash (attrName);
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
	ViewCreatorRegistry& registry = getCreatorRegistry ();
	registry.add (viewCreator.getViewName (), &viewCreator);
}

//-----------------------------------------------------------------------------
void UIViewFactory::unregisterViewCreator (const IViewCreator& viewCreator)
{
	ViewCreatorRegistry& registry = getCreatorRegistry ();
	registry.remove (&viewCreator);
}

} // namespace
