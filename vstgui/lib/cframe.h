// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __cframe__
#define __cframe__

#include "vstguifwd.h"
#include "cviewcontainer.h"
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

extern IdStringPtr kMsgNewFocusView;			///< Message send to all parents of the new focus view
extern IdStringPtr kMsgOldFocusView;			///< Message send to all parents of the old focus view

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
	bool open (void* pSystemWindow, PlatformType systemWindowType = kDefaultNative, IPlatformFrameConfig* = nullptr);
	void close ();							///< closes the frame and calls forget

	bool setZoom (double zoomFactor);				///< set zoom factor

	double getScaleFactor () const;

	void idle ();

	uint32_t getTicks () const;				///< get the current time (in ms)

	static int32_t kDefaultKnobMode;				///< default knob mode if host does not provide one
	int32_t getKnobMode () const;			///< get hosts knob mode

	bool setPosition (CCoord x, CCoord y);
	bool getPosition (CCoord& x, CCoord& y) const;

	bool setSize (CCoord width, CCoord height);
	bool getSize (CRect* pSize) const;
	bool getSize (CRect& pSize) const;

	bool   setModalView (CView* pView);
	CView* getModalView () const;

	void  beginEdit (int32_t index);
	void  endEdit (int32_t index);

	bool getCurrentMouseLocation (CPoint& where) const;				///< get current mouse location
	CButtonState getCurrentMouseButtons () const;					///< get current mouse buttons and key modifiers
	void setCursor (CCursorType type);								///< set mouse cursor

	void   setFocusView (CView* pView);
	CView* getFocusView () const;
	bool advanceNextFocusView (CView* oldFocus, bool reverse = false) override;

	void onViewAdded (CView* pView);
	void onViewRemoved (CView* pView);

	void onActivate (bool state);									///< called when the platform view/window is activated/deactivated

	void invalidate (const CRect& rect);

	void scrollRect (const CRect& src, const CPoint& distance);				///< scroll src rect by distance

	void enableTooltips (bool state);										///< enable or disable tooltips

	Animation::Animator* getAnimator ();									///< get animator for this frame

	SharedPointer<IDataPackage> getClipboard ();							///< get the clipboard data. data is owned by the caller
	void setClipboard (const SharedPointer<IDataPackage>& data);			///< set the clipboard data.

	IViewAddedRemovedObserver* getViewAddedRemovedObserver () const;
	void setViewAddedRemovedObserver (IViewAddedRemovedObserver* observer);

	void registerKeyboardHook (IKeyboardHook* hook);						///< register a keyboard hook
	void unregisterKeyboardHook (IKeyboardHook* hook);						///< unregister a keyboard hook

	void registerMouseObserver (IMouseObserver* observer);					///< register a mouse observer
	void unregisterMouseObserver (IMouseObserver* observer);				///< unregister a mouse observer

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
	void setFocusDrawingEnabled (bool state);				///< enable focus drawing
	bool focusDrawingEnabled () const;						///< is focus drawing enabled

	void setFocusColor (const CColor& color);				///< set focus draw color
	CColor getFocusColor () const;							///< get focus draw color

	void setFocusWidth (CCoord width);						///< set focus draw width
	CCoord getFocusWidth () const;							///< get focus draw width
	//@}

	using Function = std::function<void ()>;
	/** Queue a function which will be executed after the current event was handled.
	 *	Only allowed when inEventProcessing () is true
	 *
	 *	@param func Function to execute
	 *	@return true if the function was added to the execution queue
	 */
	bool doAfterEventProcessing (Function&& func);
	/** Queue a function which will be executed after the current event was handled.
	 *	Only allowed when inEventProcessing () is true
	 *
	 *	@param func Function to execute
	 *	@return true if the function was added to the execution queue
	 */
	bool doAfterEventProcessing (const Function& func);
	/** Returns true if an event is currently being processed. */
	bool inEventProcessing () const;

	void onStartLocalEventLoop ();

	void invalid () override { invalidRect (getViewSize ()); setDirty (false); }
	void invalidRect (const CRect& rect) override;

	bool removeView (CView* pView, bool withForget = true) override;
	bool removeAll (bool withForget = true) override;
	CView* getViewAt (const CPoint& where, const GetViewOptions& options = GetViewOptions (GetViewOptions::kNone)) const override;
	CViewContainer* getContainerAt (const CPoint& where, const GetViewOptions& options = GetViewOptions (GetViewOptions::kDeep)) const override;
	bool hitTestSubViews (const CPoint& where, const CButtonState& buttons = -1) override;

	// CView
	bool attached (CView* parent) override;
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
	
	void checkMouseViews (const CPoint& where, const CButtonState& buttons);
	void clearMouseViews (const CPoint& where, const CButtonState& buttons, bool callMouseExit = true);
	void removeFromMouseViews (CView* view);
	void setCollectInvalidRects (CollectInvalidRects* collectInvalidRects);

	// keyboard hooks
	int32_t keyboardHooksOnKeyDown (const VstKeyCode& key);
	int32_t keyboardHooksOnKeyUp (const VstKeyCode& key);

	// mouse observers
	void callMouseObserverMouseEntered (CView* view);
	void callMouseObserverMouseExited (CView* view);
	CMouseEventResult callMouseObserverMouseDown (const CPoint& where, const CButtonState& buttons);
	CMouseEventResult callMouseObserverMouseMoved (const CPoint& where, const CButtonState& buttons);

	void dispatchNewScaleFactor (double newScaleFactor);

	// platform frame
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
	void platformOnWindowActivate (bool state) override;
	void platformScaleFactorChanged (double newScaleFactor) override;
#if VSTGUI_TOUCH_EVENT_HANDLING
	void platformOnTouchEvent (ITouchEvent& event) override;
#endif

private:
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

	virtual bool beforeSizeChange (const CRect& newSize, const CRect& oldSize) { return true; } ///< frame will change size, if this returns false the upstream implementation does not allow it and thus the size of the frame will not change

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
	virtual ~IKeyboardHook () noexcept = default;
	
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

} // namespace

#endif
