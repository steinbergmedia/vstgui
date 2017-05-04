// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE


#ifndef __graphicstest__
#define __graphicstest__

#include "uidescription test.h"

namespace VSTGUI {

//------------------------------------------------------------------------
class GraphicsTestController : public UIDescriptionBaseController
{
public:
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) override;
	Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name) override;

	CView* createCustomView (const char* name, const UIAttributes& attributes, const IUIDescription* description, VST3Editor* editor) override;
	IController* createSubController (const char* name, const IUIDescription* description, VST3Editor* editor) override;

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
