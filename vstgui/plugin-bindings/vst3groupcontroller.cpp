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
	parameter->addDependent (&observer);
	vstgui_assert (parameter->getInfo ().stepCount > 0);
}

//------------------------------------------------------------------------
GroupController::~GroupController () { parameter->removeDependent (&observer); }

//------------------------------------------------------------------------
SharedPointer<CView> GroupController::verifyView (const SharedPointer<CView>& view,
												  const UIAttributes& attributes,
												  const IUIDescription& description)
{
	auto control = view.cast<CControl> ();
	if (control)
	{
		controls.push_back (control.get ());
		control->setListener (this);
		parameter->deferUpdate ();
	}
	return view;
}

//------------------------------------------------------------------------
void GroupController::valueChanged (CControl& control)
{
	Steinberg::Vst::ParamValue normValue = parameter->toNormalized (control.getTag ());
	editController->performEdit (parameter->getInfo().id, normValue);
	parameter->setNormalized (normValue);
}

//------------------------------------------------------------------------
void GroupController::controlBeginEdit (CControl& control)
{
	for (const auto& c : controls)
		c->setMouseEnabled (c == &control);
	editController->beginEdit (parameter->getInfo ().id);
}

//------------------------------------------------------------------------
void GroupController::controlEndEdit (CControl& control)
{
	editController->endEdit (parameter->getInfo ().id);
	observer.update (parameter, Steinberg::IDependent::kChanged);
}

//------------------------------------------------------------------------
void PLUGIN_API GroupController::ParameterObserver::update (Steinberg::FUnknown* changedUnknown,
															Steinberg::int32 message)
{
	auto* p = Steinberg::FCast<Steinberg::Vst::Parameter> (changedUnknown);
	if (p && p == controller.parameter)
	{
		auto plainValue = controller.parameter->toPlain (controller.parameter->getNormalized ());
		for (const auto& c : controller.controls)
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
