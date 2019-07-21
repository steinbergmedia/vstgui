// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/clistcontrol.h"
#include "../../../../lib/cscrollview.h"
#include "../../../../lib/vstkeycode.h"
#include "../../unittests.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
static SharedPointer<CListControl> createTestListControl (
    CCoord rowHeight, int32_t numRows = 10,
    int32_t rowFlags = CListControlRowDesc::Selectable | CListControlRowDesc::Hoverable)
{
	auto listControl = makeOwned<CListControl> (CRect (0, 0, 100, 100));
	auto config = makeOwned<StaticListControlConfigurator> (rowHeight, rowFlags);
	listControl->setMin (0.f);
	listControl->setMax (static_cast<float> (numRows));
	listControl->setConfigurator (config);
	listControl->recalculateLayout ();
	listControl->setValue (0.f);
	return listControl;
}

//------------------------------------------------------------------------
static SharedPointer<CScrollView> createScrollViewAndEmbedListControl (CViewContainer* parent,
                                                                       CListControl* listControl)
{
	auto scrollView =
	    makeOwned<CScrollView> (CRect (0, 0, 100, listControl->getHeight () / 2),
	                            listControl->getViewSize (), CScrollView::kVerticalScrollbar);
	scrollView->addView (listControl);
	listControl->remember ();
	parent->addView (scrollView);
	scrollView->attached (parent);
	return scrollView;
}

//------------------------------------------------------------------------
static VstKeyCode makeKeyCode (int32_t c, unsigned char virt, unsigned char modifier)
{
	return {c, virt, modifier};
}

