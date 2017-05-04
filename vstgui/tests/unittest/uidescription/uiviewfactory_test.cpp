// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../unittests.h"
#include "../../../lib/cview.h"
#include "../../../lib/cviewcontainer.h"
#include "../../../uidescription/uiviewfactory.h"
#include "../../../uidescription/uiattributes.h"
#include "../../../uidescription/detail/uiviewcreatorattributes.h"
#include <algorithm>

namespace VSTGUI {
namespace {

class BaseView : public CView
{
public:
	BaseView () : CView (CRect (0, 0, 0, 0)) {}

	enum class State
	{
		kState1,
		kState2,
		kState3
	};
	State baseState {State::kState1};
};

class View : public BaseView
{
public:
	View () {}

	int32_t value {0};
};

class CustomView : public View
{
public:
	CustomView () {}
};

static std::string baseViewAttr ("BaseViewAttr");

struct BaseViewCreator : public ViewCreatorAdapter
{
	IdStringPtr getViewName () const override { return "BaseView"; }
	UTF8StringPtr getDisplayName () const override { return "Base View"; }
	IdStringPtr getBaseViewName () const override { return nullptr; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override
	{ return new BaseView (); }
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto v = dynamic_cast<BaseView*>(view);
		if (!v)
			return false;
		auto attr = attributes.getAttributeValue (baseViewAttr);
		if (attr)
		{
			if (*attr == "1")
				v->baseState = BaseView::State::kState1;
			if (*attr == "2")
				v->baseState = BaseView::State::kState2;
			if (*attr == "3")
				v->baseState = BaseView::State::kState3;
		}
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.push_back (baseViewAttr);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == baseViewAttr)
			return kListType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto v = dynamic_cast<BaseView*>(view);
		if (!v)
			return false;
		if (attributeName == baseViewAttr)
		{
			switch (v->baseState)
			{
				case BaseView::State::kState1: stringValue = "1"; break;
				case BaseView::State::kState2: stringValue = "2"; break;
				case BaseView::State::kState3: stringValue = "3"; break;
			}
			return true;
		}
		return false;
	}
	bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const override
	{
		if (attributeName != baseViewAttr)
			return false;
		static const std::string v1 = "1";
		static const std::string v2 = "2";
		static const std::string v3 = "3";
		values.push_back (&v1);
		values.push_back (&v2);
		values.push_back (&v3);
		return true;
	}
	bool getAttributeValueRange (const std::string& attributeName, double& minValue, double &maxValue) const override { return false; }
};
BaseViewCreator baseViewCreator;

static std::string viewAttr ("ViewAttr");

struct ViewCreator : public ViewCreatorAdapter
{
	IdStringPtr getViewName () const override { return "TestView"; }
	UTF8StringPtr getDisplayName () const override { return "Test View"; }
	IdStringPtr getBaseViewName () const override { return "BaseView"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override
	{ return new View (); }
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto v = dynamic_cast<View*>(view);
		if (!v)
			return false;
		int32_t value;
		if (attributes.getIntegerAttribute(viewAttr, value))
			v->value = value;
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.push_back (viewAttr);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == viewAttr)
			return kTagType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto v = dynamic_cast<View*>(view);
		if (!v)
			return false;
		if (attributeName == viewAttr)
		{
			stringValue = std::to_string (v->value);
			return true;
		}
		return false;
	}
	bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const override { return false; }
	bool getAttributeValueRange (const std::string& attributeName, double& minValue, double &maxValue) const override
	{
		if (attributeName != viewAttr)
			return false;
		minValue = -10;
		maxValue = 10;
		return true;
	}
};

ViewCreator viewCreator;

static SharedPointer<CView> createView (IViewFactory* factory)
{
	UIAttributes a;
	a.setAttribute (UIViewCreator::kAttrClass, viewCreator.getViewName ());
	return owned (factory->createView (a, nullptr));
}

} // anonymous

