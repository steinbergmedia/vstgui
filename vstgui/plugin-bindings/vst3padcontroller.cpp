// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "vst3padcontroller.h"

namespace VSTGUI {

//------------------------------------------------------------------------
PadController::PadController (const SharedPointer<IController>& baseController,
							  Steinberg::Vst::EditController* editController,
							  Steinberg::Vst::Parameter* xParam, Steinberg::Vst::Parameter* yParam)
: DelegationController (baseController)
, editController (editController)
, xParam (xParam)
, yParam (yParam)
, padControl (nullptr)
{
	if (xParam)
		xParam->addDependent (&observer);
	if (yParam)
		yParam->addDependent (&observer);
}

//------------------------------------------------------------------------
PadController::~PadController ()
{
	if (xParam)
		xParam->removeDependent (&observer);
	if (yParam)
		yParam->removeDependent (&observer);
}

//------------------------------------------------------------------------
SharedPointer<CView> PadController::verifyView (const SharedPointer<CView>& view,
												const UIAttributes& attributes,
												const IUIDescription& description)
{
	auto pad = view.cast<CXYPad> ();
	if (pad)
	{
		padControl = pad.get ();
		padControl->setListener (this);
		observer.update (xParam, Steinberg::IDependent::kChanged);
	}
	return view;
}

//------------------------------------------------------------------------
void PadController::valueChanged (CControl& control)
{
	if (&control == padControl && xParam && yParam)
	{
		float x, y;
		CXYPad::calculateXY (padControl->getValue (), x, y);

		auto xId = xParam->getInfo ().id;
		if (editController->setParamNormalized (xId, x) == Steinberg::kResultTrue)
			editController->performEdit (xId, editController->getParamNormalized (xId));

		auto yId = yParam->getInfo ().id;
		if (editController->setParamNormalized (yId, y) == Steinberg::kResultTrue)
			editController->performEdit (yId, editController->getParamNormalized (yId));
	}
	else
	{
		DelegationController::valueChanged (control);
	}
}

//------------------------------------------------------------------------
void PadController::controlBeginEdit (CControl& control)
{
	if (&control == padControl && xParam && yParam)
	{
		editController->startGroupEdit ();
		editController->beginEdit (xParam->getInfo ().id);
		editController->beginEdit (yParam->getInfo ().id);
	}
	else
	{
		DelegationController::controlBeginEdit (control);
	}
}

//------------------------------------------------------------------------
void PadController::controlEndEdit (CControl& control)
{
	if (&control == padControl && xParam && yParam)
	{
		editController->endEdit (xParam->getInfo ().id);
		editController->endEdit (yParam->getInfo ().id);
		editController->finishGroupEdit ();
	}
	else
	{
		DelegationController::controlEndEdit (control);
	}
}

//------------------------------------------------------------------------
void PLUGIN_API PadController::ParameterObserver::update (Steinberg::FUnknown* changedUnknown,
														  Steinberg::int32 message)
{
	if (controller.padControl)
	{
		auto* p = Steinberg::FCast<Steinberg::Vst::Parameter> (changedUnknown);
		if (p && (p == controller.xParam || p == controller.yParam))
		{
			if (message == kChanged)
			{
				float value = CXYPad::calculateValue (controller.xParam->getNormalized (),
													  controller.yParam->getNormalized ());
				controller.padControl->setValue (value);
				controller.padControl->invalid ();
			}
			else if (message == kWillDestroy)
			{
				if (controller.xParam)
					controller.xParam->removeDependent (this);
				if (controller.yParam)
					controller.yParam->removeDependent (this);
				controller.xParam = nullptr;
				controller.yParam = nullptr;
			}
		}
	}
}

} // namespace
