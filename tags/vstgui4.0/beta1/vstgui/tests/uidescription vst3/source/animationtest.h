/*
 *  animationtest.h
 *  uidescription test
 *
 *  Created by Arne Scheffler on 2/19/10.
 *  Copyright 2010 Arne Scheffler. All rights reserved.
 *
 */

#ifndef __animationtest__
#define __animationtest__

#include "uidescription test.h"

namespace VSTGUI {

//------------------------------------------------------------------------
class AnimationTestProcessor : public UIDescriptionTestProcessor
{
public:
	AnimationTestProcessor ();
	
	static Steinberg::FUnknown* createInstance (void*) { return (Steinberg::Vst::IAudioProcessor*)new AnimationTestProcessor; }
	static Steinberg::FUID cid;
};

//------------------------------------------------------------------------
class AnimationTestController : public UIDescriptionBaseController
{
public:
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context);
	Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name);

	IController* createSubController (const char* name, IUIDescription* description, VST3Editor* editor);
	CView* createCustomView (const char* name, const UIAttributes& attributes, IUIDescription* description, VST3Editor* editor);

	static Steinberg::FUnknown* createInstance (void*) { return (Steinberg::Vst::IEditController*)new AnimationTestController; }
	static Steinberg::FUID cid;
};


} // namespace

#endif // __animationtest__
