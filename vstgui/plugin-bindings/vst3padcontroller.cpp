// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "vst3padcontroller.h"

namespace VSTGUI {

//------------------------------------------------------------------------
PadController::PadController (IController* baseController, Steinberg::Vst::EditController* editController, Steinberg::Vst::Parameter* xParam, Steinberg::Vst::Parameter* yParam)
: DelegationController (baseController)
, editController (editController)
, xParam (xParam)
, yParam (yParam)
, padControl (0)
{
	xParam->addDependent (this);
	yParam->addDependent (this);
}

//------------------------------------------------------------------------
PadController::~PadController ()
{
	xParam->removeDependent (this);
	yParam->removeDependent (this);
}

//------------------------------------------------------------------------
CView* PadController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	CXYPad* pad = dynamic_cast<CXYPad*>(view);
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
	if (pControl == padControl)
	{
		float x, y;
		CXYPad::calculateXY (pControl->getValue (), x, y);
		editController->performEdit (xParam->getInfo ().id, x);
		editController->setParamNormalized (xParam->getInfo ().id, x);
		editController->performEdit (yParam->getInfo ().id, y);
		editController->setParamNormalized (yParam->getInfo ().id, y);
	}
	else
	{
		DelegationController::valueChanged (pControl);
	}
}

//------------------------------------------------------------------------
void PadController::controlBeginEdit (CControl* pControl)
{
	if (pControl == padControl)
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
	if (pControl == padControl)
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
void PLUGIN_API PadController::update (Steinberg::FUnknown* changedUnknown, Steinberg::int32 message)
{
	if (padControl)
	{
		Steinberg::Vst::Parameter* p = Steinberg::FCast<Steinberg::Vst::Parameter> (changedUnknown);
		if (p && (p == xParam || p == yParam))
		{
			float value = CXYPad::calculateValue (xParam->getNormalized (), yParam->getNormalized ());
			padControl->setValue (value);
			padControl->invalid ();
		}
	}
}

} // namespace

