// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"
#include "cpoint.h"
#include "crect.h"
#include "vstkeycode.h"
#include "cbuttonstate.h"
#include "cgraphicstransform.h"
#include <memory>

namespace VSTGUI {

//-----------------------------------------------------------------------------
// Definitions of special characters in a platform independent way
/** degree sign */
extern UTF8StringPtr kDegreeSymbol;
/** infinity sign */
extern UTF8StringPtr kInfiniteSymbol;
/** copyright sign */
extern UTF8StringPtr kCopyrightSymbol;
/** trade mark sign */
extern UTF8StringPtr kTrademarkSymbol;
/** registered sign */
extern UTF8StringPtr kRegisteredSymbol;
/** micro sign */
extern UTF8StringPtr kMicroSymbol;
/** per mille sign */
extern UTF8StringPtr kPerthousandSymbol;

//-----------------------------------------------------------------------------
/** Message send to parent that the size of the view has changed */
extern IdStringPtr kMsgViewSizeChanged;

//-----------------------------------------------------------------------------
// Attributes
//		all attributes where the first letter is lowercase are reserved for the vstgui lib
static constexpr CViewAttributeID kCViewAttributeReferencePointer = 'cvrp';
static constexpr CViewAttributeID kCViewTooltipAttribute = 'cvtt';
static constexpr CViewAttributeID kCViewControllerAttribute = 'ictr';

//-----------------------------------------------------------------------------
// CView Declaration
//! @brief Base Class of all view objects
/// @ingroup views
//-----------------------------------------------------------------------------
class CView : public CBaseObject
{
public:
	explicit CView (const CRect& size);
	CView (const CView& view);

	//-----------------------------------------------------------------------------
	/// @name Draw and Update Methods
	//-----------------------------------------------------------------------------
	//@{
	/** called if the view should draw itself */
	virtual void draw (CDrawContext *pContext);
	/** called if the view should draw itself */
	virtual void drawRect (CDrawContext *pContext, const CRect& updateRect) { draw (pContext); }
	virtual bool checkUpdate (const CRect& updateRect) const { return updateRect.rectOverlap (getViewSize ()); }

	/** check if view is dirty */
	virtual bool isDirty () const { return hasViewFlag (kDirty); }
	/** set the view to dirty so that it is redrawn in the next idle. Thread Safe ! */
	virtual void setDirty (bool val = true);
	/** if this is true, setting a view dirty will call invalid() instead of checking it in idle. Default value is false. */
	static bool kDirtyCallAlwaysOnMainThread;

	/** mark rect as invalid */
	virtual void invalidRect (const CRect& rect);
	/** mark whole view as invalid */
	virtual void invalid () { setDirty (false); invalidRect (getViewSize ()); }

	/** set visibility state */
	virtual void setVisible (bool state);
	/** get visibility state */
	bool isVisible () const { return hasViewFlag (kVisible) && getAlphaValue () > 0.f; }
	//@}

	//-----------------------------------------------------------------------------
	/// @name Mouse Methods
	//-----------------------------------------------------------------------------
	//@{
	/** called when a mouse down event occurs */
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);
	/** called when a mouse up event occurs */
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons);
	/** called when a mouse move event occurs */
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons);
	/** called when mouse tracking should be canceled */
	virtual CMouseEventResult onMouseCancel ();

	/** called when the mouse enters this view */
	virtual CMouseEventResult onMouseEntered (CPoint& where, const CButtonState& buttons) {return kMouseEventNotImplemented;}
	/** called when the mouse leaves this view */
	virtual CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons) {return kMouseEventNotImplemented;}

	void setHitTestPath (CGraphicsPath* path);
	/** check if where hits this view */
	virtual bool hitTest (const CPoint& where, const CButtonState& buttons = -1);

	VSTGUI_DEPRECATED(
	/** \deprecated never called anymore, please override the method below for wheel handling */
	virtual bool onWheel (const CPoint& where, const float& distance, const CButtonState& buttons) final { return false; })
	/** called if a mouse wheel event is happening over this view */
	virtual bool onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons);

	/** turn on/off mouse usage for this view */
	virtual void setMouseEnabled (bool bEnable = true);
	/** get the state of wheather this view uses the mouse or not */
	bool getMouseEnabled () const { return hasViewFlag (kMouseEnabled); }

	/** set the area in which the view reacts to the mouse */
	void setMouseableArea (const CRect& rect);
	VSTGUI_DEPRECATED(
	/** get the area in which the view reacts to the mouse */
	CRect& getMouseableArea (CRect& rect) const;)
	/** get the area in which the view reacts to the mouse */
	CRect getMouseableArea () const;
	//@}

