// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../lib/vstguifwd.h"
#include "../uidescription/uidescriptionfwd.h"
#include <cstdint>

//------------------------------------------------------------------------
namespace VSTGUI {
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
	virtual bool findParameter (const CPoint& pos, uint32_t& paramID, VST3Editor& editor) = 0;
	/** check if parameter ID is private and should not be exposed to the host */
	virtual bool isPrivateParameter (const uint32_t paramID) = 0;
	/** called after the editor was opened */
	virtual void didOpen (VST3Editor& editor) = 0;
	/** called before the editor will close */
	virtual void willClose (VST3Editor& editor) = 0;
	/** create the context menu for the editor, will be added to the host menu */
	virtual SharedPointer<COptionMenu> createContextMenu (const CPoint& pos,
														  VST3Editor& editor) = 0;
	/** Called when a subcontroller should be created.
		The editor owns the controller and will delete it when the frame closes. */
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
	bool findParameter (const CPoint& pos, uint32_t& paramID, VST3Editor& editor) override
	{
		return false;
	}
	bool isPrivateParameter (const uint32_t paramID) override { return false; }
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

//------------------------------------------------------------------------
} // VSTGUI
