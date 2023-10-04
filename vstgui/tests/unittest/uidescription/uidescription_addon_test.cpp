// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../uidescription/uidescriptionaddonregistry.h"
#include "../../../uidescription/uidescription.h"
#include "../../../uidescription/uicontentprovider.h"
#include "../../../lib/cview.h"
#include "../unittests.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace {

//------------------------------------------------------------------------
constexpr auto createViewUIDesc = R"(
{
	"vstgui-ui-description": {
		"version": "1",
		"templates": {
			"view": {
				"attributes": {
					"background-color": "~ TransparentCColor",
					"background-color-draw-style": "filled and stroked",
					"class": "CViewContainer",
					"mouse-enabled": "true",
					"opacity": "1",
					"origin": "0, 0",
					"size": "400, 235",
					"transparent": "false"
				},
				"children": {
					"CView": {
						"attributes": {
							"class": "CView",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "4, 10",
							"size": "392, 40",
							"transparent": "false"
						}
					}
				}
			}
		}
	}
}
)";

} // anonymous

//------------------------------------------------------------------------
TEST_CASE (UIDescriptionAddOnTest, BasicFunctionality)
{
	struct BaseAddOn : UIDescriptionAddOnAdapter
	{
		void afterParsing (IUIDescription* desc) override { afterParsingCalled = true; }
		void beforeSaving (IUIDescription* desc) override { beforeSavingCalled = true; }
		void onDestroy (IUIDescription* desc) override { onDestroyCalled = true; }
		CreateTemplateViewFunc onCreateTemplateView (const IUIDescription* desc,
													 const CreateTemplateViewFunc& f) override
		{
			onCreateTemplateViewCalled = true;
			return f;
		}
		IViewFactory* getViewFactory (IUIDescription* desc, IViewFactory* of) override
		{
			getViewFactoryCalled = true;
			return of;
		};

		bool afterParsingCalled {false};
		bool beforeSavingCalled {false};
		bool onDestroyCalled {false};
		bool onCreateTemplateViewCalled {false};
		bool getViewFactoryCalled {false};
	};
	auto myAddOn = std::make_unique<BaseAddOn> ();
	auto myAddOnPtr = myAddOn.get ();
	auto token = UIDescriptionAddOnRegistry::add (std::move (myAddOn));
	{
		MemoryContentProvider provider (createViewUIDesc,
										static_cast<uint32_t> (strlen (createViewUIDesc)));
		UIDescription desc (&provider);
		EXPECT_TRUE (myAddOnPtr->getViewFactoryCalled);
		EXPECT_TRUE (desc.parse ());
		EXPECT_TRUE (myAddOnPtr->afterParsingCalled);
		auto view = desc.createView ("view", nullptr);
		EXPECT_NE (view, nullptr);
		EXPECT_TRUE (myAddOnPtr->onCreateTemplateViewCalled);
		view->forget ();
	}
	EXPECT_TRUE (myAddOnPtr->onDestroyCalled);

	UIDescriptionAddOnRegistry::remove (token);
}

//------------------------------------------------------------------------
} // VSTGUI
