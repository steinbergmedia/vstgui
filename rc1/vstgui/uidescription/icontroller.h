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

#ifndef __icontroller__
#define __icontroller__

#include "../lib/controls/icontrollistener.h"
#include "../lib/cview.h"

namespace VSTGUI {

class UIAttributes;
class IUIDescription;

//-----------------------------------------------------------------------------
/// @brief extension to IControlListener used by UIDescription
/// @ingroup new_in_4_0
//-----------------------------------------------------------------------------
class IController : public IControlListener
{
public:
	virtual int32_t getTagForName (UTF8StringPtr name, int32_t registeredTag) const { return registeredTag; }
	virtual IControlListener* getControlListener (UTF8StringPtr controlTagName) { return this; }
	virtual CView* createView (const UIAttributes& attributes, const IUIDescription* description) { return 0; }
	virtual CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) { return view; }
	virtual IController* createSubController (UTF8StringPtr name, const IUIDescription* description) { return 0; }

#if DEBUG && VSTGUI_ENABLE_DEPRECATED_METHODS
	// the method arguments have changed for the above methods so here are definitions that will generate a compiler error
	// if someone has not updated its methods
	virtual int32_t createView (const UIAttributes& attributes, IUIDescription* description) VSTGUI_FINAL_VMETHOD { return 0; }
	virtual int32_t verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description) VSTGUI_FINAL_VMETHOD { return 0; }
	virtual int32_t createSubController (UTF8StringPtr name, IUIDescription* description) VSTGUI_FINAL_VMETHOD { return 0;}
#endif
};

//-----------------------------------------------------------------------------
//! @brief extension to IController
//!			The VST3Editor checks all controllers of the views under the mouse on a right click if they have
//!			this interface implemented and calls the appendContextMenuItems before showing the context menu to the user
//! @ingroup new_in_4_3
//-----------------------------------------------------------------------------
class IContextMenuController
{
public:
	virtual ~IContextMenuController () {}
	
	virtual void appendContextMenuItems (COptionMenu& contextMenu, const CPoint& where) = 0;
};

//-----------------------------------------------------------------------------
/* helper method to get the controller of a view */
inline IController* getViewController (const CView* view, bool deep = false)
{
	IController* controller = 0;
	uint32_t size = sizeof (IController*);
	if (view->getAttribute (kCViewControllerAttribute, sizeof (IController*), &controller, size) == false && deep)
	{
		if (view->getParentView () && view->getParentView () != view)
		{
			return getViewController (view->getParentView (), deep);
		}
	}
	return controller;
}

} // namespace

#endif // __icontroller__
