// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "vst3padcontroller.h"

namespace VSTGUI {

//------------------------------------------------------------------------
PadController::PadController (IController* baseController,
                              Steinberg::Vst::EditController* editController,
                              Steinberg::Vst::Parameter* xParam, Steinberg::Vst::Parameter* yParam)
: DelegationController (baseController)
, editController (editController)
, xParam (xParam)
, yParam (yParam)
, padControl (nullptr)
{
	if (xParam)
		xParam->addDependent (this);
	if (yParam)
		yParam->addDependent (this);
}

//------------------------------------------------------------------------
PadController::~PadController ()
{
	if (xParam)
		xParam->removeDependent (this);
	if (yParam)
		yParam->removeDependent (this);
}

//------------------------------------------------------------------------
CView* PadController::verifyView (CView* view, const UIAttributes& attributes,
                                  const IUIDescription* description)
{
	auto* pad = dynamic_cast<CXYPad*> (view);
	if (pad)
	{
		padControl = pad;
		padControl->setListener (this);
		update (xParam, kChanged);
	}
	return view;
}

//------------------------------------------------------------------------
void PadController::valueChanged (CControl* pControl)
{
	if (pControl == padControl && xParam && yParam)
	{
		float x, y;
		CXYPad::calculateXY (pControl->getValue (), x, y);

		auto xId = xParam->getInfo ().id;
		if (editController->setParamNormalized (xId, x) == Steinberg::kResultTrue)
			editController->performEdit (xId, editController->getParamNormalized (xId));

		auto yId = yParam->getInfo ().id;
		if (editController->setParamNormalized (yId, y) == Steinberg::kResultTrue)
			editController->performEdit (yId, editController->getParamNormalized (yId));
	}
	else
	{
		DelegationController::valueChanged (pControl);
	}
}

//------------------------------------------------------------------------
void PadController::controlBeginEdit (CControl* pControl)
{
	if (pControl == padControl && xParam && yParam)
	{
		editController->startGroupEdit ();
		editController->beginEdit (xParam->getInfo ().id);
		editController->beginEdit (yParam->getInfo ().id);
	}
	else
	{
		DelegationController::controlBeginEdit (pControl);
	}
}

//------------------------------------------------------------------------
void PadController::controlEndEdit (CControl* pControl)
{
	if (pControl == padControl && xParam && yParam)
	{
		editController->endEdit (xParam->getInfo ().id);
		editController->endEdit (yParam->getInfo ().id);
		editController->finishGroupEdit ();
	}
	else
	{
		DelegationController::controlEndEdit (pControl);
	}
}

//------------------------------------------------------------------------
void PLUGIN_API PadController::update (Steinberg::FUnknown* changedUnknown,
                                       Steinberg::int32 message)
{
	if (padControl)
	{
		auto* p = Steinberg::FCast<Steinberg::Vst::Parameter> (changedUnknown);
		if (p && (p == xParam || p == yParam))
		{
			if (message == kChanged)
			{
				float value =
				    CXYPad::calculateValue (xParam->getNormalized (), yParam->getNormalized ());
				padControl->setValue (value);
				padControl->invalid ();
			}
			else if (message == kWillDestroy)
			{
				if (xParam)
					xParam->removeDependent (this);
				if (yParam)
					yParam->removeDependent (this);
				xParam = nullptr;
				yParam = nullptr;
			}
		}
	}
}

} // namespace