TESTCASE(UIViewFactoryTests,

	static SharedPointer<UIViewFactory> factory;
	
	SETUP(
		factory = owned (new UIViewFactory ());
		factory->registerViewCreator (baseViewCreator);
		factory->registerViewCreator (viewCreator);
	);

	TEARDOWN(
		factory->unregisterViewCreator (baseViewCreator);
		factory->unregisterViewCreator (viewCreator);
		factory = nullptr;
	);

	TEST(registerViewCreator,
		UIViewFactory::StringPtrList registeredViews;
		factory->collectRegisteredViewNames (registeredViews);
		auto found = std::find_if (registeredViews.begin(), registeredViews.end(), [&] (const std::string*& str) {
			return *str == viewCreator.getViewName ();
		});
		EXPECT(found != registeredViews.end ());
	);
	
	TEST(collectFilteredViewNames,
		UIViewFactory::StringPtrList registeredViews;
		factory->collectRegisteredViewNames (registeredViews, "BaseView");
		EXPECT(registeredViews.size() == 1);
	);
	
	TEST (collectRegisteredViewAndDisplayNames,
		auto list = factory->collectRegisteredViewAndDisplayNames ();
		auto it = std::find_if (list.begin (), list.end (), [] (const auto& value) {
			return *value.first == "BaseView";
		});
		EXPECT(it != list.end ());
		EXPECT(UTF8StringView (it->second) == UTF8StringView ("Base View"));
	);
	
	TEST(createView,
		auto v = createView (factory);
		EXPECT(v != nullptr);
		EXPECT(v.cast<View> () != nullptr);
	);
	
	TEST(createUnknownView,
		UIAttributes a;
		a.setAttribute (UIViewCreator::kAttrClass, "Unknown");
		auto view = owned (factory->createView (a, nullptr));
		EXPECT(view == nullptr);
	);
	
	TEST(applyAttributes,
		auto v = createView (factory);
		auto view = v.cast<View>();
		EXPECT(view->value == 0);
		UIAttributes a;
		a.setIntegerAttribute (viewAttr, 1);
		factory->applyAttributeValues (v, a, nullptr);
		EXPECT(view->value == 1);
	);

	TEST(getAttributeValue,
		auto v = createView (factory);
		std::string value;
		factory->getAttributeValue (v, viewAttr, value, nullptr);
		EXPECT(value == "0");
	);

	TEST(getAttributeNames,
		auto v = createView (factory);
		UIViewFactory::StringList attributeNames;
		EXPECT(factory->getAttributeNamesForView (v, attributeNames) == true);
		EXPECT(attributeNames.size () == 2);
		EXPECT(attributeNames.front() == viewAttr);
	);
	
	TEST(getAttributesForView,
		auto v = createView (factory);
		UIAttributes a;
		factory->getAttributesForView (v, nullptr, a);
		EXPECT(a.hasAttribute (viewAttr) == true);
		EXPECT(a.hasAttribute (baseViewAttr) == true);
	);

	TEST(getPossibleListValues,
		auto v = createView (factory);
		UIViewFactory::StringPtrList values;
		EXPECT(factory->getPossibleAttributeListValues (v, baseViewAttr, values) == true);
		EXPECT(values.size () == 3);
	);

	TEST(getAttributeValueRange,
		auto v = createView (factory);
		double minValue;
		double maxValue;
		EXPECT (factory->getAttributeValueRange (v, viewAttr, minValue, maxValue) == true);
		EXPECT(minValue == -10.);
		EXPECT(maxValue == 10.);
		EXPECT (factory->getAttributeValueRange (v, baseViewAttr, minValue, maxValue) == false);
	);
	
	TEST(defaultViewCreation,
		UIAttributes a;
		auto v = owned (factory->createView (a, nullptr));
		EXPECT(v.cast<CViewContainer> ());
	);

	TEST(applyCustomViewAttributes,
		auto view = owned (new CustomView ());
		UIAttributes a;
		a.setAttribute (baseViewAttr, "3");
		EXPECT(factory->applyCustomViewAttributeValues (view, "TestView", a, nullptr));
		EXPECT(view->baseState == BaseView::State::kState3);
	);
);

} // VSTGUI
