// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/clistcontrol.h"
#include "../../../../lib/cscrollview.h"
#include "../../unittests.h"
#include "../eventhelpers.h"

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
static KeyboardEvent makeKeyboardEvent (int32_t c, VirtualKey virt,
                                        ModifierKey modifier = ModifierKey::None)
{
	KeyboardEvent event;
	event.type = EventType::KeyDown;
	event.character = c;
	event.virt = virt;
	event.modifiers.add (modifier);
	return event;
}

TEST_CASE (CListControlTest, minMax)
{
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
}

TEST_CASE (CListControlTest, MouseRowSelection)
{
	constexpr auto rowHeight = 20;
	auto listControl = createTestListControl (rowHeight);

	dispatchMouseEvent<MouseDownEvent> (listControl, {0., rowHeight}, MouseButton::Left);
	dispatchMouseEvent<MouseUpEvent> (listControl, {0., rowHeight}, MouseButton::Left);
	EXPECT_EQ (listControl->getValue (), 1.f);
	dispatchMouseEvent<MouseDownEvent> (listControl, {0., rowHeight * 3.}, MouseButton::Left);
	dispatchMouseEvent<MouseUpEvent> (listControl, {0., rowHeight * 3.}, MouseButton::Left);
	EXPECT_EQ (listControl->getValue (), 3.f);
}

TEST_CASE (CListControlTest, Hovering)
{
	constexpr auto rowHeight = 20;
	constexpr auto numRows = 5;
	auto listControl = createTestListControl (rowHeight, numRows);

	EXPECT_FALSE (listControl->getHoveredRow ());
	dispatchMouseEvent<MouseMoveEvent>(listControl, {10., 5.});
	EXPECT_TRUE (listControl->getHoveredRow ());
	EXPECT_EQ (*listControl->getHoveredRow (), 0);

	dispatchMouseEvent<MouseMoveEvent>(listControl, {10., 5. + rowHeight});
	EXPECT_TRUE (listControl->getHoveredRow ());
	EXPECT_EQ (*listControl->getHoveredRow (), 1);

	dispatchMouseEvent<MouseExitEvent>(listControl, {10., 5. + rowHeight});
	EXPECT_FALSE (listControl->getHoveredRow ());
}

TEST_CASE (CListControlTest, RowRect)
{
	constexpr auto rowHeight = 20;
	auto listControl = createTestListControl (rowHeight);

	auto rr = listControl->getRowRect (1);
	EXPECT (rr);
	if (rr)
	{
		EXPECT (*rr == CRect (0, rowHeight, 100, rowHeight * 2.));
	}
}

TEST_CASE (CListControlTest, KeyDownOnUnselectableRows)
{
	constexpr auto rowHeight = 20;
	constexpr auto numRows = 20;
	auto listControl = createTestListControl (rowHeight, numRows, 0);
	listControl->setValue (2.f);
	EXPECT (listControl->getValue () == 2.f);

	auto event = makeKeyboardEvent (0, VirtualKey::Down);
	listControl->onKeyboardEvent (event);
	EXPECT (event.consumed == false);
	EXPECT (listControl->getValue () == 2.f);
}

TEST_CASE (CListControlTest, KeyWithModifier)
{
	constexpr auto rowHeight = 20;
	constexpr auto numRows = 20;
	auto listControl = createTestListControl (rowHeight, numRows, 0);

	listControl->setValue (1.f);

	auto event = makeKeyboardEvent (0, VirtualKey::Down, ModifierKey::Shift);
	listControl->onKeyboardEvent (event);
	EXPECT (event.consumed == false);
	EXPECT (listControl->getValue () == 1.f);

	event = makeKeyboardEvent (0, VirtualKey::Up, ModifierKey::Shift);
	listControl->onKeyboardEvent (event);
	EXPECT (event.consumed == false);
	EXPECT (listControl->getValue () == 1.f);

	event = makeKeyboardEvent (0, VirtualKey::Home, ModifierKey::Shift);
	listControl->onKeyboardEvent (event);
	EXPECT (event.consumed == false);
	EXPECT (listControl->getValue () == 1.f);

	event = makeKeyboardEvent (0, VirtualKey::End, ModifierKey::Shift);
	listControl->onKeyboardEvent (event);
	EXPECT (event.consumed == false);
	EXPECT (listControl->getValue () == 1.f);

	event = makeKeyboardEvent (0, VirtualKey::PageUp, ModifierKey::Shift);
	listControl->onKeyboardEvent (event);
	EXPECT (event.consumed == false);
	EXPECT (listControl->getValue () == 1.f);

	event = makeKeyboardEvent (0, VirtualKey::PageDown, ModifierKey::Shift);
	listControl->onKeyboardEvent (event);
	EXPECT (event.consumed == false);
	EXPECT (listControl->getValue () == 1.f);
}

