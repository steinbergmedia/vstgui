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

//-----------------------------------------------------------------------------
class CScrollView : public CViewContainer, CControlListener
//-----------------------------------------------------------------------------
{
public:
	CScrollView (const CRect &size, const CRect &containerSize, CFrame* pParent, long style, long scrollbarWidth = 16, CBitmap* pBackground = 0);
	virtual ~CScrollView ();

	// style
	enum {
		kHorizontalScrollbar	= 1 << 1,
		kVerticalScrollbar 		= 1 << 2
	};

	virtual void setContainerSize (const CRect& cs);
	virtual void addView (CView *pView);
	virtual void drawBackgroundRect (CDrawContext *pContext, CRect& _updateRect);
	virtual bool onWheel (CDrawContext *pContext, const CPoint &where, float distance);
	virtual void valueChanged (CDrawContext *pContext, CControl *pControl);

	virtual CScrollbar* getVerticalScrollbar () const { return vsb; }
	virtual CScrollbar* getHorizontalScrollbar () const { return hsb; }
	
//-----------------------------------------------------------------------------
protected:
	CScrollContainer* sc;
	CScrollbar* hsb;
	CScrollbar* vsb;

	CRect containerSize;
	long style;

private:
	enum {
		kHSBTag,
		kVSBTag,
	};
};

//-----------------------------------------------------------------------------
class CScrollContainer : public CViewContainer
//-----------------------------------------------------------------------------
{
public:
	CScrollContainer (const CRect &size, const CRect &containerSize, CFrame *pParent, CBitmap *pBackground = 0);
	virtual ~CScrollContainer ();

	void setScrollOffset (CPoint offset, bool withRedraw = false);
	void getScrollOffset (CPoint& off) const { off = offset; } 

	CRect getContainerSize () const { return containerSize; }
	void setContainerSize (const CRect& cs);
	
	virtual bool isDirty ();

//-----------------------------------------------------------------------------
protected:
	CRect containerSize;
	CPoint offset;
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

	void setFrameColor (const CColor& color) { frameColor = color; }
	void setScrollerColor (const CColor& color) { scrollerColor = color; }
	void setBackgroundColor (const CColor& color) { backgroundColor = color; }

	virtual void draw (CDrawContext* pContext);
	virtual void mouse (CDrawContext* pContext, CPoint& where, long buttons = -1);
	virtual bool onWheel (CDrawContext *pContext, const CPoint &where, float distance);

protected:
	void drawBackground (CDrawContext* pContext);
	void drawScroller (CDrawContext* pContext, const CRect& size);

	void calculateScrollerLength ();
	CRect getScrollerRect ();

	long style;
	CRect scrollSize;
	CRect scrollerArea;

	float stepValue;	
	long scrollerLength;

	CColor frameColor;
	CColor scrollerColor;
	CColor backgroundColor;
	
	IScrollbarDrawer* drawer;
};

END_NAMESPACE_VSTGUI

#endif
