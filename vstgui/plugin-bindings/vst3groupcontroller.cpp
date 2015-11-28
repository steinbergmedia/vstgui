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

#include "vst3groupcontroller.h"
#include <cassert>

namespace VSTGUI {

//------------------------------------------------------------------------
GroupController::GroupController (Steinberg::Vst::Parameter* parameter, Steinberg::Vst::EditController* editController)
: parameter (parameter)
, editController (editController)
{
	parameter->addDependent (this);
	vstgui_assert (parameter->getInfo ().stepCount > 0);
}

//------------------------------------------------------------------------
GroupController::~GroupController ()
{
	parameter->removeDependent (this);
}

//------------------------------------------------------------------------
CView* GroupController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	CControl* control = dynamic_cast<CControl*>(view);
	if (control)
	{
		controls.push_back (control);
		control->setListener (this);
		parameter->deferUpdate ();
	}
	return view;
}

//------------------------------------------------------------------------
void GroupController::valueChanged (CControl* pControl)
{
	Steinberg::Vst::ParamValue normValue = parameter->toNormalized (pControl->getTag ());
	editController->performEdit (parameter->getInfo().id, normValue);
	parameter->setNormalized (normValue);
}

//------------------------------------------------------------------------
void GroupController::controlBeginEdit (CControl* pControl)
{
	VSTGUI_RANGE_BASED_FOR_LOOP (ControlList, controls, CControl*, c)
		c->setMouseEnabled (c == pControl);
	VSTGUI_RANGE_BASED_FOR_LOOP_END
	editController->beginEdit (parameter->getInfo ().id);
}

//------------------------------------------------------------------------
void GroupController::controlEndEdit (CControl* pControl)
{
	editController->endEdit (parameter->getInfo ().id);
	update (parameter, kChanged);
}

//------------------------------------------------------------------------
void PLUGIN_API GroupController::update (Steinberg::FUnknown* changedUnknown, Steinberg::int32 message)
{
	Steinberg::Vst::Parameter* p = Steinberg::FCast<Steinberg::Vst::Parameter> (changedUnknown);
	if (p && p == parameter)
	{
		Steinberg::Vst::ParamValue plainValue = parameter->toPlain (parameter->getNormalized ());
		VSTGUI_RANGE_BASED_FOR_LOOP (ControlList, controls, CControl*, c)
			if (c->getTag () == plainValue)
			{
				c->setValue (1);
				c->setMouseEnabled (false);
			}
			else
			{
				c->setValue (0);
				c->setMouseEnabled (true);
			}
			c->invalid ();
		VSTGUI_RANGE_BASED_FOR_LOOP_END
	}
}

} // namespace
