// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
	for (const auto& c : controls)
		c->setMouseEnabled (c == pControl);
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
		for (const auto& c : controls)
		{
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
		}
	}
}

} // namespace