#if VSTGUI_TOUCH_EVENT_HANDLING
	//-----------------------------------------------------------------------------
	/// @name Touch Event Handling Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void onTouchEvent (ITouchEvent& event) {}
	virtual bool wantsMultiTouchEvents () const { return false; }
	//@}
#endif

	//-----------------------------------------------------------------------------
	/// @name Drag & Drop Methods
	//-----------------------------------------------------------------------------
	//@{
	/** start a drag operation */
	bool doDrag (const DragDescription& dragDescription, const SharedPointer<IDragCallback>& callback = {});
	/** get the drag target for drag and drop handling */
	virtual SharedPointer<IDropTarget> getDropTarget ();
	/** set a custom drop target */
	void setDropTarget (const SharedPointer<IDropTarget>& dt);

	/** \deprecated start a drag operation. See CDropSource to create the source data package */
	VSTGUI_DEPRECATED(DragResult doDrag (IDataPackage* source, const CPoint& offset = CPoint (0, 0), CBitmap* dragBitmap = nullptr);)
	//@}

	//-----------------------------------------------------------------------------
	/// @name Keyboard Methods
	//-----------------------------------------------------------------------------
	//@{
	/** called if a key down event occurs and this view has focus */
	virtual int32_t onKeyDown (VstKeyCode& keyCode);
	/** called if a key up event occurs and this view has focus */
	virtual int32_t onKeyUp (VstKeyCode& keyCode);
	//@}

	//-----------------------------------------------------------------------------
	/// @name View Size Methods
	//-----------------------------------------------------------------------------
	//@{
	/** get the height of the view */
	CCoord getHeight () const { return getViewSize ().getHeight (); }
	/** get the width of the view */
	CCoord getWidth ()  const { return getViewSize ().getWidth (); }
	/** set views size */
	virtual void setViewSize (const CRect& rect, bool invalid = true);
	/** read only access to view size */
	const CRect& getViewSize () const;
	/** returns the visible size of the view */
	virtual CRect getVisibleViewSize () const;
	/** notification that one of the views parent has changed its size */
	virtual void parentSizeChanged () {}
	/** conversion from frame coordinates to local view coordinates */
	virtual CPoint& frameToLocal (CPoint& point) const;
	/** conversion from local view coordinates to frame coordinates */
	virtual CPoint& localToFrame (CPoint& point) const;
	/** set autosize flags */
	virtual void setAutosizeFlags (int32_t flags);
	/** get autosize flags */
	int32_t getAutosizeFlags () const;
	/** resize view to optimal size */
	virtual bool sizeToFit () { return false; }
	//@}

	//-----------------------------------------------------------------------------
	/// @name Focus Methods
	//-----------------------------------------------------------------------------
	//@{
	/** called if view should loose focus */
	virtual void looseFocus ();
	/** called if view should take focus */
	virtual void takeFocus ();
	/** check if view supports focus */
	virtual bool wantsFocus () const { return hasViewFlag (kWantsFocus); }
	/** set focus support on/off */
	virtual void setWantsFocus (bool state);
	//@}

	//-----------------------------------------------------------------------------
	/// @name Attribute Methods
	//-----------------------------------------------------------------------------
	//@{
	/** get the size of an attribute */
	bool getAttributeSize (const CViewAttributeID id, uint32_t& outSize) const;
	/** get an attribute */
	bool getAttribute (const CViewAttributeID id, const uint32_t inSize, void* outData, uint32_t& outSize) const;
	/** set an attribute */
	bool setAttribute (const CViewAttributeID id, const uint32_t inSize, const void* inData);
	/** remove an attribute */
	bool removeAttribute (const CViewAttributeID id);

	/** set an attribute */
	template<typename T>
	bool setAttribute (const CViewAttributeID id, const T& data)
	{
		return setAttribute (id, sizeof (T), &data);
	}
	
	/** get an attribute */
	template <typename T>
	bool getAttribute (const CViewAttributeID id, T& data) const
	{
		uint32_t outSize;
		if (getAttribute (id, sizeof (T), &data, outSize))
			return outSize == sizeof (T);
		return false;
	}
	//@}

	//-----------------------------------------------------------------------------
	/// @name Background Methods
	//-----------------------------------------------------------------------------
	//@{
	/** set the background image of this view */
	virtual void setBackground (CBitmap* background);
	/** get the background image of this view */
	CBitmap* getBackground () const;

	/** set background image used when the mouse is not enabled */
	virtual void setDisabledBackground (CBitmap* background);
	/** get background image used when the mouse is not enabled */
	CBitmap* getDisabledBackground () const;

	/** get the bitmap which is drawn depending on the enabled state. */
	CBitmap* getDrawBackground () const;
	//@}

	//-----------------------------------------------------------------------------
	/// @name Transparency Methods
	//-----------------------------------------------------------------------------
	//@{
	/** set views transparent state */
	virtual void setTransparency (bool val);
	/** get views transparent state */
	bool getTransparency () const { return hasViewFlag (kTransparencyEnabled); }

	/** set alpha value which will be applied when drawing this view */
	virtual void setAlphaValue (float alpha);
	/** get alpha value */
	float getAlphaValue () const;
	//@}

	//-----------------------------------------------------------------------------
	/// @name Attaching Methods
	//-----------------------------------------------------------------------------
	//@{
	/** view is removed from parent view */
	virtual bool removed (CView* parent);
	/** view is attached to a parent view */
	virtual bool attached (CView* parent);
	/** is view attached to a parentView */
	bool isAttached () const { return hasViewFlag (kIsAttached); }
	//@}

	void setSubviewState (bool state);
	bool isSubview () const { return hasViewFlag (kIsSubview); }

	//-----------------------------------------------------------------------------
	/// @name Parent Methods
	//-----------------------------------------------------------------------------
	//@{
	/** get parent view */
	CView* getParentView () const;
	/** get frame */
	CFrame* getFrame () const;
	/** get editor */
	virtual VSTGUIEditorInterface* getEditor () const;
	//@}

	//-----------------------------------------------------------------------------
	/// @name Animation Methods
	//-----------------------------------------------------------------------------
	//@{
	VSTGUI_DEPRECATED(void addAnimation (IdStringPtr name, Animation::IAnimationTarget* target, Animation::ITimingFunction* timingFunction, CBaseObject* notificationObject);)
	void addAnimation (IdStringPtr name, Animation::IAnimationTarget* target,
	                   Animation::ITimingFunction* timingFunction,
	                   const Animation::DoneFunction& doneFunc = nullptr);
	void removeAnimation (IdStringPtr name);
	void removeAllAnimations ();
	//@}
	
	//-----------------------------------------------------------------------------
	/// @name Idle Methods
	//! Should be used when a view needs to do a task periodically.
	//! The onIdle() method will be called only if the view is attached.
	//-----------------------------------------------------------------------------
	//@{
	/** called on idle when view wants idle */
	virtual void onIdle () {}
	/** enable/disable onIdle() callback */
	void setWantsIdle (bool state);
	/** returns if the view wants idle callback or not */
	bool wantsIdle () const { return hasViewFlag (kWantsIdle); }
	/** global idle rate in Hz, defaults to 30 Hz*/
	static uint32_t idleRate;
	//@}

	/** whether this view wants to be informed if the window's active state changes */
	virtual bool wantsWindowActiveStateChangeNotification () const { return false; }
	/** called when the active state of the window changes */
	virtual void onWindowActivate (bool state) {}
	
	void setTooltipText (UTF8StringPtr text);
	
	//-----------------------------------------------------------------------------
	/// @name View Listener Methods
	//-----------------------------------------------------------------------------
	//@{
	void registerViewListener (IViewListener* listener);
	void unregisterViewListener (IViewListener* listener);
	
	void registerViewMouseListener (IViewMouseListener* listener);
	void unregisterViewMouseListener (IViewMouseListener* listener);
	//@}

	//-----------------------------------------------------------------------------
	/// @name Coordinate translation Methods
	//-----------------------------------------------------------------------------
	//@{
	/** get the active global transform for this view */
	CGraphicsTransform getGlobalTransform (bool ignoreFrame = false) const;
	/** translates a local coordinate to a global one using parent transforms */
	template<typename T> T& translateToGlobal (T& t, bool ignoreFrame = false) const { getGlobalTransform (ignoreFrame).transform (t); return t; }
	/** translates a local coordinate to a global one using parent transforms */
	template<typename T> T translateToGlobal (const T& t, bool ignoreFrame = false) const { T tmp (t); getGlobalTransform (ignoreFrame).transform (tmp); return tmp; }
	/** translates a global coordinate to a local one using parent transforms */
	template<typename T> T& translateToLocal (T& t, bool ignoreFrame = false) const { getGlobalTransform (ignoreFrame).inverse ().transform (t); return t; }
	/** translates a local coordinate to a global one using parent transforms */
	template<typename T> T translateToLocal (const T& t, bool ignoreFrame = false) const { T tmp (t); getGlobalTransform (ignoreFrame).inverse ().transform (tmp); return tmp; }
	//@}

	virtual CViewContainer* asViewContainer () { return nullptr; }
	virtual const CViewContainer* asViewContainer () const { return nullptr; }

	enum class MouseListenerCall
	{
		MouseDown,
		MouseMoved,
		MouseUp,
		MouseCancel
	};
	CMouseEventResult callMouseListener (MouseListenerCall type, CPoint pos, CButtonState buttons);
	void callMouseListenerEnteredExited (bool mouseEntered);
	
	// overwrites
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;
	void beforeDelete () override;

	#if DEBUG
	virtual void dumpInfo ();
	#endif
	//-------------------------------------------
	CLASS_METHODS(CView, CBaseObject)
