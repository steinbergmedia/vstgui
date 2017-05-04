// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../unittests.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../lib/cgradientview.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(CGradientViewCreatorTest,

	TEST(frameColor,
		DummyUIDescription uidesc;
		testAttribute<CGradientView>(kCGradientView, kAttrFrameColor, kColorName, &uidesc, [&] (CGradientView* v) {
			return v->getFrameColor () == uidesc.color;
		});
	);

	TEST(gradientAngle,
		DummyUIDescription uidesc;
		testAttribute<CGradientView>(kCGradientView, kAttrGradientAngle, 5., &uidesc, [&] (CGradientView* v) {
			return v->getGradientAngle() == 5.;
		});
	);

	TEST(roundRectRadius,
		DummyUIDescription uidesc;
		testAttribute<CGradientView>(kCGradientView, kAttrRoundRectRadius, 35., &uidesc, [&] (CGradientView* v) {
			return v->getRoundRectRadius() == 35.;
		});
	);

	TEST(frameWidth,
		DummyUIDescription uidesc;
		testAttribute<CGradientView>(kCGradientView, kAttrFrameWidth, 5., &uidesc, [&] (CGradientView* v) {
			return v->getFrameWidth() == 5.;
		});
	);

	TEST(drawAntialiased,
		DummyUIDescription uidesc;
		testAttribute<CGradientView>(kCGradientView, kAttrDrawAntialiased, true, &uidesc, [&] (CGradientView* v) {
			return v->getDrawAntialised();
		});
		testAttribute<CGradientView>(kCGradientView, kAttrDrawAntialiased, false, &uidesc, [&] (CGradientView* v) {
			return v->getDrawAntialised() == false;
		});
	);

	TEST(gradientStyle,
		DummyUIDescription uidesc;
		testAttribute<CGradientView>(kCGradientView, kAttrGradientStyle, "radial", &uidesc, [&] (CGradientView* v) {
			return v->getGradientStyle() == CGradientView::kRadialGradient;
		});
		testAttribute<CGradientView>(kCGradientView, kAttrGradientStyle, "linear", &uidesc, [&] (CGradientView* v) {
			return v->getGradientStyle() == CGradientView::kLinearGradient;
		});
	);

	TEST(radialCenter,
		DummyUIDescription uidesc;
		CPoint p (20, 20);
		testAttribute<CGradientView>(kCGradientView, kAttrRadialCenter, p, &uidesc, [&] (CGradientView* v) {
			return v->getRadialCenter() == p;
		});
	);

	TEST(radialRadius,
		DummyUIDescription uidesc;
		testAttribute<CGradientView>(kCGradientView, kAttrRadialRadius, 25., &uidesc, [&] (CGradientView* v) {
			return v->getRadialRadius() == 25.;
		});
	);

	TEST(gradient,
		DummyUIDescription uidesc;
		testAttribute<CGradientView>(kCGradientView, kAttrGradient, kGradientName, &uidesc, [&] (CGradientView* v) {
			return v->getGradient() == uidesc.gradient;
		});
	);

	TEST(gradientStyleValues,
		DummyUIDescription uidesc;
		testPossibleValues(kCGradientView, kAttrGradientStyle, &uidesc, {"radial", "linear"});
	);

	TEST(gradientAngleMinMax,
		DummyUIDescription uidesc;
		testMinMaxValues(kCGradientView, kAttrGradientAngle, &uidesc, 0., 360.);
	);

	TEST(legacyGradient,
		DummyUIDescription uidesc;
		UIViewFactory factory;
		UIAttributes a;
		a.setAttribute (kAttrClass, kCGradientView);
		a.setAttribute (kAttrGradientStartColor, kColorName);

		auto v = owned (factory.createView (a, &uidesc));
		auto view = v.cast<CGradientView> ();
		EXPECT(view);
		EXPECT(view->getGradient () == nullptr);

		a.setAttribute (kAttrGradientEndColor, kColorName);

		v = owned (factory.createView (a, &uidesc));
		view = v.cast<CGradientView> ();
		EXPECT(view);
		EXPECT(view->getGradient () == nullptr);

		a.setDoubleAttribute (kAttrGradientStartColorOffset, 0.);

		v = owned (factory.createView (a, &uidesc));
		view = v.cast<CGradientView> ();
		EXPECT(view);
		EXPECT(view->getGradient () == nullptr);

		a.setDoubleAttribute (kAttrGradientEndColorOffset, 1.);

		v = owned (factory.createView (a, &uidesc));
		view = v.cast<CGradientView> ();
		EXPECT(view);
		EXPECT(view->getGradient ());

	);
);

} // VSTGUI
