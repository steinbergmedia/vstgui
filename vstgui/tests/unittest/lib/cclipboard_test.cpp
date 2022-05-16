// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/cclipboard.h"
#include "../unittests.h"

namespace VSTGUI {

#if !WINDOWS
// the OLE clipboard functionality returns Ole not initialized even tho we call OleInitialize()
// so we disable the test for now on Windows

TEST_CASE (CClipboardTest, String)
{
	EXPECT_TRUE (CClipboard::setString ("This is a test string"));
	auto cbStr = CClipboard::getString ();
	EXPECT_TRUE (cbStr);
	EXPECT (*cbStr == "This is a test string");
}

TEST_CASE (CClipboardTest, FilePath)
{
#if WINDOWS
	constexpr auto path = "C:\\Windows\\test.txt";
#else
	constexpr auto path = "/tmp/test.txt";
#endif
	EXPECT_TRUE (CClipboard::setFilePath (path));
	auto cbStr = CClipboard::getFilePath ();
	EXPECT_TRUE (cbStr);
	EXPECT (*cbStr == path);
}
#endif

} // VSTGUI
