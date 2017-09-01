// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../unittests.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../lib/cview.h"
#include "../../../../lib/cbitmap.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

namespace {

bool getViewAttributeString (CView* view, const CViewAttributeID attrID, std::string& value)
{
	uint32_t attrSize = 0;
	if (view->getAttributeSize (attrID, attrSize))
	{
		char* cstr = new char [attrSize+1];
		if (view->getAttribute (attrID, attrSize, cstr, attrSize))
			value = cstr;
		else
			value = "";
		delete [] cstr;
		return true;
	}
	return false;
}

} // anonymous

TESTCASE(CViewCreatorTest,
	
	TEST(origin,
		CPoint origin (20, 20);
		testAttribute<CView>(kCView, kAttrOrigin, origin, nullptr, [&] (CView* v) {
			return v->getViewSize().getTopLeft() == origin;
		});
	);

	TEST(size,
		CPoint size (20, 20);
		testAttribute<CView>(kCView, kAttrSize, size, nullptr, [&] (CView* v) {
			return v->getViewSize().getSize() == size;
		});
	);

	TEST(bitmap,
		DummyUIDescription uiDesc;
		testAttribute<CView>(kCView, kAttrBitmap, kBitmapName, &uiDesc, [&] (CView* v) {
			return v->getBackground() == uiDesc.bitmap;
		});
	);

	TEST(disabledBitmap,
		DummyUIDescription uiDesc;
		testAttribute<CView>(kCView, kAttrDisabledBitmap, kBitmapName, &uiDesc, [&] (CView* v) {
			return v->getDisabledBackground() == uiDesc.bitmap;
		});
	);

	TEST(transparent,
		testAttribute<CView>(kCView, kAttrTransparent, true, nullptr, [&] (CView* v) {
			return v->getTransparency ();
		});
		testAttribute<CView>(kCView, kAttrTransparent, false, nullptr, [&] (CView* v) {
			return v->getTransparency () == false;
		});
	);

	TEST(mouseEnabled,
		testAttribute<CView>(kCView, kAttrMouseEnabled, true, nullptr, [&] (CView* v) {
			return v->getMouseEnabled ();
		});
		testAttribute<CView>(kCView, kAttrMouseEnabled, false, nullptr, [&] (CView* v) {
			return v->getMouseEnabled () == false;
		});
	);

	TEST(autosize,
		testAttribute<CView>(kCView, kAttrAutosize, "left ", nullptr, [&] (CView* v) {
			return v->getAutosizeFlags () & kAutosizeLeft;
		});
		testAttribute<CView>(kCView, kAttrAutosize, "top ", nullptr, [&] (CView* v) {
			return v->getAutosizeFlags () & kAutosizeTop;
		});
		testAttribute<CView>(kCView, kAttrAutosize, "right ", nullptr, [&] (CView* v) {
			return v->getAutosizeFlags () & kAutosizeRight;
		});
		testAttribute<CView>(kCView, kAttrAutosize, "bottom ", nullptr, [&] (CView* v) {
			return v->getAutosizeFlags () & kAutosizeBottom;
		});
		testAttribute<CView>(kCView, kAttrAutosize, "row ", nullptr, [&] (CView* v) {
			return v->getAutosizeFlags () & kAutosizeRow;
		});
		testAttribute<CView>(kCView, kAttrAutosize, "column ", nullptr, [&] (CView* v) {
			return v->getAutosizeFlags () & kAutosizeColumn;
		});
	);

	TEST(tooltip,
		std::string tooltipStr = "This is a tooltip";
		testAttribute<CView>(kCView, kAttrTooltip, tooltipStr.c_str (), nullptr, [&] (CView* v) {
			std::string str;
			EXPECT(getViewAttributeString (v, kCViewTooltipAttribute, str));
			return str == tooltipStr;
		});

		UIViewFactory factory;
		UIAttributes a;
		a.setAttribute (kAttrClass, kCView);
		a.setAttribute (kAttrTooltip, "");
		
		auto view = owned (factory.createView (a, nullptr));
		std::string str;
		EXPECT(getViewAttributeString (view, kCViewTooltipAttribute, str) == false);
	);

	TEST(customViewName,
		std::string customViewName = "CustomView";
		testAttribute<CView>(kCView, kAttrCustomViewName, customViewName.c_str (), nullptr, [&] (CView* v) {
			std::string str;
			EXPECT(getViewAttributeString (v, 'uicv', str));
			return str == customViewName;
		});
	);

	TEST(subControllerName,
		std::string subControllerName = "SubController";
		testAttribute<CView>(kCView, kAttrSubController, subControllerName.c_str (), nullptr, [&] (CView* v) {
			std::string str;
			EXPECT(getViewAttributeString (v, 'uisc', str));
			return str == subControllerName;
		});
	);

	TEST(opacity,
		testAttribute<CView>(kCView, kAttrOpacity, 0.5, nullptr, [&] (CView* v) {
			return v->getAlphaValue() == 0.5;
		});
	);

	TEST(opacityValueRange,
		testMinMaxValues(kCView, kAttrOpacity, nullptr, 0., 1.);
	);

);

} // VSTGUI
