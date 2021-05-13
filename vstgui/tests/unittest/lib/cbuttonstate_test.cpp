// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../unittests.h"
#include "../../../lib/cbuttonstate.h"

namespace VSTGUI {

TEST_CASE (CButtonStateTests, Test)
{
	CButtonState s;
	EXPECT_EQ (s.getButtonState (), 0);
	EXPECT_EQ (s.getModifierState (), 0);
	s = kLButton;
	EXPECT_TRUE (s.isLeftButton ());
	s |= kShift;
	EXPECT_TRUE (s.isLeftButton ());
	EXPECT_EQ (s.getModifierState (), kShift);
	s = kRButton;
	EXPECT_TRUE (s.isRightButton ());
	s |= kDoubleClick;
	EXPECT_TRUE (s.isDoubleClick ());
	EXPECT (s & CButtonState (kDoubleClick));
	CButtonState s2 (s);
	EXPECT_EQ (s, s2);
	s2 = ~s;
	EXPECT_NE (s, s2);
}

} // VSTGUI
