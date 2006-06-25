//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// CScrollView written 2004 by Arne Scheffler
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// © 2003, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __cscrollview__
#define __cscrollview__

#ifndef __vstgui__
#include "vstgui.h"
#endif

BEGIN_NAMESPACE_VSTGUI

class CScrollbar;
class CScrollContainer;
class CVSTGUITimer;

//-----------------------------------------------------------------------------
class CScrollView : public CViewContainer, CControlListener
//! \brief a scrollable container view with scrollbars
/// \nosubgrouping
//-----------------------------------------------------------------------------
{
public:
	CScrollView (const CRect &size, const CRect &containerSize, CFrame* pParent, long style, long scrollbarWidth = 16, CBitmap* pBackground = 0);
	virtual ~CScrollView ();

	enum CScrollViewStyle 
	{
		kHorizontalScrollbar	= 1 << 1,	///< add a horizontal scrollbar
		kVerticalScrollbar 		= 1 << 2,	///< add a vertical scrollbar
		kDontDrawFrame			= 1 << 3,	///< don't draw frame
	};

	virtual void setContainerSize (const CRect& cs, bool keepVisibleArea = false);	///< set the virtual size of this container
	const CPoint& getScrollOffset () const;				///< get scroll offset
	
	CScrollbar* getVerticalScrollbar () const { return vsb; }
	CScrollbar* getHorizontalScrollbar () const { return hsb; }

	virtual void makeRectVisible (const CRect& rect);	///< set scrollview to show rect
	
	// overwrite
	bool addView (CView *pView);
	bool addView (CView *pView, CRect &mouseableArea, bool mouseEnabled = true);
	bool removeView (CView *pView, const bool &withForget = true);
	bool removeAll (const bool &withForget = true);
	bool isChild (CView *pView) const;
	long getNbViews () const;
	CView *getView (long index) const;
	void drawBackgroundRect (CDrawContext *pContext, CRect& _updateRect);
	bool onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const long &buttons);
	void valueChanged (CControl *pControl);
	void setTransparency (bool val);
	void setBackgroundColor (const CColor& color);

	CLASS_METHODS(CScrollView, CViewContainer)
//-----------------------------------------------------------------------------
protected:
	CScrollContainer* sc;
	CScrollbar* vsb;
	CScrollbar* hsb;

	CRect containerSize;
	long style;

	enum {
		kHSBTag,
		kVSBTag,
	};
};

//-----------------------------------------------------------------------------
class IScrollbarDrawer
//-----------------------------------------------------------------------------
{
public:
	virtual void drawScrollbarBackground (CDrawContext* pContext, const CRect& size, long style, CScrollbar* bar) = 0;
	virtual void drawScrollbarScroller (CDrawContext* pContext, const CRect& size, long style, CScrollbar* bar) = 0;
};

//-----------------------------------------------------------------------------
class CScrollbar : public CControl
//! \brief a scrollbar control
/// \nosubgrouping
//-----------------------------------------------------------------------------
{
public:
	CScrollbar (const CRect& size, CControlListener* listener, long tag, long style, const CRect& scrollSize);
	virtual ~CScrollbar ();
	
	enum {
		kHorizontal,
		kVertical,
	};

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

	// overwrite
	void draw (CDrawContext* pContext);
	bool onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const long &buttons);
	CMouseEventResult onMouseDown (CPoint &where, const long& buttons);
	CMouseEventResult onMouseUp (CPoint &where, const long& buttons);
	CMouseEventResult onMouseMoved (CPoint &where, const long& buttons);
	CMessageResult notify (CBaseObject* sender, const char* message);
	void setViewSize (CRect &rect, bool invalid);

	CLASS_METHODS(CScrollbar, CControl)

	VSTGUI_DEPRECATED(virtual void mouse (CDrawContext* pContext, CPoint& where, long buttons = -1);)
//-----------------------------------------------------------------------------
protected:
	void drawBackground (CDrawContext* pContext);
	void drawScroller (CDrawContext* pContext, const CRect& size);

	void calculateScrollerLength ();
	CRect getScrollerRect ();
	void doStepping ();

	long style;
	CRect scrollSize;
	CRect scrollerArea;

	float stepValue;	
	CCoord scrollerLength;

	CColor frameColor;
	CColor scrollerColor;
	CColor backgroundColor;
	
	IScrollbarDrawer* drawer;
private:
	CVSTGUITimer* timer;
	CPoint startPoint;
	CRect scrollerRect;
	bool scrolling;
	float startValue;
};

END_NAMESPACE_VSTGUI

#endif
