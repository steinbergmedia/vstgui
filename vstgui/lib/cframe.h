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

#ifndef __cframe__
#define __cframe__

#include "vstguifwd.h"
#include "cviewcontainer.h"
#include "platform/iplatformframe.h"
#include <list>

namespace VSTGUI {

//----------------------------
// @brief Knob Mode
//----------------------------
enum CKnobMode
{
	kCircularMode = 0,
	kRelativCircularMode,
	kLinearMode
};

extern IdStringPtr kMsgNewFocusView;			///< Message send to all parents of the new focus view
extern IdStringPtr kMsgOldFocusView;			///< Message send to all parents of the old focus view

//-----------------------------------------------------------------------------
// CFrame Declaration
//! @brief The CFrame is the parent container of all views
/// @ingroup containerviews
//-----------------------------------------------------------------------------
class CFrame : public CViewContainer, public IPlatformFrameCallback 
{
public:
	CFrame (const CRect& size, VSTGUIEditorInterface* pEditor);

	//-----------------------------------------------------------------------------
	/// @name CFrame Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual bool open (void* pSystemWindow, PlatformType systemWindowType = kDefaultNative);
	virtual void close ();							///< closes the frame and calls forget

	bool setZoom (double zoomFactor);				///< set zoom factor

	virtual void idle ();

	virtual uint32_t getTicks () const;				///< get the current time (in ms)

	static int32_t kDefaultKnobMode;				///< default knob mode if host does not provide one
	virtual int32_t getKnobMode () const;			///< get hosts knob mode

	virtual bool setPosition (CCoord x, CCoord y);
	virtual bool getPosition (CCoord& x, CCoord& y) const;

	virtual bool setSize (CCoord width, CCoord height);
	virtual bool getSize (CRect* pSize) const;
	virtual bool getSize (CRect& pSize) const;

	virtual bool   setModalView (CView* pView);
	virtual CView* getModalView () const { return pModalView; }

	virtual void  beginEdit (int32_t index);
	virtual void  endEdit (int32_t index);

	virtual bool getCurrentMouseLocation (CPoint& where) const;				///< get current mouse location
	virtual CButtonState getCurrentMouseButtons () const;					///< get current mouse buttons and key modifiers
	virtual void setCursor (CCursorType type);								///< set mouse cursor

	virtual void   setFocusView (CView* pView);
	virtual CView* getFocusView () const { return pFocusView; }
	virtual bool advanceNextFocusView (CView* oldFocus, bool reverse = false) override;

	virtual void onViewAdded (CView* pView);
	virtual void onViewRemoved (CView* pView);

	virtual void onActivate (bool state);									///< called when the platform view/window is activated/deactivated

	virtual void invalidate (const CRect& rect);

	void scrollRect (const CRect& src, const CPoint& distance);				///< scroll src rect by distance

	void enableTooltips (bool state);										///< enable or disable tooltips

	Animation::Animator* getAnimator ();									///< get animator for this frame

	IDataPackage* getClipboard ();											///< get the clipboard data. data is owned by the caller
	void setClipboard (IDataPackage* data);									///< set the clipboard data.

	virtual IViewAddedRemovedObserver* getViewAddedRemovedObserver () const { return pViewAddedRemovedObserver; }
	virtual void setViewAddedRemovedObserver (IViewAddedRemovedObserver* observer) { pViewAddedRemovedObserver = observer; }

	void registerKeyboardHook (IKeyboardHook* hook);						///< register a keyboard hook
	void unregisterKeyboardHook (IKeyboardHook* hook);						///< unregister a keyboard hook

	void registerMouseObserver (IMouseObserver* observer);					///< register a mouse observer
	void unregisterMouseObserver (IMouseObserver* observer);				///< unregister a mouse observer

	void registerScaleFactorChangedListeneer (IScaleFactorChangedListener* listener);
	void unregisterScaleFactorChangedListeneer (IScaleFactorChangedListener* listener);
	//@}

