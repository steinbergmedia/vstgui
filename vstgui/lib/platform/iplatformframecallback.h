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
	virtual ~IPlatformFrameCallback () = default;

	virtual void platformDrawRects (const PlatformGraphicsDeviceContextPtr& context,
									double scaleFactor, const std::vector<CRect>& rects) = 0;
	
	virtual void platformOnEvent (Event& event) = 0;

	virtual DragOperation platformOnDragEnter (DragEventData data) = 0;
	virtual DragOperation platformOnDragMove (DragEventData data) = 0;
	virtual void platformOnDragLeave (DragEventData data) = 0;
	virtual bool platformOnDrop (DragEventData data) = 0;

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
