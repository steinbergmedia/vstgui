// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/csplitview.h"
#include "../../../lib/events.h"
#include "../../../uidescription/icontroller.h"
#include "../unittests.h"
#include <array>

namespace VSTGUI {

namespace {

class SplitViewController : public IController, public ISplitViewController
{
public:
	void valueChanged (CControl* pControl) override {}
	bool getSplitViewSizeConstraint (int32_t index, CCoord& minSize, CCoord& maxSize,
	                                 CSplitView* splitView) override
	{
		if (index == 0)
		{
			minSize = 10;
			maxSize = 50;
		}
		else
		{
			minSize = 10;
			maxSize = 100;
		}
		return true;
	}

	ISplitViewSeparatorDrawer* getSplitViewSeparatorDrawer (CSplitView* splitView) override
	{
		return nullptr;
	}

	bool storeViewSize (int32_t index, const CCoord& size, CSplitView* splitView) override
	{
		sizes[static_cast<size_t> (index)] = size;
		return true;
	}

	bool restoreViewSize (int32_t index, CCoord& size, CSplitView* splitView) override
	{
		if (index == 0)
			size = 20;
		else if (index == 1)
			size = 70;
		return true;
	}
	std::array<CCoord, 2> sizes;
};

class SeparatorSubView : public CView
{
public:
	SeparatorSubView () : CView (CRect (0, 0, 0, 0)) {}

	bool mouseDownCalled {false};
	bool mouseMovedCalled {false};
	bool mouseUpCalled {false};
	bool mouseCancelCalled {false};

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override
	{
		mouseDownCalled = true;
		return kMouseEventHandled;
	}

	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override
	{
		mouseMovedCalled = true;
		return kMouseEventHandled;
	}

	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override
	{
		mouseUpCalled = true;
		return kMouseEventHandled;
	}

