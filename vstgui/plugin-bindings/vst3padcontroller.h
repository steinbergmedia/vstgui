// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../lib/controls/cxypad.h"
#include "../uidescription/delegationcontroller.h"
#include "../uidescription/uidescription.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "public.sdk/source/vst/vstparameters.h"
#include "pluginterfaces/base/iupdatehandler.h"
#include "pluginterfaces/base/funknownimpl.h"

namespace VSTGUI {

//------------------------------------------------------------------------
class PadController : public DelegationController,
					  public NonAtomicReferenceCounted
{
public:
	PadController (const SharedPointer<IController>& baseController,
				   Steinberg::Vst::EditController* editController,
				   Steinberg::Vst::Parameter* xParam, Steinberg::Vst::Parameter* yParam);
	~PadController () override;

	SharedPointer<CView> verifyView (const SharedPointer<CView>& view,
									 const UIAttributes& attributes,
									 const IUIDescription& description) override;

	void valueChanged (CControl& control) override;
	void controlBeginEdit (CControl& control) override;
	void controlEndEdit (CControl& control) override;

	//-----------------------------------------------------------------------------
protected:
	struct ParameterObserver
	: Steinberg::U::ImplementsNonDestroyable<Steinberg::U::Directly<Steinberg::IDependent>>
	{
		ParameterObserver (PadController& controller) : controller (controller) {}

		void PLUGIN_API update (Steinberg::FUnknown* changedUnknown,
								Steinberg::int32 message) override;
		PadController& controller;
	};

	ParameterObserver observer {*this};
	Steinberg::Vst::EditController* editController;
	Steinberg::Vst::Parameter* xParam;
	Steinberg::Vst::Parameter* yParam;
	CXYPad* padControl;
	SharedPointer<UIDescription> uiDescription;
};

} // namespace
