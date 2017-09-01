// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/clinestyle.h"
#include "../unittests.h"

namespace VSTGUI {

TESTCASE(CLineStyleTest,

	TEST(defaultConstructor,
		CLineStyle style;
		EXPECT(style.getLineCap () == CLineStyle::kLineCapButt);
		EXPECT(style.getLineJoin () == CLineStyle::kLineJoinMiter);
		EXPECT(style.getDashPhase () == 0.);
		EXPECT(style.getDashCount () == 0);
		EXPECT(style.getDashLengths().size() == 0);
	);
	
	TEST(solidLine,
		CLineStyle style;
		EXPECT(style == kLineSolid);
	);

	TEST(onOffDashLine,
		CLineStyle style (CLineStyle::kLineCapButt, CLineStyle::kLineJoinMiter, 0., {1., 1.});
		EXPECT(kLineOnOffDash == style);
	);

	TEST(dashLengths,
		CLineStyle style (CLineStyle::kLineCapButt, CLineStyle::kLineJoinMiter, 0., {1., 3., 2.});
		EXPECT(style.getDashCount () == 3);
		const auto& dashLengths = style.getDashLengths ();
		EXPECT(dashLengths[0] == 1.);
		EXPECT(dashLengths[1] == 3.);
		EXPECT(dashLengths[2] == 2.);
		style.getDashLengths().push_back (6.);
		EXPECT(style.getDashCount () == 4);
		EXPECT(style.getDashLengths()[3] == 6.);
	);
	
	TEST(lineCap,
		CLineStyle style;
		style.setLineCap (CLineStyle::kLineCapButt);
		EXPECT(style.getLineCap () == CLineStyle::kLineCapButt);
		style.setLineCap (CLineStyle::kLineCapRound);
		EXPECT(style.getLineCap () == CLineStyle::kLineCapRound);
		style.setLineCap (CLineStyle::kLineCapSquare);
		EXPECT(style.getLineCap () == CLineStyle::kLineCapSquare);
	);

	TEST(lineJoin,
		CLineStyle style;
		style.setLineJoin (CLineStyle::kLineJoinMiter);
		EXPECT(style.getLineJoin () == CLineStyle::kLineJoinMiter);
		style.setLineJoin (CLineStyle::kLineJoinRound);
		EXPECT(style.getLineJoin () == CLineStyle::kLineJoinRound);
		style.setLineJoin (CLineStyle::kLineJoinBevel);
		EXPECT(style.getLineJoin () == CLineStyle::kLineJoinBevel);
	);
	
	TEST(dashPhase,
		CLineStyle style;
		style.setDashPhase (1.5);
		EXPECT(style.getDashPhase () == 1.5);
		style.setDashPhase (2.5);
		EXPECT(style.getDashPhase () == 2.5);
	);

	TEST(copyConstructor,
		CLineStyle style;
		style.setDashPhase (2.);
		style.getDashLengths().push_back(1.);
		style.getDashLengths().push_back(2.);
		CLineStyle s2 (style);
		EXPECT(style.getLineCap () == CLineStyle::kLineCapButt);
		EXPECT(style.getLineJoin () == CLineStyle::kLineJoinMiter);
		EXPECT(style.getDashPhase () == 2.);
		EXPECT(style.getDashCount () == 2);
		EXPECT(style.getDashLengths()[0] == 1.);
		EXPECT(style.getDashLengths()[1] == 2.);
	);
	
	TEST(moveConstructor,
		CLineStyle style;
		style.getDashLengths().push_back(1.);
		style.getDashLengths().push_back(2.);
		CLineStyle s2 (std::move (style));
		EXPECT(style.getDashCount () == 0);
		EXPECT(s2.getDashCount () == 2);
	);

	TEST(vectorConstructor,
		CLineStyle::CoordVector dashLengths ({2., 4.});
		CLineStyle style (CLineStyle::kLineCapButt, CLineStyle::kLineJoinMiter, 0., dashLengths);
		EXPECT(dashLengths == style.getDashLengths ());
	);
	
	TEST(unequalOperator,
		EXPECT(kLineSolid != kLineOnOffDash);
	);
);

} // VSTGUI
