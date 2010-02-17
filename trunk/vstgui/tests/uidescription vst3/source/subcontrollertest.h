/*
 *  subcontrollertest.h
 *  uidescription test
 *
 *  Created by Arne Scheffler on 2/12/10.
 *  Copyright 2010 Arne Scheffler. All rights reserved.
 *
 */

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
class SubControllerTestController : public UIDescriptionBaseController, public VST3EditorDelegate
{
public:
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context);
	Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name);

	IController* createSubController (const char* name, IUIDescription* description, VST3Editor* editor);

	static Steinberg::FUnknown* createInstance (void*) { return (Steinberg::Vst::IEditController*)new SubControllerTestController; }
	static Steinberg::FUID cid;
};


} // namespace

#endif // __subcontrollertest__
