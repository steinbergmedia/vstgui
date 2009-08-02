/*
 *  dialog.cpp
 *  VST3PlugIns
 *
 *  Created by Arne Scheffler on 7/16/09.
 *  Copyright 2009 Arne Scheffler. All rights reserved.
 *
 */

#if VSTGUI_LIVE_EDITING

#include "dialog.h"
#include "../vstkeycode.h"

BEGIN_NAMESPACE_VSTGUI

//-----------------------------------------------------------------------------
bool Dialog::runViewModal (CPoint& position, CView* view, long style, const char* title)
{
	Dialog dialog (position, view, style, title);
	return dialog.run ();
}

//-----------------------------------------------------------------------------
class SimpleButton : public CKickButton
{
public:
	SimpleButton (const CRect& size, CControlListener* listener, long tag, const char* _title)
	: CKickButton (size, listener, tag, 0)
	{
		if (_title)
			title = _title;
		setWantsFocus (true);
	}

	void draw (CDrawContext* context)
	{
		context->setDrawMode (kCopyMode);
		context->setLineWidth (1);
		context->setLineStyle (kLineSolid);
		context->setFillColor (value > 0.5 ? kGreyCColor : kWhiteCColor);
		context->setFrameColor (kBlackCColor);
		context->drawRect (size, kDrawFilledAndStroked);
		if (getFrame ()->getFocusView () == this)
		{
			CRect r (size);
			r.inset (1, 1);
			context->setFrameColor (MakeCColor (255, 100, 100, 255));
			context->drawRect (r, kDrawStroked);
		}
		context->setFont (kSystemFont);
		context->setFontColor (kBlackCColor);
		context->drawStringUTF8 (title.c_str (), size);
	}

	long onKeyDown (VstKeyCode& keyCode)
	{
		if ((keyCode.virt == VKEY_ENTER || keyCode.virt == VKEY_RETURN) && getFrame ()->getFocusView () == this)
		{
			beginEdit ();
			value = 1;
			if (listener)
				listener->valueChanged (this);
			value = 0;
			endEdit ();
			return 1;
		}
		return -1;
	}
	
protected:
	std::string title;
};

//-----------------------------------------------------------------------------
enum {
	kOkTag,
	kCancelTag
};

#if MAC
#define okIsRightMost 1
#else
#define okIsRightMost 0
#endif

//-----------------------------------------------------------------------------
Dialog::Dialog (const CPoint& position, CView* rootView, long style, const char* title)
: platformWindow (0)
, result (false)
{
	const CCoord kMargin = 10;
	const CCoord kControlHeight = 22;
	CRect size (rootView->getViewSize ());
	size.offset (-size.left, -size.top);
	size.offset (kMargin, kMargin);
	rootView->setViewSize (size);
	rootView->setMouseableArea (size);
	size.offset (-kMargin, -kMargin);
	size.bottom += kMargin*3+kControlHeight;
	size.right += kMargin*2;
	size.offset (position.x, position.y);
	platformWindow = PlatformWindow::create (size, title, PlatformWindow::kWindowType, 0, 0);
	if (platformWindow)
	{
		if (position.x == -1 && position.y == -1)
			platformWindow->center ();
		size.offset (position.x, position.y);
		#if MAC && !__LP64__
		CFrame::setCocoaMode (true);
		#endif
		frame = new CFrame (size, platformWindow->getPlatformHandle (), this);
		frame->setKeyboardHook (this);
		CViewContainer* rootContainer = dynamic_cast<CViewContainer*> (rootView);
		if (rootContainer)
		{
			frame->setTransparency (rootContainer->getTransparency ());
			frame->setBackgroundColor (rootContainer->getBackgroundColor ());
		}
		else
		{
			frame->setTransparency (true);
		}
		frame->addView (rootView);
		rootView->remember (); // caller is responsible to forget it

		CRect r (rootView->getViewSize ());
		r.top = r.bottom+kMargin;
		r.bottom += kControlHeight+kMargin;
		r.setWidth (80);
		r.offset (size.right-(2*kMargin+r.getWidth ()), 0);

		bool okIsFirst = (okIsRightMost || style == kOkButton);
		SimpleButton* button;
		button = new SimpleButton (r, this, okIsFirst ? kOkTag : kCancelTag, okIsRightMost ? "OK" : "Cancel");
		frame->addView (button);
		if (style == kOkCancelButtons)
		{
			r.offset (-(r.getWidth () + kMargin), 0);
			button = new SimpleButton (r, this, okIsRightMost ? kCancelTag : kOkTag, okIsRightMost ? "Cancel" : "OK");
			frame->addView (button);
		}
	}
}

//-----------------------------------------------------------------------------
Dialog::~Dialog ()
{
	if (frame)
		frame->forget ();
	if (platformWindow)
		platformWindow->forget ();
}

//-----------------------------------------------------------------------------
bool Dialog::run ()
{
	if (platformWindow)
	{
		platformWindow->show ();
		frame->advanceNextFocusView (0, false);
		platformWindow->runModal ();
	}
	return result;
}

//-----------------------------------------------------------------------------
long Dialog::onKeyDown (const VstKeyCode& code, CFrame* frame)
{
	static bool recursion = false;
	if (recursion)
		return -1;

	recursion = true;

	VstKeyCode keyCode = code;
	long keyResult = frame->onKeyDown (keyCode);

	recursion = false;

	if (keyResult == -1)
	{
		if (code.virt == VKEY_ESCAPE)
		{
			platformWindow->stopModal ();
			result = false;
			return 1;
		}
		else if (code.virt == VKEY_RETURN)
		{
			platformWindow->stopModal ();
			result = true;
			return 1;
		}
	}
	return keyResult;
}

//-----------------------------------------------------------------------------
long Dialog::onKeyUp (const VstKeyCode& code, CFrame* frame)
{
	return -1;
}

//-----------------------------------------------------------------------------
void Dialog::valueChanged (CControl* pControl)
{
	switch (pControl->getTag ())
	{
		case kOkTag:
		{
			if (pControl->getValue () > 0.5)
			{
				platformWindow->stopModal ();
				result = true;
			}
			break;
		}
		case kCancelTag:
		{
			if (pControl->getValue () > 0.5)
			{
				platformWindow->stopModal ();
				result = false;
			}
			break;
		}
	}
}

END_NAMESPACE_VSTGUI

#endif // VSTGUI_LIVE_EDITING
