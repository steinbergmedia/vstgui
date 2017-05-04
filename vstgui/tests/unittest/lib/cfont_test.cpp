// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../unittests.h"
#include "../../../lib/cfont.h"

namespace VSTGUI {

TESTCASE(CFontTests,

	TEST(attributes,
		CFontDesc f;
		EXPECT(f.getName ().empty ());
		EXPECT(f.getSize () == 0.);
		EXPECT(f.getStyle () == kNormalFace);
		f.setName ("Test");
		EXPECT(strcmp (f.getName (), "Test") == 0);
		f.setSize (20.2);
		EXPECT(f.getSize () == 20.2);
		f.setStyle (kBoldFace|kItalicFace);
		EXPECT(f.getStyle () == (kBoldFace|kItalicFace));
		CFontDesc::cleanup ();
	);

	TEST(copyConstructor,
		CFontDesc f (*kSystemFont);
		EXPECT(f == *kSystemFont);
	);

	TEST(notEqualOperator,
		CFontDesc f (*kSystemFont);
		EXPECT(f == *kSystemFont);
		f.setSize (f.getSize ()+1);
		EXPECT(f != *kSystemFont);
		f = *kSystemFont;
		f.setStyle (kBoldFace);
		EXPECT(f != *kSystemFont);
		f = *kSystemFont;
		f.setName ("Bla");
		EXPECT(f != *kSystemFont);
	);

);

} // VSTGUI
