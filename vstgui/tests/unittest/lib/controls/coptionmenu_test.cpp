// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/coptionmenu.h"
#include "../../unittests.h"
#include "../eventhelpers.h"

namespace VSTGUI {

//------------------------------------------------------------------------
class TestCommandMenuItemTarget
: public CommandMenuItemTargetAdapter
, public NonAtomicReferenceCounted
{
};

TEST_CASE (CCommandMenuItemTest, DescConstructor1)
{
	auto target = makeOwned<TestCommandMenuItemTarget> ();
	CCommandMenuItem item (
		{"Title", "k", 0, nullptr, CMenuItem::kNoFlags, target, "CommandCategory", "CommandName"});
	EXPECT_EQ (item.getTitle (), "Title");
	EXPECT_EQ (item.getKeycode (), "k");
	EXPECT_EQ (item.getIcon (), nullptr);
	EXPECT_EQ (item.getItemTarget (), target);
	EXPECT_EQ (item.getCommandCategory (), "CommandCategory");
	EXPECT_EQ (item.getCommandName (), "CommandName");
}

TEST_CASE (CCommandMenuItemTest, DescConstructor2)
{
	auto target = makeOwned<TestCommandMenuItemTarget> ();
	CCommandMenuItem item ({"Title", 100, target, "CommandCat", "CmdName"});
	EXPECT_EQ (item.getTitle (), "Title");
	EXPECT_EQ (item.getItemTarget (), target);
	EXPECT_EQ (item.getCommandCategory (), "CommandCat");
	EXPECT_EQ (item.getCommandName (), "CmdName");
	EXPECT_EQ (item.getTag (), 100);
}

TEST_CASE (CCommandMenuItemTest, DescConstructor3)
{
	auto target = makeOwned<TestCommandMenuItemTarget> ();
	CCommandMenuItem item ({"MenuItem", target, "CmdCat", "CommandNme"});
	EXPECT_EQ (item.getTitle (), "MenuItem");
	EXPECT_EQ (item.getItemTarget (), target);
	EXPECT_EQ (item.getCommandCategory (), "CmdCat");
	EXPECT_EQ (item.getCommandName (), "CommandNme");
}

TEST_CASE (COptionMenuTest, GetMaxWhenEmpty)
{
	COptionMenu menu;
	EXPECT_EQ (menu.getMax (), 0.f);
}

} // VSTGUI
