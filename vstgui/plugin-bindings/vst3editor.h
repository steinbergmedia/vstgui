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
	virtual SharedPointer<CView> createCustomView (UTF8StringPtr name,
												   const UIAttributes& attributes,
												   const IUIDescription& description,
												   VST3Editor& editor) = 0;
	/** verify a view after it was created */
	virtual SharedPointer<CView> verifyView (const SharedPointer<CView>& view,
											 const UIAttributes& attributes,
											 const IUIDescription& description,
											 VST3Editor& editor) = 0;
	/** find a parameter */
	virtual bool findParameter (const CPoint& pos, Steinberg::Vst::ParamID& paramID,
								VST3Editor& editor) = 0;
	/** check if parameter ID is private and should not be exposed to the host */
	virtual bool isPrivateParameter (const Steinberg::Vst::ParamID paramID) = 0;
	/** called after the editor was opened */
	virtual void didOpen (VST3Editor& editor) = 0;
	/** called before the editor will close */
	virtual void willClose (VST3Editor& editor) = 0;
	/** create the context menu for the editor, will be added to the host menu */
	virtual SharedPointer<COptionMenu> createContextMenu (const CPoint& pos,
														  VST3Editor& editor) = 0;
	/** called when a sub controller should be created.
	    The controller is now owned by the editor, which will call forget() if it is a CBaseObject,
	   release() if it is a Steinberg::FObject or it will be simply deleted if the frame gets
	   closed. */
	virtual SharedPointer<IController> createSubController (UTF8StringPtr name,
															const IUIDescription& description,
															VST3Editor& editor) = 0;
	/** called when the user zoom factor of the editor was changed */
	virtual void onZoomChanged (VST3Editor& editor, double newZoom) = 0;
};

//------------------------------------------------------------------------
/** Default adapter implementation for IVST3EditorDelegate */
class VST3EditorDelegate : public IVST3EditorDelegate
{
public:
	SharedPointer<CView> createCustomView (UTF8StringPtr name, const UIAttributes& attributes,
										   const IUIDescription& description,
										   VST3Editor& editor) override
	{
		return nullptr;
	}
	SharedPointer<CView> verifyView (const SharedPointer<CView>& view,
									 const UIAttributes& attributes,
									 const IUIDescription& description, VST3Editor& editor) override
	{
		return view;
	}
	bool findParameter (const CPoint& pos, Steinberg::Vst::ParamID& paramID,
						VST3Editor& editor) override
	{
		return false;
	}
	bool isPrivateParameter (const Steinberg::Vst::ParamID paramID) override { return false; }
	void didOpen (VST3Editor& editor) override {}
	void willClose (VST3Editor& editor) override {}
	SharedPointer<COptionMenu> createContextMenu (const CPoint& pos, VST3Editor& editor) override
	{
		return nullptr;
	}
	SharedPointer<IController> createSubController (UTF8StringPtr name,
													const IUIDescription& description,
													VST3Editor& editor) override
	{
		return nullptr;
	}
	void onZoomChanged (VST3Editor& editor, double newZoom) override {}
};

//-----------------------------------------------------------------------------
//! @brief VST3 Editor with automatic parameter binding
//! @ingroup new_in_4_0
//-----------------------------------------------------------------------------
class VST3Editor : public Steinberg::Vst::VSTGUIEditor,
				   public Steinberg::Vst::IParameterFinder,
				   public IViewAddedRemovedObserver,
				   public IMouseObserver
#ifdef VST3_CONTENT_SCALE_SUPPORT
,
				   public Steinberg::IPlugViewContentScaleSupport
