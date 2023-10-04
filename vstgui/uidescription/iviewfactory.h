// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../lib/vstguifwd.h"
#include "iviewcreator.h"
#include <string>

namespace VSTGUI {
class UIAttributes;
class IUIDescription;

//-----------------------------------------------------------------------------
class IViewFactory
{
public:
	virtual ~IViewFactory () noexcept = default;
	
	virtual CView* createView (const UIAttributes& attributes, const IUIDescription* description) const = 0;
	virtual bool applyAttributeValues (CView* view, const UIAttributes& attributes, const IUIDescription* desc) const = 0;
	virtual bool applyCustomViewAttributeValues (CView* customView, IdStringPtr baseViewName, const UIAttributes& attributes, const IUIDescription* desc) const = 0;
	virtual bool getAttributeValue (CView* view, const std::string& attributeName,
									std::string& stringValue, const IUIDescription* desc) const = 0;
	virtual bool viewIsTypeOf (CView* view, const std::string& typeName) const = 0;

	static IdStringPtr getViewName (CView* view);
};

//------------------------------------------------------------------------
class IViewFactoryEditingSupport
{
public:
	using StringPtrList = std::list<const std::string*>;
	using StringList = std::list<std::string>;
	using ViewAndDisplayNameList = std::list<std::pair<const std::string*, const std::string>>;

	virtual ~IViewFactoryEditingSupport () noexcept = default;

	virtual bool getAttributeNamesForView (CView* view, StringList& attributeNames) const = 0;
	virtual IViewCreator::AttrType getAttributeType (CView* view, const std::string& attributeName) const = 0;
	virtual void collectRegisteredViewNames (StringPtrList& viewNames, IdStringPtr baseClassNameFilter = nullptr) const = 0;
	virtual bool getAttributesForView (CView* view, const IUIDescription* desc, UIAttributes& attr) const = 0;
	// list type support
	virtual bool getPossibleAttributeListValues (CView* view, const std::string& attributeName, StringPtrList& values) const = 0;
	virtual bool getAttributeValueRange (CView* view, const std::string& attributeName, double& minValue, double& maxValue) const = 0;

	virtual ViewAndDisplayNameList collectRegisteredViewAndDisplayNames (IdStringPtr baseClassNameFilter = nullptr) const = 0;
	virtual UTF8StringPtr getViewDisplayName (CView* view) const = 0;
};

//------------------------------------------------------------------------
class ViewFactoryDelegate : public IViewFactory,
							public IViewFactoryEditingSupport
{
public:
	ViewFactoryDelegate (IViewFactory* orig)
	{
		of = orig;
		ofes = dynamic_cast<IViewFactoryEditingSupport*> (orig);
	}

	CView* createView (const UIAttributes& attributes, const IUIDescription* description) const
	{
		return of->createView (attributes, description);
	}
	bool applyAttributeValues (CView* view, const UIAttributes& attributes,
							   const IUIDescription* desc) const
	{
		return of->applyAttributeValues (view, attributes, desc);
	}
	bool applyCustomViewAttributeValues (CView* customView, IdStringPtr baseViewName,
										 const UIAttributes& attributes,
										 const IUIDescription* desc) const
	{
		return of->applyCustomViewAttributeValues (customView, baseViewName, attributes, desc);
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue,
							const IUIDescription* desc) const
	{
		return of->getAttributeValue (view, attributeName, stringValue, desc);
	}
	bool viewIsTypeOf (CView* view, const std::string& typeName) const
	{
		return of->viewIsTypeOf (view, typeName);
	}
	bool getAttributeNamesForView (CView* view, StringList& attributeNames) const
	{
		return ofes ? ofes->getAttributeNamesForView (view, attributeNames) : false;
	}
	IViewCreator::AttrType getAttributeType (CView* view, const std::string& attributeName) const
	{
		return ofes ? ofes->getAttributeType (view, attributeName)
					: IViewCreator::AttrType::kUnknownType;
	}
	void collectRegisteredViewNames (StringPtrList& viewNames,
									 IdStringPtr baseClassNameFilter) const
	{
		if (ofes)
			ofes->collectRegisteredViewNames (viewNames, baseClassNameFilter);
	}
	bool getAttributesForView (CView* view, const IUIDescription* desc, UIAttributes& attr) const
	{
		return ofes ? ofes->getAttributesForView (view, desc, attr) : false;
	}
	bool getPossibleAttributeListValues (CView* view, const std::string& attributeName,
										 StringPtrList& values) const
	{
		return ofes ? ofes->getPossibleAttributeListValues (view, attributeName, values) : false;
	}
	bool getAttributeValueRange (CView* view, const std::string& attributeName, double& minValue,
								 double& maxValue) const
	{
		return ofes ? ofes->getAttributeValueRange (view, attributeName, minValue, maxValue)
					: false;
	}
	ViewAndDisplayNameList
		collectRegisteredViewAndDisplayNames (IdStringPtr baseClassNameFilter) const
	{
		return ofes ? ofes->collectRegisteredViewAndDisplayNames (baseClassNameFilter)
					: ViewAndDisplayNameList {};
	}
	UTF8StringPtr getViewDisplayName (CView* view) const
	{
		return ofes ? ofes->getViewDisplayName (view) : "";
	}

protected:
	IViewFactory* getViewFactory () const { return of; }
	IViewFactoryEditingSupport* getViewFactoryEditingSupport () const { return ofes; }

private:
	IViewFactory* of {nullptr};
	IViewFactoryEditingSupport* ofes {nullptr};
};

//------------------------------------------------------------------------
} // VSTGUI VSTGUI