TEST_CASE (CListControlTest, KeyHome)
{
	constexpr auto rowHeight = 20;
	auto listControl = createTestListControl (rowHeight);
	listControl->setValue (5.f);
	EXPECT (listControl->getValue () == 5.f);

	auto event = makeKeyboardEvent (0, VirtualKey::Home);
	listControl->onKeyboardEvent (event);
	EXPECT (event.consumed == true);
	EXPECT (listControl->getValue () == 0.f);
}

TEST_CASE (CListControlTest, KeyEnd)
{
	constexpr auto rowHeight = 20;
	constexpr auto numRows = 20;
	auto listControl = createTestListControl (rowHeight, numRows);
	listControl->setValue (0.f);
	EXPECT (listControl->getValue () == 0.f);

	auto event = makeKeyboardEvent (0, VirtualKey::End);
	listControl->onKeyboardEvent (event);
	EXPECT (event.consumed == true);
	EXPECT (listControl->getValue () == numRows);
}

TEST_CASE (CListControlTest, KeyUp)
{
	constexpr auto rowHeight = 20;
	constexpr auto numRows = 20;
	auto listControl = createTestListControl (rowHeight, numRows);
	listControl->setValue (2.f);
	EXPECT (listControl->getValue () == 2.f);

	auto event = makeKeyboardEvent (0, VirtualKey::Up);
	listControl->onKeyboardEvent (event);
	EXPECT (event.consumed == true);
	EXPECT (listControl->getValue () == 1.f);
	event.consumed.reset ();

	listControl->setValue (0.f);
	listControl->onKeyboardEvent (event);
	EXPECT (event.consumed == true);
	EXPECT (listControl->getValue () == numRows);
}

TEST_CASE (CListControlTest, KeyDown)
{
	constexpr auto rowHeight = 20;
	constexpr auto numRows = 20;
	auto listControl = createTestListControl (rowHeight, numRows);
	listControl->setValue (2.f);
	EXPECT (listControl->getValue () == 2.f);

	auto event = makeKeyboardEvent (0, VirtualKey::Down);
	listControl->onKeyboardEvent (event);
	EXPECT (event.consumed == true);
	EXPECT (listControl->getValue () == 3.f);
	event.consumed.reset ();

	listControl->setValue (numRows);
	listControl->onKeyboardEvent (event);
	EXPECT (event.consumed == true);
	EXPECT (listControl->getValue () == 0.f);
}

TEST_CASE (CListControlTest, PageUp)
{
	constexpr auto rowHeight = 20;
	constexpr auto numRows = 30;
	auto parent = makeOwned<CViewContainer> (CRect (0, 0, 1000, 1000));
	auto listControl = createTestListControl (rowHeight, numRows);
	auto scrollView = createScrollViewAndEmbedListControl (parent, listControl);
	listControl->setValue (numRows);
	auto rect = listControl->getRowRect (0);
	scrollView->makeRectVisible (*rect);

	auto event = makeKeyboardEvent (0, VirtualKey::PageUp);
	listControl->onKeyboardEvent (event);
	EXPECT (event.consumed == true);
	EXPECT (listControl->getValue () == 15.f);
	event.consumed.reset ();

	listControl->setValue (16);
	listControl->onKeyboardEvent (event);
	EXPECT (event.consumed == true);
	EXPECT (listControl->getValue () == 15.f);
	event.consumed.reset ();

	listControl->onKeyboardEvent (event);
	EXPECT (event.consumed == true);
	EXPECT (listControl->getValue () == 0.f);

	scrollView->removed (parent);
	parent->removeAll (false);
}

TEST_CASE (CListControlTest, PageDown)
{
	constexpr auto rowHeight = 20;
	constexpr auto numRows = 30;
	auto parent = makeOwned<CViewContainer> (CRect (0, 0, 1000, 1000));
	auto listControl = createTestListControl (rowHeight, numRows);
	auto scrollView = createScrollViewAndEmbedListControl (parent, listControl);
	listControl->setValue (0.f);
	auto rect = listControl->getRowRect (numRows);
	scrollView->makeRectVisible (*rect);

	auto event = makeKeyboardEvent (0, VirtualKey::PageDown);
	listControl->onKeyboardEvent (event);
	EXPECT (event.consumed == true);
	EXPECT (listControl->getValue () == 15.f);
	event.consumed.reset ();

	listControl->setValue (14);
	listControl->onKeyboardEvent (event);
	EXPECT (event.consumed == true);
	EXPECT (listControl->getValue () == 15.f);
	event.consumed.reset ();

	listControl->onKeyboardEvent (event);
	EXPECT (event.consumed == true);
	EXPECT (listControl->getValue () == 30.f);

	scrollView->removed (parent);
	parent->removeAll (false);
}

//------------------------------------------------------------------------
} // VSTGUI
