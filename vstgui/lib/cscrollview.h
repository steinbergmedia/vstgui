//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins :
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __cscrollview__
#define __cscrollview__

#include "cviewcontainer.h"
#include "controls/ccontrol.h"

namespace VSTGUI {

class CScrollbar;
class CScrollContainer;
class CVSTGUITimer;

//-----------------------------------------------------------------------------
// CScrollView Declaration
//! @brief a scrollable container view with scrollbars
/// @ingroup containerviews
//-----------------------------------------------------------------------------
class CScrollView : public CViewContainer, public CControlListener
{
protected:
	enum
	{
		kHorizontalScrollbarFlag,
		kVerticalScrollbarFlag,
		kDontDrawFrameFlag,
		kAutoDragScollingFlag,
		kOverlayScrollbarsFlag,
		kFollowFocusViewFlag,
		kAutoHideScrollbarsFlag,

		kLastScrollViewStyleFlag
	};

public:
	CScrollView (const CRect& size, const CRect& containerSize, int32_t style, CCoord scrollbarWidth = 16, CBitmap* pBackground = 0);
	VSTGUI_DEPRECATED(CScrollView (const CRect& size, const CRect& containerSize, CFrame* pParent, int32_t style, CCoord scrollbarWidth = 16, CBitmap* pBackground = 0);)
	CScrollView (const CScrollView& scrollView);

	/** Scroll View Style Flags */
	enum CScrollViewStyle
	{
		kHorizontalScrollbar	= 1 << kHorizontalScrollbarFlag,	///< add a horizontal scrollbar
		kVerticalScrollbar 		= 1 << kVerticalScrollbarFlag,		///< add a vertical scrollbar
		kDontDrawFrame			= 1 << kDontDrawFrameFlag,			///< don't draw frame
		kAutoDragScrolling		= 1 << kAutoDragScollingFlag,		///< automatic scrolling for drag moves
		kOverlayScrollbars		= 1 << kOverlayScrollbarsFlag,		///< scrollbars are overlayed of the content
		kFollowFocusView		= 1 << kFollowFocusViewFlag, 		///< scroll to focus view when focus view changes
		kAutoHideScrollbars		= 1 << kAutoHideScrollbarsFlag		///< automatically hides the scrollbar if the container size is smaller than the size of the scrollview
	};

	//-----------------------------------------------------------------------------
	/// @name CScrollView Methods
	//-----------------------------------------------------------------------------
	//@{
	int32_t getStyle () const { return style; }
	void setStyle (int32_t newStyle);
	
	int32_t getActiveScrollbars () const { return activeScrollbarStyle; }

	CCoord getScrollbarWidth () const { return scrollbarWidth; }
	void setScrollbarWidth (CCoord width);

	virtual void setContainerSize (const CRect& cs, bool keepVisibleArea = false);	///< set the virtual size of this container
	const CRect& getContainerSize () const { return containerSize; }
	const CPoint& getScrollOffset () const;				///< get scroll offset
	void resetScrollOffset ();

	CScrollbar* getVerticalScrollbar () const { return vsb; }	///< get the vertical scrollbar
	CScrollbar* getHorizontalScrollbar () const { return hsb; }	///< get the horizontal scrollbar

	virtual void makeRectVisible (const CRect& rect);	///< set scrollview to show rect
	//@}

	// overwrite
	bool addView (CView* pView) VSTGUI_OVERRIDE_VMETHOD;
	bool addView (CView* pView, const CRect& mouseableArea, bool mouseEnabled = true) VSTGUI_OVERRIDE_VMETHOD;
	bool addView (CView* pView, CView* pBefore) VSTGUI_OVERRIDE_VMETHOD;
	bool removeView (CView* pView, bool withForget = true) VSTGUI_OVERRIDE_VMETHOD;
	bool removeAll (bool withForget = true) VSTGUI_OVERRIDE_VMETHOD;
	int32_t getNbViews () const VSTGUI_OVERRIDE_VMETHOD;
	CView* getView (int32_t index) const VSTGUI_OVERRIDE_VMETHOD;
	void drawBackgroundRect (CDrawContext* pContext, const CRect& _updateRect) VSTGUI_OVERRIDE_VMETHOD;
	bool onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	void valueChanged (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;
	void setTransparency (bool val) VSTGUI_OVERRIDE_VMETHOD;
	void setBackgroundColor (const CColor& color) VSTGUI_OVERRIDE_VMETHOD;
	void setViewSize (const CRect& rect, bool invalid = true) VSTGUI_OVERRIDE_VMETHOD;
	void setAutosizeFlags (int32_t flags) VSTGUI_OVERRIDE_VMETHOD;
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD;

	CLASS_METHODS(CScrollView, CViewContainer)
//-----------------------------------------------------------------------------
protected:
	~CScrollView ();
	virtual void recalculateSubViews ();

	CScrollContainer* sc;
	CScrollbar* vsb;
	CScrollbar* hsb;

	CRect containerSize;
	CCoord scrollbarWidth;
	int32_t style;
	int32_t activeScrollbarStyle;

	enum {
		kHSBTag,
		kVSBTag
	};
};

class IScrollbarDrawer;

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

	CScrollbar (const CRect& size, CControlListener* listener, int32_t tag, ScrollbarDirection style, const CRect& scrollSize);
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

	virtual CMouseEventResult onMouseEntered (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;

	CLASS_METHODS(CScrollbar, CControl)
//-----------------------------------------------------------------------------
protected:
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

	IScrollbarDrawer* drawer;
private:
	~CScrollbar ();
	CVSTGUITimer* timer;
	CPoint startPoint;
	CRect scrollerRect;
	bool scrolling;
	float startValue;
};

//-----------------------------------------------------------------------------
class IScrollbarDrawer
//-----------------------------------------------------------------------------
{
public:
	virtual void drawScrollbarBackground (CDrawContext* pContext, const CRect& size, CScrollbar::ScrollbarDirection direction, CScrollbar* bar) = 0;
	virtual void drawScrollbarScroller (CDrawContext* pContext, const CRect& size, CScrollbar::ScrollbarDirection direction, CScrollbar* bar) = 0;
};

} // namespace

#endif
