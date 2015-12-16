//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins
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

#include "uidialogcontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uieditcontroller.h"
#include "../uiattributes.h"
#include "../../lib/coffscreencontext.h"
#include "../../lib/cbitmapfilter.h"
#include "../../lib/clayeredviewcontainer.h"
#include "../../lib/controls/ctextlabel.h"
#include "../../lib/controls/cbuttons.h"
#include "../../lib/animation/animations.h"
#include "../../lib/animation/animator.h"
#include "../../lib/animation/timingfunctions.h"
#include <cmath>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
IdStringPtr UIDialogController::kMsgDialogButton1Clicked = "UIDialogController::kMsgDialogButton1Clicked";
IdStringPtr UIDialogController::kMsgDialogButton2Clicked = "UIDialogController::kMsgDialogButton2Clicked";
IdStringPtr UIDialogController::kMsgDialogShow = "UIDialogController::kMsgDialogShow";

//----------------------------------------------------------------------------------------------------
UIDialogController::UIDialogController (IController* baseController, CFrame* frame)
: DelegationController (baseController)
, frame (frame)
{
}

//----------------------------------------------------------------------------------------------------
UIDialogController::~UIDialogController ()
{
	
}

//----------------------------------------------------------------------------------------------------
void UIDialogController::run (UTF8StringPtr _templateName, UTF8StringPtr _dialogTitle, UTF8StringPtr _button1, UTF8StringPtr _button2, IController* _dialogController, UIDescription* _description)
{
	collectOpenGLViews (frame);

	templateName = _templateName;
	dialogTitle = _dialogTitle;
	dialogButton1 = _button1;
	dialogButton2 = _button2 != 0 ? _button2 : "";
	dialogController = dynamic_cast<CBaseObject*> (_dialogController);
	dialogDescription = _description;
	CView* view = UIEditController::getEditorDescription ().createView ("dialog", this);
	if (view)
	{
		CLayeredViewContainer* layeredView = dynamic_cast<CLayeredViewContainer*>(view);
		if (layeredView)
			layeredView->setZIndex (10);

		CRect size = view->getViewSize ();
		size.right += sizeDiff.x;
		size.bottom += sizeDiff.y;
		CRect frameSize = frame->getViewSize ();
		size.centerInside (frameSize);
		size.makeIntegral ();
		view->setViewSize (size);
		view->setMouseableArea (size);
		view->setAlphaValue (0.f);

		frame->setModalView (view);
		frame->registerKeyboardHook (this);
		frame->registerViewListener (this);
		view->registerViewListener (this);
		if (button1)
			frame->setFocusView (button1);
		setOpenGLViewsVisible (false);
		if (dialogController)
			dialogController->notify (this, kMsgDialogShow);

		view->addAnimation ("AlphaAnimation", new Animation::AlphaValueAnimation (1.f), new Animation::LinearTimingFunction (160));
	}
	else
	{
		forget ();
	}
}

//----------------------------------------------------------------------------------------------------
CMessageResult UIDialogController::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == Animation::kMsgAnimationFinished)
	{
		close ();
		return kMessageNotified;
	}
	return kMessageUnknown;
}

//----------------------------------------------------------------------------------------------------
void UIDialogController::close ()
{
	frame->unregisterKeyboardHook (this);
	frame->unregisterViewListener (this);
	if (button1)
		button1->setListener (0);
	if (button2)
		button2->setListener (0);
	setOpenGLViewsVisible (true);

	CView* dialog = frame->getModalView ();
	if (dialog)
	{
		dialog->unregisterViewListener (this);
		frame->setModalView (0);
		dialog->forget ();
	}
	forget ();
}

//----------------------------------------------------------------------------------------------------
void UIDialogController::viewSizeChanged (CView* view, const CRect& oldSize)
{
	if (view == frame)
	{
		CView* dialog = frame->getModalView ();
		CRect viewSize = dialog->getViewSize ();
		viewSize.centerInside (frame->getViewSize ());
		dialog->setViewSize (viewSize);
		dialog->setMouseableArea (viewSize);
	}
}

//----------------------------------------------------------------------------------------------------
void UIDialogController::viewRemoved (CView* view)
{
	if (view != frame)
	{
		view->unregisterViewListener (this);
		close ();
	}
}

//----------------------------------------------------------------------------------------------------
void UIDialogController::valueChanged (CControl* control)
{
	if (control->getValue () == control->getMax ())
	{
		switch (control->getTag ())
		{
			case kButton1Tag:
			{
				if (dialogController)
					dialogController->notify (this, kMsgDialogButton1Clicked);
				break;
			}
			case kButton2Tag:
			{
				if (dialogController)
					dialogController->notify (this, kMsgDialogButton2Clicked);
				break;
			}
		}
		CView* modalView = frame->getModalView ();
		modalView->addAnimation ("AlphaAnimation", new Animation::AlphaValueAnimation (0.f), new Animation::LinearTimingFunction (160), this);
	}
}

//----------------------------------------------------------------------------------------------------
IControlListener* UIDialogController::getControlListener (UTF8StringPtr controlTagName)
{
	return this;
}

