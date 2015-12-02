//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

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