	//-----------------------------------------------------------------------------
	/// @name Focus Drawing Methods [new in 4.0]
	//! If focus drawing is enabled, the focus view will get a focus ring around it defined with the focus width and the focus color.
	//! Views can define their own shape with the IFocusDrawing interface.
	//-----------------------------------------------------------------------------
	//@{
	virtual void setFocusDrawingEnabled (bool state);				///< enable focus drawing
	virtual bool focusDrawingEnabled () const;						///< is focus drawing enabled

	virtual void setFocusColor (const CColor& color);				///< set focus draw color
	virtual CColor getFocusColor () const;							///< get focus draw color

	virtual void setFocusWidth (CCoord width);						///< set focus draw width
	virtual CCoord getFocusWidth () const;							///< get focus draw width
	//@}

	void onStartLocalEventLoop ();

	void invalid () override { invalidRect (getViewSize ()); setDirty (false); }
	void invalidRect (const CRect& rect) override;

	IPlatformFrame* getPlatformFrame () const { return platformFrame; }

	bool removeView (CView* pView, bool withForget = true) override;
	bool removeAll (bool withForget = true) override;
	CView* getViewAt (const CPoint& where, const GetViewOptions& options = GetViewOptions (GetViewOptions::kNone)) const override;
	CViewContainer* getContainerAt (const CPoint& where, const GetViewOptions& options = GetViewOptions (GetViewOptions::kDeep)) const override;
	bool hitTestSubViews (const CPoint& where, const CButtonState& buttons = -1) override;

	// CView
	virtual bool attached (CView* parent) override;
	void draw (CDrawContext* pContext) override;
	void drawRect (CDrawContext* pContext, const CRect& updateRect) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons) override;
	bool onWheel (const CPoint& where, const float& distance, const CButtonState& buttons) override;
	bool onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons) override;
	int32_t onKeyDown (VstKeyCode& keyCode) override;
	int32_t onKeyUp (VstKeyCode& keyCode) override;
	DragResult doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap) override;
	void setViewSize (const CRect& rect, bool invalid = true) override;

	virtual VSTGUIEditorInterface* getEditor () const override { return pEditor; }

	#if DEBUG
	virtual void dumpHierarchy () override;
	#endif

	CLASS_METHODS(CFrame, CViewContainer)

	//-------------------------------------------
protected:
	~CFrame ();
	void checkMouseViews (const CPoint& where, const CButtonState& buttons);
	void clearMouseViews (const CPoint& where, const CButtonState& buttons, bool callMouseExit = true);
	void removeFromMouseViews (CView* view);

	VSTGUIEditorInterface*		pEditor;
	IViewAddedRemovedObserver*	pViewAddedRemovedObserver;
	CTooltipSupport*			pTooltips;
	Animation::Animator*		pAnimator;

	CView   *pModalView;
	CView   *pFocusView;
	CView   *pActiveFocusView;
	
	typedef std::list<CView*> ViewList;
	ViewList pMouseViews;

	bool	bActive;

	// keyboard hooks
	typedef std::list<IKeyboardHook*> KeyboardHookList;
	KeyboardHookList* pKeyboardHooks;
	int32_t keyboardHooksOnKeyDown (const VstKeyCode& key);
	int32_t keyboardHooksOnKeyUp (const VstKeyCode& key);

	// mouse observers
	typedef std::list<IMouseObserver*> MouseObserverList;
	MouseObserverList* pMouseObservers;
	void callMouseObserverMouseEntered (CView* view);
	void callMouseObserverMouseExited (CView* view);
	CMouseEventResult callMouseObserverMouseDown (const CPoint& where, const CButtonState& buttons);
	CMouseEventResult callMouseObserverMouseMoved (const CPoint& where, const CButtonState& buttons);

	// scale factor changed listener
	typedef std::list<IScaleFactorChangedListener*> ScaleFactorChangedListenerList;
	ScaleFactorChangedListenerList* pScaleFactorChangedListenerList;

	// platform frame
	IPlatformFrame* platformFrame;
	bool platformDrawRect (CDrawContext* context, const CRect& rect) override;
	CMouseEventResult platformOnMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult platformOnMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult platformOnMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult platformOnMouseExited (CPoint& where, const CButtonState& buttons) override;
	bool platformOnMouseWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons) override;
	bool platformOnDrop (IDataPackage* drag, const CPoint& where) override;
	void platformOnDragEnter (IDataPackage* drag, const CPoint& where) override;
	void platformOnDragLeave (IDataPackage* drag, const CPoint& where) override;
	void platformOnDragMove (IDataPackage* drag, const CPoint& where) override;
	bool platformOnKeyDown (VstKeyCode& keyCode) override;
	bool platformOnKeyUp (VstKeyCode& keyCode) override;
	void platformOnActivate (bool state) override;
	void platformScaleFactorChanged () override;
