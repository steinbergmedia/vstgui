// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE


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
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) override;
	Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name) override;

	IController* createSubController (const char* name, const IUIDescription* description, VST3Editor* editor) override;
	CView* createCustomView (const char* name, const UIAttributes& attributes, const IUIDescription* description, VST3Editor* editor) override;

	static Steinberg::FUnknown* createInstance (void*) { return (Steinberg::Vst::IEditController*)new AnimationTestController; }
	static Steinberg::FUID cid;
};


} // namespace

#endif // __animationtest__
