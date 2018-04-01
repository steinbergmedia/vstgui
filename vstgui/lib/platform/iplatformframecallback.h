// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __iplatformframecallback__
#define __iplatformframecallback__

/// @cond ignore

#include "../vstguifwd.h"

struct VstKeyCode;

namespace VSTGUI {

//-----------------------------------------------------------------------------
enum PlatformType {
	kHWND,		// Windows HWND
	kWindowRef,	// macOS WindowRef (Carbon)
	kNSView,	// macOS NSView
	kUIView,	// iOS UIView
	kHWNDTopLevel,	// Windows HWDN Top Level (non child)
	kX11EmbedWindowID,	// X11 XID

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

	virtual DragOperation platformOnDragEnter (IDataPackage* dragData, CPoint pos, CButtonState buttons) = 0;
	virtual DragOperation platformOnDragMove (IDataPackage* dragData, CPoint pos, CButtonState buttons) = 0;
	virtual void platformOnDragLeave (IDataPackage* dragData, CPoint pos, CButtonState buttons) = 0;
	virtual bool platformOnDrop (IDataPackage* dragData, CPoint pos, CButtonState buttons) = 0;

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
} // namespace

/// @endcond

#endif // __iplatformframecallback__
