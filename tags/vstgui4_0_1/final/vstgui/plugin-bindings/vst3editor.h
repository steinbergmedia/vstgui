//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2011, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#ifndef __vst3editor__
#define __vst3editor__

#include "public.sdk/source/vst/vstguieditor.h"
#include "pluginterfaces/vst/ivstplugview.h"
#include "../uidescription/uidescription.h"
#include <string>
#include <map>
#include <deque>

namespace VSTGUI {
class ParameterChangeListener;
class VST3Editor;

//-----------------------------------------------------------------------------
//! @brief delegate extension to Steinberg::Vst::EditController for a VST3 Editor
//! @ingroup new_in_4_0
//-----------------------------------------------------------------------------
class VST3EditorDelegate
{
public:
	virtual ~VST3EditorDelegate () {}
	
	virtual CView* createCustomView (UTF8StringPtr name, const UIAttributes& attributes, IUIDescription* description, VST3Editor* editor) { return 0; } ///< create a custom view
	virtual bool findParameter (const CPoint& pos, Steinberg::Vst::ParamID& paramID, VST3Editor* editor) { return false; } ///< find a parameter
	virtual bool isPrivateParameter (const Steinberg::Vst::ParamID paramID) { return false; } ///< check if parameter ID is private and should not be exposed to the host
	virtual void didOpen (VST3Editor* editor) {}	///< called after the editor was opened
	virtual void willClose (VST3Editor* editor) {}	///< called before the editor will close

	/** called when a sub controller should be created.
	    The controller is now owned by the editor, which will call forget() if it is a CBaseObject, release() if it is a Steinberg::FObject or it will be simply deleted if the frame gets closed. */
	virtual IController* createSubController (UTF8StringPtr name, IUIDescription* description, VST3Editor* editor) { return 0; } ///< create a sub controller
};

//-----------------------------------------------------------------------------
//! @brief VST3 Editor with automatic parameter binding
//! @ingroup new_in_4_0
//-----------------------------------------------------------------------------
class VST3Editor : public Steinberg::Vst::VSTGUIEditor, public Steinberg::Vst::IParameterFinder, public IController, public IViewAddedRemovedObserver, public IMouseObserver
{
public:
	VST3Editor (Steinberg::Vst::EditController* controller, UTF8StringPtr templateName, UTF8StringPtr xmlFile);
	VST3Editor (UIDescription* desc, Steinberg::Vst::EditController* controller, UTF8StringPtr templateName, UTF8StringPtr xmlFile = 0);

	bool exchangeView (UTF8StringPtr templateName);
	void enableTooltips (bool state);

//-----------------------------------------------------------------------------
	DELEGATE_REFCOUNT(Steinberg::Vst::VSTGUIEditor)
	Steinberg::tresult PLUGIN_API queryInterface (const ::Steinberg::TUID iid, void** obj);
protected:
	~VST3Editor ();
	void init ();
	ParameterChangeListener* getParameterChangeListener (int32_t tag);
	void recreateView ();

	#if VSTGUI_LIVE_EDITING
	void runNewTemplateDialog (IdStringPtr baseViewName);
	void runTemplateSettingsDialog ();
	void syncParameterTags ();
	#endif // VSTGUI_LIVE_EDITING
	
	bool PLUGIN_API open (void* parent);
	void PLUGIN_API close ();

	void beginEdit (int32_t index);
	void endEdit (int32_t index);

	CView* createView (const UIAttributes& attributes, IUIDescription* description);
	CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description);

	CMessageResult notify (CBaseObject* sender, IdStringPtr message);

	Steinberg::tresult PLUGIN_API onSize (Steinberg::ViewRect* newSize);
	Steinberg::tresult PLUGIN_API canResize ();
	Steinberg::tresult PLUGIN_API checkSizeConstraint (Steinberg::ViewRect* rect);

	// IParameterFinder
	Steinberg::tresult PLUGIN_API findParameter (Steinberg::int32 xPos, Steinberg::int32 yPos, Steinberg::Vst::ParamID& resultTag);

	// CControlListener
	virtual void valueChanged (CControl* pControl);
	virtual void controlBeginEdit (CControl* pControl);
	virtual void controlEndEdit (CControl* pControl);
	virtual void controlTagWillChange (CControl* pControl);
	virtual void controlTagDidChange (CControl* pControl);

	// IViewAddedRemovedObserver
	void onViewAdded (CFrame* frame, CView* view);
	void onViewRemoved (CFrame* frame, CView* view);

	// IMouseObserver
	void onMouseEntered (CView* view, CFrame* frame) {}
	void onMouseExited (CView* view, CFrame* frame) {}
	CMouseEventResult onMouseMoved (CFrame* frame, const CPoint& where, const CButtonState& buttons) { return kMouseEventNotHandled; }
	CMouseEventResult onMouseDown (CFrame* frame, const CPoint& where, const CButtonState& buttons);

	// @cond ignore
	struct SubController
	{
		IController* controller;
		std::string name;
		
		SubController (IController* c, const std::string& n) : controller (c), name (n) {}
	};
	// @endcond

	UIDescription* description;
	VST3EditorDelegate* delegate;
	std::map<int32_t, ParameterChangeListener*> paramChangeListeners;
	std::deque<SubController> subControllerStack;
	std::list<IController*> subControllers;
	std::string viewName;
	std::string xmlFile;
	bool tooltipsEnabled;
	bool doCreateView;
	
	CPoint minSize;
	CPoint maxSize;
};

} // namespace

#endif
