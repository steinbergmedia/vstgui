//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2008, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#ifndef __cframe__
#define __cframe__

#include "cviewcontainer.h"

#if MAC_CARBON
	#include <Carbon/Carbon.h>
#endif // MAC_CARBON

#if WINDOWS
	#include <windows.h>

	#if GDIPLUS
	#include <objidl.h>
	#include <gdiplus.h>
	#endif
#endif // WINDOWS


BEGIN_NAMESPACE_VSTGUI
class VSTGUIEditorInterface;
class IMouseObserver;
class IKeyboardHook;

//----------------------------
// @brief Cursor Type
//----------------------------
enum CCursorType
{
	kCursorDefault = 0,				///< arrow cursor
	kCursorWait,					///< wait cursor
	kCursorHSize,					///< horizontal size cursor
	kCursorVSize,					///< vertical size cursor
	kCursorSizeAll,					///< size all cursor
	kCursorNESWSize,				///< northeast and southwest size cursor
	kCursorNWSESize,				///< northwest and southeast size cursor
	kCursorCopy,					///< copy cursor (mainly for drag&drop operations)
	kCursorNotAllowed,				///< not allowed cursor (mainly for drag&drop operations)
	kCursorHand						///< hand cursor
};

//----------------------------
// @brief Knob Mode
//----------------------------
enum CKnobMode
{
	kCircularMode = 0,
	kRelativCircularMode,
	kLinearMode
};

extern const char* kMsgNewFocusView;			///< Message send to all parents of the new focus view
extern const char* kMsgOldFocusView;			///< Message send to all parents of the old focus view

//-----------------------------------------------------------------------------
// CFrame Declaration
//! @brief The CFrame is the parent container of all views
/// @ingroup containerviews
//-----------------------------------------------------------------------------
class CFrame : public CViewContainer
{
public:
	CFrame (const CRect &size, void *pSystemWindow, VSTGUIEditorInterface *pEditor);

	//-----------------------------------------------------------------------------
	/// @name CFrame Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void idle ();
	virtual void doIdleStuff ();

	virtual unsigned long getTicks () const;	///< get the current time (in ms)
	virtual long getKnobMode () const;			///< get hosts knob mode

	virtual bool setPosition (CCoord x, CCoord y);
	virtual bool getPosition (CCoord &x, CCoord &y) const;

	virtual bool setSize (CCoord width, CCoord height);
	virtual bool getSize (CRect *pSize) const;
	virtual bool getSize (CRect &pSize) const;

	virtual bool   setModalView (CView *pView);
	virtual CView *getModalView () const { return pModalView; }

	virtual void  beginEdit (long index);
	virtual void  endEdit (long index);

	virtual bool getCurrentMouseLocation (CPoint &where) const;				///< get current mouse location
	virtual long getCurrentMouseButtons () const;							///< get current mouse buttons and key modifiers
	virtual void setCursor (CCursorType type);								///< set mouse cursor

	virtual void   setFocusView (CView *pView);
	virtual CView *getFocusView () const { return pFocusView; }
	virtual bool advanceNextFocusView (CView* oldFocus, bool reverse = false);

	virtual void onViewAdded (CView* pView);
	virtual void onViewRemoved (CView* pView);

	virtual void onActivate (bool state);									///< called when the platform view/window is activated/deactivated

	virtual bool setDropActive (bool val);
	virtual bool isDropActive () const { return bDropActive; };

	VSTGUI_DEPRECATED(CDrawContext* createDrawContext ();)

	virtual void invalidate (const CRect &rect);

	void scrollRect (const CRect& src, const CPoint& distance);		///< scroll src rect by distance
	//@}

	//-----------------------------------------------------------------------------
	/// @name Focus Drawing Methods [new in 4.0]
	//! If focus drawing is enabled, the focus view will get a focus ring around it defined with the focus width and the focus color.
	//! Views can define their own shape with the IFocusDrawing interface. Works only if CGraphicsPath is supported.
	//-----------------------------------------------------------------------------
	//@{
	virtual void setFocusDrawingEnabled (bool state);				///< enable focus drawing
	virtual bool focusDrawingEnabled () const;						///< is focus drawing enabled

	virtual void setFocusColor (const CColor& color);				///< set focus draw color
	virtual CColor getFocusColor () const;							///< get focus draw color

	virtual void setFocusWidth (CCoord width);						///< set focus draw width
	virtual CCoord getFocusWidth () const;							///< get focus draw width
	//@}

	void invalid () { invalidRect (size); bDirty = false; }
	void invalidRect (const CRect rect);

	#if MAC_COCOA && MAC_CARBON
	static void setCocoaMode (bool state);
	#endif

