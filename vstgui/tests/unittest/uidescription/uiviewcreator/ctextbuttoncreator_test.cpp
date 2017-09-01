// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
		auto defTB = owned (new CTextButton (CRect (0, 0, 100, 20), nullptr, -1, ""));
		
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