	CMouseEventResult onMouseCancel () override
	{
		mouseCancelCalled = true;
		return kMouseEventHandled;
	}
};

} // anonymous

TEST_CASE (CSplitViewTest, AddViewsHorizontal)
{
	auto sv = owned (new CSplitView (CRect (0, 0, 100, 100)));
	sv->setStyle (CSplitView::kHorizontal);
	sv->setSeparatorWidth (10);
	auto view1 = new CView (CRect (0, 0, 50, 100));
	auto view2 = new CView (CRect (0, 0, 40, 100));
	sv->addView (view1);
	EXPECT (sv->getNbViews () == 1);
	sv->addView (view2);
	EXPECT (sv->getNbViews () == 3);
	EXPECT (view1->getViewSize () == CRect (0, 0, 50, 100));
	EXPECT (view2->getViewSize () == CRect (60, 0, 100, 100));
	sv->removeView (view1);
	EXPECT (sv->getNbViews () == 1);
	EXPECT (view2->getViewSize () == CRect (60, 0, 100, 100));
}

TEST_CASE (CSplitViewTest, AddViewsVertical)
{
	auto sv = owned (new CSplitView (CRect (0, 0, 100, 100)));
	sv->setStyle (CSplitView::kVertical);
	sv->setSeparatorWidth (10);
	auto view1 = new CView (CRect (0, 0, 100, 50));
	auto view2 = new CView (CRect (0, 0, 100, 40));
	sv->addView (view1);
	EXPECT (sv->getNbViews () == 1);
	sv->addView (view2);
	EXPECT (sv->getNbViews () == 3);
	EXPECT (view1->getViewSize () == CRect (0, 0, 100, 50));
	EXPECT (view2->getViewSize () == CRect (0, 60, 100, 100));
	sv->removeView (view2);
	EXPECT (sv->getNbViews () == 1);
	EXPECT (view1->getViewSize () == CRect (0, 0, 100, 50));
}

TEST_CASE (CSplitViewTest, ResizeAllViewsHorizontal)
{
	auto sv = owned (new CSplitView (CRect (0, 0, 100, 100)));
	sv->setStyle (CSplitView::kHorizontal);
	sv->setResizeMethod (CSplitView::kResizeAllViews);
	sv->setSeparatorWidth (10);
	auto view1 = new CView (CRect (0, 0, 50, 100));
	auto view2 = new CView (CRect (0, 0, 40, 100));
	sv->addView (view1);
	sv->addView (view2);
	sv->setViewSize (CRect (0, 0, 120, 100));
	EXPECT (view1->getViewSize () == CRect (0, 0, 60, 100));
	EXPECT (view2->getViewSize () == CRect (70, 0, 120, 100));
}

TEST_CASE (CSplitViewTest, ResizeAllViewsVertical)
{
	auto sv = owned (new CSplitView (CRect (0, 0, 100, 100)));
	sv->setStyle (CSplitView::kVertical);
	sv->setResizeMethod (CSplitView::kResizeAllViews);
	sv->setSeparatorWidth (10);
	auto view1 = new CView (CRect (0, 0, 100, 50));
	auto view2 = new CView (CRect (0, 0, 100, 40));
	sv->addView (view1);
	sv->addView (view2);
	sv->setViewSize (CRect (0, 0, 100, 120));
	EXPECT (view1->getViewSize () == CRect (0, 0, 100, 60));
	EXPECT (view2->getViewSize () == CRect (0, 70, 100, 120));
}

TEST_CASE (CSplitViewTest, ResizeFirstViewHorizontal)
{
	auto sv = owned (new CSplitView (CRect (0, 0, 100, 100)));
	sv->setStyle (CSplitView::kHorizontal);
	sv->setResizeMethod (CSplitView::kResizeFirstView);
	sv->setSeparatorWidth (10);
	auto view1 = new CView (CRect (0, 0, 50, 100));
	auto view2 = new CView (CRect (0, 0, 40, 100));
	sv->addView (view1);
	sv->addView (view2);
	sv->setViewSize (CRect (0, 0, 120, 100));
	EXPECT (view1->getViewSize () == CRect (0, 0, 70, 100));
	EXPECT (view2->getViewSize () == CRect (80, 0, 120, 100));
}

TEST_CASE (CSplitViewTest, ResizeFirstViewVertical)
{
	auto sv = owned (new CSplitView (CRect (0, 0, 100, 100)));
	sv->setStyle (CSplitView::kVertical);
	sv->setResizeMethod (CSplitView::kResizeFirstView);
	sv->setSeparatorWidth (10);
	auto view1 = new CView (CRect (0, 0, 100, 50));
	auto view2 = new CView (CRect (0, 0, 100, 40));
	sv->addView (view1);
	sv->addView (view2);
	sv->setViewSize (CRect (0, 0, 100, 120));
	EXPECT (view1->getViewSize () == CRect (0, 0, 100, 70));
	EXPECT (view2->getViewSize () == CRect (0, 80, 100, 120));
}

TEST_CASE (CSplitViewTest, ResizeLastViewHorizontal)
{
	auto sv = owned (new CSplitView (CRect (0, 0, 100, 100)));
	sv->setStyle (CSplitView::kHorizontal);
	sv->setResizeMethod (CSplitView::kResizeLastView);
	sv->setSeparatorWidth (10);
	auto view1 = new CView (CRect (0, 0, 50, 100));
	auto view2 = new CView (CRect (0, 0, 40, 100));
	sv->addView (view1);
	sv->addView (view2);
	sv->setViewSize (CRect (0, 0, 120, 100));
	EXPECT (view1->getViewSize () == CRect (0, 0, 50, 100));
	EXPECT (view2->getViewSize () == CRect (60, 0, 120, 100));
}

TEST_CASE (CSplitViewTest, resizeLastViewVertical)
{
	auto sv = owned (new CSplitView (CRect (0, 0, 100, 100)));
	sv->setStyle (CSplitView::kVertical);
	sv->setResizeMethod (CSplitView::kResizeLastView);
	sv->setSeparatorWidth (10);
	auto view1 = new CView (CRect (0, 0, 100, 50));
	auto view2 = new CView (CRect (0, 0, 100, 40));
	sv->addView (view1);
	sv->addView (view2);
	sv->setViewSize (CRect (0, 0, 100, 120));
	EXPECT (view1->getViewSize () == CRect (0, 0, 100, 50));
	EXPECT (view2->getViewSize () == CRect (0, 60, 100, 120));
}

TEST_CASE (CSplitViewTest, ResizeSecondViewHorizontal)
{
	auto sv = owned (new CSplitView (CRect (0, 0, 100, 100)));
	sv->setStyle (CSplitView::kHorizontal);
	sv->setResizeMethod (CSplitView::kResizeSecondView);
	sv->setSeparatorWidth (10);
	auto view1 = new CView (CRect (0, 0, 50, 100));
	auto view2 = new CView (CRect (0, 0, 20, 100));
	auto view3 = new CView (CRect (0, 0, 10, 100));
	sv->addView (view1);
	sv->addView (view2);
	sv->addView (view3);
	sv->setViewSize (CRect (0, 0, 120, 100));
	EXPECT (view1->getViewSize () == CRect (0, 0, 50, 100));
	EXPECT (view2->getViewSize () == CRect (60, 0, 100, 100));
	EXPECT (view3->getViewSize () == CRect (110, 0, 120, 100));
}

TEST_CASE (CSplitViewTest, ResizeSecondViewVertical)
{
	auto sv = owned (new CSplitView (CRect (0, 0, 100, 100)));
	sv->setStyle (CSplitView::kVertical);
	sv->setResizeMethod (CSplitView::kResizeSecondView);
	sv->setSeparatorWidth (10);
	auto view1 = new CView (CRect (0, 0, 100, 50));
	auto view2 = new CView (CRect (0, 0, 100, 20));
	auto view3 = new CView (CRect (0, 0, 100, 10));
	sv->addView (view1);
	sv->addView (view2);
	sv->addView (view3);
	sv->setViewSize (CRect (0, 0, 100, 120));
	EXPECT (view1->getViewSize () == CRect (0, 0, 100, 50));
	EXPECT (view2->getViewSize () == CRect (0, 60, 100, 100));
	EXPECT (view3->getViewSize () == CRect (0, 110, 100, 120));
}

TEST_CASE (CSplitViewTest, SetSeparatorWidth)
{
	auto sv = owned (new CSplitView (CRect (0, 0, 100, 100)));
	sv->setStyle (CSplitView::kHorizontal);
	sv->setResizeMethod (CSplitView::kResizeFirstView);
	sv->setSeparatorWidth (10);
	auto view1 = new CView (CRect (0, 0, 40, 100));
	auto view2 = new CView (CRect (0, 0, 20, 100));
	auto view3 = new CView (CRect (0, 0, 20, 100));
	sv->addView (view1);
	sv->addView (view2);
	sv->addView (view3);
	EXPECT (view1->getViewSize () == CRect (0, 0, 40, 100));
	EXPECT (view2->getViewSize () == CRect (50, 0, 70, 100));
	EXPECT (view3->getViewSize () == CRect (80, 0, 100, 100));
	sv->setSeparatorWidth (20);
	EXPECT (view1->getViewSize () == CRect (0, 0, 40, 100));
	EXPECT (view2->getViewSize () == CRect (60, 0, 70, 100));
	EXPECT (view3->getViewSize () == CRect (90, 0, 100, 100));
}

TEST_CASE (CSplitViewTest, ControllerHorizontal)
{
	auto controller = new SplitViewController ();
	auto container = owned (new CViewContainer (CRect (0, 0, 100, 100)));
	auto sv = owned (new CSplitView (CRect (0, 0, 100, 100)));
	sv->setAttribute (kCViewControllerAttribute, controller);
	sv->setStyle (CSplitView::kHorizontal);
	sv->setSeparatorWidth (10);
	auto view1 = new CView (CRect (0, 0, 40, 100));
	auto view2 = new CView (CRect (0, 0, 50, 100));
	sv->addView (view1);
	sv->addView (view2);
	sv->attached (container);
	EXPECT (view1->getViewSize () == CRect (0, 0, 20, 100));
	EXPECT (view2->getViewSize () == CRect (30, 0, 100, 100));

	MouseDownEvent downEvent ({25, 1}, MouseEventButtonState::Left);
	sv->onMouseDownEvent (downEvent);
	MouseMoveEvent moveEvent ({55, 1}, MouseEventButtonState::Left);
	sv->onMouseMoveEvent (moveEvent);
	EXPECT (view1->getViewSize () == CRect (0, 0, 50, 100));
	EXPECT (view2->getViewSize () == CRect (60, 0, 100, 100));
	moveEvent.mousePosition (65, 1);
	sv->onMouseMoveEvent (moveEvent);
	EXPECT (view1->getViewSize () == CRect (0, 0, 50, 100));
	EXPECT (view2->getViewSize () == CRect (60, 0, 100, 100));
	moveEvent.mousePosition (15, 1);
	sv->onMouseMoveEvent (moveEvent);
	EXPECT (view1->getViewSize () == CRect (0, 0, 10, 100));
	EXPECT (view2->getViewSize () == CRect (20, 0, 100, 100));
	moveEvent.mousePosition (1, 1);
	sv->onMouseMoveEvent (moveEvent);
	EXPECT (view1->getViewSize () == CRect (0, 0, 10, 100));
	EXPECT (view2->getViewSize () == CRect (20, 0, 100, 100));
	MouseUpEvent upEvent (moveEvent.mousePosition, moveEvent.buttonState);
	sv->onMouseUpEvent (upEvent);

	sv->removed (container);
	EXPECT (controller->sizes[0] == 10);
	EXPECT (controller->sizes[1] == 80);
}

TEST_CASE (CSplitViewTest, ControllerVertical)
{
	auto controller = new SplitViewController ();
	auto container = owned (new CViewContainer (CRect (0, 0, 100, 100)));
	auto sv = owned (new CSplitView (CRect (0, 0, 100, 100)));
	sv->setAttribute (kCViewControllerAttribute, controller);
	sv->setStyle (CSplitView::kVertical);
	sv->setSeparatorWidth (10);
	auto view1 = new CView (CRect (0, 0, 100, 40));
	auto view2 = new CView (CRect (0, 0, 100, 50));
	sv->addView (view1);
	sv->addView (view2);
	sv->attached (container);
	EXPECT (view1->getViewSize () == CRect (0, 0, 100, 20));
	EXPECT (view2->getViewSize () == CRect (0, 30, 100, 100));
	MouseDownEvent downEvent ({1, 25}, MouseEventButtonState::Left);
	sv->onMouseDownEvent (downEvent);
	MouseMoveEvent moveEvent ({1, 55}, MouseEventButtonState::Left);
	sv->onMouseMoveEvent (moveEvent);
	EXPECT (view1->getViewSize () == CRect (0, 0, 100, 50));
	EXPECT (view2->getViewSize () == CRect (0, 60, 100, 100));
	moveEvent.mousePosition (1, 65);
	sv->onMouseMoveEvent (moveEvent);
	EXPECT (view1->getViewSize () == CRect (0, 0, 100, 50));
	EXPECT (view2->getViewSize () == CRect (0, 60, 100, 100));
	moveEvent.mousePosition (1, 15);
	sv->onMouseMoveEvent (moveEvent);
	EXPECT (view1->getViewSize () == CRect (0, 0, 100, 10));
	EXPECT (view2->getViewSize () == CRect (00, 20, 100, 100));
	moveEvent.mousePosition (1, 1);
	sv->onMouseMoveEvent (moveEvent);
	EXPECT (view1->getViewSize () == CRect (0, 0, 100, 10));
	EXPECT (view2->getViewSize () == CRect (0, 20, 100, 100));
	MouseUpEvent upEvent (moveEvent.mousePosition, moveEvent.buttonState);
	sv->onMouseUpEvent (upEvent);

	sv->removed (container);
	EXPECT (controller->sizes[0] == 10);
	EXPECT (controller->sizes[1] == 80);
}

TEST_CASE (CSplitViewTest, SeparatorSubView)
{
	auto container = owned (new CViewContainer (CRect (0, 0, 100, 100)));
	auto sv = owned (new CSplitView (CRect (0, 0, 100, 100)));
	sv->setStyle (CSplitView::kHorizontal);
	sv->setResizeMethod (CSplitView::kResizeFirstView);
	sv->setSeparatorWidth (10);
	auto view1 = new CView (CRect (0, 0, 40, 100));
	auto view2 = new CView (CRect (0, 0, 20, 100));
	sv->addView (view1);
	sv->addView (view2);
	auto sepView = new SeparatorSubView ();
	sepView->setViewSize (CRect (0, 0, 10, 10));
	sepView->setMouseableArea (CRect (0, 0, 10, 10));
	sv->addViewToSeparator (0, sepView);
	sv->attached (container);
	MouseDownEvent downEvent ({41, 25}, MouseEventButtonState::Left);
	sv->onMouseDownEvent (downEvent);
	EXPECT (sepView->mouseDownCalled == false);
	downEvent.mousePosition (41, 1);
	sv->onMouseDownEvent (downEvent);
	EXPECT (sepView->mouseDownCalled);
	MouseMoveEvent moveEvent ({41, 3}, MouseEventButtonState::Left);
	sv->onMouseMoveEvent (moveEvent);
	EXPECT (sepView->mouseMovedCalled);
	MouseUpEvent upEvent ({41, 3}, MouseEventButtonState::Left);
	sv->onMouseUpEvent (upEvent);
	EXPECT (sepView->mouseUpCalled);
	downEvent.mousePosition (41, 1);
	sv->onMouseDownEvent (downEvent);
	sv->onMouseCancel ();
	EXPECT (sepView->mouseCancelCalled);
	sv->removed (container);
}

} // VSTGUI