//----------------------------------------------------------------------------------------------------
CView* UIDialogController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	CControl* control = dynamic_cast<CControl*>(view);
	if (control)
	{
		if (control->getTag () == kButton1Tag)
		{
			CTextButton* button = dynamic_cast<CTextButton*>(control);
			if (button)
			{
				button1 = button;
				button->setTitle (dialogButton1.c_str ());
				layoutButtons ();
			}
		}
		else if (control->getTag () == kButton2Tag)
		{
			CTextButton* button = dynamic_cast<CTextButton*>(control);
			if (button)
			{
				button2 = button;
				if (dialogButton2.empty ())
				{
					button->setVisible (false);
				}
				else
				{
					button->setTitle (dialogButton2.c_str ());
				}
				layoutButtons ();
			}
		}
		else if (control->getTag () == kTitleTag)
		{
			CTextLabel* label = dynamic_cast<CTextLabel*>(control);
			if (label)
			{
				label->setText (dialogTitle.c_str ());
			}
		}
	}
	const std::string* name = attributes.getAttributeValue (IUIDescription::kCustomViewName);
	if (name)
	{
		if (*name == "view")
		{
			IController* controller = dialogController.cast<IController> ();
			CView* subView = dialogDescription->createView (templateName.c_str (), controller);
			if (subView)
			{
				subView->setAttribute (kCViewControllerAttribute, sizeof (IController*), &controller);
				sizeDiff.x = subView->getWidth () - view->getWidth ();
				sizeDiff.y = subView->getHeight () - view->getHeight ();
				CRect size = view->getViewSize ();
				size.setWidth (subView->getWidth ());
				size.setHeight (subView->getHeight ());
				view->setViewSize (size);
				view->setMouseableArea (size);
				CViewContainer* container = dynamic_cast<CViewContainer*> (view);
				if (container)
					container->addView (subView);
			}
		}
	}
	return view;
}

//----------------------------------------------------------------------------------------------------
void UIDialogController::layoutButtons ()
{
	if (button1 && button2)
	{
		CRect b1r1 = button1->getViewSize ();
		CRect b2r1 = button2->getViewSize ();
		CCoord margin = b1r1.left - b2r1.right;
		button1->sizeToFit ();
		button2->sizeToFit ();
		CRect b1r2 = button1->getViewSize ();
		CRect b2r2 = button2->getViewSize ();

		b1r2.offset (b1r1.getWidth () - b1r2.getWidth (), b1r1.getHeight () - b1r2.getHeight ());
		button1->setViewSize (b1r2);
		button1->setMouseableArea (b1r2);

		b2r2.offset (b2r1.getWidth () - b2r2.getWidth (), b2r1.getHeight () - b2r2.getHeight ());
		b2r2.offset ((b1r2.left - margin) - b2r2.right, 0); 
		button2->setViewSize (b2r2);
		button2->setMouseableArea (b2r2);
	}
}

//----------------------------------------------------------------------------------------------------
int32_t UIDialogController::onKeyDown (const VstKeyCode& code, CFrame* frame)
{
	CBaseObjectGuard guard (this);
	int32_t result = -1;
	CView* focusView = frame->getFocusView ();
	if (focusView)
		result = focusView->onKeyDown (const_cast<VstKeyCode&> (code));
	if (result == -1)
	{
		if (code.virt == VKEY_RETURN && code.modifier == 0)
		{
			button1->setValue (button1->getMax ());
			button1->valueChanged ();
			return 1;
		}
		if (code.virt == VKEY_ESCAPE && code.modifier == 0 && button2->isVisible ())
		{
			button2->setValue (button2->getMax ());
			button2->valueChanged ();
			return 1;
		}
	}
	return result;
}

//----------------------------------------------------------------------------------------------------
int32_t UIDialogController::onKeyUp (const VstKeyCode& code, CFrame* frame)
{
	CView* focusView = frame->getFocusView ();
	if (focusView)
		return focusView->onKeyUp (const_cast<VstKeyCode&> (code));
	return -1;
}

//----------------------------------------------------------------------------------------------------
void UIDialogController::collectOpenGLViews (CViewContainer* container)
{
#if VSTGUI_OPENGL_SUPPORT
	ViewIterator it (container);
	while (*it)
	{
		COpenGLView* openGLView = dynamic_cast<COpenGLView*>(*it);
		if (openGLView && openGLView->isVisible ())
			openglViews.push_back (openGLView);
		else
		{
			CViewContainer* childContainer = dynamic_cast<CViewContainer*>(*it);
			if (childContainer)
				collectOpenGLViews (childContainer);
		}
		it++;
	}
#endif
}

//----------------------------------------------------------------------------------------------------
void UIDialogController::setOpenGLViewsVisible (bool state)
{
#if VSTGUI_OPENGL_SUPPORT
	for (std::list<SharedPointer<COpenGLView> >::const_iterator it = openglViews.begin(); it != openglViews.end (); it++)
	{
		(*it)->setVisible (state);
	}
#endif
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
