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

#include "uiviewfactory.h"
#include <map>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class ViewCreatorRegistry : public std::map<std::string,const IViewCreator*>
{
public:
	const_iterator find (IdStringPtr name)
	{
		if (name)
			return std::map<std::string,const IViewCreator*>::find (name);
		return end ();
	}
};

//-----------------------------------------------------------------------------
static ViewCreatorRegistry& getCreatorRegistry ()
{
	static ViewCreatorRegistry creatorRegistry;
	return creatorRegistry;
}

//-----------------------------------------------------------------------------
UIViewFactory::UIViewFactory ()
{
}

//-----------------------------------------------------------------------------
UIViewFactory::~UIViewFactory ()
{
}

//-----------------------------------------------------------------------------
CView* UIViewFactory::createViewByName (const std::string* className, const UIAttributes& attributes, IUIDescription* description)
{
	ViewCreatorRegistry& registry = getCreatorRegistry ();
	ViewCreatorRegistry::const_iterator iter = registry.find (className->c_str ());
	if (iter != registry.end ())
	{
		CView* view = (*iter).second->create (attributes, description);
		if (view)
		{
			IdStringPtr viewName = (*iter).second->getViewName ();
			view->setAttribute ('cvcr', sizeof (IdStringPtr), &viewName);
			while (iter != registry.end () && (*iter).second->apply (view, attributes, description))
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
CView* UIViewFactory::createView (const UIAttributes& attributes, IUIDescription* description)
{
	const std::string* className = attributes.getAttributeValue ("class");
	if (className)
		return createViewByName (className, attributes, description);
	else
	{
		std::string viewContainerName ("CViewContainer");
		return createViewByName (&viewContainerName, attributes, description);
	}
	return 0;
}

//-----------------------------------------------------------------------------
bool UIViewFactory::applyAttributeValues (CView* view, const UIAttributes& attributes, IUIDescription* desc) const
{
	bool result = false;
	ViewCreatorRegistry& registry = getCreatorRegistry ();
	ViewCreatorRegistry::const_iterator iter = registry.find (getViewName (view));
	while (iter != registry.end () && (result = (*iter).second->apply (view, attributes, desc)) && (*iter).second->getBaseViewName ())
	{
		iter = registry.find ((*iter).second->getBaseViewName ());
	}
	return result;
}

//-----------------------------------------------------------------------------
bool UIViewFactory::applyCustomViewAttributeValues (CView* customView, IdStringPtr baseViewName, const UIAttributes& attributes, IUIDescription* desc) const
{
	bool result = false;
	ViewCreatorRegistry& registry = getCreatorRegistry ();
	ViewCreatorRegistry::const_iterator iter = registry.find (baseViewName);
	if (iter != registry.end ())
	{
		IdStringPtr viewName = (*iter).second->getViewName ();
		customView->setAttribute ('cvcr', sizeof (IdStringPtr), &viewName);
	}
	while (iter != registry.end () && (result = (*iter).second->apply (customView, attributes, desc)) && (*iter).second->getBaseViewName ())
	{
		iter = registry.find ((*iter).second->getBaseViewName ());
	}
	return result;
}

//-----------------------------------------------------------------------------
IdStringPtr UIViewFactory::getViewName (CView* view) const
{
	IdStringPtr viewName = 0;
	int32_t size = sizeof (IdStringPtr);
	view->getAttribute ('cvcr', size, &viewName, size);
	return viewName;
}

#if VSTGUI_LIVE_EDITING
//-----------------------------------------------------------------------------
bool UIViewFactory::getAttributeNamesForView (CView* view, std::list<std::string>& attributeNames) const
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
bool UIViewFactory::getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
{
	bool result = false;
	ViewCreatorRegistry& registry = getCreatorRegistry ();
	ViewCreatorRegistry::const_iterator iter = registry.find (getViewName (view));
	while (iter != registry.end () && !(result = (*iter).second->getAttributeValue (view, attributeName, stringValue, desc)) && (*iter).second->getBaseViewName ())
	{
		iter = registry.find ((*iter).second->getBaseViewName ());
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
bool UIViewFactory::getAttributesForView (CView* view, IUIDescription* desc, UIAttributes& attr) const
{
	bool result = false;
	std::list<std::string> attrNames;
	if (getAttributeNamesForView (view, attrNames))
	{
		std::list<std::string>::const_iterator it = attrNames.begin ();
		while (it != attrNames.end ())
		{
			std::string value;
			if (getAttributeValue (view, (*it), value, desc))
				attr.setAttribute ((*it).c_str (), value.c_str ());
			it++;
		}
		attr.setAttribute ("class", getViewName (view));
		result = true;
	}
	return result;
}

//-----------------------------------------------------------------------------
void UIViewFactory::collectRegisteredViewNames (std::list<const std::string*>& viewNames, IdStringPtr baseClassNameFilter) const
{
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
				if ((*iter2).first == baseClassNameFilter)
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
		if ((*iter).first != "CControl")
			viewNames.push_back (&(*iter).first);
		iter++;
	}
}

#endif

//-----------------------------------------------------------------------------
void UIViewFactory::registerViewCreator (const IViewCreator& viewCreator)
{
	ViewCreatorRegistry& registry = getCreatorRegistry ();
#if DEBUG
	if (registry.find (viewCreator.getViewName ()) != registry.end ())
	{
		DebugPrint ("ViewCreateFunction for '%s' already registered\n", viewCreator.getViewName ());
	}
#endif
	registry.insert (std::make_pair (viewCreator.getViewName (), &viewCreator));
}

} // namespace
