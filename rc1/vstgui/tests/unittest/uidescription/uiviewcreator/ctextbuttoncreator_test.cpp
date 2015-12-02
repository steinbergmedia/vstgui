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
#include "../../../../lib/controls/cbuttons.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(CTextButtonCreatorTest,

	TEST(title,
		DummyUIDescription uidesc;
		UTF8String title ("Title");
		testAttribute<CTextButton>(kCTextButton, kAttrTitle, title, &uidesc, [&] (CTextButton* v) {
			return v->getTitle () == title;
		});
	);
	
	TEST(font,
		DummyUIDescription uidesc;
		testAttribute<CTextButton>(kCTextButton, kAttrFont, kFontName, &uidesc, [&] (CTextButton* v) {
			return v->getFont () == uidesc.font;
		}, true);
	);

	TEST(textColor,
		DummyUIDescription uidesc;
		testAttribute<CTextButton>(kCTextButton, kAttrTextColor, kColorName, &uidesc, [&] (CTextButton* v) {
			return v->getTextColor () == uidesc.color;
		});
	);

	TEST(textColorHighlighted,
		DummyUIDescription uidesc;
		testAttribute<CTextButton>(kCTextButton, kAttrTextColorHighlighted, kColorName, &uidesc, [&] (CTextButton* v) {
			return v->getTextColorHighlighted () == uidesc.color;
		});
	);

	TEST(frameColor,
		DummyUIDescription uidesc;
		testAttribute<CTextButton>(kCTextButton, kAttrFrameColor, kColorName, &uidesc, [&] (CTextButton* v) {
			return v->getFrameColor () == uidesc.color;
		});
	);

	TEST(frameColorHighlighted,
		DummyUIDescription uidesc;
		testAttribute<CTextButton>(kCTextButton, kAttrFrameColorHighlighted, kColorName, &uidesc, [&] (CTextButton* v) {
			return v->getFrameColorHighlighted () == uidesc.color;
		});
	);

	TEST(frameWidth,
		DummyUIDescription uidesc;
		testAttribute<CTextButton>(kCTextButton, kAttrFrameWidth, 5., &uidesc, [&] (CTextButton* v) {
			return v->getFrameWidth () == 5.;
		});
	);

	TEST(roundRadius,
		DummyUIDescription uidesc;
		testAttribute<CTextButton>(kCTextButton, kAttrRoundRadius, 5., &uidesc, [&] (CTextButton* v) {
			return v->getRoundRadius () == 5.;
		});
	);

	TEST(iconTextMargin,
		DummyUIDescription uidesc;
		testAttribute<CTextButton>(kCTextButton, kAttrIconTextMargin, 5., &uidesc, [&] (CTextButton* v) {
			return v->getTextMargin () == 5.;
		});
	);

	TEST(kickStyle,
		DummyUIDescription uidesc;
		testAttribute<CTextButton>(kCTextButton, kAttrKickStyle, true, &uidesc, [&] (CTextButton* v) {
			return v->getStyle() == CTextButton::kKickStyle;
		});
		testAttribute<CTextButton>(kCTextButton, kAttrKickStyle, false, &uidesc, [&] (CTextButton* v) {
			return v->getStyle() == CTextButton::kOnOffStyle;
		});
	);

	TEST(icon,
		DummyUIDescription uidesc;
		testAttribute<CTextButton>(kCTextButton, kAttrIcon, kBitmapName, &uidesc, [&] (CTextButton* v) {
			return v->getIcon () == uidesc.bitmap;
		});
	);

	TEST(iconHighlighted,
		DummyUIDescription uidesc;
		testAttribute<CTextButton>(kCTextButton, kAttrIconHighlighted, kBitmapName, &uidesc, [&] (CTextButton* v) {
			return v->getIconHighlighted () == uidesc.bitmap;
		});
	);

	TEST(iconPosition,
		DummyUIDescription uidesc;
		testAttribute<CTextButton>(kCTextButton, kAttrIconPosition, "left", &uidesc, [&] (CTextButton* v) {
			return v->getIconPosition () == CDrawMethods::kIconLeft;
		});
		testAttribute<CTextButton>(kCTextButton, kAttrIconPosition, "right", &uidesc, [&] (CTextButton* v) {
			return v->getIconPosition () == CDrawMethods::kIconRight;
		});
		testAttribute<CTextButton>(kCTextButton, kAttrIconPosition, "center above text", &uidesc, [&] (CTextButton* v) {
			return v->getIconPosition () == CDrawMethods::kIconCenterAbove;
		});
		testAttribute<CTextButton>(kCTextButton, kAttrIconPosition, "center below text", &uidesc, [&] (CTextButton* v) {
			return v->getIconPosition () == CDrawMethods::kIconCenterBelow;
		});
	);

	TEST(textAlignment,
		DummyUIDescription uiDesc;
		testAttribute<CTextButton>(kCTextButton, kAttrTextAlignment, "left", &uiDesc, [&] (CTextButton* v) {
			return v->getTextAlignment () == kLeftText;
		});
		testAttribute<CTextButton>(kCTextButton, kAttrTextAlignment, "center", &uiDesc, [&] (CTextButton* v) {
			return v->getTextAlignment () == kCenterText;
		});
		testAttribute<CTextButton>(kCTextButton, kAttrTextAlignment, "right", &uiDesc, [&] (CTextButton* v) {
			return v->getTextAlignment () == kRightText;
		});
	);

	TEST(gradient,
		DummyUIDescription uidesc;
		testAttribute<CTextButton>(kCTextButton, kAttrGradient, kGradientName, &uidesc, [&] (CTextButton* v) {
			return v->getGradient () == uidesc.gradient;
		});
	);

	TEST(gradientHighlighted,
		DummyUIDescription uidesc;
		testAttribute<CTextButton>(kCTextButton, kAttrGradientHighlighted, kGradientName, &uidesc, [&] (CTextButton* v) {
			return v->getGradientHighlighted () == uidesc.gradient;
		});
	);
	
	TEST(iconPositionValues,
		DummyUIDescription uidesc;
		testPossibleValues(kCTextButton, kAttrIconPosition, &uidesc, {"left", "right", "center above text", "center below text"});
	);

	TEST(legacyGradient,
		auto defTB = owned (new CTextButton (CRect (0, 0, 100, 20), 0, -1, ""));
		
		DummyUIDescription uidesc;
		UIViewFactory factory;
		UIAttributes a;
		a.setAttribute (kAttrClass, kCTextButton);
		a.setAttribute (kAttrGradientStartColor, kColorName);

		auto v = owned (factory.createView (a, &uidesc));
		auto view = v.cast<CTextButton> ();
		EXPECT(view);
		EXPECT(*view->getGradient () == *defTB->getGradient ());
		EXPECT(*view->getGradientHighlighted () == *defTB->getGradientHighlighted ());

		a.setAttribute (kAttrGradientStartColorHighlighted, kColorName);

		v = owned (factory.createView (a, &uidesc));
		view = v.cast<CTextButton> ();
		EXPECT(view);
		EXPECT(*view->getGradient () == *defTB->getGradient ());
		EXPECT(*view->getGradientHighlighted () == *defTB->getGradientHighlighted ());

		a.setAttribute (kAttrGradientEndColor, kColorName);

		v = owned (factory.createView (a, &uidesc));
		view = v.cast<CTextButton> ();
		EXPECT(view);
		EXPECT(*view->getGradient () == *defTB->getGradient ());
		EXPECT(*view->getGradientHighlighted () == *defTB->getGradientHighlighted ());

		a.setAttribute (kAttrGradientEndColorHighlighted, kColorName);

		v = owned (factory.createView (a, &uidesc));
		view = v.cast<CTextButton> ();
		EXPECT(view);
		EXPECT(*view->getGradient () != *defTB->getGradient ());
		EXPECT(*view->getGradientHighlighted () != *defTB->getGradientHighlighted ());

		UIAttributes a2;
		factory.getAttributesForView (view, &uidesc, a2);
		auto str = a2.getAttributeValue (kAttrGradient);
		EXPECT(str);
		str = a2.getAttributeValue (kAttrGradientHighlighted);
		EXPECT(str);
	);
);

} // VSTGUI
