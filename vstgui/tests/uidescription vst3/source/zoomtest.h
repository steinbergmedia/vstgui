// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) override;
	Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name) override;
	IController* createSubController (UTF8StringPtr name, const IUIDescription* description, VST3Editor* editor) override;

	void PLUGIN_API update (Steinberg::FUnknown* changedUnknown, Steinberg::int32 message) override;

	void editorAttached (Steinberg::Vst::EditorView* editor) override;
	void editorRemoved (Steinberg::Vst::EditorView* editor) override;

	static Steinberg::FUnknown* createInstance (void*) { return (Steinberg::Vst::IEditController*)new ZoomTestController; }
	static Steinberg::FUID cid;
private:
	typedef std::vector<Steinberg::Vst::EditorView*> EditorVector;
	EditorVector editors;
};


} // namespace

#endif // __zoomtest__
