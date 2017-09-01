// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __cview__
#define __cview__

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
extern UTF8StringPtr kDegreeSymbol;			///< degree sign
extern UTF8StringPtr kInfiniteSymbol;		///< infinity
extern UTF8StringPtr kCopyrightSymbol;		///< copyright sign
extern UTF8StringPtr kTrademarkSymbol;		///< trade mark sign
extern UTF8StringPtr kRegisteredSymbol;		///< registered sign
extern UTF8StringPtr kMicroSymbol;			///< micro sign
extern UTF8StringPtr kPerthousandSymbol;	///< per mille sign

//-----------------------------------------------------------------------------
extern IdStringPtr kMsgViewSizeChanged;		///< Message send to parent that the size of the view has changed

//-----------------------------------------------------------------------------
// Attributes
//		all attributes where the first letter is lowercase are reserved for the vstgui lib

extern const CViewAttributeID kCViewAttributeReferencePointer;	// 'cvrp'
extern const CViewAttributeID kCViewTooltipAttribute;			// 'cvtt'
extern const CViewAttributeID kCViewControllerAttribute;		// 'ictr' ///< see @ref IController

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
	virtual void draw (CDrawContext *pContext);															///< called if the view should draw itself
	virtual void drawRect (CDrawContext *pContext, const CRect& updateRect) { draw (pContext); }		///< called if the view should draw itself
	virtual bool checkUpdate (const CRect& updateRect) const { return updateRect.rectOverlap (getViewSize ()); }

	virtual bool isDirty () const { return hasViewFlag (kDirty); }										///< check if view is dirty
	virtual void setDirty (bool val = true);															///< set the view to dirty so that it is redrawn in the next idle. Thread Safe !
	static bool kDirtyCallAlwaysOnMainThread;															///< if this is true, setting a view dirty will call invalid() instead of checking it in idle. Default value is false.

	virtual void invalidRect (const CRect& rect);														///< mark rect as invalid
	virtual void invalid () { setDirty (false); invalidRect (getViewSize ()); }									///< mark whole view as invalid

	virtual void setVisible (bool state);																///< set visibility state
	bool isVisible () const { return hasViewFlag (kVisible) && getAlphaValue () > 0.f; }						///< get visibility state
	//@}

	//-----------------------------------------------------------------------------
	/// @name Mouse Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);											///< called when a mouse down event occurs
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons);											///< called when a mouse up event occurs
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons);										///< called when a mouse move event occurs
	virtual CMouseEventResult onMouseCancel ();																					///< called when mouse tracking should be canceled

	virtual CMouseEventResult onMouseEntered (CPoint& where, const CButtonState& buttons) {return kMouseEventNotImplemented;}	///< called when the mouse enters this view
	virtual CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons) {return kMouseEventNotImplemented;}	///< called when the mouse leaves this view

	void setHitTestPath (CGraphicsPath* path);
	virtual bool hitTest (const CPoint& where, const CButtonState& buttons = -1);												///< check if where hits this view

	virtual bool onWheel (const CPoint& where, const float& distance, const CButtonState& buttons);									///< called if a mouse wheel event is happening over this view
	virtual bool onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons);	///< called if a mouse wheel event is happening over this view

	virtual void setMouseEnabled (bool bEnable = true);											///< turn on/off mouse usage for this view
	bool getMouseEnabled () const { return hasViewFlag (kMouseEnabled); }						///< get the state of wheather this view uses the mouse or not

	virtual void setMouseableArea (const CRect& rect);											///< set the area in which the view reacts to the mouse
	CRect& getMouseableArea (CRect& rect) const;												///< get the area in which the view reacts to the mouse
	const CRect& getMouseableArea () const;														///< read only access to the mouseable area
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
	virtual DragResult doDrag (IDataPackage* source, const CPoint& offset = CPoint (0, 0), CBitmap* dragBitmap = nullptr);	///< start a drag operation. See CDropSource to create the source data package
	virtual bool onDrop (IDataPackage* drag, const CPoint& where) { return false; }				///< called if a drag is dropped onto this view
	virtual void onDragEnter (IDataPackage* drag, const CPoint& where) {}						///< called if a drag is entering this view
	virtual void onDragLeave (IDataPackage* drag, const CPoint& where) {}						///< called if a drag is leaving this view
	virtual void onDragMove (IDataPackage* drag, const CPoint& where) {}						///< called if a drag is moved inside this view
	//@}

	//-----------------------------------------------------------------------------
	/// @name Keyboard Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual int32_t onKeyDown (VstKeyCode& keyCode);											///< called if a key down event occurs and this view has focus
	virtual int32_t onKeyUp (VstKeyCode& keyCode);												///< called if a key up event occurs and this view has focus
	//@}

	//-----------------------------------------------------------------------------
	/// @name View Size Methods
	//-----------------------------------------------------------------------------
	//@{
	CCoord getHeight () const { return getViewSize ().getHeight (); }							///< get the height of the view
	CCoord getWidth ()  const { return getViewSize ().getWidth (); }							///< get the width of the view
	virtual void setViewSize (const CRect& rect, bool invalid = true);							///< set views size
	const CRect& getViewSize () const;															///< read only access to view size
	virtual CRect getVisibleViewSize () const;													///< returns the visible size of the view
	virtual void parentSizeChanged () {}														///< notification that one of the views parent has changed its size
	virtual CPoint& frameToLocal (CPoint& point) const;											///< conversion from frame coordinates to local view coordinates
	virtual CPoint& localToFrame (CPoint& point) const;											///< conversion from local view coordinates to frame coordinates
	virtual void setAutosizeFlags (int32_t flags);												///< set autosize flags
	int32_t getAutosizeFlags () const;															///< get autosize flags
	virtual bool sizeToFit () { return false; }													///< resize view to optimal size
	//@}

	//-----------------------------------------------------------------------------
	/// @name Focus Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void looseFocus ();																	///< called if view should loose focus
	virtual void takeFocus ();																	///< called if view should take focus
	virtual bool wantsFocus () const { return hasViewFlag (kWantsFocus); }						///< check if view supports focus
	virtual void setWantsFocus (bool state);													///< set focus support on/off
	//@}

	//-----------------------------------------------------------------------------
	/// @name Attribute Methods
	//-----------------------------------------------------------------------------
	//@{
	bool getAttributeSize (const CViewAttributeID id, uint32_t& outSize) const;										///< get the size of an attribute
	bool getAttribute (const CViewAttributeID id, const uint32_t inSize, void* outData, uint32_t& outSize) const;	///< get an attribute
	bool setAttribute (const CViewAttributeID id, const uint32_t inSize, const void* inData);						///< set an attribute
	bool removeAttribute (const CViewAttributeID id);																///< remove an attribute
	//@}

	//-----------------------------------------------------------------------------
	/// @name Background Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setBackground (CBitmap* background);											///< set the background image of this view
	CBitmap* getBackground () const;															///< get the background image of this view

	virtual void setDisabledBackground (CBitmap* background);									///< set background image used when the mouse is not enabled
	CBitmap* getDisabledBackground () const;													///< get background image used when the mouse is not enabled

	CBitmap* getDrawBackground () const;
	//@}

	//-----------------------------------------------------------------------------
	/// @name Transparency Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setTransparency (bool val);													///< set views transparent state
	bool getTransparency () const { return hasViewFlag (kTransparencyEnabled); }				///< get views transparent state

	virtual void setAlphaValue (float alpha);													///< set alpha value which will be applied when drawing this view
	float getAlphaValue () const;																///< get alpha value
	//@}

	//-----------------------------------------------------------------------------
	/// @name Attaching Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual bool removed (CView* parent);														///< view is removed from parent view
	virtual bool attached (CView* parent);														///< view is attached to a parent view
	bool isAttached () const { return hasViewFlag (kIsAttached); }								///< is view attached to a parentView
	//@}

	void setSubviewState (bool state);
	bool isSubview () const { return hasViewFlag (kIsSubview); }

	//-----------------------------------------------------------------------------
	/// @name Parent Methods
	//-----------------------------------------------------------------------------
	//@{
	CView* getParentView () const;																///< get parent view
	CFrame* getFrame () const;																	///< get frame
	virtual VSTGUIEditorInterface* getEditor () const;											///< get editor
	//@}

	//-----------------------------------------------------------------------------
	/// @name Animation Methods
	//-----------------------------------------------------------------------------
	//@{
	void addAnimation (IdStringPtr name, Animation::IAnimationTarget* target, Animation::ITimingFunction* timingFunction, CBaseObject* notificationObject = nullptr);
	void addAnimation (IdStringPtr name, Animation::IAnimationTarget* target, Animation::ITimingFunction* timingFunction, const Animation::DoneFunction& doneFunc);
	void removeAnimation (IdStringPtr name);
	void removeAllAnimations ();
	//@}
	
	//-----------------------------------------------------------------------------
	/// @name Idle Methods
	//! Should be used when a view needs to do a task periodically.
	//! The onIdle() method will be called only if the view is attached.
	//-----------------------------------------------------------------------------
	//@{
	virtual void onIdle () {}																	///< called on idle when view wants idle
	void setWantsIdle (bool state);																///< enable/disable onIdle() callback
	bool wantsIdle () const { return hasViewFlag (kWantsIdle); }								///< returns if the view wants idle callback or not
	static uint32_t idleRate;																	///< global idle rate in Hz, defaults to 30 Hz
	//@}

	/** weather this view wants to be informed if the window's active state changes */
	virtual bool wantsWindowActiveStateChangeNotification () const { return false; }
	/** called when the active state of the window changes */
	virtual void onWindowActivate (bool state) {}
	
	//-----------------------------------------------------------------------------
	/// @name View Listener Methods
	//-----------------------------------------------------------------------------
	//@{
	void registerViewListener (IViewListener* listener);
	void unregisterViewListener (IViewListener* listener);
	//@}

	CGraphicsTransform getGlobalTransform () const;
	template<typename T> T& translateToGlobal (T& t) const { getGlobalTransform ().transform (t); return t; } ///< translates a local coordinate to a global one using parent transforms
	template<typename T> T translateToGlobal (const T& t) const { T tmp (t); getGlobalTransform ().transform (tmp); return tmp; } ///< translates a local coordinate to a global one using parent transforms
	template<typename T> T& translateToLocal (T& t) const { getGlobalTransform ().inverse ().transform (t); return t; } ///< translates a global coordinate to a local one using parent transforms
	template<typename T> T translateToLocal (const T& t) const { T tmp (t); getGlobalTransform ().inverse ().transform (tmp); return tmp; } ///< translates a local coordinate to a global one using parent transforms

	#if DEBUG
	virtual void dumpInfo ();
	#endif

	virtual CViewContainer* asViewContainer () { return nullptr; }
	virtual const CViewContainer* asViewContainer () const { return nullptr; }

	// overwrites
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;
	void beforeDelete () override;

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
		kLastCViewFlag			= 7
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
		kFile = 0,								///< File (MacOSX = UTF8 String)
		kText,									///< ASCII Text
		kUnicodeText,							///< UTF8 Text
		
		kUnknown = -1,
		kError = -2
	};
protected:
	
	IDataPackage* drag {nullptr};
	int32_t index {0};
};

} // namespace

#endif
