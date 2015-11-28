//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins
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

#ifndef __uidialogcontroller__
#define __uidialogcontroller__

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "../delegationcontroller.h"
#include "../../lib/cframe.h"
#include "../../lib/iviewlistener.h"
#include "../../lib/copenglview.h"
#include <string>
#include <list>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UIDialogController : public CBaseObject, public DelegationController, public IKeyboardHook, public IViewListenerAdapter
{
public:
	UIDialogController (IController* baseController, CFrame* frame);
	~UIDialogController ();
	
	void run (UTF8StringPtr templateName, UTF8StringPtr dialogTitle, UTF8StringPtr button1, UTF8StringPtr button2, IController* controller, UIDescription* description);

	static IdStringPtr kMsgDialogButton1Clicked;
	static IdStringPtr kMsgDialogButton2Clicked;
	static IdStringPtr kMsgDialogShow;
protected:
	void valueChanged (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;
	IControlListener* getControlListener (UTF8StringPtr controlTagName) VSTGUI_OVERRIDE_VMETHOD;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD;

	void viewSizeChanged (CView* view, const CRect& oldSize) VSTGUI_OVERRIDE_VMETHOD;
	void viewRemoved (CView* view) VSTGUI_OVERRIDE_VMETHOD;
	
	void close ();
	void layoutButtons ();
	void collectOpenGLViews (CViewContainer* container);
	void setOpenGLViewsVisible (bool state);

	int32_t onKeyDown (const VstKeyCode& code, CFrame* frame) VSTGUI_OVERRIDE_VMETHOD;
	int32_t onKeyUp (const VstKeyCode& code, CFrame* frame) VSTGUI_OVERRIDE_VMETHOD;

	CFrame* frame;
	SharedPointer<CBaseObject> dialogController;
	UIDescription* dialogDescription;
	SharedPointer<CControl> button1;
	SharedPointer<CControl> button2;
	CPoint sizeDiff;
	std::string templateName;
	std::string dialogTitle;
	std::string dialogButton1;
	std::string dialogButton2;

	std::list<SharedPointer<COpenGLView> > openglViews;
		
	enum {
		kButton1Tag,
		kButton2Tag,
		kTitleTag
	};
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uidialogcontroller__
