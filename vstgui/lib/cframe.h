//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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
#include "platform/iplatformframe.h"
#include <list>

namespace VSTGUI {
class VSTGUIEditorInterface;
class IMouseObserver;
class IKeyboardHook;
class IViewAddedRemovedObserver;
class CTooltipSupport;
namespace Animation {
	class Animator;
}

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
	CFrame (const CRect &size, void *pSystemWindow, VSTGUIEditorInterface *pEditor);

	//-----------------------------------------------------------------------------
	/// @name CFrame Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void close ();						///< closes the frame and calls forget

	virtual void idle ();
	virtual void doIdleStuff ();

	virtual uint32_t getTicks () const;	///< get the current time (in ms)
	virtual int32_t getKnobMode () const;			///< get hosts knob mode

	virtual bool setPosition (CCoord x, CCoord y);
	virtual bool getPosition (CCoord &x, CCoord &y) const;

	virtual bool setSize (CCoord width, CCoord height);
	virtual bool getSize (CRect *pSize) const;
	virtual bool getSize (CRect &pSize) const;

	virtual bool   setModalView (CView *pView);
	virtual CView *getModalView () const { return pModalView; }

	virtual void  beginEdit (int32_t index);
	virtual void  endEdit (int32_t index);

	virtual bool getCurrentMouseLocation (CPoint &where) const;				///< get current mouse location
	virtual CButtonState getCurrentMouseButtons () const;							///< get current mouse buttons and key modifiers
	virtual void setCursor (CCursorType type);								///< set mouse cursor

	virtual void   setFocusView (CView *pView);
	virtual CView *getFocusView () const { return pFocusView; }
	virtual bool advanceNextFocusView (CView* oldFocus, bool reverse = false);

	virtual void onViewAdded (CView* pView);
	virtual void onViewRemoved (CView* pView);

	virtual void onActivate (bool state);									///< called when the platform view/window is activated/deactivated

	VSTGUI_DEPRECATED(CDrawContext* createDrawContext ();)

	virtual void invalidate (const CRect &rect);

	void scrollRect (const CRect& src, const CPoint& distance);				///< scroll src rect by distance

	void enableTooltips (bool state);										///< enable or disable tooltips

	Animation::Animator* getAnimator ();									///< get animator for this frame
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

	void invalid () { invalidRect (size); bDirty = false; }
	void invalidRect (const CRect& rect);

	#if MAC_COCOA && MAC_CARBON
	static void setCocoaMode (bool state);
	static bool getCocoaMode ();
	#endif

	IPlatformFrame* getPlatformFrame () const { return platformFrame; }

	bool removeView (CView *pView, const bool &withForget = true);
	bool removeAll (const bool &withForget = true);
	CView* getViewAt (const CPoint& where, bool deep = false) const;
	CViewContainer* getContainerAt (const CPoint& where, bool deep = true) const;

	// CView
	void draw (CDrawContext *pContext);
	void drawRect (CDrawContext *pContext, const CRect& updateRect);
	CMouseEventResult onMouseDown (CPoint &where, const CButtonState& buttons);
	CMouseEventResult onMouseUp (CPoint &where, const CButtonState& buttons);
	CMouseEventResult onMouseMoved (CPoint &where, const CButtonState& buttons);
	CMouseEventResult onMouseExited (CPoint &where, const CButtonState& buttons);
	bool onWheel (const CPoint &where, const float &distance, const CButtonState &buttons);
	bool onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const CButtonState &buttons);
	int32_t onKeyDown (VstKeyCode& keyCode);
	int32_t onKeyUp (VstKeyCode& keyCode);
	DragResult doDrag (CDropSource* source, const CPoint& offset, CBitmap* dragBitmap);
	void setViewSize (const CRect& rect, bool invalid = true);

	virtual VSTGUIEditorInterface* getEditor () const { return pEditor; }
	virtual IMouseObserver* getMouseObserver () const { return pMouseObserver; }
	virtual void setMouseObserver (IMouseObserver* observer) { pMouseObserver = observer; }
	virtual IKeyboardHook* getKeyboardHook () const { return pKeyboardHook; }
	virtual void setKeyboardHook (IKeyboardHook* hook) { pKeyboardHook = hook; }
	virtual IViewAddedRemovedObserver* getViewAddedRemovedObserver () const { return pViewAddedRemovedObserver; }
	virtual void setViewAddedRemovedObserver (IViewAddedRemovedObserver* observer) { pViewAddedRemovedObserver = observer; }

	#if DEBUG
	virtual void dumpHierarchy ();
	#endif

	CLASS_METHODS(CFrame, CViewContainer)

	//-------------------------------------------
protected:
	~CFrame ();
	bool initFrame (void *pSystemWin);
	void checkMouseViews (const CPoint& where, const CButtonState& buttons);
	void clearMouseViews (const CPoint& where, const CButtonState& buttons, bool callMouseExit = true);
	void removeFromMouseViews (CView* view);

	VSTGUIEditorInterface*		pEditor;
	IMouseObserver*				pMouseObserver;
	IKeyboardHook*				pKeyboardHook;
	IViewAddedRemovedObserver*	pViewAddedRemovedObserver;
	CTooltipSupport*			pTooltips;
	Animation::Animator*		pAnimator;

	CView   *pModalView;
	CView   *pFocusView;
	CView   *pActiveFocusView;
	std::list<CView*> pMouseViews;

	bool	bActive;

	IPlatformFrame* platformFrame;

	// IPlatformFrameCallback
	bool platformDrawRect (CDrawContext* context, const CRect& rect);
	CMouseEventResult platformOnMouseDown (CPoint& where, const CButtonState& buttons);
	CMouseEventResult platformOnMouseMoved (CPoint& where, const CButtonState& buttons);
	CMouseEventResult platformOnMouseUp (CPoint& where, const CButtonState& buttons);
	CMouseEventResult platformOnMouseExited (CPoint& where, const CButtonState& buttons);
	bool platformOnMouseWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const CButtonState &buttons);
	bool platformOnDrop (CDragContainer* drag, const CPoint& where);
	void platformOnDragEnter (CDragContainer* drag, const CPoint& where);
	void platformOnDragLeave (CDragContainer* drag, const CPoint& where);
	void platformOnDragMove (CDragContainer* drag, const CPoint& where);
	bool platformOnKeyDown (VstKeyCode& keyCode);
	bool platformOnKeyUp (VstKeyCode& keyCode);
	void platformOnActivate (bool state);
};

//----------------------------------------------------
class VSTGUIEditorInterface
{
public:
	virtual void doIdleStuff () {}
	virtual int32_t getKnobMode () const { return 0; }
	
	virtual void beginEdit (int32_t index) {}
	virtual void endEdit (int32_t index) {}

	virtual bool beforeSizeChange (const CRect& newSize, const CRect& oldSize) { return true; } ///< frame will change size, if this returns false the upstream implementation does not allow it and thus the size of the frame will not change
	virtual bool isControlDefaultButton (const CButtonState& state) const { return state == (kControl|kLButton); } ///< check if button combination should be used to set controls to its default value

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
//! @brief generic keyboard hook interface for CFrame [new in 4.0]
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
//! @brief view added removed observer interface for CFrame [new in 4.0]
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
