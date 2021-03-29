// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
UIDialogController::UIDialogController (IController* baseController, CFrame* frame)
: DelegationController (baseController)
, frame (frame)
{
}

//----------------------------------------------------------------------------------------------------
void UIDialogController::run (UTF8StringPtr _templateName, UTF8StringPtr _dialogTitle,
                              UTF8StringPtr _button1, UTF8StringPtr _button2,
                              const SharedPointer<IDialogController>& _dialogController,
                              UIDescription* _description)
{
	collectOpenGLViews (frame);

	templateName = _templateName;
	dialogTitle = _dialogTitle;
	dialogButton1 = _button1;
	dialogButton2 = _button2 != nullptr ? _button2 : "";
	dialogController = _dialogController;
	dialogDescription = _description;
	CView* view = UIEditController::getEditorDescription ()->createView ("dialog", this);
	if (view)
	{
		auto* layeredView = dynamic_cast<CLayeredViewContainer*>(view);
		if (layeredView)
			layeredView->setZIndex (10);

		CRect size = view->getViewSize ();
		size.right += sizeDiff.x;
		size.bottom += sizeDiff.y;
		CRect frameSize = frame->getViewSize ();
		frame->getTransform ().inverse ().transform (frameSize);
		size.centerInside (frameSize);
		size.makeIntegral ();
		view->setViewSize (size);
		view->setMouseableArea (size);
		view->setAlphaValue (0.f);

		modalSession = frame->beginModalViewSession (view);
		frame->registerKeyboardHook (this);
		frame->registerViewListener (this);
		view->registerViewListener (this);
		if (button1)
			frame->setFocusView (button1);
		setOpenGLViewsVisible (false);
		if (dialogController)
			dialogController->onDialogShow (this);

		using namespace Animation;
		view->addAnimation (
		    "AlphaAnimation", new AlphaValueAnimation (1.f),
		    new CubicBezierTimingFunction (CubicBezierTimingFunction::easyInOut (160)));
	}
	else
	{
		forget ();
	}
}

//----------------------------------------------------------------------------------------------------
void UIDialogController::close ()
{
	frame->unregisterKeyboardHook (this);
	frame->unregisterViewListener (this);
	if (button1)
		button1->setListener (nullptr);
	if (button2)
		button2->setListener (nullptr);
	setOpenGLViewsVisible (true);

	if (modalSession)
	{
		if (auto dialog = frame->getModalView ())
			dialog->unregisterViewListener (this);
		frame->endModalViewSession (*modalSession);
		modalSession = {};
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
		CRect frameSize = frame->getViewSize ();
		frame->getTransform ().inverse ().transform (frameSize);
		viewSize.centerInside (frameSize);
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
					dialogController->onDialogButton1Clicked (this);
				break;
			}
			case kButton2Tag:
			{
				if (dialogController)
					dialogController->onDialogButton2Clicked (this);
				break;
			}
		}
		CView* modalView = frame->getModalView ();
		using namespace Animation;
		modalView->addAnimation (
		    "AlphaAnimation", new AlphaValueAnimation (0.f),
		    new CubicBezierTimingFunction (CubicBezierTimingFunction::easyInOut (160)),
		    [this] (CView*, const IdStringPtr, IAnimationTarget*) { close (); });
	}
}

//----------------------------------------------------------------------------------------------------
IControlListener* UIDialogController::getControlListener (UTF8StringPtr controlTagName)
{
	return this;
}

//----------------------------------------------------------------------------------------------------
CView* UIDialogController::verifyView (CView* view, const UIAttributes& attributes,
                                       const IUIDescription* description)
{
	auto* control = dynamic_cast<CControl*>(view);
	if (control)
	{
		if (control->getTag () == kButton1Tag)
		{
			auto* button = dynamic_cast<CTextButton*>(control);
			if (button)
			{
				button1 = button;
				button->setTitle (dialogButton1.c_str ());
				layoutButtons ();
			}
		}
		else if (control->getTag () == kButton2Tag)
		{
			auto* button = dynamic_cast<CTextButton*>(control);
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
			auto* label = dynamic_cast<CTextLabel*>(control);
			if (label)
			{
				label->setText (dialogTitle.c_str ());
			}
		}
	}
	const std::string* name = attributes.getAttributeValue (IUIDescription::kCustomViewName);
	if (name)
	{
		if (*name == "view" && view)
		{
			auto controller = dialogController.cast<IController> ();
			if (auto subView = dialogDescription->createView (templateName.c_str (), controller))
			{
				subView->setAttribute (kCViewControllerAttribute, controller);
				sizeDiff.x = subView->getWidth () - view->getWidth ();
				sizeDiff.y = subView->getHeight () - view->getHeight ();
				CRect size = view->getViewSize ();
				size.setWidth (subView->getWidth ());
				size.setHeight (subView->getHeight ());
				view->setViewSize (size);
				view->setMouseableArea (size);
				if (auto container = view->asViewContainer ())
					container->addView (subView);
				if (controller)
					dialogController->remember ();
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
int32_t UIDialogController::onKeyDown (const VstKeyCode& code, CFrame* inFrame)
{
	auto guard = shared (this);
	int32_t result = -1;
	CView* focusView = inFrame->getFocusView ();
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
int32_t UIDialogController::onKeyUp (const VstKeyCode& code, CFrame* inFrame)
{
	CView* focusView = inFrame->getFocusView ();
	if (focusView)
		return focusView->onKeyUp (const_cast<VstKeyCode&> (code));
	return -1;
}

//----------------------------------------------------------------------------------------------------
void UIDialogController::collectOpenGLViews (CViewContainer* container)
{
#if VSTGUI_OPENGL_SUPPORT
	container->forEachChild ([this] (CView* view) {
		auto openGLView = dynamic_cast<COpenGLView*> (view);
		if (openGLView && openGLView->isVisible ())
			openglViews.emplace_back (openGLView);
		else if (auto childContainer = view->asViewContainer ())
			collectOpenGLViews (childContainer);
	});
#endif
}

//----------------------------------------------------------------------------------------------------
void UIDialogController::setOpenGLViewsVisible (bool state)
{
#if VSTGUI_OPENGL_SUPPORT
	for (auto& v : openglViews)
		v->setVisible (state);
#endif
}

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
