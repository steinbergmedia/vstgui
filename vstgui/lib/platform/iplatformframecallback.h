// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

/// @cond ignore

#include "../vstguifwd.h"

struct VstKeyCode;

namespace VSTGUI {

//-----------------------------------------------------------------------------
enum class PlatformType : int32_t {
	kHWND,		// Windows HWND
	kWindowRef,	// macOS WindowRef (Carbon)
	kNSView,	// macOS NSView
	kUIView,	// iOS UIView
	kHWNDTopLevel,	// Windows HWDN Top Level (non child)
	kX11EmbedWindowID,	// X11 XID
	kGdkWindow, // GdkWindow

	kDefaultNative = -1
};

//-----------------------------------------------------------------------------
// Callback interface from IPlatformFrame implementations
//-----------------------------------------------------------------------------
class IPlatformFrameCallback
{
public:
	virtual bool platformDrawRect (CDrawContext* context, const CRect& rect) = 0;
	
	virtual CMouseEventResult platformOnMouseDown (CPoint& where, const CButtonState& buttons) = 0;
	virtual CMouseEventResult platformOnMouseMoved (CPoint& where, const CButtonState& buttons) = 0;
	virtual CMouseEventResult platformOnMouseUp (CPoint& where, const CButtonState& buttons) = 0;
	virtual CMouseEventResult platformOnMouseExited (CPoint& where, const CButtonState& buttons) = 0;
	virtual bool platformOnMouseWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const CButtonState &buttons) = 0;

	virtual DragOperation platformOnDragEnter (DragEventData data) = 0;
	virtual DragOperation platformOnDragMove (DragEventData data) = 0;
	virtual void platformOnDragLeave (DragEventData data) = 0;
	virtual bool platformOnDrop (DragEventData data) = 0;

	virtual bool platformOnKeyDown (VstKeyCode& keyCode) = 0;
	virtual bool platformOnKeyUp (VstKeyCode& keyCode) = 0;

	virtual void platformOnActivate (bool state) = 0;
	virtual void platformOnWindowActivate (bool state) = 0;
	
	virtual void platformScaleFactorChanged (double newScaleFactor) = 0;

#if VSTGUI_TOUCH_EVENT_HANDLING
	virtual void platformOnTouchEvent (ITouchEvent& event) = 0;
#endif
//------------------------------------------------------------------------------------
};

//------------------------------------------------------------------------------------
class IPlatformFrameConfig
{
public:
	virtual ~IPlatformFrameConfig () noexcept = default;
};

//------------------------------------------------------------------------------------
} // VSTGUI

/// @endcond
