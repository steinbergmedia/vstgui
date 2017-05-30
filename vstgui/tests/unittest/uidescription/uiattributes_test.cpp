// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../unittests.h"
#include "../../../uidescription/uiattributes.h"
#include "../../../uidescription/cstream.h"
#include "../../../lib/cpoint.h"
#include "../../../lib/crect.h"

namespace VSTGUI {

static UTF8StringPtr attributes [] = {"K1", "V1", "K2", "V2", nullptr};

TESTCASE(UIAttributesTest,

	TEST(arrayConstructor,
		UIAttributes a (attributes);
		EXPECT(a.hasAttribute ("K1"));
		EXPECT(a.hasAttribute ("K2"));
		EXPECT(*a.getAttributeValue ("K1") == "V1");
		EXPECT(*a.getAttributeValue ("K2") == "V2");
	);

	TEST(setGetStringValue,
		UIAttributes a;
		a.setAttribute ("Key", "Value");
		EXPECT(a.hasAttribute ("Key"));
		EXPECT(*a.getAttributeValue("Key") == "Value");
	);

	TEST(removeAttribute,
		UIAttributes a;
		a.setAttribute ("Key", "Value");
		a.removeAttribute ("Key");
		EXPECT(a.hasAttribute ("Key") == false);
	);

	TEST(boolAttribute,
		UIAttributes a;
		bool value = false;
		EXPECT(a.getBooleanAttribute ("Key", value) == false);
		a.setBooleanAttribute ("Key", true);
		EXPECT(a.getBooleanAttribute ("Key", value));
		EXPECT(value == true);
		a.setBooleanAttribute("Key", false);
		EXPECT(a.getBooleanAttribute ("Key", value));
		EXPECT(value == false);
	);

	TEST(intAttribute,
		UIAttributes a;
		int32_t value = 0;
		EXPECT(a.getIntegerAttribute ("Key", value) == false);
		a.setIntegerAttribute ("Key", 10);
		EXPECT(a.getIntegerAttribute ("Key", value));
		EXPECT(value == 10);
	);
	
	TEST(doubleAttribute,
		UIAttributes a;
		double value = 0.;
		EXPECT(a.getDoubleAttribute ("Key", value) == false);
		a.setDoubleAttribute ("Key", 3.45);
		EXPECT(a.getDoubleAttribute ("Key", value));
		EXPECT(value == 3.45);
	);

	TEST(pointAttribute,
		UIAttributes a;
		CPoint value;
		EXPECT(a.getPointAttribute ("Key", value) == false);
		a.setPointAttribute ("Key", CPoint (5,5));
		EXPECT(a.getPointAttribute ("Key", value));
		EXPECT(value == CPoint (5, 5));
	);
	
	TEST(rectAttribute,
		UIAttributes a;
		CRect value;
		EXPECT(a.getRectAttribute ("Key", value) == false);
		a.setRectAttribute ("Key", CRect (10, 20, 30, 40));
		EXPECT(a.getRectAttribute ("Key", value));
		EXPECT(value == CRect (10, 20, 30, 40));
	);

	TEST(stringArrayAttribute,
		UIAttributes a;
		UIAttributes::StringArray array;
		array.push_back ("1");
		array.push_back ("2");
		array.push_back ("3");
		UIAttributes::StringArray array2;
		EXPECT(a.getStringArrayAttribute ("Key", array2) == false);
		a.setStringArrayAttribute ("Key", array);
		EXPECT(a.getStringArrayAttribute ("Key", array2));
		EXPECT(array == array2);
	);

	TEST(removeAll,
		UIAttributes a;
		a.setRectAttribute ("Key1", CRect (10, 20, 30, 40));
		a.setDoubleAttribute ("Key2", 3.45);
		a.removeAll();
		EXPECT(a.begin () == a.end ());
	);
	
	TEST(storeRestore,
		UIAttributes a;
		CMemoryStream s2;
		EXPECT(a.restore (s2) == false);
		a.setAttribute ("Key1", "Value");
		a.setBooleanAttribute ("Key2", true);
		a.setIntegerAttribute ("Key3", 10);
		a.setDoubleAttribute ("Key4", 3.45);
		a.setPointAttribute ("Key5", CPoint (5,5));
		a.setRectAttribute ("Key6", CRect (10, 20, 30, 40));
		a.setDoubleAttribute ("Key7", 3.45);

		CMemoryStream s;
		a.store (s);
		s.rewind ();
		UIAttributes a2;
		a2.restore (s);
		for (auto& v : a)
		{
			EXPECT(a2.hasAttribute (v.first));
		}
	);
	
	TEST(restoreFromInvalidStream,
		CMemoryStream s;
		UIAttributes a;
		EXPECT(a.restore(s) == false);
	);
);

} // VSTGUI
