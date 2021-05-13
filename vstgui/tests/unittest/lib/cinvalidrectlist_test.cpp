// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/cinvalidrectlist.h"
#include "../unittests.h"

namespace VSTGUI {

TEST_CASE (CInvalidRectListTest, RectEqual)
{
	CInvalidRectList list;
	EXPECT_TRUE (list.add ({0, 0, 100, 100}));
	EXPECT_FALSE (list.add ({0, 0, 100, 100}));
	EXPECT_EQ (list.data ().size (), 1u);
}

TEST_CASE (CInvalidRectListTest, AddBiggerOne)
{
	CInvalidRectList list;
	EXPECT_TRUE (list.add ({0, 0, 100, 100}));
	EXPECT_TRUE (list.add ({0, 0, 200, 200}));
	EXPECT_EQ (list.data ().size (), 1u);
}

TEST_CASE (CInvalidRectListTest, AddSmallerOne)
{
	CInvalidRectList list;
	EXPECT_TRUE (list.add ({0, 0, 100, 100}));
	EXPECT_FALSE (list.add ({10, 10, 20, 20}));
	EXPECT_EQ (list.data ().size (), 1u);
}

TEST_CASE (CInvalidRectListTest, AddOverlappingOne)
{
	CInvalidRectList list;
	EXPECT_TRUE (list.add ({0, 0, 100, 100}));
	EXPECT_TRUE (list.add ({90, 0, 120, 100}));
	EXPECT_EQ (list.data ().size (), 1u);
}

TEST_CASE (CInvalidRectListTest, AddMulti)
{
	CInvalidRectList list;
	EXPECT_TRUE (list.add ({0, 0, 10, 10}));
	EXPECT_TRUE (list.add ({20, 20, 30, 30}));
	EXPECT_EQ (list.data ().size (), 2u);
}

} // VSTGUI
