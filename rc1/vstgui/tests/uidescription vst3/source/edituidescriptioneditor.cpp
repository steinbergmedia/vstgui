//
//  edituidescriptioneditor.cpp
//  uidescription test
//
//  Created by Arne Scheffler on 22/10/15.
//
//

#include "edituidescriptioneditor.h"
#include "vstgui/plugin-bindings/vst3editor.h"

//------------------------------------------------------------------------
namespace VSTGUI {

using namespace Steinberg;

//------------------------------------------------------------------------
FUID EditEditorProcessor::cid (0x74F52487, 0xAA4140D1, 0x8A7E6E10, 0x4F554088);
FUID EditEditorController::cid (0x2118D144, 0xB5ED4967, 0xBC57F4E1, 0x8CA030E3);

//------------------------------------------------------------------------
IPlugView* PLUGIN_API EditEditorController::createView (FIDString name)
{
	std::string descPath (__FILE__);
	unixfyPath (descPath);
	removeLastPathComponent (descPath);
	removeLastPathComponent (descPath);
	removeLastPathComponent (descPath);
	removeLastPathComponent (descPath);
	descPath += "/uidescription/editing/uidescriptioneditor.uidesc";
	return new VST3Editor (this, "view", descPath.c_str ());
}

//------------------------------------------------------------------------
EditEditorProcessor::EditEditorProcessor ()
{
	setControllerClass (EditEditorController::cid);
}

} // VSTGUI
