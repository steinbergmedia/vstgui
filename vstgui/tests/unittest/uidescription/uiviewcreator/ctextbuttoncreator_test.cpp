// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cbuttons.h"
#include "../../../../lib/cstring.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../unittests.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TEST_CASE (CTextButtonCreatorTest, Title)
{
	DummyUIDescription uidesc;
	UTF8String title ("Title");
	testAttribute<CTextButton> (kCTextButton, kAttrTitle, title, &uidesc,
	                            [&] (CTextButton* v) { return v->getTitle () == title; });
}

TEST_CASE (CTextButtonCreatorTest, Font)
{
	DummyUIDescription uidesc;
	testAttribute<CTextButton> (kCTextButton, kAttrFont, kFontName, &uidesc,
	                            [&] (CTextButton* v) { return v->getFont () == uidesc.font; },
	                            true);
}

TEST_CASE (CTextButtonCreatorTest, TextColor)
{
	DummyUIDescription uidesc;
	testAttribute<CTextButton> (
	    kCTextButton, kAttrTextColor, kColorName, &uidesc,
	    [&] (CTextButton* v) { return v->getTextColor () == uidesc.color; });
}

TEST_CASE (CTextButtonCreatorTest, TextColorHighlighted)
{
	DummyUIDescription uidesc;
	testAttribute<CTextButton> (
	    kCTextButton, kAttrTextColorHighlighted, kColorName, &uidesc,
	    [&] (CTextButton* v) { return v->getTextColorHighlighted () == uidesc.color; });
}

TEST_CASE (CTextButtonCreatorTest, FrameColor)
{
	DummyUIDescription uidesc;
	testAttribute<CTextButton> (
	    kCTextButton, kAttrFrameColor, kColorName, &uidesc,
	    [&] (CTextButton* v) { return v->getFrameColor () == uidesc.color; });
}

TEST_CASE (CTextButtonCreatorTest, FrameColorHighlighted)
{
	DummyUIDescription uidesc;
	testAttribute<CTextButton> (
	    kCTextButton, kAttrFrameColorHighlighted, kColorName, &uidesc,
	    [&] (CTextButton* v) { return v->getFrameColorHighlighted () == uidesc.color; });
}

TEST_CASE (CTextButtonCreatorTest, FrameWidth)
{
	DummyUIDescription uidesc;
	testAttribute<CTextButton> (kCTextButton, kAttrFrameWidth, 5., &uidesc,
	                            [&] (CTextButton* v) { return v->getFrameWidth () == 5.; });
}

TEST_CASE (CTextButtonCreatorTest, RoundRadius)
{
	DummyUIDescription uidesc;
	testAttribute<CTextButton> (kCTextButton, kAttrRoundRadius, 5., &uidesc,
	                            [&] (CTextButton* v) { return v->getRoundRadius () == 5.; });
}

TEST_CASE (CTextButtonCreatorTest, IconTextMargin)
{
	DummyUIDescription uidesc;
	testAttribute<CTextButton> (kCTextButton, kAttrIconTextMargin, 5., &uidesc,
	                            [&] (CTextButton* v) { return v->getTextMargin () == 5.; });
}

TEST_CASE (CTextButtonCreatorTest, KickStyle)
{
	DummyUIDescription uidesc;
	testAttribute<CTextButton> (kCTextButton, kAttrKickStyle, true, &uidesc, [&] (CTextButton* v) {
		return v->getStyle () == CTextButton::kKickStyle;
	});
	testAttribute<CTextButton> (kCTextButton, kAttrKickStyle, false, &uidesc, [&] (CTextButton* v) {
		return v->getStyle () == CTextButton::kOnOffStyle;
	});
}

TEST_CASE (CTextButtonCreatorTest, Icon)
{
	DummyUIDescription uidesc;
	testAttribute<CTextButton> (kCTextButton, kAttrIcon, kBitmapName, &uidesc,
	                            [&] (CTextButton* v) { return v->getIcon () == uidesc.bitmap; });
}

TEST_CASE (CTextButtonCreatorTest, IconHighlighted)
{
	DummyUIDescription uidesc;
	testAttribute<CTextButton> (
	    kCTextButton, kAttrIconHighlighted, kBitmapName, &uidesc,
	    [&] (CTextButton* v) { return v->getIconHighlighted () == uidesc.bitmap; });
}

