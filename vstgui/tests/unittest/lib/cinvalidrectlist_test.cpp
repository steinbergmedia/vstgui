// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../unittests.h"
#include "../../../lib/cinvalidrectlist.h"

namespace VSTGUI {

TESTCASE(CInvalidRectListTest,

	TEST(rectEqual,
		CInvalidRectList list;
		EXPECT (list.add ({0, 0, 100, 100}));
		EXPECT (list.add ({0, 0, 100, 100}) == false);
		EXPECT (list.data ().size () == 1u);
	);

	TEST(addBiggerOne,
		CInvalidRectList list;
		EXPECT (list.add ({0, 0, 100, 100}));
		EXPECT (list.add ({0, 0, 200, 200}));
		EXPECT (list.data ().size () == 1u);
	);

	TEST(addSmallerOne,
		CInvalidRectList list;
		EXPECT (list.add ({0, 0, 100, 100}));
		EXPECT (list.add ({10, 10, 20, 20}) == false);
		EXPECT (list.data ().size () == 1u);
	);

	TEST(addOverlappingOne,
		CInvalidRectList list;
		EXPECT (list.add ({0, 0, 100, 100}));
		EXPECT (list.add ({90, 0, 120, 100}));
		EXPECT (list.data ().size () == 1u);
	);

);

} // VSTGUI
