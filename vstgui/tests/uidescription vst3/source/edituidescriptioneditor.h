// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __edituidescriptioneditor__
#define __edituidescriptioneditor__

#include "uidescription test.h"

namespace VSTGUI {

//------------------------------------------------------------------------
class EditEditorController : public UIDescriptionBaseController
{
public:
	Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name) override;

	static Steinberg::FUnknown* createInstance (void*) { return (Steinberg::Vst::IEditController*)new EditEditorController; }
	static Steinberg::FUID cid;
};

//------------------------------------------------------------------------
class EditEditorProcessor : public UIDescriptionTestProcessor
{
public:
	EditEditorProcessor ();
	
	static Steinberg::FUnknown* createInstance (void*) { return (Steinberg::Vst::IAudioProcessor*)new EditEditorProcessor; }
	static Steinberg::FUID cid;
};


} // VSTGUI

#endif /* edituidescriptioneditor_hpp */
