// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "ivst3editordelegate.h"
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

//-----------------------------------------------------------------------------
//! @brief VST3 Editor with automatic parameter binding
//! @ingroup new_in_4_0
//-----------------------------------------------------------------------------
class VST3Editor : public Steinberg::Vst::VSTGUIEditor,
				   public Steinberg::Vst::IParameterFinder
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
