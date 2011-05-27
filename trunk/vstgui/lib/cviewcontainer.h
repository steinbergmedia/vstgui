//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2011, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __cviewcontainer__
#define __cviewcontainer__

#include "cview.h"
#include "ccolor.h"
#include <list>

namespace VSTGUI {
class CCView;
template<bool reverse> class CViewIterator;

#if VSTGUI_ENABLE_DEPRECATED_METHODS
extern IdStringPtr kMsgCheckIfViewContainer;	///< Message to check if View is a CViewContainer	@deprecated use dynamic_cast
#endif
extern IdStringPtr kMsgLooseFocus;				///< Message of a view loosing focus (only CTextEdit and COptionMenu send this yet)

//-----------------------------------------------------------------------------
// CViewContainer Declaration
//! @brief Container Class of CView objects
/// @ingroup containerviews
//-----------------------------------------------------------------------------
class CViewContainer : public CView
{
public:
	CViewContainer (const CRect& size, CFrame* pParent = 0, CBitmap* pBackground = 0);
	CViewContainer (const CViewContainer& viewContainer);

	//-----------------------------------------------------------------------------
	/// @name Sub View Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual bool addView (CView* pView);	///< add a child view
	virtual bool addView (CView* pView, const CRect& mouseableArea, bool mouseEnabled = true);	///< add a child view
	virtual bool addView (CView* pView, CView* pBefore);	///< add a child view before another view
	virtual bool removeView (CView* pView, bool withForget = true);	///< remove a child view
	virtual bool removeAll (bool withForget = true);	///< remove all child views
	virtual bool isChild (CView* pView) const;	///< check if pView is a child view of this container
	virtual bool isChild (CView* pView, bool deep) const;	///< check if pView is a child view of this container
	virtual int32_t getNbViews () const;			///< get the number of child views
	virtual CView* getView (int32_t index) const;	///< get the child view at index
	virtual CView* getViewAt (const CPoint& where, bool deep = false) const;	///< get the view at point where
	virtual CViewContainer* getContainerAt (const CPoint& where, bool deep = true) const;		///< get the container at point where
	virtual bool getViewsAt (const CPoint& where, std::list<CView*>& views, bool deep = true) const;	///< get all views at point where, top->down
	virtual bool changeViewZOrder (CView* view, int32_t newIndex);	///< change view z order position

	virtual void setAutosizingEnabled (bool state);					///< enable or disable autosizing subviews. Per default this is enabled.
	bool getAutosizingEnabled () const { return (viewFlags & kAutosizeSubviews) ? true : false; }
	//@}

	//-----------------------------------------------------------------------------
	/// @name Background Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setBackgroundColor (const CColor& color);	///< set the background color (will only be drawn if this container is not set to transparent and does not have a background bitmap)
	virtual CColor getBackgroundColor () const { return backgroundColor; }	///< get the background color
	virtual void setBackgroundOffset (const CPoint& p) { backgroundOffset = p; }	///< set the offset of the background bitmap
	virtual const CPoint& getBackgroundOffset () const { return backgroundOffset; }	///< get the offset of the background bitmap
	virtual void drawBackgroundRect (CDrawContext* pContext, const CRect& _updateRect);	///< draw the background
	//@}

	void modifyDrawContext (CCoord save[4], CDrawContext* pContext);
	void restoreDrawContext (CDrawContext* pContext, CCoord save[4]);

	virtual bool advanceNextFocusView (CView* oldFocus, bool reverse = false);

