// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) override;
	Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name) override;
	IController* createSubController (const char* name, const IUIDescription* description, VST3Editor* editor) override;

	static Steinberg::FUnknown* createInstance (void*) { return (Steinberg::Vst::IEditController*)new OpenGLViewTestController; }
	static Steinberg::FUID cid;
};


} // namespace


#endif // __openglviewtest__
