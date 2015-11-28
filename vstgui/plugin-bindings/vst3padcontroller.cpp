//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins :
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

