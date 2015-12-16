//
//  edituidescriptioneditor.hpp
//  uidescription test
//
//  Created by Arne Scheffler on 22/10/15.
//
//

#ifndef __edituidescriptioneditor__
#define __edituidescriptioneditor__

#include "uidescription test.h"

namespace VSTGUI {

//------------------------------------------------------------------------
class EditEditorController : public UIDescriptionBaseController
{
public:
	Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name) VSTGUI_OVERRIDE_VMETHOD;

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
