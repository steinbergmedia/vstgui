//
//  ZoomTest.h
//  uidescription test
//
//  Created by Arne Scheffler on 26/06/14.
//
//

#ifndef __zoomtest__
#define __zoomtest__

#include "uidescription test.h"
#include <vector>

namespace VSTGUI {

//------------------------------------------------------------------------
class ZoomTestProcessor : public UIDescriptionTestProcessor
{
public:
	ZoomTestProcessor ();
	
	static Steinberg::FUnknown* createInstance (void*) { return (Steinberg::Vst::IAudioProcessor*)new ZoomTestProcessor; }
	static Steinberg::FUID cid;
};

//------------------------------------------------------------------------
class ZoomTestController : public UIDescriptionBaseController
{
public:
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) VSTGUI_OVERRIDE_VMETHOD;
	Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name) VSTGUI_OVERRIDE_VMETHOD;
	IController* createSubController (UTF8StringPtr name, const IUIDescription* description, VST3Editor* editor) VSTGUI_OVERRIDE_VMETHOD;

	void PLUGIN_API update (Steinberg::FUnknown* changedUnknown, Steinberg::int32 message) VSTGUI_OVERRIDE_VMETHOD;

	void editorAttached (Steinberg::Vst::EditorView* editor) VSTGUI_OVERRIDE_VMETHOD;
	void editorRemoved (Steinberg::Vst::EditorView* editor) VSTGUI_OVERRIDE_VMETHOD;

	static Steinberg::FUnknown* createInstance (void*) { return (Steinberg::Vst::IEditController*)new ZoomTestController; }
	static Steinberg::FUID cid;
private:
	typedef std::vector<Steinberg::Vst::EditorView*> EditorVector;
	EditorVector editors;
};


} // namespace

#endif // __zoomtest__