#if VSTGUI_TOUCH_EVENT_HANDLING
	void platformOnTouchEvent (ITouchEvent& event) override;
#endif

	struct CollectInvalidRects : public CBaseObject
	{
		explicit CollectInvalidRects (CFrame* frame);
		~CollectInvalidRects ();
		
		void addRect (const CRect& rect);
		void flush ();
	private:
		SharedPointer<CFrame> frame;
		typedef std::vector<CRect> InvalidRects;
		InvalidRects invalidRects;
		uint32_t lastTicks;
	#if VSTGUI_LOG_COLLECT_INVALID_RECTS
		uint32_t numAddedRects;
	#endif
	};

	void setCollectInvalidRects (CollectInvalidRects* collectInvalidRects);
	CollectInvalidRects* collectInvalidRects;
};

//----------------------------------------------------
class VSTGUIEditorInterface
{
public:
	virtual void doIdleStuff () {}
	virtual int32_t getKnobMode () const { return -1; }
	
	virtual void beginEdit (int32_t index) {}
	virtual void endEdit (int32_t index) {}

	virtual bool beforeSizeChange (const CRect& newSize, const CRect& oldSize) { return true; } ///< frame will change size, if this returns false the upstream implementation does not allow it and thus the size of the frame will not change

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
	virtual CMouseEventResult onMouseMoved (CFrame* frame, const CPoint& where, const CButtonState& buttons) { return kMouseEventNotHandled; }	///< a mouse move event happend on the frame at position where. If the observer handles this, the event won't be propagated further
	virtual CMouseEventResult onMouseDown (CFrame* frame, const CPoint& where, const CButtonState& buttons) { return kMouseEventNotHandled; }	///< a mouse down event happend on the frame at position where. If the observer handles this, the event won't be propagated further
};

//-----------------------------------------------------------------------------
// IKeyboardHook Declaration
//! @brief generic keyboard hook interface for CFrame
//! @ingroup new_in_4_0
//-----------------------------------------------------------------------------
class IKeyboardHook
{
public:
	virtual ~IKeyboardHook () {}
	
	virtual int32_t onKeyDown (const VstKeyCode& code, CFrame* frame) = 0;	///< should return 1 if no further key down processing should apply, otherwise -1
	virtual int32_t onKeyUp (const VstKeyCode& code, CFrame* frame) = 0;	///< should return 1 if no further key up processing should apply, otherwise -1
};

//-----------------------------------------------------------------------------
// IViewAddedRemovedObserver Declaration
//! @brief view added removed observer interface for CFrame
//! @ingroup new_in_4_0
//-----------------------------------------------------------------------------
class IViewAddedRemovedObserver
{
public:
	virtual ~IViewAddedRemovedObserver () {}
	
	virtual void onViewAdded (CFrame* frame, CView* view) = 0;
	virtual void onViewRemoved (CFrame* frame, CView* view) = 0;
};

} // namespace

#endif
