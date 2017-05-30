// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE


#ifndef __subcontrollertest__
#define __subcontrollertest__

#include "uidescription test.h"

namespace VSTGUI {

//------------------------------------------------------------------------
class SubControllerTestProcessor : public UIDescriptionTestProcessor
{
public:
	SubControllerTestProcessor ();
	
	static Steinberg::FUnknown* createInstance (void*) { return (Steinberg::Vst::IAudioProcessor*)new SubControllerTestProcessor; }
	static Steinberg::FUID cid;
};

//------------------------------------------------------------------------
class SubControllerTestController : public UIDescriptionBaseController
{
public:
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) override;
	Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name) override;

	IController* createSubController (const char* name, const IUIDescription* description, VST3Editor* editor) override;

	static Steinberg::FUnknown* createInstance (void*) { return (Steinberg::Vst::IEditController*)new SubControllerTestController; }
	static Steinberg::FUID cid;
};


} // namespace

#endif // __subcontrollertest__
