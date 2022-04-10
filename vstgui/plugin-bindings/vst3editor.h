// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "public.sdk/source/vst/vstguieditor.h"
#include "pluginterfaces/vst/ivstplugview.h"
#include "../uidescription/uidescription.h"
#include "../uidescription/icontroller.h"
#include "../lib/controls/icommandmenuitemtarget.h"
#include "../lib/optional.h"
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
//! @brief delegate interface for a VST3Editor.
//!
//! You either extend Steinberg::Vst::EditController with this interface and pass the editor
//! controller to the constructor of the VST3Editor class, or you create a delegate without
//! extending Steinberg::Vst::EditController and explicitly set the delegate of the VST3Editor.
//!
//! @ingroup new_in_4_0
//-----------------------------------------------------------------------------
class IVST3EditorDelegate
{
public:
	virtual ~IVST3EditorDelegate () = default;

	/** create a custom view */
	virtual CView* createCustomView (UTF8StringPtr name, const UIAttributes& attributes,
									 const IUIDescription* description, VST3Editor* editor) = 0;
	/** verify a view after it was created */
	virtual CView* verifyView (CView* view, const UIAttributes& attributes,
							   const IUIDescription* description, VST3Editor* editor) = 0;
	/** find a parameter */
	virtual bool findParameter (const CPoint& pos, Steinberg::Vst::ParamID& paramID,
								VST3Editor* editor) = 0;
	/** check if parameter ID is private and should not be exposed to the host */
	virtual bool isPrivateParameter (const Steinberg::Vst::ParamID paramID) = 0;
	/** called after the editor was opened */
	virtual void didOpen (VST3Editor* editor) = 0;
	/** called before the editor will close */
	virtual void willClose (VST3Editor* editor) = 0;
	/** create the context menu for the editor, will be added to the host menu */
	virtual COptionMenu* createContextMenu (const CPoint& pos, VST3Editor* editor) = 0;
	/** called when a sub controller should be created.
	    The controller is now owned by the editor, which will call forget() if it is a CBaseObject,
	   release() if it is a Steinberg::FObject or it will be simply deleted if the frame gets
	   closed. */
	virtual IController* createSubController (UTF8StringPtr name, const IUIDescription* description,
											  VST3Editor* editor) = 0;
	/** called when the user zoom factor of the editor was changed */
	virtual void onZoomChanged (VST3Editor* editor, double newZoom) = 0;
};

