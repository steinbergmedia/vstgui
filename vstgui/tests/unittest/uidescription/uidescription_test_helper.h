// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../../uidescription/icontroller.h"
#include "../../../uidescription/uidescription.h"
#include "../../../uidescription/uidescriptionlistener.h"
#include "../unittests.h"

namespace VSTGUI {
namespace UIDescriptionTesting {

struct SaveUIDescription : public UIDescription
{
	SaveUIDescription (IContentProvider* xmlContentProvider) : UIDescription (xmlContentProvider) {}

	using UIDescription::saveToStream;
};

struct Controller : public IController
{
	void valueChanged (CControl* pControl) override {};
	int32_t getTagForName (UTF8StringPtr name, int32_t registeredTag) const override
	{
		return registeredTag;
	}
	IControlListener* getControlListener (UTF8StringPtr controlTagName) override { return this; }
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override
	{
		return nullptr;
	}
	CView* verifyView (CView* view, const UIAttributes& attributes,
	                   const IUIDescription* description) override
	{
		return view;
	}
	IController* createSubController (UTF8StringPtr name,
	                                  const IUIDescription* description) override
	{
		return nullptr;
	}
};

enum class UIDescTestCase
{
	TagChanged,
	ColorChanged,
	FontChanged,
	BitmapChanged,
	TemplateChanged,
	GradientChanged,
	BeforeSave
};

//-----------------------------------------------------------------------------
class DescriptionListenerMock : public UIDescriptionListenerAdapter
{
public:
	DescriptionListenerMock (UIDescTestCase tc) { setTestCase (tc); }

	void setTestCase (UIDescTestCase tc)
	{
		testCase = tc;
		called = 0u;
	}

	uint32_t callCount () const { return called; }

	bool doUIDescTemplateUpdate (UIDescription* desc, UTF8StringPtr name) override
	{
		EXPECT (false);
		return true;
	}
	void onUIDescTagChanged (UIDescription* desc) override
	{
		++called;
		EXPECT (testCase == UIDescTestCase::TagChanged)
	}
	void onUIDescColorChanged (UIDescription* desc) override
	{
		++called;
		EXPECT (testCase == UIDescTestCase::ColorChanged)
	}
	void onUIDescFontChanged (UIDescription* desc) override
	{
		++called;
		EXPECT (testCase == UIDescTestCase::FontChanged)
	}
	void onUIDescBitmapChanged (UIDescription* desc) override
	{
		++called;
		EXPECT (testCase == UIDescTestCase::BitmapChanged)
	}
	void onUIDescTemplateChanged (UIDescription* desc) override
	{
		++called;
		EXPECT (testCase == UIDescTestCase::TemplateChanged)
	}
	void onUIDescGradientChanged (UIDescription* desc) override
	{
		++called;
		EXPECT (testCase == UIDescTestCase::GradientChanged)
	}
	void beforeUIDescSave (UIDescription* desc) override
	{
		++called;
		EXPECT (testCase == UIDescTestCase::BeforeSave)
	}

private:
	UIDescTestCase testCase;
	uint32_t called;
};

} // UIDescriptionTesting
} // VSTGUI
