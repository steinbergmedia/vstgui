// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __uidescription_test__
#define __uidescription_test__

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "vstgui/plugin-bindings/vst3editor.h"
#include "pluginterfaces/base/ustring.h"

namespace VSTGUI {

//------------------------------------------------------------------------
class UIDescriptionBaseController : public Steinberg::Vst::EditController, public VST3EditorDelegate
{
public:
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) override;
	Steinberg::tresult beginEdit (Steinberg::Vst::ParamID tag) override;
	Steinberg::tresult performEdit (Steinberg::Vst::ParamID tag, Steinberg::Vst::ParamValue valueNormalized) override;
	Steinberg::tresult endEdit (Steinberg::Vst::ParamID tag) override;
	Steinberg::Vst::Parameter* getParameterObject (Steinberg::Vst::ParamID tag) override;

	bool isPrivateParameter (const Steinberg::Vst::ParamID paramID) override;
protected:
	Steinberg::Vst::ParameterContainer uiParameters;
};

//------------------------------------------------------------------------
class UIDescriptionTestProcessor : public Steinberg::Vst::AudioEffect
{
public:
	UIDescriptionTestProcessor ();
	
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) override;
	Steinberg::tresult PLUGIN_API setBusArrangements (Steinberg::Vst::SpeakerArrangement* inputs, Steinberg::int32 numIns, Steinberg::Vst::SpeakerArrangement* outputs, Steinberg::int32 numOuts) override;
	Steinberg::tresult PLUGIN_API process (Steinberg::Vst::ProcessData& data) override;
	Steinberg::tresult PLUGIN_API setProcessing (Steinberg::TBool state) override;

	static Steinberg::FUnknown* createInstance (void*) { return (Steinberg::Vst::IAudioProcessor*)new UIDescriptionTestProcessor; }
	static Steinberg::FUID cid;
private:
	float peak;
};

//------------------------------------------------------------------------
class UIDescriptionTestController : public UIDescriptionBaseController
{
public:
	UIDescriptionTestController ();
	
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) override;
	Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name) override;
	void willClose (VST3Editor* editor) override;
	COptionMenu* createContextMenu (const CPoint& pos, VST3Editor* editor) override;

	IController* createSubController (const char* name, const IUIDescription* description, VST3Editor* editor) override;
	CView* createCustomView (UTF8StringPtr name, const UIAttributes& attributes, const IUIDescription* description, VST3Editor* editor) override;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description, VST3Editor* editor) override;

	static Steinberg::FUnknown* createInstance (void*) { return (Steinberg::Vst::IEditController*)new UIDescriptionTestController; }
	static Steinberg::FUID cid;
protected:
	CBaseObject* splitViewController;
};

} // namespace

#endif // __uidescription_test__