//------------------------------------------------------------------------
/** Default adapter implementation for IVST3EditorDelegate */
class VST3EditorDelegate : public IVST3EditorDelegate
{
public:
	CView* createCustomView (UTF8StringPtr name, const UIAttributes& attributes,
							 const IUIDescription* description, VST3Editor* editor) override
	{
		return nullptr;
	}
	CView* verifyView (CView* view, const UIAttributes& attributes,
					   const IUIDescription* description, VST3Editor* editor) override
	{
		return view;
	}
	bool findParameter (const CPoint& pos, Steinberg::Vst::ParamID& paramID,
						VST3Editor* editor) override
	{
		return false;
	}
	bool isPrivateParameter (const Steinberg::Vst::ParamID paramID) override { return false; }
	void didOpen (VST3Editor* editor) override {}
	void willClose (VST3Editor* editor) override {}
	COptionMenu* createContextMenu (const CPoint& pos, VST3Editor* editor) override
	{
		return nullptr;
	}
	IController* createSubController (UTF8StringPtr name, const IUIDescription* description,
									  VST3Editor* editor) override
	{
		return nullptr;
	}
	void onZoomChanged (VST3Editor* editor, double newZoom) override {}
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
                   public CommandMenuItemTargetAdapter
#ifdef VST3_CONTENT_SCALE_SUPPORT
				 , public Steinberg::IPlugViewContentScaleSupport
#endif
{
public:
	VST3Editor (Steinberg::Vst::EditController* controller, UTF8StringPtr templateName, UTF8StringPtr xmlFile);
	VST3Editor (UIDescription* desc, Steinberg::Vst::EditController* controller, UTF8StringPtr templateName, UTF8StringPtr xmlFile = nullptr);

	bool exchangeView (UTF8StringPtr templateName);
	void enableTooltips (bool state);

	bool setEditorSizeConstrains (const CPoint& newMinimumSize, const CPoint& newMaximumSize);
	void getEditorSizeConstrains (CPoint& minimumSize, CPoint& maximumSize) const;
	bool requestResize (const CPoint& newSize);

	void setZoomFactor (double factor);
	double getZoomFactor () const { return zoomFactor; }
	
	void setAllowedZoomFactors (std::vector<double> zoomFactors) { allowedZoomFactors = zoomFactors; }

	/** set the delegate of the editor. no reference counting is happening here. */
	void setDelegate (IVST3EditorDelegate* delegate);
	IVST3EditorDelegate* getDelegate () const;
	UIDescription* getUIDescription () const;

	//-----------------------------------------------------------------------------
	DELEGATE_REFCOUNT(Steinberg::Vst::VSTGUIEditor)
	Steinberg::tresult PLUGIN_API queryInterface (const ::Steinberg::TUID iid, void** obj) override;
protected:
	~VST3Editor () override;
	void init ();
	double getAbsScaleFactor () const;
	ParameterChangeListener* getParameterChangeListener (int32_t tag) const;
	void recreateView ();
	void requestRecreateView ();

	void syncParameterTags ();
	void save (bool saveAs = false);
	bool enableEditing (bool state);

	bool PLUGIN_API open (void* parent, const PlatformType& type) override;
	void PLUGIN_API close () override;

	void beginEdit (int32_t index) override;
	void endEdit (int32_t index) override;

	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override;
	IController* createSubController (UTF8StringPtr name, const IUIDescription* description) override;

	bool beforeSizeChange (const CRect& newSize, const CRect& oldSize) override;

	Steinberg::tresult PLUGIN_API onSize (Steinberg::ViewRect* newSize) override;
	Steinberg::tresult PLUGIN_API canResize () override;
	Steinberg::tresult PLUGIN_API checkSizeConstraint (Steinberg::ViewRect* rect) override;

	// IParameterFinder
	Steinberg::tresult PLUGIN_API findParameter (Steinberg::int32 xPos, Steinberg::int32 yPos, Steinberg::Vst::ParamID& resultTag) override;

	// IControlListener
	virtual void valueChanged (CControl* pControl) override;
	virtual void controlBeginEdit (CControl* pControl) override;
	virtual void controlEndEdit (CControl* pControl) override;
	virtual void controlTagWillChange (CControl* pControl) override;
	virtual void controlTagDidChange (CControl* pControl) override;

	// IViewAddedRemovedObserver
	void onViewAdded (CFrame* frame, CView* view) override;
	void onViewRemoved (CFrame* frame, CView* view) override;

	// IMouseObserver
	void onMouseEntered (CView* view, CFrame* frame) override {}
	void onMouseExited (CView* view, CFrame* frame) override {}
	void onMouseEvent (MouseEvent& event, CFrame* frame) override;

	// CommandMenuItemTargetAdapter
	bool validateCommandMenuItem (CCommandMenuItem* item) override;
	bool onCommandMenuItemSelected (CCommandMenuItem* item) override;

#ifdef VST3_CONTENT_SCALE_SUPPORT
	Steinberg::tresult PLUGIN_API setContentScaleFactor (ScaleFactor factor) override;
#endif

	struct KeyboardHook;
	KeyboardHook* keyboardHook {nullptr};
	UIDescription* description {nullptr};
	IVST3EditorDelegate* delegate {nullptr};
	IController* originalController {nullptr};
	using ParameterChangeListenerMap = std::map<int32_t, ParameterChangeListener*>;
	ParameterChangeListenerMap paramChangeListeners;
	std::string viewName;
	std::string xmlFile;
	bool tooltipsEnabled {true};
	bool doCreateView {false};
	bool editingEnabled {false};

	double contentScaleFactor {1.};
	double zoomFactor {1.};
	std::vector<double> allowedZoomFactors;
	
	CPoint minSize;
	CPoint maxSize;
	CRect nonEditRect;

	Optional<CPoint> sizeRequest;
};

//------------------------------------------------------------------------
} // VSTGUI
