//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins :
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

#ifndef __iviewlistener__
#define __iviewlistener__

#include "vstguifwd.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
/// @brief View Listener Interface
//-----------------------------------------------------------------------------
class IViewListener
{
public:
	virtual void viewSizeChanged (CView* view, const CRect& oldSize) = 0;
	virtual void viewAttached (CView* view) = 0;
	virtual void viewRemoved (CView* view) = 0;
	virtual void viewLostFocus (CView* view) = 0;
	virtual void viewTookFocus (CView* view) = 0;
	virtual void viewWillDelete (CView* view) = 0;
};

//-----------------------------------------------------------------------------
/// @brief ViewContainer Listener Interface
//-----------------------------------------------------------------------------
class IViewContainerListener
{
public:
	virtual void viewContainerViewAdded (CViewContainer* container, CView* view) = 0;
	virtual void viewContainerViewRemoved (CViewContainer* container, CView* view) = 0;
	virtual void viewContainerViewZOrderChanged (CViewContainer* container, CView* view) = 0;
	virtual void viewContainerTransformChanged (CViewContainer* container) = 0;
};

//-----------------------------------------------------------------------------
/// @brief View Listener Interface Adapter
//-----------------------------------------------------------------------------
class IViewListenerAdapter : public IViewListener
{
public:
	virtual ~IViewListenerAdapter () {}

	void viewSizeChanged (CView* view, const CRect& oldSize) VSTGUI_OVERRIDE_VMETHOD {}
	void viewAttached (CView* view) VSTGUI_OVERRIDE_VMETHOD {}
	void viewRemoved (CView* view) VSTGUI_OVERRIDE_VMETHOD {}
	void viewLostFocus (CView* view) VSTGUI_OVERRIDE_VMETHOD {}
	void viewTookFocus (CView* view) VSTGUI_OVERRIDE_VMETHOD {}
	void viewWillDelete (CView* view) VSTGUI_OVERRIDE_VMETHOD {}
};

//-----------------------------------------------------------------------------
/// @brief ViewContainer Listener Interface Adapter
//-----------------------------------------------------------------------------
class IViewContainerListenerAdapter : public IViewContainerListener
{
public:
	void viewContainerViewAdded (CViewContainer* container, CView* view) VSTGUI_OVERRIDE_VMETHOD {}
	void viewContainerViewRemoved (CViewContainer* container, CView* view) VSTGUI_OVERRIDE_VMETHOD {}
	void viewContainerViewZOrderChanged (CViewContainer* container, CView* view) VSTGUI_OVERRIDE_VMETHOD {}
	void viewContainerTransformChanged (CViewContainer* container) VSTGUI_OVERRIDE_VMETHOD {}
};

}


#endif // __iviewlistener__
