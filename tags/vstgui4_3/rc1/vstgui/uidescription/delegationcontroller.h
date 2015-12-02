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

#ifndef __delegationcontroller__
#define __delegationcontroller__

#include "icontroller.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
class DelegationController : public IController
{
public:
	DelegationController (IController* controller) : controller (controller) {}

	// IControlListener
	void valueChanged (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD { controller->valueChanged (pControl); }
	int32_t controlModifierClicked (CControl* pControl, CButtonState button) VSTGUI_OVERRIDE_VMETHOD { return controller->controlModifierClicked (pControl, button); }
	void controlBeginEdit (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD { controller->controlBeginEdit (pControl); }
	void controlEndEdit (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD { controller->controlEndEdit (pControl); }
	void controlTagWillChange (VSTGUI::CControl* pControl) VSTGUI_OVERRIDE_VMETHOD { controller->controlTagWillChange (pControl); }
	void controlTagDidChange (VSTGUI::CControl* pControl) VSTGUI_OVERRIDE_VMETHOD { controller->controlTagDidChange (pControl); }
	// IController
	int32_t getTagForName (UTF8StringPtr name, int32_t registeredTag) const VSTGUI_OVERRIDE_VMETHOD { return controller->getTagForName (name, registeredTag); }
	IControlListener* getControlListener (UTF8StringPtr name) VSTGUI_OVERRIDE_VMETHOD { return controller->getControlListener (name); }
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD { return controller->createView (attributes, description); }
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD { return controller->verifyView (view, attributes, description); }
	IController* createSubController (IdStringPtr name, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD { return controller->createSubController (name, description); }
protected:
	IController* controller;
};

}

#endif // __delegationcontroller__
