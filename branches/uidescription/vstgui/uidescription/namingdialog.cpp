/*
 *  namingdialog.cpp
 *  VST3PlugIns
 *
 *  Created by Arne Scheffler on 6/3/09.
 *  Copyright 2009 Arne Scheffler. All rights reserved.
 *
 */

#include "namingdialog.h"
#include "../vstkeycode.h"

BEGIN_NAMESPACE_VSTGUI

//-----------------------------------------------------------------------------
bool NamingDialog::askForName (std::string& result, const char* dialogTitle)
{
	NamingDialog dialog (dialogTitle);
	return dialog.run (result);
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
	kTextEditTag,
	kOkTag,
	kCancelTag
};

#if MAC
#define okIsRightMost 1
#else
#define okIsRightMost 0
#endif

//-----------------------------------------------------------------------------
NamingDialog::NamingDialog (const char* title)
: platformWindow (0)
, frame (0)
, textEdit (0)
, result (false)
{
	const CCoord kMargin = 10;
	const CCoord kControlHeight = 22;
	CRect size (0, 0, 250+2*kMargin, 2*kControlHeight+3*kMargin);
	size.offset (100, 100);
	platformWindow = PlatformWindow::create (size, title, PlatformWindow::kWindowType, 0, 0);
	if (platformWindow)
	{
		size.offset (-100, -100);
		frame = new CFrame (size, platformWindow->getPlatformHandle (), 0);
		frame->setBackgroundColor (MakeCColor (200, 200, 200, 255));
		CRect r (kMargin, kMargin, size.getWidth () - kMargin, kControlHeight + kMargin);
		textEdit = new CTextEdit (r, this, kTextEditTag);
		textEdit->setBackColor (kWhiteCColor);
		textEdit->setFontColor (kBlackCColor);
		frame->addView (textEdit);
		
		r.offset (0, kControlHeight+kMargin);
		r.setWidth (80);
		r.offset (size.right-(2*kMargin+r.getWidth ()), 0);
		SimpleButton* button;

		button = new SimpleButton (r, this, okIsRightMost ? kOkTag : kCancelTag, okIsRightMost ? "OK" : "Cancel");
		frame->addView (button);
		r.offset (-(r.getWidth () + kMargin), 0);
		button = new SimpleButton (r, this, okIsRightMost ? kCancelTag : kOkTag, okIsRightMost ? "Cancel" : "OK");
		frame->addView (button);
	}
}

//-----------------------------------------------------------------------------
NamingDialog::~NamingDialog ()
{
	if (frame)
		frame->forget ();
	if (platformWindow)
		platformWindow->forget ();
}

//-----------------------------------------------------------------------------
bool NamingDialog::run (std::string& str)
{
	if (platformWindow && textEdit)
	{
		textEdit->setText (str.c_str ());
		platformWindow->center ();
		platformWindow->show ();
		textEdit->beginEdit ();
		textEdit->takeFocus ();
		platformWindow->runModal ();
		if (result)
		{
			str = textEdit->getText ();
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
void NamingDialog::valueChanged (CControl* pControl)
{
	switch (pControl->getTag ())
	{
		case kTextEditTag:
		{
			if (textEdit->bWasReturnPressed)
			{
				platformWindow->stopModal ();
				result = true;
			}
			break;
		}
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
