/*
 *  viewfactory.cpp
 *  VST3PlugIns
 *
 *  Created by Arne Scheffler on 4/28/09.
 *  Copyright 2009 Arne Scheffler. All rights reserved.
 *
 */

#include "viewfactory.h"

BEGIN_NAMESPACE_VSTGUI

//-----------------------------------------------------------------------------
class ViewCreator
{
public:
	ViewCreator (const char* _name, const char* _baseClass, ViewCreateFunction _createFunction, ViewApplyAttributesFunction _applyFunction)
	: name (_name)
	, createFunction (_createFunction)
	, applyFunction (_applyFunction)
	{
		if (_baseClass)
			baseClass = _baseClass;
	}
	
	const std::string& getName () const { return name; }
	const std::string& getBaseClass () const { return baseClass; }
	ViewCreateFunction getCreateFunction () const { return createFunction; }
	ViewApplyAttributesFunction getApplyFunction () const { return applyFunction; }

protected:
	std::string name;
	std::string baseClass;
	ViewCreateFunction createFunction;
	ViewApplyAttributesFunction applyFunction;
};

//-----------------------------------------------------------------------------
class ViewCreatorRegistry : public std::map<std::string,ViewCreator*>
{
public:
	~ViewCreatorRegistry ()
	{
		iterator iter = begin ();
		while (iter != end ())
		{
			delete iter->second;
			iter++;
		}
	}
};

//-----------------------------------------------------------------------------
ViewCreatorRegistry& getCreatorRegistry ()
{
	static ViewCreatorRegistry creatorRegistry;
	return creatorRegistry;
}

//-----------------------------------------------------------------------------
ViewFactory::ViewFactory ()
{
}

//-----------------------------------------------------------------------------
ViewFactory::~ViewFactory ()
{
}

//-----------------------------------------------------------------------------
CView* ViewFactory::createViewByName (const std::string* className, const UIAttributes& attributes, UIDescription* description)
{
	ViewCreatorRegistry& registry = getCreatorRegistry ();
	ViewCreatorRegistry::const_iterator iter = registry.find (*className);
	if (iter != registry.end ())
	{
		CView* view = (*iter).second->getCreateFunction () (attributes, this, description);
		if (view)
		{
			while (iter != registry.end () && (*iter).second->getApplyFunction () (view, attributes, this, description))
			{
				iter = registry.find ((*iter).second->getBaseClass ());
			}
			return view;
		}
	}
	else
	{
		#if DEBUG
		DebugPrint ("ViewFactory::createView(..): Could not find view of class: %s\n", className->c_str ());
		#endif
	}
	return 0;
}

//-----------------------------------------------------------------------------
CView* ViewFactory::createView (const UIAttributes& attributes, UIDescription* description)
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
void ViewFactory::registerViewCreateFunction (const char* className, const char* baseClass, ViewCreateFunction createFunction, ViewApplyAttributesFunction applyFunction)
{
	if (className == 0 || createFunction == 0 || applyFunction == 0)
	{
		#if DEBUG
		DebugPrint ("ViewFactory::registerViewCreateFunction(..) needs className, createFunction and applyFunction\n");
		#endif
		return;
	}
	ViewCreatorRegistry& registry = getCreatorRegistry ();
	if (registry.find (className) != registry.end ())
	{
		#if DEBUG
		DebugPrint ("ViewCreateFunction for '%s' already registered\n", className);
		#endif
	}
	else
	{
		registry.insert (std::make_pair (className, new ViewCreator (className, baseClass, createFunction, applyFunction)));
	}
}

END_NAMESPACE_VSTGUI