TEST_CASE (CTextButtonCreatorTest, IconPosition)
{
	DummyUIDescription uidesc;
	testAttribute<CTextButton> (
	    kCTextButton, kAttrIconPosition, "left", &uidesc,
	    [&] (CTextButton* v) { return v->getIconPosition () == CDrawMethods::kIconLeft; });
	testAttribute<CTextButton> (
	    kCTextButton, kAttrIconPosition, "right", &uidesc,
	    [&] (CTextButton* v) { return v->getIconPosition () == CDrawMethods::kIconRight; });
	testAttribute<CTextButton> (
	    kCTextButton, kAttrIconPosition, "center above text", &uidesc,
	    [&] (CTextButton* v) { return v->getIconPosition () == CDrawMethods::kIconCenterAbove; });
	testAttribute<CTextButton> (
	    kCTextButton, kAttrIconPosition, "center below text", &uidesc,
	    [&] (CTextButton* v) { return v->getIconPosition () == CDrawMethods::kIconCenterBelow; });
}

TEST_CASE (CTextButtonCreatorTest, TextAlignment)
{
	DummyUIDescription uiDesc;
	testAttribute<CTextButton> (
	    kCTextButton, kAttrTextAlignment, "left", &uiDesc,
	    [&] (CTextButton* v) { return v->getTextAlignment () == kLeftText; });
	testAttribute<CTextButton> (
	    kCTextButton, kAttrTextAlignment, "center", &uiDesc,
	    [&] (CTextButton* v) { return v->getTextAlignment () == kCenterText; });
	testAttribute<CTextButton> (
	    kCTextButton, kAttrTextAlignment, "right", &uiDesc,
	    [&] (CTextButton* v) { return v->getTextAlignment () == kRightText; });
}

TEST_CASE (CTextButtonCreatorTest, Gradient)
{
	DummyUIDescription uidesc;
	testAttribute<CTextButton> (
	    kCTextButton, kAttrGradient, kGradientName, &uidesc,
	    [&] (CTextButton* v) { return v->getGradient () == uidesc.gradient; });
}

TEST_CASE (CTextButtonCreatorTest, GradientHighlighted)
{
	DummyUIDescription uidesc;
	testAttribute<CTextButton> (
	    kCTextButton, kAttrGradientHighlighted, kGradientName, &uidesc,
	    [&] (CTextButton* v) { return v->getGradientHighlighted () == uidesc.gradient; });
}

TEST_CASE (CTextButtonCreatorTest, IconPositionValues)
{
	DummyUIDescription uidesc;
	testPossibleValues (kCTextButton, kAttrIconPosition, &uidesc,
	                    {"left", "right", "center above text", "center below text"});
}

TEST_CASE (CTextButtonCreatorTest, LegacyGradient)
{
	auto defTB = owned (new CTextButton (CRect (0, 0, 100, 20), nullptr, -1, ""));

	DummyUIDescription uidesc;
	UIViewFactory factory;
	UIAttributes a;
	a.setAttribute (kAttrClass, kCTextButton);
	a.setAttribute (kAttrGradientStartColor, kColorName);

	auto v = owned (factory.createView (a, &uidesc));
	auto view = v.cast<CTextButton> ();
	EXPECT (view);
	EXPECT (*view->getGradient () == *defTB->getGradient ());
	EXPECT (*view->getGradientHighlighted () == *defTB->getGradientHighlighted ());

	a.setAttribute (kAttrGradientStartColorHighlighted, kColorName);

	v = owned (factory.createView (a, &uidesc));
	view = v.cast<CTextButton> ();
	EXPECT (view);
	EXPECT (*view->getGradient () == *defTB->getGradient ());
	EXPECT (*view->getGradientHighlighted () == *defTB->getGradientHighlighted ());

	a.setAttribute (kAttrGradientEndColor, kColorName);

	v = owned (factory.createView (a, &uidesc));
	view = v.cast<CTextButton> ();
	EXPECT (view);
	EXPECT (*view->getGradient () == *defTB->getGradient ());
	EXPECT (*view->getGradientHighlighted () == *defTB->getGradientHighlighted ());

	a.setAttribute (kAttrGradientEndColorHighlighted, kColorName);

	v = owned (factory.createView (a, &uidesc));
	view = v.cast<CTextButton> ();
	EXPECT (view);
	EXPECT (*view->getGradient () != *defTB->getGradient ());
	EXPECT (*view->getGradientHighlighted () != *defTB->getGradientHighlighted ());

	UIAttributes a2;
	factory.getAttributesForView (view, &uidesc, a2);
	auto str = a2.getAttributeValue (kAttrGradient);
	EXPECT (str);
	str = a2.getAttributeValue (kAttrGradientHighlighted);
	EXPECT (str);
}

} // VSTGUI
