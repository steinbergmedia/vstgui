// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/cpoint.h"
#include "../../../lib/crect.h"
#include "../../../uidescription/cstream.h"
#include "../../../uidescription/uiattributes.h"
#include "../unittests.h"

namespace VSTGUI {

static UTF8StringPtr attributes[] = {"K1", "V1", "K2", "V2", nullptr};

TEST_CASE (UIAttributesTest, ArrayConstructor)
{
	UIAttributes a (attributes);
	EXPECT (a.hasAttribute ("K1"));
	EXPECT (a.hasAttribute ("K2"));
	EXPECT (*a.getAttributeValue ("K1") == "V1");
	EXPECT (*a.getAttributeValue ("K2") == "V2");
}

TEST_CASE (UIAttributesTest, SetGetStringValue)
{
	UIAttributes a;
	a.setAttribute ("Key", "Value");
	EXPECT (a.hasAttribute ("Key"));
	EXPECT (*a.getAttributeValue ("Key") == "Value");
}

TEST_CASE (UIAttributesTest, RemoveAttribute)
{
	UIAttributes a;
	a.setAttribute ("Key", "Value");
	a.removeAttribute ("Key");
	EXPECT (a.hasAttribute ("Key") == false);
}

TEST_CASE (UIAttributesTest, BoolAttribute)
{
	UIAttributes a;
	bool value = false;
	EXPECT (a.getBooleanAttribute ("Key", value) == false);
	a.setBooleanAttribute ("Key", true);
	EXPECT (a.getBooleanAttribute ("Key", value));
	EXPECT (value == true);
	a.setBooleanAttribute ("Key", false);
	EXPECT (a.getBooleanAttribute ("Key", value));
	EXPECT (value == false);
}

TEST_CASE (UIAttributesTest, IntegerAttribute)
{
	UIAttributes a;
	int32_t value = 0;
	EXPECT (a.getIntegerAttribute ("Key", value) == false);
	a.setIntegerAttribute ("Key", 10);
	EXPECT (a.getIntegerAttribute ("Key", value));
	EXPECT (value == 10);
}

TEST_CASE (UIAttributesTest, DoubleAttribute)
{
	UIAttributes a;
	double value = 0.;
	EXPECT (a.getDoubleAttribute ("Key", value) == false);
	a.setDoubleAttribute ("Key", 3.45);
	EXPECT (a.getDoubleAttribute ("Key", value));
	EXPECT (value == 3.45);
}

TEST_CASE (UIAttributesTest, PointAttribute)
{
	UIAttributes a;
	CPoint value;
	EXPECT (a.getPointAttribute ("Key", value) == false);
	a.setPointAttribute ("Key", CPoint (5, 5));
	EXPECT (a.getPointAttribute ("Key", value));
	EXPECT (value == CPoint (5, 5));
}

TEST_CASE (UIAttributesTest, RectAttribute)
{
	UIAttributes a;
	CRect value;
	EXPECT (a.getRectAttribute ("Key", value) == false);
	a.setRectAttribute ("Key", CRect (10, 20, 30, 40));
	EXPECT (a.getRectAttribute ("Key", value));
	EXPECT (value == CRect (10, 20, 30, 40));
}

TEST_CASE (UIAttributesTest, StringArrayAttribute)
{
	UIAttributes a;
	UIAttributes::StringArray array;
	array.push_back ("1");
	array.push_back ("2");
	array.push_back ("3");
	UIAttributes::StringArray array2;
	EXPECT (a.getStringArrayAttribute ("Key", array2) == false);
	a.setStringArrayAttribute ("Key", array);
	EXPECT (a.getStringArrayAttribute ("Key", array2));
	EXPECT (array == array2);
}

TEST_CASE (UIAttributesTest, RemoveAll)
{
	UIAttributes a;
	a.setRectAttribute ("Key1", CRect (10, 20, 30, 40));
	a.setDoubleAttribute ("Key2", 3.45);
	a.removeAll ();
	EXPECT (a.begin () == a.end ());
}

TEST_CASE (UIAttributesTest, StoreRestore)
{
	UIAttributes a;
	CMemoryStream s2;
	EXPECT (a.restore (s2) == false);
	a.setAttribute ("Key1", "Value");
	a.setBooleanAttribute ("Key2", true);
	a.setIntegerAttribute ("Key3", 10);
	a.setDoubleAttribute ("Key4", 3.45);
	a.setPointAttribute ("Key5", CPoint (5, 5));
	a.setRectAttribute ("Key6", CRect (10, 20, 30, 40));
	a.setDoubleAttribute ("Key7", 3.45);

	CMemoryStream s;
	a.store (s);
	s.rewind ();
	UIAttributes a2;
	a2.restore (s);
	for (auto& v : a)
	{
		auto value = a2.getAttributeValue (v.first);
		EXPECT (value);
		EXPECT_EQ (*value, v.second);
	}
}

TEST_CASE (UIAttributesTest, RestoreFromInvalidStream)
{
	CMemoryStream s;
	UIAttributes a;
	EXPECT (a.restore (s) == false);
	s.rewind ();
	uint32_t someValue = 0;
	s.writeRaw (&someValue, sizeof (someValue));
	s.rewind ();
	EXPECT (a.restore (s) == false);
}

TEST_CASE (UIAttributesTest, StringToBool)
{
	bool b;
	EXPECT (UIAttributes::stringToBool ("hola", b) == false)
	EXPECT (UIAttributes::stringToBool ("true 5", b) == false)

	EXPECT (UIAttributes::stringToBool ("true", b) && b == true)
	EXPECT (UIAttributes::stringToBool ("false", b) && b == false)
}

TEST_CASE (UIAttributesTest, StringToInteger)
{
	int32_t i;
	EXPECT (UIAttributes::stringToInteger ("s5s5", i) == false)
	EXPECT (UIAttributes::stringToInteger ("5s5", i) == false)
	EXPECT (UIAttributes::stringToInteger ("1.0", i) == false)
	EXPECT (UIAttributes::stringToInteger ("hola", i) == false)

	EXPECT (UIAttributes::stringToInteger ("151515", i) && i == 151515)
}

TEST_CASE (UIAttributesTest, StringToDouble)
{
	double d;
	EXPECT (UIAttributes::stringToDouble ("as5.5", d) == false)
	EXPECT (UIAttributes::stringToDouble ("5.5sa", d) == false)
	EXPECT (UIAttributes::stringToDouble ("5.567.5", d) == false)
	EXPECT (UIAttributes::stringToDouble ("hola", d) == false)

	EXPECT (UIAttributes::stringToDouble ("25.5", d) && d == 25.5)
	EXPECT (UIAttributes::stringToDouble (".5", d) && d == 0.5)
	EXPECT (UIAttributes::stringToDouble (" -0.5", d) && d == -0.5)
}

TEST_CASE (UIAttributesTest, StringToPoint)
{
	CPoint p;
	EXPECT (UIAttributes::stringToPoint ("30, 20, 50", p) == false)
	EXPECT (UIAttributes::stringToPoint ("30, 20a", p) == false)
	EXPECT (UIAttributes::stringToPoint ("a, b", p) == false)
	EXPECT (UIAttributes::stringToPoint ("20", p) == false)

	EXPECT (UIAttributes::stringToPoint ("15, 25", p) && p == CPoint (15, 25))
	EXPECT (UIAttributes::stringToPoint ("1.768, 25", p) && p == CPoint (1.768, 25))
}

TEST_CASE (UIAttributesTest, StringToRect)
{
	CRect r;
	EXPECT (UIAttributes::stringToRect ("30, 20, 50", r) == false)
	EXPECT (UIAttributes::stringToRect ("30, 20, 50, 60, 80", r) == false)
	EXPECT (UIAttributes::stringToRect ("30, 20, 50, 60a", r) == false)
	EXPECT (UIAttributes::stringToRect ("a, b, c, d", r) == false)

	EXPECT (UIAttributes::stringToRect ("0, 12.5, 5, 8", r) && r == CRect (0, 12.5, 5, 8))
}

TEST_CASE (UIAttributesTest, StringArrayToStringWithEmptyStringArray)
{
	const UIAttributes::StringArray strings;
	const auto s = UIAttributes::stringArrayToString (strings);
	EXPECT (s.empty ())
}

} // VSTGUI