//------------------------------------------------------------------------
TESTCASE(CListControlTest,

	TEST(minMax,
		constexpr auto rowHeight = 20;
		auto listControl = createTestListControl (rowHeight);
		listControl->setMin (-5.f);
		listControl->setMax (5.f);
		
		auto row = listControl->getRowAtPoint (CPoint (0, 0));
		EXPECT (row);
		if (row)
		{
			EXPECT (*row == -5);
		}
		row = listControl->getRowAtPoint (CPoint (0, listControl->getHeight () - 1));
		EXPECT (row);
		if (row)
		{
			EXPECT (*row == 5);
		}
	);

	TEST(mouseRowSelection,
		constexpr auto rowHeight = 20;
		auto listControl = createTestListControl (rowHeight);
		
		CPoint where (0, rowHeight);
		listControl->onMouseDown (where, kLButton);
		listControl->onMouseUp (where, kLButton);
		EXPECT (listControl->getValue () == 1.f);
		where (0, rowHeight * 3);
		listControl->onMouseDown (where, kLButton);
		listControl->onMouseUp (where, kLButton);
		EXPECT (listControl->getValue () == 3.f);
	);

	TEST(hovering,
		constexpr auto rowHeight = 20;
		constexpr auto numRows = 5;
		auto listControl = createTestListControl (rowHeight, numRows);

		EXPECT(!listControl->getHoveredRow ());
		CPoint where (10, 5);
		listControl->onMouseMoved (where, 0);
		EXPECT(listControl->getHoveredRow ());
		EXPECT(*listControl->getHoveredRow () == 0);

		where (10, 5 + rowHeight);
		listControl->onMouseMoved (where, 0);
		EXPECT(listControl->getHoveredRow ());
		EXPECT(*listControl->getHoveredRow () == 1);

		listControl->onMouseExited (where, 0);
		EXPECT(!listControl->getHoveredRow ());

	);

	TEST(rowRect,
		constexpr auto rowHeight = 20;
		auto listControl = createTestListControl (rowHeight);

		auto rr = listControl->getRowRect (1);
		EXPECT (rr);
		if (rr)
		{
			EXPECT (*rr == CRect (0, rowHeight, 100, rowHeight * 2.));
		}
	);

	TEST(keyDownOnUnselectableRows,
		constexpr auto rowHeight = 20;
		constexpr auto numRows = 20;
		auto listControl = createTestListControl (rowHeight, numRows, 0);
		listControl->setValue (2.f);
		EXPECT (listControl->getValue () == 2.f);

		auto code = makeKeyCode (0, VKEY_DOWN, 0);
		EXPECT (listControl->onKeyDown (code) == -1);
		EXPECT (listControl->getValue () == 2.f);
	);

	TEST(keyWithModifier,
		constexpr auto rowHeight = 20;
		constexpr auto numRows = 20;
		auto listControl = createTestListControl (rowHeight, numRows, 0);

		listControl->setValue (1.f);

		auto code = makeKeyCode (0, VKEY_DOWN, MODIFIER_SHIFT);
		EXPECT (listControl->onKeyDown (code) == -1);
		EXPECT (listControl->getValue () == 1.f);

		code = makeKeyCode (0, VKEY_UP, MODIFIER_SHIFT);
		EXPECT (listControl->onKeyDown (code) == -1);
		EXPECT (listControl->getValue () == 1.f);

		code = makeKeyCode (0, VKEY_HOME, MODIFIER_SHIFT);
		EXPECT (listControl->onKeyDown (code) == -1);
		EXPECT (listControl->getValue () == 1.f);

		code = makeKeyCode (0, VKEY_END, MODIFIER_SHIFT);
		EXPECT (listControl->onKeyDown (code) == -1);
		EXPECT (listControl->getValue () == 1.f);

		code = makeKeyCode (0, VKEY_PAGEUP, MODIFIER_SHIFT);
		EXPECT (listControl->onKeyDown (code) == -1);
		EXPECT (listControl->getValue () == 1.f);

		code = makeKeyCode (0, VKEY_PAGEDOWN, MODIFIER_SHIFT);
		EXPECT (listControl->onKeyDown (code) == -1);
		EXPECT (listControl->getValue () == 1.f);
	);

	TEST(keyHome,
		constexpr auto rowHeight = 20;
		auto listControl = createTestListControl (rowHeight);
		listControl->setValue (5.f);
		EXPECT (listControl->getValue () == 5.f);

		auto code = makeKeyCode (0, VKEY_HOME, 0);
		EXPECT (listControl->onKeyDown (code) == 1);
		EXPECT (listControl->getValue () == 0.f);
	);

	TEST(keyEnd,
		constexpr auto rowHeight = 20;
		constexpr auto numRows = 20;
		auto listControl = createTestListControl (rowHeight, numRows);
		listControl->setValue (0.f);
		EXPECT (listControl->getValue () == 0.f);

		auto code = makeKeyCode (0, VKEY_END, 0);
		EXPECT (listControl->onKeyDown (code) == 1);
		EXPECT (listControl->getValue () == numRows);
	);

	TEST(keyUp,
		constexpr auto rowHeight = 20;
		constexpr auto numRows = 20;
		auto listControl = createTestListControl (rowHeight, numRows);
		listControl->setValue (2.f);
		EXPECT (listControl->getValue () == 2.f);

		auto code = makeKeyCode (0, VKEY_UP, 0);
		EXPECT (listControl->onKeyDown (code) == 1);
		EXPECT (listControl->getValue () == 1.f);

		listControl->setValue (0.f);
		EXPECT (listControl->onKeyDown (code) == 1);
		EXPECT (listControl->getValue () == numRows);
	);

	TEST(keyDown,
		constexpr auto rowHeight = 20;
		constexpr auto numRows = 20;
		auto listControl = createTestListControl (rowHeight, numRows);
		listControl->setValue (2.f);
		EXPECT (listControl->getValue () == 2.f);

		auto code = makeKeyCode (0, VKEY_DOWN, 0);
		EXPECT (listControl->onKeyDown (code) == 1);
		EXPECT (listControl->getValue () == 3.f);

		listControl->setValue (numRows);
		EXPECT (listControl->onKeyDown (code) == 1);
		EXPECT (listControl->getValue () == 0.f);
	);

	TEST(pageUp,
		constexpr auto rowHeight = 20;
		constexpr auto numRows = 30;
		auto parent = makeOwned<CViewContainer>(CRect (0, 0, 1000, 1000));
		auto listControl = createTestListControl (rowHeight, numRows);
		auto scrollView = createScrollViewAndEmbedListControl (parent, listControl);
		listControl->setValue (numRows);
		auto rect = listControl->getRowRect (0);
		scrollView->makeRectVisible (*rect);

		auto code = makeKeyCode (0, VKEY_PAGEUP, 0);
		EXPECT (listControl->onKeyDown (code) == 1);
		EXPECT (listControl->getValue () == 15.f);
		listControl->setValue (16);
		EXPECT (listControl->onKeyDown (code) == 1);
		EXPECT (listControl->getValue () == 15.f);

		EXPECT (listControl->onKeyDown (code) == 1);
		EXPECT (listControl->getValue () == 0.f);

		scrollView->removed (parent);
		parent->removeAll (false);
	);

	TEST(pageDown,
		constexpr auto rowHeight = 20;
		constexpr auto numRows = 30;
		auto parent = makeOwned<CViewContainer>(CRect (0, 0, 1000, 1000));
		auto listControl = createTestListControl (rowHeight, numRows);
		auto scrollView = createScrollViewAndEmbedListControl (parent, listControl);
		listControl->setValue (0.f);
		auto rect = listControl->getRowRect (numRows);
		scrollView->makeRectVisible (*rect);

		auto code = makeKeyCode (0, VKEY_PAGEDOWN, 0);
		EXPECT (listControl->onKeyDown (code) == 1);
		EXPECT (listControl->getValue () == 15.f);
		listControl->setValue (14);
		EXPECT (listControl->onKeyDown (code) == 1);
		EXPECT (listControl->getValue () == 15.f);

		EXPECT (listControl->onKeyDown (code) == 1);
		EXPECT (listControl->getValue () == 30.f);

		scrollView->removed (parent);
		parent->removeAll (false);
	);

);

//------------------------------------------------------------------------
} // VSTGUI
