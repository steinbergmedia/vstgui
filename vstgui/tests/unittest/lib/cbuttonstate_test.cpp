// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../unittests.h"
#include "../../../lib/cbuttonstate.h"

namespace VSTGUI {

TESTCASE(CButtonStateTests,

	TEST(test,
		CButtonState s;
		EXPECT(s.getButtonState () == 0);
		EXPECT(s.getModifierState () == 0);
		s = kLButton;
		EXPECT(s.isLeftButton () == true);
		s |= kShift;
		EXPECT(s.isLeftButton () == true);
		EXPECT(s.getModifierState () == kShift);
		s = kRButton;
		EXPECT(s.isRightButton () == true);
		s |= kDoubleClick;
		EXPECT(s.isDoubleClick () == true);
		EXPECT(s & CButtonState (kDoubleClick));
		CButtonState s2 (s);
		EXPECT(s == s2);
		s2 = ~s;
		EXPECT(s != s2);
	);
);

} // VSTGUI
