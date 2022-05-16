// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"
#include "cviewcontainer.h"
#include "optional.h"
#include "platform/iplatformframecallback.h"

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

/** Message send to all parents of the new focus view */
extern IdStringPtr kMsgNewFocusView;
/** Message send to all parents of the old focus view */
extern IdStringPtr kMsgOldFocusView;

//-----------------------------------------------------------------------------
// CFrame Declaration
//! @brief The CFrame is the parent container of all views
/// @ingroup containerviews
//-----------------------------------------------------------------------------
class CFrame final : public CViewContainer, public IPlatformFrameCallback
{
public:
	CFrame (const CRect& size, VSTGUIEditorInterface* pEditor);

	//-----------------------------------------------------------------------------
	/// @name CFrame Methods
	//-----------------------------------------------------------------------------
	//@{
	bool open (void* pSystemWindow, PlatformType systemWindowType = PlatformType::kDefaultNative, IPlatformFrameConfig* = nullptr);
	/** closes the frame and calls forget */
	void close ();

	/** set zoom factor */
	bool setZoom (double zoomFactor);
	/** get zoom factor */
	double getZoom () const;

	void setBitmapInterpolationQuality (BitmapInterpolationQuality quality);	///< set interpolation quality for bitmaps
	BitmapInterpolationQuality getBitmapInterpolationQuality () const;			///< get interpolation quality for bitmaps

	double getScaleFactor () const;

	void idle ();

	/** get the current time (in ms) */
	uint64_t getTicks () const;

	/** default knob mode if host does not provide one */
	static int32_t kDefaultKnobMode;
	/** get hosts knob mode */
	int32_t getKnobMode () const;

	bool setPosition (CCoord x, CCoord y);
	bool getPosition (CCoord& x, CCoord& y) const;

	bool setSize (CCoord width, CCoord height);
	bool getSize (CRect* pSize) const;
	bool getSize (CRect& pSize) const;

	VSTGUI_DEPRECATED (
	/** set a modal view. deprecated use beginModalViewSession instead */
	bool setModalView (CView* pView);)
	/** get the currently active modal view or nullptr if there is none */
	CView* getModalView () const;

	/** begin a new modal view session
	 *
	 *	A modal view session is active until endModalViewSession is called and in that time all UI
	 *	events are only dispatched to the modal view or its child views.
	 *	Modal view sessions can be stacked but must be ended in the same order.
	 *
	 *	@param view new modal view (ownership is transfered to frame, the same as addView)
	 *	@return a unique session identifier
	 */
	Optional<ModalViewSessionID> beginModalViewSession (CView* view);
	/** end a modal view session
	 *
	 *	@param session a session identifer
	 *	@return true on success
	 */
	bool endModalViewSession (ModalViewSessionID session);

	void  beginEdit (int32_t index);
	void  endEdit (int32_t index);

	/** get current mouse location */
	bool getCurrentMouseLocation (CPoint& where) const;
	/** get current mouse buttons and key modifiers */
	CButtonState getCurrentMouseButtons () const;
	/** set mouse cursor */
	void setCursor (CCursorType type);

	void   setFocusView (CView* pView);
	CView* getFocusView () const;
	bool advanceNextFocusView (CView* oldFocus, bool reverse = false) override;

	void onViewAdded (CView* pView);
	void onViewRemoved (CView* pView);

	/** called when the platform view/window is activated/deactivated */
	void onActivate (bool state);

	void invalidate (const CRect& rect);

	/** scroll src rect by distance */
	void scrollRect (const CRect& src, const CPoint& distance);

	/** enable or disable tooltips */
	void enableTooltips (bool state, uint32_t delayTimeInMs = 1000);

	/** get animator for this frame */
	Animation::Animator* getAnimator ();

	/** get the clipboard data. data is owned by the caller */
	SharedPointer<IDataPackage> getClipboard ();
	/** set the clipboard data. */
	void setClipboard (const SharedPointer<IDataPackage>& data);

	IViewAddedRemovedObserver* getViewAddedRemovedObserver () const;
	void setViewAddedRemovedObserver (IViewAddedRemovedObserver* observer);

	/** register a keyboard hook */
	void registerKeyboardHook (IKeyboardHook* hook);
	/** unregister a keyboard hook */
	void unregisterKeyboardHook (IKeyboardHook* hook);

	/** register a mouse observer */
	void registerMouseObserver (IMouseObserver* observer);
	/** unregister a mouse observer */
	void unregisterMouseObserver (IMouseObserver* observer);

