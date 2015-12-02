//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "../unittests.h"
#include "../../../lib/csplitview.h"
#include "../../../uidescription/icontroller.h"
#include <array>

namespace VSTGUI {

namespace {

class SplitViewController : public IController, public ISplitViewController
{
public:
	void valueChanged (CControl* pControl) override {}
	bool getSplitViewSizeConstraint (int32_t index, CCoord& minSize, CCoord& maxSize, CSplitView* splitView) override
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
	
	ISplitViewSeparatorDrawer* getSplitViewSeparatorDrawer (CSplitView* splitView) override { return nullptr; }

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

TESTCASE(CSplitViewTests,

	TEST(addViewsHorizontal,
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
	);

	TEST(addViewsVertical,
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
	);
	
	TEST(resizeAllViewsHorizontal,
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
	);

	TEST(resizeAllViewsVertical,
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
	);

	TEST(resizeFirstViewHorizontal,
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
	);

	TEST(resizeFirstViewVertical,
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
	);

	TEST(resizeLastViewHorizontal,
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
	);

	TEST(resizeLastViewVertical,
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
	);

	TEST(resizeSecondViewHorizontal,
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
	);

	TEST(resizeSecondViewVertical,
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
	);

	TEST(setSeparatorWidth,
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
	);
	
	TEST(controllerHorizontal,
		auto controller = new SplitViewController ();
		auto container = owned (new CViewContainer (CRect (0, 0, 100, 100)));
		auto sv = owned (new CSplitView (CRect (0, 0, 100, 100)));
		sv->setAttribute (kCViewControllerAttribute, sizeof (IController*), &controller);
		sv->setStyle (CSplitView::kHorizontal);
		sv->setSeparatorWidth (10);
		auto view1 = new CView (CRect (0, 0, 40, 100));
		auto view2 = new CView (CRect (0, 0, 50, 100));
		sv->addView (view1);
		sv->addView (view2);
		sv->attached (container);
		EXPECT (view1->getViewSize () == CRect (0, 0, 20, 100));
		EXPECT (view2->getViewSize () == CRect (30, 0, 100, 100));
		CPoint p (25, 1);
		sv->onMouseDown (p, kLButton);
		p (55, 1);
		sv->onMouseMoved (p, kLButton);
		EXPECT (view1->getViewSize () == CRect (0, 0, 50, 100));
		EXPECT (view2->getViewSize () == CRect (60, 0, 100, 100));
		p (65, 1);
		sv->onMouseMoved (p, kLButton);
		EXPECT (view1->getViewSize () == CRect (0, 0, 50, 100));
		EXPECT (view2->getViewSize () == CRect (60, 0, 100, 100));
		p (15, 1);
		sv->onMouseMoved (p, kLButton);
		EXPECT (view1->getViewSize () == CRect (0, 0, 10, 100));
		EXPECT (view2->getViewSize () == CRect (20, 0, 100, 100));
		p (1, 1);
		sv->onMouseMoved (p, kLButton);
		EXPECT (view1->getViewSize () == CRect (0, 0, 10, 100));
		EXPECT (view2->getViewSize () == CRect (20, 0, 100, 100));
		sv->onMouseUp (p, kLButton);
		
		sv->removed (container);
		EXPECT(controller->sizes[0] == 10);
		EXPECT(controller->sizes[1] == 80);
	);

	TEST(controllerVertical,
		auto controller = new SplitViewController ();
		auto container = owned (new CViewContainer (CRect (0, 0, 100, 100)));
		auto sv = owned (new CSplitView (CRect (0, 0, 100, 100)));
		sv->setAttribute (kCViewControllerAttribute, sizeof (IController*), &controller);
		sv->setStyle (CSplitView::kVertical);
		sv->setSeparatorWidth (10);
		auto view1 = new CView (CRect (0, 0, 100, 40));
		auto view2 = new CView (CRect (0, 0, 100, 50));
		sv->addView (view1);
		sv->addView (view2);
		sv->attached (container);
		EXPECT (view1->getViewSize () == CRect (0, 0, 100, 20));
		EXPECT (view2->getViewSize () == CRect (0, 30, 100, 100));
		CPoint p (1, 25);
		sv->onMouseDown (p, kLButton);
		p (1, 55);
		sv->onMouseMoved (p, kLButton);
		EXPECT (view1->getViewSize () == CRect (0, 0, 100, 50));
		EXPECT (view2->getViewSize () == CRect (0, 60, 100, 100));
		p (1, 65);
		sv->onMouseMoved (p, kLButton);
		EXPECT (view1->getViewSize () == CRect (0, 0, 100, 50));
		EXPECT (view2->getViewSize () == CRect (0, 60, 100, 100));
		p (1, 15);
		sv->onMouseMoved (p, kLButton);
		EXPECT (view1->getViewSize () == CRect (0, 0, 100, 10));
		EXPECT (view2->getViewSize () == CRect (00, 20, 100, 100));
		p (1, 1);
		sv->onMouseMoved (p, kLButton);
		EXPECT (view1->getViewSize () == CRect (0, 0, 100, 10));
		EXPECT (view2->getViewSize () == CRect (0, 20, 100, 100));
		sv->onMouseUp (p, kLButton);
		
		sv->removed (container);
		EXPECT(controller->sizes[0] == 10);
		EXPECT(controller->sizes[1] == 80);
	);
	
	TEST(separatorSubView,
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
		CPoint p (41, 25);
		sv->onMouseDown (p, kLButton);
		EXPECT(sepView->mouseDownCalled == false);
		p (41, 1);
		sv->onMouseDown (p, kLButton);
		EXPECT(sepView->mouseDownCalled);
		p (41, 3);
		sv->onMouseMoved (p, kLButton);
		EXPECT(sepView->mouseMovedCalled);
		p (41, 3);
		sv->onMouseUp (p, kLButton);
		EXPECT(sepView->mouseUpCalled);
		p (41, 1);
		sv->onMouseDown (p, kLButton);
		sv->onMouseCancel ();
		EXPECT(sepView->mouseCancelCalled);
		sv->removed (container);
	);
);

} // VSTGUI
