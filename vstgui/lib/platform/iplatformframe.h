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

#ifndef __iplatformframe__
#define __iplatformframe__

/// @cond ignore

#include "../vstguifwd.h"

struct VstKeyCode;

namespace VSTGUI {
class IPlatformTextEdit;
class IPlatformTextEditCallback;
class IPlatformOptionMenu;
class IPlatformOpenGLView;
class IPlatformViewLayer;
class IPlatformViewLayerDelegate;

enum PlatformType {
	kHWND,		// Windows HWND
	kWindowRef,	// macOS WindowRef (Carbon)
	kNSView,	// macOS NSView
	kUIView,	// iOS UIView
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

	virtual bool platformOnDrop (IDataPackage* drag, const CPoint& where) = 0;
	virtual void platformOnDragEnter (IDataPackage* drag, const CPoint& where) = 0;
	virtual void platformOnDragLeave (IDataPackage* drag, const CPoint& where) = 0;
	virtual void platformOnDragMove (IDataPackage* drag, const CPoint& where) = 0;

	virtual bool platformOnKeyDown (VstKeyCode& keyCode) = 0;
	virtual bool platformOnKeyUp (VstKeyCode& keyCode) = 0;

	virtual void platformOnActivate (bool state) = 0;

	virtual void platformScaleFactorChanged () = 0;

#if VSTGUI_TOUCH_EVENT_HANDLING
	virtual void platformOnTouchEvent (ITouchEvent& event) = 0;
#endif
//------------------------------------------------------------------------------------
};

//-----------------------------------------------------------------------------
class IPlatformFrame : public CBaseObject
{
public:
	static IPlatformFrame* createPlatformFrame (IPlatformFrameCallback* frame, const CRect& size, void* parent, PlatformType parentType);	///< create platform representation
	static uint32_t getTicks ();

	virtual bool getGlobalPosition (CPoint& pos) const = 0;	///< get the top left position in global coordinates
	virtual bool setSize (const CRect& newSize) = 0;	///< set size of platform representation relative to parent
	virtual bool getSize (CRect& size) const = 0;	///< get size of platform representation relative to parent
	
	virtual bool getCurrentMousePosition (CPoint& mousePosition) const = 0;	///< get current mouse position out of event stream
	virtual bool getCurrentMouseButtons (CButtonState& buttons) const = 0;	///< get current mouse buttons out of event stream
	virtual bool setMouseCursor (CCursorType type) = 0;	///< set mouse cursor shape
	
	virtual bool invalidRect (const CRect& rect) = 0;	///< invalidates rect in platform representation
	virtual bool scrollRect (const CRect& src, const CPoint& distance) = 0; ///< blit scroll the src rect by distance, return false if not supported

	virtual bool showTooltip (const CRect& rect, const char* utf8Text) = 0; ///< show tooltip
	virtual bool hideTooltip () = 0;	///< hide tooltip

	virtual void* getPlatformRepresentation () const = 0;	// TODO: remove this call later when everything is done

	virtual IPlatformTextEdit* createPlatformTextEdit (IPlatformTextEditCallback* textEdit) = 0; ///< create a native text edit control
	virtual IPlatformOptionMenu* createPlatformOptionMenu () = 0; ///< create a native popup menu
#if VSTGUI_OPENGL_SUPPORT
	virtual IPlatformOpenGLView* createPlatformOpenGLView () = 0; ///< create a native opengl sub view
#endif // VSTGUI_OPENGL_SUPPORT
	
	virtual IPlatformViewLayer* createPlatformViewLayer (IPlatformViewLayerDelegate* drawDelegate, IPlatformViewLayer* parentLayer = 0) = 0; ///< create a native view layer, may return 0 if not supported
	
	virtual COffscreenContext* createOffscreenContext (CCoord width, CCoord height, double scaleFactor = 1.) = 0; ///< create an offscreen draw device

	virtual DragResult doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap) = 0; ///< start a drag operation

	virtual void setClipboard (IDataPackage* data) = 0;	///< set clipboard data
	virtual IDataPackage* getClipboard () = 0;			///< get clipboard data

//-----------------------------------------------------------------------------
protected:
	IPlatformFrame (IPlatformFrameCallback* frame) : frame (frame) {}
	IPlatformFrameCallback* frame;
};

//-----------------------------------------------------------------------------
class IPlatformFrameRunLoopExt
{
public:
	virtual ~IPlatformFrameRunLoopExt () {};

	virtual void handleNextEvents() = 0;
};

} // namespace

/// @endcond

#endif // __iplatformframe__