	// CView
	virtual void draw (CDrawContext* pContext);
	virtual void drawRect (CDrawContext* pContext, const CRect& updateRect);
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons);
	virtual bool onWheel (const CPoint& where, const float& distance, const CButtonState& buttons);
	virtual bool onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons);
	virtual bool hitTest (const CPoint& where, const CButtonState& buttons = -1);
	virtual CMessageResult notify (CBaseObject* sender, IdStringPtr message);

	virtual bool onDrop (CDragContainer* drag, const CPoint& where);
	virtual void onDragEnter (CDragContainer* drag, const CPoint& where);
	virtual void onDragLeave (CDragContainer* drag, const CPoint& where);
	virtual void onDragMove (CDragContainer* drag, const CPoint& where);

	virtual void looseFocus ();
	virtual void takeFocus ();

	virtual bool isDirty () const;

	virtual void invalid ();
	virtual void invalidRect (const CRect& rect);
	virtual bool invalidateDirtyViews ();
	
	virtual void setViewSize (const CRect& rect, bool invalid = true);
	virtual void parentSizeChanged ();
	virtual CRect getVisibleSize (const CRect rect) const;
	virtual bool sizeToFit ();

	virtual bool removed (CView* parent);
	virtual bool attached (CView* parent);
		
	virtual CPoint& frameToLocal (CPoint& point) const;
	virtual CPoint& localToFrame (CPoint& point) const;

	CLASS_METHODS(CViewContainer, CView)

	#if DEBUG
	virtual void dumpInfo ();
	virtual void dumpHierarchy ();
	#endif

	//-------------------------------------------
protected:
	~CViewContainer ();
	virtual bool checkUpdateRect (CView* view, const CRect& rect);
	virtual bool hitTestSubViews (const CPoint& where, const CButtonState buttons = -1);
	VSTGUI_DEPRECATED(void drawBackToFront (CDrawContext* context, const CRect& rect);)

	enum {
		kAutosizeSubviews = 1 << (CView::kLastCViewFlag + 1)
	};

	friend class CViewIterator<true>;
	friend class CViewIterator<false>;

	CCView  *pFirstView;
	CCView  *pLastView;
	CColor backgroundColor;
	CPoint backgroundOffset;
	CRect lastDrawnFocus;

	CView* currentDragView;
	CView* mouseDownView;
};

/// @cond ignore
//-----------------------------------------------------------------------------
// CCView Declaration
//-----------------------------------------------------------------------------
class CCView
{
public:
	CCView (CView* pView);
	~CCView ();

	CView    *pView;
	CCView   *pNext;
	CCView   *pPrevious;
};

#ifndef FOREACHSUBVIEW
	#define FOREACHSUBVIEW for (CCView* pSv = pFirstView; pSv; pSv = pSv->pNext) {CView* pV = pSv->pView;
#endif
#ifndef FOREACHSUBVIEW_REVERSE
	#define FOREACHSUBVIEW_REVERSE(reverse) for (CCView* pSv = reverse ? pLastView : pFirstView; pSv; pSv = reverse ? pSv->pPrevious : pSv->pNext) {CView* pV = pSv->pView;
#endif
#ifndef ENDFOREACHSUBVIEW
	#define ENDFOREACHSUBVIEW }
#endif

/// @endcond

//-----------------------------------------------------------------------------
template<bool reverse>
class CViewIterator
{
public:
	CViewIterator<reverse> (CViewContainer* container) : current (reverse ? container->pLastView : container->pFirstView) {}
	CViewIterator<reverse> (const CViewIterator& vi) : current (vi.current) {}

	CViewIterator<reverse>& operator++ ()
	{
		if (current)
			current = reverse ? current->pPrevious : current->pNext;
		return *this;
	}

	CViewIterator<reverse> operator++ (int)
	{
		CViewIterator<reverse> old (*this);
		if (current)
			current = reverse ? current->pPrevious : current->pNext;
		return old;
	}

	CViewIterator<reverse>& operator-- ()
	{
		if (current)
			current = reverse ? current->pNext : current->pPrevious;
		return *this;
	}
	
	CViewIterator<reverse> operator-- (int)
	{
		CViewIterator<reverse> old (*this);
		if (current)
			current = reverse ? current->pNext : current->pPrevious;
		return old;
	}
	
	CView* operator* () const
	{
		return current ? current->pView : 0;
	}

protected:
	CCView* current;
};

typedef CViewIterator<false> ViewIterator;
typedef CViewIterator<true> ReverseViewIterator;

} // namespace

#endif