#endif
{
public:
	VST3Editor (Steinberg::Vst::EditController* controller, UTF8StringPtr templateName, UTF8StringPtr xmlFile);
	VST3Editor (const SharedPointer<UIDescription>& desc,
				Steinberg::Vst::EditController* controller, UTF8StringPtr templateName,
				UTF8StringPtr xmlFile = nullptr);

	bool exchangeView (UTF8StringPtr templateName);
	void enableTooltips (bool state);

	bool setEditorSizeConstrains (const CPoint& newMinimumSize, const CPoint& newMaximumSize);
	void getEditorSizeConstrains (CPoint& minimumSize, CPoint& maximumSize) const;
	bool requestResize (const CPoint& newSize);

	void setZoomFactor (double factor);
	double getZoomFactor () const;

	void setAllowedZoomFactors (std::vector<double> zoomFactors);

	/** set the delegate of the editor. no reference counting is happening here. */
	void setDelegate (IVST3EditorDelegate* delegate);
	IVST3EditorDelegate* getDelegate () const;
	SharedPointer<UIDescription> getUIDescription () const;

	bool inEditMode () const;
	const std::string& getCurrentTemplateName () const;

	//-----------------------------------------------------------------------------
	DELEGATE_REFCOUNT(Steinberg::Vst::VSTGUIEditor)
	Steinberg::tresult PLUGIN_API queryInterface (const ::Steinberg::TUID iid, void** obj) override;
protected:
	~VST3Editor () override;
	void init ();
	double getAbsScaleFactor () const;
	double getContentScaleFactor () const;
	ParameterChangeListener* getParameterChangeListener (int32_t tag) const;
	void recreateView ();
	void requestRecreateView ();

	void syncParameterTags ();
	void save (bool saveAs = false);
	bool enableEditing (bool state);
	void saveScreenshot ();
	bool enableShowEditButton () const;
	void enableShowEditButton (bool state);
	void showEditButton (bool state);

	bool PLUGIN_API open (void* parent, const PlatformType& type) override;
	void PLUGIN_API close () override;

	void beginEdit (int32_t index) override;
	void endEdit (int32_t index) override;

	bool beforeSizeChange (const CRect& newSize, const CRect& oldSize) override;

	Steinberg::tresult PLUGIN_API onSize (Steinberg::ViewRect* newSize) override;
	Steinberg::tresult PLUGIN_API canResize () override;
	Steinberg::tresult PLUGIN_API checkSizeConstraint (Steinberg::ViewRect* rect) override;

	// IParameterFinder
	Steinberg::tresult PLUGIN_API findParameter (Steinberg::int32 xPos, Steinberg::int32 yPos, Steinberg::Vst::ParamID& resultTag) override;

	// IViewAddedRemovedObserver
	void onViewAdded (CFrame& frame, CView& view) override;
	void onViewRemoved (CFrame& frame, CView& view) override;

	// IMouseObserver
	void onMouseEntered (CView& view, CFrame& frame) override {}
	void onMouseExited (CView& view, CFrame& frame) override {}
	void onMouseEvent (MouseEvent& event, CFrame& frame) override;

#ifdef VST3_CONTENT_SCALE_SUPPORT
	Steinberg::tresult PLUGIN_API setContentScaleFactor (ScaleFactor factor) override;
#endif

private:
	struct Controller;
	struct Impl;
	std::unique_ptr<Impl> pImpl;
};

//-----------------------------------------------------------------------------
//! @brief An extended VST3 Editor which scales its contents when resized
//! @ingroup new_in_4_14
//-----------------------------------------------------------------------------
class AspectRatioVST3Editor : public VST3Editor
{
public:
	using VST3Editor::VST3Editor;

	void setMinZoomFactor (double factor);
	double getMinZoomFactor () const;

protected:
	bool canCalculateAspectRatio () const;

	bool PLUGIN_API open (void* parent, const PlatformType& type) override;
	Steinberg::tresult PLUGIN_API onSize (Steinberg::ViewRect* newSize) override;
	Steinberg::tresult PLUGIN_API checkSizeConstraint (Steinberg::ViewRect* rect) override;
#ifdef VST3_CONTENT_SCALE_SUPPORT
	Steinberg::tresult PLUGIN_API setContentScaleFactor (ScaleFactor factor) override;
#endif

private:
	CPoint initialSize {};
	double minZoomFactor {1.};
	double calcZoomFactor {1.};
};

//------------------------------------------------------------------------
} // VSTGUI
