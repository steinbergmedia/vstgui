//
//  cscrollbar.h
//  vstgui
//
//  Created by Arne Scheffler on 20/08/14.
//
//

#ifndef __cscrollbar__
#define __cscrollbar__

#include "ccontrol.h"
#include "../ccolor.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CScrollbar Declaration
//! @brief a scrollbar control
/// @ingroup controls
//-----------------------------------------------------------------------------
class CScrollbar : public CControl
{
public:
	enum ScrollbarDirection {
		kHorizontal,
		kVertical
	};
	
	CScrollbar (const CRect& size, IControlListener* listener, int32_t tag, ScrollbarDirection style, const CRect& scrollSize);
	CScrollbar (const CScrollbar& scrollbar);
	
	//-----------------------------------------------------------------------------
	/// @name CScrollbar Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setDrawer (IScrollbarDrawer* d) { drawer = d; }
	virtual void setScrollSize (const CRect& ssize);
	virtual void setStep (float newStep) { stepValue = newStep; }
	
	CRect& getScrollSize (CRect& rect) const { rect = scrollSize; return rect; }
	float getStep () const { return stepValue; }
	
	virtual void setFrameColor (const CColor& color) { frameColor = color; }
	virtual void setScrollerColor (const CColor& color) { scrollerColor = color; }
	virtual void setBackgroundColor (const CColor& color) { backgroundColor = color; }
	
	CColor getFrameColor () const { return frameColor; }
	CColor getScrollerColor () const { return scrollerColor; }
	CColor getBackgroundColor () const { return backgroundColor; }
	
	bool getOverlayStyle () const { return overlayStyle; }
	virtual void setOverlayStyle (bool state);
	
	virtual void onVisualChange ();
	CRect getScrollerRect ();
	//@}
	
	// overwrite
	void draw (CDrawContext* pContext) VSTGUI_OVERRIDE_VMETHOD;
	bool onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD;
	void setViewSize (const CRect& newSize, bool invalid) VSTGUI_OVERRIDE_VMETHOD;
	
	CMouseEventResult onMouseEntered (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	
	CLASS_METHODS(CScrollbar, CControl)
	//-----------------------------------------------------------------------------
protected:
	~CScrollbar ();

	void drawBackground (CDrawContext* pContext);
	void drawScroller (CDrawContext* pContext, const CRect& size);
	
	void calculateScrollerLength ();
	void doStepping ();
	
	ScrollbarDirection direction;
	CRect scrollSize;
	CRect scrollerArea;
	
	float stepValue;
	CCoord scrollerLength;
	
	CColor frameColor;
	CColor scrollerColor;
	CColor backgroundColor;
	
	bool overlayStyle;
	bool mouseIsInside;
	
	IScrollbarDrawer* drawer;
private:
	CVSTGUITimer* timer;
	CPoint startPoint;
	CRect scrollerRect;
	bool scrolling;
};

//-----------------------------------------------------------------------------
class IScrollbarDrawer
//-----------------------------------------------------------------------------
{
public:
	virtual void drawScrollbarBackground (CDrawContext* pContext, const CRect& size, CScrollbar::ScrollbarDirection direction, CScrollbar* bar) = 0;
	virtual void drawScrollbarScroller (CDrawContext* pContext, const CRect& size, CScrollbar::ScrollbarDirection direction, CScrollbar* bar) = 0;
};


}

#endif // __cscrollbar__