	#if WINDOWS
	HWND getOuterWindow () const;
	void *getParentSystemWindow () const { return pSystemWindow; }
	void setParentSystemWindow (void *val) { pSystemWindow = val; }
	COffscreenContext* getBackBuffer ();
	#endif // WINDOWS
	
	void *getSystemWindow () const;	///< get platform window
	
	bool removeView (CView *pView, const bool &withForget = true);
	bool removeAll (const bool &withForget = true);

	// CView
	void draw (CDrawContext *pContext);
	void drawRect (CDrawContext *pContext, const CRect& updateRect);
	CMouseEventResult onMouseDown (CPoint &where, const long& buttons);
	CMouseEventResult onMouseUp (CPoint &where, const long& buttons);
	CMouseEventResult onMouseMoved (CPoint &where, const long& buttons);
	CMouseEventResult onMouseExited (CPoint &where, const long& buttons);
	bool onWheel (const CPoint &where, const float &distance, const long &buttons);
	bool onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const long &buttons);
	long onKeyDown (VstKeyCode& keyCode);
	long onKeyUp (VstKeyCode& keyCode);
	void setViewSize (CRect& rect, bool invalid = true);

	virtual VSTGUIEditorInterface* getEditor () const { return pEditor; }
	virtual IMouseObserver* getMouseObserver () const { return pMouseObserver; }
	virtual void setMouseObserver (IMouseObserver* observer) { pMouseObserver = observer; }
	virtual IKeyboardHook* getKeyboardHook () const { return pKeyboardHook; }
	virtual void setKeyboardHook (IKeyboardHook* hook) { pKeyboardHook = hook; }

	#if DEBUG
	virtual void dumpHierarchy ();
	#endif

	CLASS_METHODS(CFrame, CViewContainer)

	//-------------------------------------------
protected:
	~CFrame ();
	bool   initFrame (void *pSystemWin);

	VSTGUIEditorInterface   *pEditor;
	IMouseObserver			*pMouseObserver;
	IKeyboardHook			*pKeyboardHook;
	
	void    *pSystemWindow;
	CView   *pModalView;
	CView   *pFocusView;
	CView   *pActiveFocusView;
	CView   *pMouseOverView;

	bool    bDropActive;
	bool	bActive;

#if WINDOWS
	void      *pHwnd;
	HINSTANCE hInstMsimg32dll;
	void*     dropTarget;
	COffscreenContext* backBuffer;
	bool      bMouseInside;
#endif // WINDOWS

#if MAC_CARBON
	static pascal OSStatus carbonMouseEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
	static pascal OSStatus carbonEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
	
	HIViewRef controlRef;
	bool hasFocus;
	EventHandlerRef dragEventHandler;
	EventHandlerRef mouseEventHandler;
	public:
	void* getPlatformControl () const { return controlRef; }
	CPoint hiScrollOffset;
	protected:
#endif // MAC_CARBON

#if MAC_COCOA
	void* nsView;
	public:
	void* getNSView () const { return nsView; }
	protected:
	
#endif // MAC_COCOA
	//-------------------------------------------
private:
	void     *defaultCursor;
};

//----------------------------------------------------
class VSTGUIEditorInterface
{
public:
	virtual void doIdleStuff () {}
	virtual long getKnobMode () const { return 0; }
	
	virtual void beginEdit (long index) {}
	virtual void endEdit (long index) {}

	virtual CFrame* getFrame () const { return frame; }
protected:
	VSTGUIEditorInterface () : frame (0) {}
	virtual ~VSTGUIEditorInterface () {}

	CFrame* frame;
};

//-----------------------------------------------------------------------------
// IMouseObserver Declaration
//! @brief generic mouse observer interface for CFrame
//-----------------------------------------------------------------------------
class IMouseObserver
{
public:
	virtual ~IMouseObserver() {}
	virtual void onMouseEntered (CView* view, CFrame* frame) = 0;
	virtual void onMouseExited (CView* view, CFrame* frame) = 0;
	virtual void onMouseMoved (CFrame* frame, const CPoint& where) {}
	virtual void onMouseDown (CFrame* frame, const CPoint& where) {}
};

//-----------------------------------------------------------------------------
// IKeyboardHook Declaration
//! @brief generic keyboard hook interface for CFrame [new since 4.0]
//-----------------------------------------------------------------------------
class IKeyboardHook
{
public:
	virtual ~IKeyboardHook () {}
	
	virtual long onKeyDown (const VstKeyCode& code, CFrame* frame) = 0;	///< should return 1 if no further key down processing should apply, otherwise -1
	virtual long onKeyUp (const VstKeyCode& code, CFrame* frame) = 0;	///< should return 1 if no further key up processing should apply, otherwise -1
};

END_NAMESPACE_VSTGUI

#endif
