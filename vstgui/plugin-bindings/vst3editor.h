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

#ifndef __vst3editor__
#define __vst3editor__

#include "public.sdk/source/vst/vstguieditor.h"
#include "pluginterfaces/vst/ivstplugview.h"
#include "../uidescription/uidescription.h"
#include <string>
#include <vector>
#include <map>

#if VST_VERSION >= 0x030607
#include "pluginterfaces/gui/iplugviewcontentscalesupport.h"
#define VST3_CONTENT_SCALE_SUPPORT
#endif

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
	
	virtual CView* createCustomView (UTF8StringPtr name, const UIAttributes& attributes, const IUIDescription* description, VST3Editor* editor) { return 0; } ///< create a custom view
	virtual CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description, VST3Editor* editor) { return view; } ///< verify a view after it was created
	virtual bool findParameter (const CPoint& pos, Steinberg::Vst::ParamID& paramID, VST3Editor* editor) { return false; } ///< find a parameter
	virtual bool isPrivateParameter (const Steinberg::Vst::ParamID paramID) { return false; } ///< check if parameter ID is private and should not be exposed to the host
	virtual void didOpen (VST3Editor* editor) {}	///< called after the editor was opened
	virtual void willClose (VST3Editor* editor) {}	///< called before the editor will close
	virtual COptionMenu* createContextMenu (const CPoint& pos, VST3Editor* editor) { return 0; }	///< create the context menu for the editor, will be added to the host menu

	/** called when a sub controller should be created.
	    The controller is now owned by the editor, which will call forget() if it is a CBaseObject, release() if it is a Steinberg::FObject or it will be simply deleted if the frame gets closed. */
	virtual IController* createSubController (UTF8StringPtr name, const IUIDescription* description, VST3Editor* editor) { return 0; } ///< create a sub controller
};

//-----------------------------------------------------------------------------
//! @brief VST3 Editor with automatic parameter binding
//! @ingroup new_in_4_0
//-----------------------------------------------------------------------------
class VST3Editor : public Steinberg::Vst::VSTGUIEditor,
                   public Steinberg::Vst::IParameterFinder,
                   public IController,
                   public IViewAddedRemovedObserver,
                   public IMouseObserver,
                   public IKeyboardHook
#ifdef VST3_CONTENT_SCALE_SUPPORT
				 , public Steinberg::IPlugViewContentScaleSupport
#endif
{
public:
	VST3Editor (Steinberg::Vst::EditController* controller, UTF8StringPtr templateName, UTF8StringPtr xmlFile);
	VST3Editor (UIDescription* desc, Steinberg::Vst::EditController* controller, UTF8StringPtr templateName, UTF8StringPtr xmlFile = 0);

	bool exchangeView (UTF8StringPtr templateName);
	void enableTooltips (bool state);

	bool setEditorSizeConstrains (const CPoint& newMinimumSize, const CPoint& newMaximumSize);
	void getEditorSizeConstrains (CPoint& minimumSize, CPoint& maximumSize) const;
	bool requestResize (const CPoint& newSize);

	void setZoomFactor (double factor);
	double getZoomFactor () const { return zoomFactor; }
	
	void setAllowedZoomFactors (std::vector<double> zoomFactors) { allowedZoomFactors = zoomFactors; }

//-----------------------------------------------------------------------------
	DELEGATE_REFCOUNT(Steinberg::Vst::VSTGUIEditor)
	Steinberg::tresult PLUGIN_API queryInterface (const ::Steinberg::TUID iid, void** obj) VSTGUI_OVERRIDE_VMETHOD;
protected:
	~VST3Editor ();
	void init ();
	double getAbsScaleFactor () const;
	ParameterChangeListener* getParameterChangeListener (int32_t tag) const;
	void recreateView ();

	void syncParameterTags ();
	void save (bool saveAs = false);
	bool enableEditing (bool state);

	bool PLUGIN_API open (void* parent, const PlatformType& type) VSTGUI_OVERRIDE_VMETHOD;
	void PLUGIN_API close () VSTGUI_OVERRIDE_VMETHOD;

	void beginEdit (int32_t index) VSTGUI_OVERRIDE_VMETHOD;
	void endEdit (int32_t index) VSTGUI_OVERRIDE_VMETHOD;

	CView* createView (const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	IController* createSubController (UTF8StringPtr name, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;

	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD;

	bool beforeSizeChange (const CRect& newSize, const CRect& oldSize) VSTGUI_OVERRIDE_VMETHOD;

	Steinberg::tresult PLUGIN_API onSize (Steinberg::ViewRect* newSize) VSTGUI_OVERRIDE_VMETHOD;
	Steinberg::tresult PLUGIN_API canResize () VSTGUI_OVERRIDE_VMETHOD;
	Steinberg::tresult PLUGIN_API checkSizeConstraint (Steinberg::ViewRect* rect) VSTGUI_OVERRIDE_VMETHOD;

	// IParameterFinder
	Steinberg::tresult PLUGIN_API findParameter (Steinberg::int32 xPos, Steinberg::int32 yPos, Steinberg::Vst::ParamID& resultTag) VSTGUI_OVERRIDE_VMETHOD;

	// IControlListener
	virtual void valueChanged (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;
	virtual void controlBeginEdit (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;
	virtual void controlEndEdit (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;
	virtual void controlTagWillChange (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;
	virtual void controlTagDidChange (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;

	// IViewAddedRemovedObserver
	void onViewAdded (CFrame* frame, CView* view) VSTGUI_OVERRIDE_VMETHOD;
	void onViewRemoved (CFrame* frame, CView* view) VSTGUI_OVERRIDE_VMETHOD;

	// IMouseObserver
	void onMouseEntered (CView* view, CFrame* frame) VSTGUI_OVERRIDE_VMETHOD {}
	void onMouseExited (CView* view, CFrame* frame) VSTGUI_OVERRIDE_VMETHOD {}
	CMouseEventResult onMouseMoved (CFrame* frame, const CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD { return kMouseEventNotHandled; }
	CMouseEventResult onMouseDown (CFrame* frame, const CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;

	// IKeyboardHook
	int32_t onKeyDown (const VstKeyCode& code, CFrame* frame) VSTGUI_OVERRIDE_VMETHOD;
	int32_t onKeyUp (const VstKeyCode& code, CFrame* frame) VSTGUI_OVERRIDE_VMETHOD;

#ifdef VST3_CONTENT_SCALE_SUPPORT
	Steinberg::tresult PLUGIN_API setContentScaleFactor (ScaleFactor factor) VSTGUI_OVERRIDE_VMETHOD;
#endif

	UIDescription* description;
	VST3EditorDelegate* delegate;
	IController* originalController;
	typedef std::map<int32_t, ParameterChangeListener*> ParameterChangeListenerMap;
	ParameterChangeListenerMap paramChangeListeners;
	std::string viewName;
	std::string xmlFile;
	bool tooltipsEnabled;
	bool doCreateView;
	bool editingEnabled;
	bool requestResizeGuard;

	double contentScaleFactor;
	double zoomFactor;
	std::vector<double> allowedZoomFactors;
	
	CPoint minSize;
	CPoint maxSize;
	CRect nonEditRect;
};

} // namespace

#endif