protected:
	enum {
		kMouseEnabled			= 1 << 0,
		kTransparencyEnabled	= 1 << 1,
		kWantsFocus				= 1 << 2,
		kIsAttached				= 1 << 3,
		kVisible				= 1 << 4,
		kDirty					= 1 << 5,
		kWantsIdle				= 1 << 6,
		kIsSubview				= 1 << 7,
		kHasAlpha				= 1 << 8,
		kHasBackground			= 1 << 9,
		kHasDisabledBackground	= 1 << 10,
		kHasMouseableArea		= 1 << 11,
		kLastCViewFlag			= 11
	};

	~CView () noexcept override;

	CGraphicsPath* getHitTestPath () const;
	
	bool hasViewFlag (int32_t bit) const;
	void setViewFlag (int32_t bit, bool state);
	
	void setAlphaValueNoInvalidate (float value);
	void setParentFrame (CFrame* frame);
	void setParentView (CView* parent);

private:
	struct Impl;
	std::unique_ptr<Impl> pImpl;
};

//-----------------------------------------------------------------------------
///	@brief Helper class to port old code which used CDragContainer
///	@ingroup new_in_4_2
//-----------------------------------------------------------------------------
class CDragContainerHelper
{
public:
	explicit CDragContainerHelper (IDataPackage* drag);
	CDragContainerHelper () = delete;

	void* first (int32_t& size, int32_t& type);
	void* next (int32_t& size, int32_t& type);
	
	int32_t getType (int32_t idx) const;
	int32_t getCount () const;
	
	enum CDragType {
		/** File (MacOSX = UTF8 String) */
		kFile = 0,
		/** ASCII Text */
		kText,
		/** UTF8 Text */
		kUnicodeText,
		
		kUnknown = -1,
		kError = -2
	};
protected:
	
	IDataPackage* drag {nullptr};
	int32_t index {0};
};

} // VSTGUI
