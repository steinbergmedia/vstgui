//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

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
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) VSTGUI_OVERRIDE_VMETHOD;
	Steinberg::tresult beginEdit (Steinberg::Vst::ParamID tag) VSTGUI_OVERRIDE_VMETHOD;
	Steinberg::tresult performEdit (Steinberg::Vst::ParamID tag, Steinberg::Vst::ParamValue valueNormalized) VSTGUI_OVERRIDE_VMETHOD;
	Steinberg::tresult endEdit (Steinberg::Vst::ParamID tag) VSTGUI_OVERRIDE_VMETHOD;
	Steinberg::Vst::Parameter* getParameterObject (Steinberg::Vst::ParamID tag) VSTGUI_OVERRIDE_VMETHOD;

	bool isPrivateParameter (const Steinberg::Vst::ParamID paramID) VSTGUI_OVERRIDE_VMETHOD;
protected:
	Steinberg::Vst::ParameterContainer uiParameters;
};

//------------------------------------------------------------------------
class UIDescriptionTestProcessor : public Steinberg::Vst::AudioEffect
{
public:
	UIDescriptionTestProcessor ();
	
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) VSTGUI_OVERRIDE_VMETHOD;
	Steinberg::tresult PLUGIN_API setBusArrangements (Steinberg::Vst::SpeakerArrangement* inputs, Steinberg::int32 numIns, Steinberg::Vst::SpeakerArrangement* outputs, Steinberg::int32 numOuts) VSTGUI_OVERRIDE_VMETHOD;
	Steinberg::tresult PLUGIN_API process (Steinberg::Vst::ProcessData& data) VSTGUI_OVERRIDE_VMETHOD;
	Steinberg::tresult PLUGIN_API setProcessing (Steinberg::TBool state) VSTGUI_OVERRIDE_VMETHOD;

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
	
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) VSTGUI_OVERRIDE_VMETHOD;
	Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name) VSTGUI_OVERRIDE_VMETHOD;
	void willClose (VST3Editor* editor) VSTGUI_OVERRIDE_VMETHOD;
	COptionMenu* createContextMenu (const CPoint& pos, VST3Editor* editor) VSTGUI_OVERRIDE_VMETHOD;

	IController* createSubController (const char* name, const IUIDescription* description, VST3Editor* editor) VSTGUI_OVERRIDE_VMETHOD;
	CView* createCustomView (UTF8StringPtr name, const UIAttributes& attributes, const IUIDescription* description, VST3Editor* editor) VSTGUI_OVERRIDE_VMETHOD;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description, VST3Editor* editor) VSTGUI_OVERRIDE_VMETHOD;

	static Steinberg::FUnknown* createInstance (void*) { return (Steinberg::Vst::IEditController*)new UIDescriptionTestController; }
	static Steinberg::FUID cid;
protected:
	CBaseObject* splitViewController;
};

} // namespace

#endif // __uidescription_test__