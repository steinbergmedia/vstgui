// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __plugguieditor__
#include "plugguieditor.h"
#endif

#if WINDOWS
#include <windows.h>
#include <ole2.h>
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
// PluginGUIEditor Implementation
//-----------------------------------------------------------------------------
/*! @class PluginGUIEditor
This is the same as the AEffGUIEditor class except that this one allows
the VSTGUI lib to build without VST dependencies.
*/
PluginGUIEditor::PluginGUIEditor (void *pEffect)
	: effect (pEffect)
{
	systemWindow = nullptr;

	#if WINDOWS
	OleInitialize (nullptr);
	#endif
}

//-----------------------------------------------------------------------------
PluginGUIEditor::~PluginGUIEditor ()
{
	#if WINDOWS
	OleUninitialize ();
	#endif
}

//-----------------------------------------------------------------------------
bool PluginGUIEditor::open (void *ptr)
{
	systemWindow = ptr;
	return true;
}

//-----------------------------------------------------------------------------
void PluginGUIEditor::idle ()
{
	if (frame)
		frame->idle ();
}

//-----------------------------------------------------------------------------
int32_t PluginGUIEditor::knobMode = kCircularMode;

//-----------------------------------------------------------------------------
int32_t PluginGUIEditor::setKnobMode (int32_t val)
{
	PluginGUIEditor::knobMode = val;
	return 1;
}

//-----------------------------------------------------------------------------
void PluginGUIEditor::doIdleStuff ()
{
	vstgui_assert (false, "unexpected call");
}

//-----------------------------------------------------------------------------
bool PluginGUIEditor::getRect (ERect **ppErect)
{
	*ppErect = &rect;
	return true;
}

} // VSTGUI
