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
#include "../../../lib/cstring.h"
#include "../../../lib/cview.h"
#include "../../../lib/cviewcontainer.h"
#include "../../../lib/iviewlistener.h"
#include "../../../lib/idatapackage.h"

#if MAC
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace VSTGUI {
namespace {
class View : public CView
{
public:
	View () : CView (CRect (0, 0, 10, 10)) {}
	void onIdle () override { onIdleCalled = true; }
	
	bool onIdleCalled {false};
};

struct ViewListener : public IViewListener
{
	void viewSizeChanged (CView* view, const CRect& oldSize) override
	{
		sizeChangedCalled = true;
	}
	void viewAttached (CView* view) override
	{
		attachedCalled = true;
	}
	void viewRemoved (CView* view) override
	{
		removedCalled = true;
	}
	void viewLostFocus (CView* view) override
	{
		lostFocusCalled = true;
	}
	void viewTookFocus (CView* view) override
	{
		tookFocusCalled = true;
	}
	void viewWillDelete (CView* view) override
	{
		view->unregisterViewListener (this);
		willDeleteCalled = true;
	}

	bool sizeChangedCalled {false};
	bool attachedCalled {false};
	bool removedCalled {false};
	bool lostFocusCalled {false};
	bool tookFocusCalled {false};
	bool willDeleteCalled {false};
};

} // anonymous

TESTCASE(CViewTest,

	TEST(visibleState,
		View v;
		EXPECT(v.isVisible () == true);
		v.setVisible(false);
		EXPECT(v.isVisible () == false);
		v.setVisible(true);
		EXPECT(v.isVisible () == true);
		v.setAlphaValue (0.f);
		EXPECT(v.isVisible () == false);
	);

	TEST(transparencyState,
		View v;
		EXPECT(v.getTransparency () == false);
		v.setTransparency (true);
		EXPECT(v.getTransparency () == true);
		v.setTransparency (false);
		EXPECT(v.getTransparency () == false);
	);

	TEST(focusState,
		View v;
		EXPECT(v.wantsFocus () == false);
		v.setWantsFocus (true);
		EXPECT(v.wantsFocus () == true);
		v.setWantsFocus (false);
		EXPECT(v.wantsFocus () == false);
	);

	TEST(idleState,
		View v;
		EXPECT(v.wantsIdle () == false);
		v.setWantsIdle (true);
		EXPECT(v.wantsIdle () == true);
		v.setWantsIdle (false);
		EXPECT(v.wantsIdle () == false);
	);

	TEST(mouseEnabledState,
		View v;
		EXPECT(v.getMouseEnabled () == true);
		v.setMouseEnabled (false);
		EXPECT(v.getMouseEnabled () == false);
		v.setMouseEnabled (true);
		EXPECT(v.getMouseEnabled () == true);
	);

	TEST(autosizeFlags,
		View v;
		EXPECT(v.getAutosizeFlags () == kAutosizeNone);
		v.setAutosizeFlags (kAutosizeLeft);
		EXPECT(v.getAutosizeFlags () == kAutosizeLeft);
		v.setAutosizeFlags (kAutosizeLeft|kAutosizeTop);
		EXPECT(v.getAutosizeFlags () == (kAutosizeLeft|kAutosizeTop));
	);

	TEST(attributes,
		View v;
		uint32_t outSize;
		void* outData = nullptr;
		EXPECT(v.getAttribute (0, 10, outData, outSize) == false);
		EXPECT(v.removeAttribute (0) == false);
		uint64_t myAttr = 500;
		EXPECT(v.setAttribute (0, 0, &myAttr) == false);
		EXPECT(v.setAttribute (0, sizeof(myAttr), nullptr) == false);
		EXPECT(v.setAttribute ('myAt', sizeof(myAttr), &myAttr) == true);
		myAttr = 10;
		EXPECT(v.getAttributeSize ('myAt', outSize) == true);
		EXPECT(outSize == sizeof (myAttr));
		EXPECT(v.getAttribute ('myAt', sizeof(myAttr), &myAttr, outSize) == true);
		EXPECT(myAttr == 500);
		myAttr = 100;
		EXPECT(v.setAttribute ('myAt', sizeof(myAttr), &myAttr) == true);
		myAttr = 102;
		EXPECT(v.getAttribute ('myAt', sizeof(myAttr), &myAttr, outSize) == true);
		EXPECT(myAttr == 100);
		EXPECT(v.removeAttribute ('myAt') == true);
		EXPECT(v.getAttribute ('myAt', sizeof(myAttr), &myAttr, outSize) == false);
	);

	TEST(resizeAttribute,
		View v;
		uint32_t outSize;
		uint8_t firstData = 8;
		EXPECT(v.setAttribute(0, sizeof(firstData), &firstData));
		firstData = 0;
		EXPECT(v.getAttribute (0, sizeof(firstData), &firstData, outSize));
		EXPECT(firstData == 8);
		uint32_t secondData = 32;
		EXPECT(v.setAttribute(0, sizeof(secondData), &secondData));
		secondData = 0;
		EXPECT(v.getAttribute (0, sizeof(firstData), &firstData, outSize) == false);
		EXPECT(v.getAttribute (0, sizeof(secondData), &secondData, outSize));
		EXPECT(secondData == 32);
		
	);

	TEST(viewListener,
		ViewListener listener;
		{
			auto v = new View ();
			v->registerViewListener (&listener);
			v->setViewSize (CRect (1, 2, 3, 4));
			auto container1 = owned (new CViewContainer (CRect (0, 0, 100, 100)));
			auto container2 = owned (new CViewContainer (CRect (0, 0, 100, 100)));
			container2->addView(v);
			container2->attached (container1);
			v->takeFocus ();
			v->looseFocus ();
			container2->removeView (v);
			container2->removed (container1);
		}
		EXPECT(listener.sizeChangedCalled);
		EXPECT(listener.attachedCalled);
		EXPECT(listener.removedCalled);
		EXPECT(listener.tookFocusCalled);
		EXPECT(listener.lostFocusCalled);
		EXPECT(listener.willDeleteCalled);
	);
	
	TEST(coordCalculations,
		auto parent = owned (new CViewContainer (CRect (0, 0, 100, 100)));
		auto container = owned (new CViewContainer (CRect (50, 50, 100, 100)));
		container->attached (parent);
		auto v = new View ();
		container->addView (v);
		CPoint p (0, 0);
		v->localToFrame (p);
		EXPECT(p.x == 50 && p.y == 50);
		p (52, 53);
		v->frameToLocal (p);
		EXPECT(p.x == 2 && p.y == 3);
		container->removed (parent);
	);

	TEST(visibleViewSize,
		auto parent = owned (new CViewContainer (CRect (0, 0, 100, 100)));
		auto container = owned (new CViewContainer (CRect (50, 50, 100, 100)));
		container->attached (parent);
		auto v = new View ();
		v->setViewSize (CRect (20, 20, 150, 150));
		container->addView (v);
		auto visible = v->getVisibleViewSize ();
		EXPECT(visible == CRect (20, 20, 50, 50));
		container->removeView (v);
		container->removed (parent);
	);

	TEST(globalTransform,
		auto container1 = owned (new CViewContainer (CRect (0, 0, 10, 10)));
		auto container2 = new CViewContainer (CRect (0, 0, 10, 10));
		container1->setTransform (CGraphicsTransform ().translate (10, 20));
		container2->setTransform (CGraphicsTransform ().translate (15, 35));
		container1->addView (container2);
		auto v = new View ();
		container2->addView (v);
		container2->attached (container1);
		auto transform = v->getGlobalTransform ();
		EXPECT(transform.dx == 25 && transform.dy == 55);
		
		CPoint p (0, 0);
		v->translateToGlobal (p);
		EXPECT(p.x == 25 && p.y == 55);
		v->translateToLocal (p);
		EXPECT(p.x == 0 && p.y == 0);

		auto p2 = v->translateToGlobal (CRect (0, 0, 1, 1));
		EXPECT(p2 == CRect (25, 55, 26, 56));
		p2 = v->translateToLocal(CRect (25, 55, 26, 56));
		EXPECT(p2 == CRect (0, 0, 1, 1));
		
		container2->removed (container1);
	);

	TEST(hitTest,
		auto v = owned (new View ());
		v->setMouseableArea (CRect (20, 20, 40, 40));
		EXPECT(v->hitTest (CPoint (5, 5)) == false);
		EXPECT(v->hitTest (CPoint (20, 20)) == true);
		EXPECT(v->hitTest (CPoint (40, 40)) == false);
	);

	TEST(defaultHandling,
		View v;
		VstKeyCode key;
		EXPECT(v.onKeyDown (key) == -1);
		EXPECT(v.onKeyUp (key) == -1);
		EXPECT(v.onWheel (CPoint (0, 0), kMouseWheelAxisX, 1.f, 0) == false);
		EXPECT(v.onWheel (CPoint (0, 0), kMouseWheelAxisY, 1.f, 0) == false);
		EXPECT(v.onWheel (CPoint (0, 0), 1.f, 0) == false);
		CPoint p (0, 0);
		EXPECT(v.onMouseDown (p, kLButton) == kMouseEventNotImplemented);
		EXPECT(v.onMouseUp (p, kLButton) == kMouseEventNotImplemented);
		EXPECT(v.onMouseMoved (p, kLButton) == kMouseEventNotImplemented);
		EXPECT(v.onMouseCancel () == kMouseEventNotImplemented);
		EXPECT(v.onMouseEntered (p, kLButton) == kMouseEventNotImplemented);
		EXPECT(v.onMouseExited (p, kLButton) == kMouseEventNotImplemented);
		EXPECT(v.notify (nullptr, nullptr) == kMessageUnknown);
		EXPECT(v.doDrag (nullptr) == kDragError);
		EXPECT(v.onDrop (nullptr, p) == false);
		EXPECT(v.getEditor () == nullptr);
		EXPECT(v.isDirty () == false);
		EXPECT(v.sizeToFit () == false);
		EXPECT(v.getBackground () == nullptr);
		EXPECT(v.getDisabledBackground () == nullptr);
		EXPECT(v.getDrawBackground () == nullptr);
		EXPECT(v.checkUpdate (CRect (0, 0, 5, 5)) == true);
		EXPECT(v.getWidth () == 10);
		EXPECT(v.getHeight () == 10);
		CRect r;
		EXPECT(v.getViewSize (r) == v.getViewSize ());
		EXPECT(v.getMouseableArea (r) == v.getMouseableArea ());
	);
	
);

#if MAC
TESTCASE(CViewMacTest,

	TEST(idleAfterAttached,
		auto parent = owned (new CViewContainer (CRect (0, 0, 100, 100)));
		auto container = owned (new CViewContainer (CRect (50, 50, 100, 100)));
		container->attached (parent);
		auto v = new View ();
		container->addView (v);
		v->setWantsIdle (true);
		CFRunLoopRunInMode (kCFRunLoopDefaultMode, 0.2, true);
		EXPECT(v->onIdleCalled == true);
		v->setWantsIdle (false);
		v->onIdleCalled = false;
		CFRunLoopRunInMode (kCFRunLoopDefaultMode, 0.2, true);
		EXPECT(v->onIdleCalled == false);
		container->removeView (v);
		container->removed (parent);
	);

	TEST(idleBeforeAttached,
		auto parent = owned (new CViewContainer (CRect (0, 0, 100, 100)));
		auto container = owned (new CViewContainer (CRect (50, 50, 100, 100)));
		auto v = new View ();
		container->addView (v);
		v->setWantsIdle (true);
		container->attached (parent);
		CFRunLoopRunInMode (kCFRunLoopDefaultMode, 0.2, true);
		EXPECT(v->onIdleCalled == true);
		container->removeView (v);
		container->removed (parent);
	);

);

#endif

struct DataPackage : IDataPackage
{
	UTF8String str;
	UTF8String path;
	int8_t binary[3];
	
	uint32_t getCount () const override
	{
		return 3;
	}
	
	uint32_t getDataSize (uint32_t index) const override
	{
		if (index == 0)
			return static_cast<uint32_t> (str.getByteCount ());
		else if (index == 1)
			return static_cast<uint32_t> (path.getByteCount ());
		else if (index == 2)
			return 3;
		return 0;
	}
	
	Type getDataType (uint32_t index) const override
	{
		if (index == 0)
			return kText;
		else if (index == 1)
			return kFilePath;
		else if (index == 2)
			return kBinary;
		return kError;
	}
	
	uint32_t getData (uint32_t index, const void*& buffer, Type& type) const override
	{
		type = kError;
		if (index == 0)
		{
			buffer = str.get ();
			type = kText;
		}
		else if (index == 1)
		{
			buffer = path.get ();
			type = kFilePath;
		}
		else if (index == 2)
		{
			buffer = binary;
			type = kBinary;
		}
		return getDataSize (index);
	}
	
};

TESTCASE(CDragContainerHelperTest,

	TEST(count,
		DataPackage package;
		CDragContainerHelper helper (&package);
		EXPECT(helper.getCount () == 3);
	);

	TEST(getType,
		DataPackage package;
		CDragContainerHelper helper (&package);
		EXPECT(helper.getType (0) == CDragContainerHelper::kUnicodeText);
		EXPECT(helper.getType (1) == CDragContainerHelper::kFile);
		EXPECT(helper.getType (2) == CDragContainerHelper::kUnknown);
		EXPECT(helper.getType (3) == CDragContainerHelper::kError);
	);

	TEST(iteration,
		DataPackage package;
		package.str = "Test";
		package.path = "/var/tmp/test";
		CDragContainerHelper helper (&package);
		int32_t size;
		int32_t type;
		auto res = helper.first (size, type);
		EXPECT(res == package.str.get ());
		EXPECT(size == static_cast<int32_t> (package.str.getByteCount ()));
		EXPECT(type == CDragContainerHelper::kUnicodeText);
		res = helper.next(size, type);
		EXPECT(res == package.path.get ());
		EXPECT(size == static_cast<int32_t> (package.path.getByteCount ()));
		EXPECT(type == CDragContainerHelper::kFile);
		res = helper.next(size, type);
		EXPECT(res == package.binary);
		EXPECT(size == 3);
		EXPECT(type == CDragContainerHelper::kUnknown);
		res = helper.next(size, type);
		EXPECT(res == nullptr);
		EXPECT(size == 0);
		EXPECT(type == CDragContainerHelper::kError);
	);

);

} // VSTGUI