	void registerScaleFactorChangedListeneer (IScaleFactorChangedListener* listener);
	void unregisterScaleFactorChangedListeneer (IScaleFactorChangedListener* listener);

	void registerFocusViewObserver (IFocusViewObserver* observer);
	void unregisterFocusViewObserver (IFocusViewObserver* observer);
	
	//@}

	//-----------------------------------------------------------------------------
	/// @name Focus Drawing Methods [new in 4.0]
	//! If focus drawing is enabled, the focus view will get a focus ring around it defined with the focus width and the focus color.
	//! Views can define their own shape with the IFocusDrawing interface.
	//-----------------------------------------------------------------------------
	//@{
	/** enable focus drawing */
	void setFocusDrawingEnabled (bool state);
	/** is focus drawing enabled */
	bool focusDrawingEnabled () const;

	/** set focus draw color */
	void setFocusColor (const CColor& color);
	/** get focus draw color */
	CColor getFocusColor () const;

	/** set focus draw width */
	void setFocusWidth (CCoord width);
	/** get focus draw width */
	CCoord getFocusWidth () const;
	//@}

	using EventProcessingFunction = std::function<void ()>;
	/** Queue a function which will be executed after the current event was handled.
	 *	Only allowed when inEventProcessing () is true
	 *
	 *	@param func Function to execute
	 *	@return true if the function was added to the execution queue
	 */
	bool doAfterEventProcessing (EventProcessingFunction&& func);
	/** Queue a function which will be executed after the current event was handled.
	 *	Only allowed when inEventProcessing () is true
	 *
	 *	@param func Function to execute
	 *	@return true if the function was added to the execution queue
	 */
	bool doAfterEventProcessing (const EventProcessingFunction& func);
	/** Returns true if an event is currently being processed. */
	bool inEventProcessing () const;

	void onStartLocalEventLoop ();
	bool performDrag (const DragDescription& desc, const SharedPointer<IDragCallback>& callback);

	void invalid () override { invalidRect (getViewSize ()); setDirty (false); }
	void invalidRect (const CRect& rect) override;

	bool removeView (CView* pView, bool withForget = true) override;
	bool removeAll (bool withForget = true) override;
	CView* getViewAt (const CPoint& where, const GetViewOptions& options = GetViewOptions ()) const override;
	CViewContainer* getContainerAt (const CPoint& where, const GetViewOptions& options = GetViewOptions ().deep ()) const override;
	bool getViewsAt (const CPoint& where, ViewList& views, const GetViewOptions& options = GetViewOptions ().deep ()) const override;
	bool hitTestSubViews (const CPoint& where, const Event& event) override;
	CPoint& frameToLocal (CPoint& point) const override { return point; }
	CPoint& localToFrame (CPoint& point) const override { return point; }

	// CView
	bool attached (CView* parent) override;
	void draw (CDrawContext* pContext) override;
	void drawRect (CDrawContext* pContext, const CRect& updateRect) override;
	void setViewSize (const CRect& rect, bool invalid = true) override;
	void dispatchEvent (Event& event) override;

	VSTGUIEditorInterface* getEditor () const override;
	IPlatformFrame* getPlatformFrame () const;

	#if DEBUG
	void dumpHierarchy () override;
	#endif

	CLASS_METHODS(CFrame, CViewContainer)

	//-------------------------------------------
protected:
	struct CollectInvalidRects;
	
	~CFrame () noexcept override = default;
	void beforeDelete () override;

	void checkMouseViews (const MouseEvent& event);
	void clearMouseViews (const CPoint& where, Modifiers modifiers, bool callMouseExit = true);
	void removeFromMouseViews (CView* view);
	void setCollectInvalidRects (CollectInvalidRects* collectInvalidRects);

	// keyboard hooks
	void dispatchKeyboardEventToHooks (KeyboardEvent& event);

	// mouse observers
	void callMouseObserverMouseEntered (CView* view);
	void callMouseObserverMouseExited (CView* view);
	void callMouseObserverOtherMouseEvent (MouseEvent& event);

	void dispatchNewScaleFactor (double newScaleFactor);

