//
//  uidescriptionadapter.h
//  vstgui
//
//  Created by Arne Scheffler on 30/10/15.
//
//

#pragma once

#include "../../../../uidescription/iuidescription.h"

namespace VSTGUI {

class UIDescriptionAdapter : public IUIDescription
{
public:
	CBitmap* getBitmap (UTF8StringPtr name) const override { return nullptr; }
	CFontRef getFont (UTF8StringPtr name) const override { return nullptr; }
	bool getColor (UTF8StringPtr name, CColor& color) const override { return false; }
	CGradient* getGradient (UTF8StringPtr name) const override { return nullptr; }
	int32_t getTagForName (UTF8StringPtr name) const override { return -1; }
	IControlListener* getControlListener (UTF8StringPtr name) const override { return nullptr; }
	IController* getController () const override { return nullptr; }

	UTF8StringPtr lookupColorName (const CColor& color) const override { return nullptr; }
	UTF8StringPtr lookupFontName (const CFontRef font) const override { return nullptr; }
	UTF8StringPtr lookupBitmapName (const CBitmap* bitmap) const override { return nullptr; }
	UTF8StringPtr lookupGradientName (const CGradient* gradient) const override { return nullptr; }
	UTF8StringPtr lookupControlTagName (const int32_t tag) const override { return nullptr; }

	bool getVariable (UTF8StringPtr name, double& value) const override { return false; }
	bool getVariable (UTF8StringPtr name, std::string& value) const override { return false; }

	void collectTemplateViewNames (std::list<const std::string*>& names) const override {}
	void collectColorNames (std::list<const std::string*>& names) const override {}
	void collectFontNames (std::list<const std::string*>& names) const override {}
	void collectBitmapNames (std::list<const std::string*>& names) const override {}
	void collectGradientNames (std::list<const std::string*>& names) const override {}
	void collectControlTagNames (std::list<const std::string*>& names) const override {}

	const IViewFactory* getViewFactory () const override { return nullptr; }
};

inline void testPossibleValues (const IdStringPtr className, const std::string& attrName, IUIDescription* desc, UIViewFactory::StringList expectedValues)
{
	UIViewFactory factory;
	UIAttributes a;
	a.setAttribute (UIViewCreator::kAttrClass, className);
	auto view = owned (factory.createView (a, desc));
	UIViewFactory::StringPtrList values;
	EXPECT(factory.getPossibleAttributeListValues (view, attrName, values));
	for (auto& v : expectedValues)
	{
		EXPECT(std::find_if (values.begin(), values.end(), [&] (const UIViewFactory::StringPtrList::value_type& value) {
			return *value == v;
		}) != values.end ());
	}
	EXPECT(values.size () == expectedValues.size ());
}

template<typename ViewClass, typename Proc>
void testAttribute (const IdStringPtr viewName, const std::string& attrName, const IdStringPtr attrValue, IUIDescription* desc, const Proc& proc, bool disableRememberAttributes = false)
{
	UIViewFactory factory;
	factory.disableRememberAttributes = disableRememberAttributes;
	UIAttributes a;
	a.setAttribute (UIViewCreator::kAttrClass, viewName);
	a.setAttribute (attrName, attrValue);

	auto v = owned (factory.createView (a, desc));
	auto view = v.cast<ViewClass> ();
	EXPECT(view);
	EXPECT(proc (view));

	UIAttributes a2;
	factory.getAttributesForView (view, desc, a2);
	auto str = a2.getAttributeValue (attrName);
	EXPECT(str);
	EXPECT(*str == attrValue);
}

template<typename ViewClass, typename Proc>
void testAttribute (const IdStringPtr viewName, const std::string& attrName, int32_t attrValue, IUIDescription* desc, const Proc& proc)
{
	UIViewFactory factory;
	UIAttributes a;
	a.setAttribute (UIViewCreator::kAttrClass, viewName);
	a.setIntegerAttribute (attrName, attrValue);

	auto v = owned (factory.createView (a, desc));
	auto view = v.cast<ViewClass> ();
	EXPECT(view);
	EXPECT(proc (view));

	UIAttributes a2;
	factory.getAttributesForView (view, desc, a2);
	int32_t value;
	a2.getIntegerAttribute (attrName, value);
	EXPECT(value == attrValue);
}

template<typename ViewClass, typename Proc>
void testAttribute (const IdStringPtr viewName, const std::string& attrName, bool attrValue, IUIDescription* desc, const Proc& proc)
{
	UIViewFactory factory;
	UIAttributes a;
	a.setAttribute (UIViewCreator::kAttrClass, viewName);
	a.setBooleanAttribute (attrName, attrValue);

	auto v = owned (factory.createView (a, desc));
	auto view = v.cast<ViewClass> ();
	EXPECT(view);
	EXPECT(proc (view));

	UIAttributes a2;
	factory.getAttributesForView (view, desc, a2);
	bool value;
	a2.getBooleanAttribute (attrName, value);
	EXPECT(value == attrValue);
}

template<typename ViewClass, typename Proc>
void testAttribute (const IdStringPtr viewName, const std::string& attrName, double attrValue, IUIDescription* desc, const Proc& proc)
{
	UIViewFactory factory;
	UIAttributes a;
	a.setAttribute (UIViewCreator::kAttrClass, viewName);
	a.setDoubleAttribute (attrName, attrValue);

	auto v = owned (factory.createView (a, desc));
	auto view = v.cast<ViewClass> ();
	EXPECT(view);
	EXPECT(proc (view));

	UIAttributes a2;
	factory.getAttributesForView (view, desc, a2);
	double value;
	a2.getDoubleAttribute (attrName, value);
	EXPECT(value == attrValue);
}

template<typename ViewClass, typename Proc>
void testAttribute (const IdStringPtr viewName, const std::string& attrName, const CRect& attrValue, IUIDescription* desc, const Proc& proc)
{
	UIViewFactory factory;
	UIAttributes a;
	a.setAttribute (UIViewCreator::kAttrClass, viewName);
	a.setRectAttribute (attrName, attrValue);

	auto v = owned (factory.createView (a, desc));
	auto view = v.cast<ViewClass> ();
	EXPECT(view);
	EXPECT(proc (view));

	UIAttributes a2;
	factory.getAttributesForView (view, desc, a2);
	CRect value;
	a2.getRectAttribute (attrName, value);
	EXPECT(value == attrValue);
}

template<typename ViewClass, typename Proc>
void testAttribute (const IdStringPtr viewName, const std::string& attrName, const CPoint& attrValue, IUIDescription* desc, const Proc& proc)
{
	UIViewFactory factory;
	UIAttributes a;
	a.setAttribute (UIViewCreator::kAttrClass, viewName);
	a.setPointAttribute (attrName, attrValue);

	auto v = owned (factory.createView (a, desc));
	auto view = v.cast<ViewClass> ();
	EXPECT(view);
	EXPECT(proc (view));

	UIAttributes a2;
	factory.getAttributesForView (view, desc, a2);
	CPoint value;
	a2.getPointAttribute (attrName, value);
	EXPECT(value == attrValue);
}

} // VSTGUI
