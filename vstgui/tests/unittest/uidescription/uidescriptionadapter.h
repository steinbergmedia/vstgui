// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../../uidescription/iuidescription.h"
#include "../../../uidescription/iviewfactory.h"
#include "../../../uidescription/uiattributes.h"

namespace VSTGUI {

class ViewFactoryAdapter : public NonAtomicReferenceCounted,
						   public IViewFactory
{
public:
	CView* createView (const UIAttributes& attributes,
					   const IUIDescription& description) const override
	{
		return nullptr;
	}
	bool applyAttributeValues (CView* view, const UIAttributes& attributes,
							   const IUIDescription& desc) const override
	{
		return false;
	}
	bool applyCustomViewAttributeValues (CView* customView, IdStringPtr baseViewName,
										 const UIAttributes& attributes,
										 const IUIDescription& desc) const override
	{
		return false;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue,
							const IUIDescription& desc) const override
	{
		return false;
	}
	bool viewIsTypeOf (CView* view, const std::string& typeName) const override { return false; }
};

class UIDescriptionAdapter : public IUIDescription
{
public:
	CView* createView (UTF8StringPtr name,
					   const SharedPointer<IController>& controller) const override
	{
		return nullptr;
	}
	SharedPointer<CBitmap> getBitmap (UTF8StringPtr name) const override { return nullptr; }
	SharedPointer<CFontDesc> getFont (UTF8StringPtr name) const override { return nullptr; }
	bool getColor (UTF8StringPtr name, CColor& color) const override { return false; }
	SharedPointer<CGradient> getGradient (UTF8StringPtr name) const override { return nullptr; }
	int32_t getTagForName (UTF8StringPtr name) const override { return -1; }
	IControlListener* getControlListener (UTF8StringPtr name) const override { return nullptr; }
	SharedPointer<IController> getController () const override { return nullptr; }

	UTF8StringPtr lookupColorName (const CColor& color) const override { return nullptr; }
	UTF8StringPtr lookupFontName (const SharedPointer<CFontDesc>& font) const override
	{
		return nullptr;
	}
	UTF8StringPtr lookupBitmapName (const SharedPointer<CBitmap>& bitmap) const override
	{
		return nullptr;
	}
	UTF8StringPtr lookupGradientName (const SharedPointer<CGradient>& gradient) const override
	{
		return nullptr;
	}
	UTF8StringPtr lookupControlTagName (const int32_t tag) const override { return nullptr; }

	bool getVariable (UTF8StringPtr name, double& value) const override { return false; }
	bool getVariable (UTF8StringPtr name, std::string& value) const override { return false; }

	void collectTemplateViewNames (std::list<const std::string*>& names) const override {}
	void collectColorNames (std::list<const std::string*>& names) const override {}
	void collectFontNames (std::list<const std::string*>& names) const override {}
	void collectBitmapNames (std::list<const std::string*>& names) const override {}
	void collectGradientNames (std::list<const std::string*>& names) const override {}
	void collectControlTagNames (std::list<const std::string*>& names) const override {}

	const IViewFactory& getViewFactory () const override { return viewFactory; }

	bool setCustomAttributes (UTF8StringPtr name, const SharedPointer<UIAttributes>& attr) override
	{
		return false;
	}
	SharedPointer<UIAttributes> getCustomAttributes (UTF8StringPtr name) const override
	{
		return {};
	}

	ViewFactoryAdapter viewFactory;
};

}
