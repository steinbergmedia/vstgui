//
//  uidescriptionadapter.h
//  vstgui
//
//  Created by Arne Scheffler on 30/10/15.
//
//

#pragma once

#include "../../../../uidescription/iuidescription.h"
#include "../../../../lib/cbitmap.h"
#include "../../../../lib/cgradient.h"

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

constexpr IdStringPtr kColorName = "MyColor";
constexpr IdStringPtr kFontName = "MyFont";
constexpr IdStringPtr kBitmapName = "MyBitmap";
constexpr IdStringPtr kGradientName = "MyGradient";
constexpr IdStringPtr kTagName = "tagname";

class DummyUIDescription : public UIDescriptionAdapter
{
public:
	bool getColor (UTF8StringPtr name, CColor& color) const override
	{
		if (UTF8StringView(name) == kColorName)
		{
			color = this->color;
			return true;
		}
		return false;
	}

	UTF8StringPtr lookupColorName (const CColor& color) const override
	{
		if (this->color == color)
			return kColorName;
		return nullptr;
	}

	CFontRef getFont (UTF8StringPtr name) const override
	{
		if (UTF8StringView(name) == kFontName)
			return font;
		return nullptr;
	}
	
	UTF8StringPtr lookupFontName (const CFontRef font) const override
	{
		if (font == this->font)
			return kFontName;
		return nullptr;
	}

	CBitmap* getBitmap (UTF8StringPtr name) const override
	{
		if (UTF8StringView (name) == kBitmapName)
			return bitmap;
		return nullptr;
	}

	UTF8StringPtr lookupBitmapName (const CBitmap* inBitmap) const override
	{
		if (inBitmap == bitmap)
			return kBitmapName;
		return nullptr;
	}

	CGradient* getGradient (UTF8StringPtr name) const override
	{
		if (UTF8StringView(name) == kGradientName)
			return gradient;
		return nullptr;
	}
	
	UTF8StringPtr lookupGradientName (const CGradient* gradient) const override
	{
		if (gradient == this->gradient)
			return kGradientName;
		return nullptr;
	}

	int32_t getTagForName (UTF8StringPtr name) const override
	{
		if (UTF8StringView (name) == kTagName)
			return tag;
		return -1;
	}
	UTF8StringPtr lookupControlTagName (const int32_t tag) const override
	{
		if (this->tag != -1 && tag == this->tag)
			return kTagName;
		return nullptr;
	}
	IControlListener* getControlListener (UTF8StringPtr name) const override
	{
		if (UTF8StringView (name) == kTagName)
			return listener;
		return nullptr;
	}

	int32_t tag {-1};
	CColor color {20, 30, 50, 255};
	SharedPointer<CFontDesc> font = owned (new CFontDesc ("Arial", 12));
	SharedPointer<CBitmap> bitmap = owned (new CBitmap (1, 1));
	SharedPointer<CGradient> gradient = owned (CGradient::create(0, 1, kBlackCColor, kWhiteCColor));
	IControlListener* listener {nullptr};
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

inline void testMinMaxValues (const IdStringPtr className, const std::string& attrName, IUIDescription* desc, double minValue, double maxValue)
{
	UIViewFactory factory;
	UIAttributes a;
	a.setAttribute (UIViewCreator::kAttrClass, className);
	auto view = owned (factory.createView (a, desc));
	double min, max;
	EXPECT (factory.getAttributeValueRange(view, attrName, min, max));
	EXPECT(min == minValue);
	EXPECT(max == maxValue);
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