	// platform frame
	bool platformDrawRect (CDrawContext* context, const CRect& rect) override;
	void platformOnEvent (Event& event) override;
	DragOperation platformOnDragEnter (DragEventData data) override;
	DragOperation platformOnDragMove (DragEventData data) override;
	void platformOnDragLeave (DragEventData data) override;
	bool platformOnDrop (DragEventData data) override;
	void platformOnActivate (bool state) override;
	void platformOnWindowActivate (bool state) override;
	void platformScaleFactorChanged (double newScaleFactor) override;
#if VSTGUI_TOUCH_EVENT_HANDLING
	void platformOnTouchEvent (ITouchEvent& event) override;
#endif

private:
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	void endLegacyModalViewSession ();
#endif
	void initModalViewSession (const ModalViewSession& session);
	void clearModalViewSessions ();
	void dispatchKeyboardEvent (KeyboardEvent& event);
	void dispatchMouseEvent (MouseEvent& event);
	void dispatchMouseDownEvent (MouseDownEvent& event);
	void dispatchMouseMoveEvent (MouseMoveEvent& event);
	void dispatchMouseUpEvent (MouseUpEvent& event);
	void dispatchEvent (CView* view, Event& event);
	void dispatchEventToChildren (Event& event);

	struct Impl;
	Impl* pImpl {nullptr};
};

//----------------------------------------------------
class VSTGUIEditorInterface
{
public:
	virtual void doIdleStuff () {}
	virtual int32_t getKnobMode () const { return -1; }
	
	virtual void beginEdit (int32_t index) {}
	virtual void endEdit (int32_t index) {}

	/** frame will change size, if this returns false the upstream implementation does not allow it and thus the size of the frame will not change */
	virtual bool beforeSizeChange (const CRect& newSize, const CRect& oldSize) { return true; }

	virtual CFrame* getFrame () const { return frame; }
protected:
	VSTGUIEditorInterface () = default;
	virtual ~VSTGUIEditorInterface () noexcept = default;

	CFrame* frame {nullptr};
};

//-----------------------------------------------------------------------------
// IMouseObserver Declaration
//! @brief generic mouse observer interface for CFrame
//-----------------------------------------------------------------------------
class IMouseObserver
{
public:
	virtual ~IMouseObserver() noexcept = default;
	virtual void onMouseEntered (CView* view, CFrame* frame) = 0;
	virtual void onMouseExited (CView* view, CFrame* frame) = 0;
	virtual void onMouseEvent (MouseEvent& event, CFrame* frame) = 0;
};

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
class OldMouseObserverAdapter : public IMouseObserver
{
public:
	void onMouseEntered (CView* view, CFrame* frame) override {}
	void onMouseExited (CView* view, CFrame* frame) override {}
	void onMouseEvent (MouseEvent& event, CFrame* frame) override;
	virtual CMouseEventResult onMouseMoved (CFrame* frame, const CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseDown (CFrame* frame, const CPoint& where, const CButtonState& buttons);
};
#endif

//-----------------------------------------------------------------------------
// IKeyboardHook Declaration
//! @brief generic keyboard hook interface for CFrame
//! @ingroup new_in_4_0
//-----------------------------------------------------------------------------
class IKeyboardHook
{
public:
	virtual ~IKeyboardHook () noexcept = default;

	/** the event will not be dispatched further if it is consumed. */
	virtual void onKeyboardEvent (KeyboardEvent& event, CFrame* frame) = 0;
};

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
class OldKeyboardHookAdapter : public IKeyboardHook
{
public:
	virtual int32_t onKeyDown (const VstKeyCode& code, CFrame* frame) = 0;
	virtual int32_t onKeyUp (const VstKeyCode& code, CFrame* frame) = 0;
private:
	void onKeyboardEvent (KeyboardEvent& event, CFrame* frame) override;
};
#endif

//-----------------------------------------------------------------------------
// IViewAddedRemovedObserver Declaration
//! @brief view added removed observer interface for CFrame
//! @ingroup new_in_4_0
//-----------------------------------------------------------------------------
class IViewAddedRemovedObserver
{
public:
	virtual ~IViewAddedRemovedObserver () noexcept = default;
	
	virtual void onViewAdded (CFrame* frame, CView* view) = 0;
	virtual void onViewRemoved (CFrame* frame, CView* view) = 0;
};

//-----------------------------------------------------------------------------
// IFocusViewObserver Declaration
//! @brief focus view observer interface for CFrame
//! @ingroup new_in_4_5
//-----------------------------------------------------------------------------
class IFocusViewObserver
{
public:
	virtual ~IFocusViewObserver () noexcept = default;
	
	virtual void onFocusViewChanged (CFrame* frame, CView* newFocusView, CView* oldFocusView) = 0;
};

} // VSTGUI
