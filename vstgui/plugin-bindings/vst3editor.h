// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __vst3editor__
#define __vst3editor__

#include "public.sdk/source/vst/vstguieditor.h"
#include "pluginterfaces/vst/ivstplugview.h"
#include "../uidescription/uidescription.h"
#include "../uidescription/icontroller.h"
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
                   public IMouseObserver
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
	Steinberg::tresult PLUGIN_API queryInterface (const ::Steinberg::TUID iid, void** obj) override;
protected:
	~VST3Editor ();
	void init ();
	double getAbsScaleFactor () const;
	ParameterChangeListener* getParameterChangeListener (int32_t tag) const;
	void recreateView ();

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

	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;

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
	CMouseEventResult onMouseMoved (CFrame* frame, const CPoint& where, const CButtonState& buttons) override { return kMouseEventNotHandled; }
	CMouseEventResult onMouseDown (CFrame* frame, const CPoint& where, const CButtonState& buttons) override;

#ifdef VST3_CONTENT_SCALE_SUPPORT
	Steinberg::tresult PLUGIN_API setContentScaleFactor (ScaleFactor factor) override;
#endif

	struct KeyboardHook;
	KeyboardHook* keyboardHook {nullptr};
	UIDescription* description {nullptr};
	VST3EditorDelegate* delegate {nullptr};
	IController* originalController {nullptr};
	typedef std::map<int32_t, ParameterChangeListener*> ParameterChangeListenerMap;
	ParameterChangeListenerMap paramChangeListeners;
	std::string viewName;
	std::string xmlFile;
	bool tooltipsEnabled {true};
	bool doCreateView {false};
	bool editingEnabled {false};
	bool requestResizeGuard {false};

	double contentScaleFactor {1.};
	double zoomFactor {1.};
	std::vector<double> allowedZoomFactors;
	
	CPoint minSize;
	CPoint maxSize;
	CRect nonEditRect;
};

} // namespace

#endif
