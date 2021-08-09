// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/cbitmap.h"
#include "../../../../lib/ccolor.h"
#include "../../../../lib/cpoint.h"
#include "../../../../lib/cstring.h"
#include "../../../../uidescription/uiviewcreator.h"
#include "../../unittests.h"
#include "../uidescriptionadapter.h"

namespace VSTGUI {
using namespace UIViewCreator;

TEST_CASE (UIViewCreatorTest, BitmapToString)
{
	UIDescriptionAdapter uidesc;
	std::string str;
	CBitmap bitmap (CResourceDescription ("test.png"));
	EXPECT (bitmapToString (&bitmap, str, &uidesc) == true);
	EXPECT (str == "test.png");
	CBitmap bitmap2 (CResourceDescription (100));
	EXPECT (bitmapToString (&bitmap2, str, &uidesc) == true);
	EXPECT (str == "100");
}

TEST_CASE (UIViewCreatorTest, ColorToString)
{
	UIDescriptionAdapter uidesc;
	std::string str;
	CColor c (0, 0, 0, 255);
	EXPECT (colorToString (c, str, &uidesc) == true);
	EXPECT (str == "#000000ff")
	c = CColor (0, 0, 255, 0);
	EXPECT (colorToString (c, str, &uidesc) == true);
	EXPECT (str == "#0000ff00")
	c = CColor (0, 255, 0, 0);
	EXPECT (colorToString (c, str, &uidesc) == true);
	EXPECT (str == "#00ff0000")
	c = CColor (255, 0, 0, 0);
	EXPECT (colorToString (c, str, &uidesc) == true);
	EXPECT (str == "#ff000000")
}

TEST_CASE (UIViewCreatorTest, EmptyStringToTransparentColor)
{
	UIDescriptionAdapter uidesc;
	std::string str = "";
	CColor c;
	EXPECT (stringToColor (&str, c, &uidesc) == true);
	EXPECT (c == kTransparentCColor);
}

} // VSTGUI
