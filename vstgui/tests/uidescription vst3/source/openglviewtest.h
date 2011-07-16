//
//  openglviewtest.h
//  uidescription test
//
//  Created by Arne Scheffler on 7/16/11.
//  Copyright 2011 Arne Scheffler. All rights reserved.
//

#ifndef __openglviewtest__
#define __openglviewtest__

#include "uidescription test.h"

namespace VSTGUI {

//------------------------------------------------------------------------
class OpenGLViewTestProcessor : public UIDescriptionTestProcessor
{
public:
	OpenGLViewTestProcessor ();
	
	static Steinberg::FUnknown* createInstance (void*) { return (Steinberg::Vst::IAudioProcessor*)new OpenGLViewTestProcessor; }
	static Steinberg::FUID cid;
};

//------------------------------------------------------------------------
class OpenGLViewTestController : public UIDescriptionBaseController
{
public:
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context);
	Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name);
	CView* createCustomView (UTF8StringPtr name, const UIAttributes& attributes, IUIDescription* description, VST3Editor* editor);

	static Steinberg::FUnknown* createInstance (void*) { return (Steinberg::Vst::IEditController*)new OpenGLViewTestController; }
	static Steinberg::FUID cid;
};


} // namespace


#endif // __openglviewtest__
