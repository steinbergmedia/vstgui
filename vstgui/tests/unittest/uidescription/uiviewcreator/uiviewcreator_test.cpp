// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../unittests.h"
#include "../../../../uidescription/uiviewcreator.h"
#include "../../../../lib/cstring.h"
#include "../../../../lib/cpoint.h"
#include "../../../../lib/ccolor.h"
#include "../../../../lib/cbitmap.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(UIViewCreatorTest,

	TEST(parseSize,
		CPoint p;
		EXPECT(parseSize ("10, 10", p));
		EXPECT(p == CPoint (10, 10));
		EXPECT(parseSize ("hola", p) == false);
	);

	TEST(pointToString,
		CPoint p (10, 10);
		std::string result;
		EXPECT(pointToString (p, result));
		EXPECT(result == "10, 10");
	);

);

} // VSTGUI
