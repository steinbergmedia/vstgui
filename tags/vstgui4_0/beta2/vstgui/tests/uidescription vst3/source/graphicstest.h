/*
 *  graphicstest.h
 *  uidescription test
 *
 *  Created by Arne Scheffler on 3/16/10.
 *  Copyright 2010 Arne Scheffler. All rights reserved.
 *
 */

#ifndef __graphicstest__
#define __graphicstest__

#include "uidescription test.h"

namespace VSTGUI {

//------------------------------------------------------------------------
class GraphicsTestController : public UIDescriptionBaseController
{
public:
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context);
	Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name);

	CView* createCustomView (const char* name, const UIAttributes& attributes, IUIDescription* description, VST3Editor* editor);
	IController* createSubController (const char* name, IUIDescription* description, VST3Editor* editor);

	static Steinberg::FUnknown* createInstance (void*) { return (Steinberg::Vst::IEditController*)new GraphicsTestController; }
	static Steinberg::FUID cid;
};

//------------------------------------------------------------------------
class GraphicsTestProcessor : public UIDescriptionTestProcessor
{
public:
	GraphicsTestProcessor () { setControllerClass (GraphicsTestController::cid); }
	
	static Steinberg::FUnknown* createInstance (void*) { return (Steinberg::Vst::IAudioProcessor*)new GraphicsTestProcessor; }
	static Steinberg::FUID cid;
};

} // namespace

#endif // __graphicstest__
