// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "animationtest.h"
#include "base/source/fobject.h"
#include "vstgui/lib/animation/animations.h"
#include "vstgui/uidescription/delegationcontroller.h"

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace VSTGUI {

//------------------------------------------------------------------------
FUID AnimationTestProcessor::cid (0xF1306D3E, 0xCF9F432E, 0x8A9B6FC2, 0xC59A3CAF);
FUID AnimationTestController::cid (0xCACC9448, 0xA00C47DF, 0xA7CDE656, 0xF3DD9BA1);

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
AnimationTestProcessor::AnimationTestProcessor ()
{
	setControllerClass (AnimationTestController::cid);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class TestAnimationController : public DelegationController, public FObject
{
public:
	TestAnimationController (IController* controller, Parameter* switchParameter);
	~TestAnimationController ();

	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override;
	
	void PLUGIN_API update (FUnknown* changedUnknown, int32 message) override;
	OBJ_METHODS(TestAnimationController, FObject)
protected:
	Parameter* switchParameter;
	CView* animationView;
	CRect originalRect;
};

//------------------------------------------------------------------------
class ScaleView : public CViewContainer
{
public:
	ScaleView ();
	CMouseEventResult onMouseEntered (CPoint &where, const CButtonState& buttons) override;
	CMouseEventResult onMouseExited (CPoint &where, const CButtonState& buttons) override;
protected:
	CRect origRect;
};

//------------------------------------------------------------------------
tresult PLUGIN_API AnimationTestController::initialize (FUnknown* context)
{
	tresult res = UIDescriptionBaseController::initialize (context);
	if (res == kResultTrue)
	{
		// add ui parameters
		StringListParameter* slp = new StringListParameter (USTRING("Animation Switch"), 20000);
		slp->appendString (USTRING("On"));
		slp->appendString (USTRING("Off"));
		uiParameters.addParameter (slp);
	}
	return res;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API AnimationTestController::createView (FIDString name)
{
	if (strcmp (name, ViewType::kEditor) == 0)
	{
		return new VST3Editor (this, "view", "AnimationTest.uidesc");
	}
	return 0;
}

//------------------------------------------------------------------------
IController* AnimationTestController::createSubController (const char* name, const IUIDescription* description, VST3Editor* editor)
{
	if (ConstString (name) == "AnimationController")
	{
		return new TestAnimationController (editor, uiParameters.getParameter (20000));
	}
	return 0;
}

//------------------------------------------------------------------------
CView* AnimationTestController::createCustomView (const char* name, const UIAttributes& attributes, const IUIDescription* description, VST3Editor* editor)
{
	if (strcmp (name, "ScaleView") == 0)
	{
		return new ScaleView;
	}
	return 0;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
TestAnimationController::TestAnimationController (IController* controller, Parameter* switchParameter)
: DelegationController (controller)
, switchParameter (switchParameter)
, animationView (0)
{
	switchParameter->addDependent (this);
}

//------------------------------------------------------------------------
TestAnimationController::~TestAnimationController ()
{
	switchParameter->removeDependent (this);
}

//------------------------------------------------------------------------
CView* TestAnimationController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	view = DelegationController::verifyView (view, attributes, description);
	if (view)
	{
		originalRect = view->getViewSize ();
		CRect r (originalRect);

		switchParameter->deferUpdate (kChanged);
		animationView = view;
	}
	return view;
}

//------------------------------------------------------------------------
void PLUGIN_API TestAnimationController::update (FUnknown* changedUnknown, int32 message)
{
	if (message == kChanged)
	{
		FUnknownPtr<Parameter> parameter (changedUnknown);
		if (parameter && animationView && animationView->getFrame ())
		{
			Animation::InterpolationTimingFunction* timingFunction = new Animation::InterpolationTimingFunction (200);
			timingFunction->addPoint (0.5f, 0.2f);
			int32 value = static_cast<int32> (parameter->toPlain (parameter->getNormalized ()));
			if (value)
			{
				animationView->getFrame ()->getAnimator ()->addAnimation (animationView, "SizeAnimation", new Animation::ViewSizeAnimation (originalRect), timingFunction);
			}
			else
			{
				CRect r (originalRect);
				r.bottom += 300;
				r.right += 80;
				animationView->getFrame ()->getAnimator ()->addAnimation (animationView, "SizeAnimation", new Animation::ViewSizeAnimation (r), timingFunction);
			}
		}
	}
}

//------------------------------------------------------------------------
ScaleView::ScaleView ()
:CViewContainer (CRect (0, 0, 0, 0))
{
	setAlphaValue (0.1f);
}

//------------------------------------------------------------------------
CMouseEventResult ScaleView::onMouseEntered (CPoint &where, const CButtonState& buttons)
{
	if (origRect.isEmpty ())
		origRect = getViewSize ();
	CRect r (origRect);
	r.inset (-40, -15);
	getFrame ()->getAnimator ()->removeAnimation (this, "AlphaAnimation");
	getFrame ()->getAnimator ()->removeAnimation (this, "SizeAnimation");
	getFrame ()->getAnimator ()->addAnimation (this, "AlphaAnimation", new Animation::AlphaValueAnimation (1.f), new Animation::RepeatTimingFunction (new Animation::LinearTimingFunction (300), -1, true));
	getFrame ()->getAnimator ()->addAnimation (this, "SizeAnimation", new Animation::ViewSizeAnimation (r), new Animation::RepeatTimingFunction (new Animation::LinearTimingFunction (200), -1, true));
	return CViewContainer::onMouseEntered (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult ScaleView::onMouseExited (CPoint &where, const CButtonState& buttons)
{
	getFrame ()->getAnimator ()->addAnimation (this, "AlphaAnimation", new Animation::AlphaValueAnimation (0.1f), new Animation::LinearTimingFunction (100));
	getFrame ()->getAnimator ()->addAnimation (this, "SizeAnimation", new Animation::ViewSizeAnimation (origRect), new Animation::LinearTimingFunction (100));
	return CViewContainer::onMouseExited (where, buttons);
}

} // namespace

