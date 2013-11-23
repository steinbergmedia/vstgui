//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
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

#ifndef __cviewcontainer__
#define __cviewcontainer__

#include "cview.h"
#include "ccolor.h"
#include "cdrawcontext.h"
#include "itouchevent.h"
#include <list>

namespace VSTGUI {
class CCView;

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
	CViewContainer (const CRect& size);
	VSTGUI_DEPRECATED(CViewContainer (const CRect& size, CFrame* pParent, CBitmap* pBackground = 0);)
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
	virtual bool hasChildren () const;						///< check if container has child views
	virtual int32_t getNbViews () const;			///< get the number of child views
	virtual CView* getView (int32_t index) const;	///< get the child view at index
	virtual CView* getViewAt (const CPoint& where, bool deep = false, bool mustbeMouseEnabled = false) const;	///< get the view at point where
	virtual CViewContainer* getContainerAt (const CPoint& where, bool deep = true) const;		///< get the container at point where
	virtual bool getViewsAt (const CPoint& where, std::list<SharedPointer<CView> >& views, bool deep = true) const;	///< get all views at point where, top->down
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
	
	virtual void setBackgroundColorDrawStyle (CDrawStyle style);
	CDrawStyle getBackgroundColorDrawStyle () const { return backgroundColorDrawStyle; }
	//@}

	void modifyDrawContext (CCoord save[4], CDrawContext* pContext);
	void restoreDrawContext (CDrawContext* pContext, CCoord save[4]);

	virtual bool advanceNextFocusView (CView* oldFocus, bool reverse = false);
	virtual bool invalidateDirtyViews ();
	virtual CRect getVisibleSize (const CRect& rect) const;

	// CView
	virtual void draw (CDrawContext* pContext) VSTGUI_OVERRIDE_VMETHOD;
	virtual void drawRect (CDrawContext* pContext, const CRect& updateRect) VSTGUI_OVERRIDE_VMETHOD;
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual CMouseEventResult onMouseCancel () VSTGUI_OVERRIDE_VMETHOD;
	virtual bool onWheel (const CPoint& where, const float& distance, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool hitTest (const CPoint& where, const CButtonState& buttons = -1) VSTGUI_OVERRIDE_VMETHOD;
	virtual CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD;

#if VSTGUI_TOUCH_EVENT_HANDLING
	virtual void onTouchEvent (ITouchEvent& event) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool wantsMultiTouchEvents () const VSTGUI_OVERRIDE_VMETHOD { return true; }
	virtual void findSingleTouchEventTarget (ITouchEvent::Touch& event);
#endif

	virtual bool onDrop (IDataPackage* drag, const CPoint& where) VSTGUI_OVERRIDE_VMETHOD;
	virtual void onDragEnter (IDataPackage* drag, const CPoint& where) VSTGUI_OVERRIDE_VMETHOD;
	virtual void onDragLeave (IDataPackage* drag, const CPoint& where) VSTGUI_OVERRIDE_VMETHOD;
	virtual void onDragMove (IDataPackage* drag, const CPoint& where) VSTGUI_OVERRIDE_VMETHOD;

	virtual void looseFocus () VSTGUI_OVERRIDE_VMETHOD;
	virtual void takeFocus () VSTGUI_OVERRIDE_VMETHOD;

	virtual bool isDirty () const VSTGUI_OVERRIDE_VMETHOD;

	virtual void invalid () VSTGUI_OVERRIDE_VMETHOD;
	virtual void invalidRect (const CRect& rect) VSTGUI_OVERRIDE_VMETHOD;
	
	virtual void setViewSize (const CRect& rect, bool invalid = true) VSTGUI_OVERRIDE_VMETHOD;
	virtual void parentSizeChanged () VSTGUI_OVERRIDE_VMETHOD;
	virtual bool sizeToFit () VSTGUI_OVERRIDE_VMETHOD;

	virtual bool removed (CView* parent) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool attached (CView* parent) VSTGUI_OVERRIDE_VMETHOD;
		
	virtual CPoint& frameToLocal (CPoint& point) const VSTGUI_OVERRIDE_VMETHOD;
	virtual CPoint& localToFrame (CPoint& point) const VSTGUI_OVERRIDE_VMETHOD;

	//-----------------------------------------------------------------------------
	typedef std::list<SharedPointer<CView> >::const_iterator ChildViewConstIterator;
	typedef std::list<SharedPointer<CView> >::const_reverse_iterator ChildViewConstReverseIterator;

	//-----------------------------------------------------------------------------
	template<bool reverse>
	class Iterator
	{
	public:
		Iterator<reverse> (const CViewContainer* container) : children (container->children) { if (reverse) riterator = children.rbegin (); else iterator = children.begin (); }
		Iterator<reverse> (const Iterator& vi) : children (vi.children), iterator (vi.iterator), riterator (vi.riterator) {}
		
		Iterator<reverse>& operator++ ()
		{
			if (reverse)
				riterator++;
			else
				iterator++;
			return *this;
		}
		
		Iterator<reverse> operator++ (int)
		{
			Iterator<reverse> old (*this);
			if (reverse)
				riterator++;
			else
				iterator++;
			return old;
		}
		
		Iterator<reverse>& operator-- ()
		{
			if (reverse)
				riterator--;
			else
				iterator--;
			return *this;
		}
		
		Iterator<reverse> operator-- (int)
		{
			Iterator<reverse> old (*this);
			if (reverse)
				riterator--;
			else
				iterator--;
			return old;
		}
		
		CView* operator* () const
		{
			if (reverse)
				return riterator != children.rend () ? *riterator : 0;
			return iterator != children.end () ? *iterator : 0;
		}
		
	protected:
		const std::list<SharedPointer<CView> >& children;
		ChildViewConstIterator iterator;
		ChildViewConstReverseIterator riterator;
	};

	/** get child views of type ViewClass. ContainerClass must be a stdc++ container */
	template<class ViewClass, class ContainerClass>
	uint32_t getChildViewsOfType (ContainerClass& result, bool deep = false) const
	{
		ChildViewConstIterator it = children.begin ();
		while (it != children.end ())
		{
			ViewClass* vObj = (*it).cast<ViewClass> ();
			if (vObj)
			{
				result.push_back (vObj);
			}
			if (deep)
			{
				if (CViewContainer* container = (*it).cast<CViewContainer> ())
				{
					container->getChildViewsOfType<ViewClass, ContainerClass> (result);
				}
			}
			it++;
		}
		return result.size ();
	}

	//-------------------------------------------
	CLASS_METHODS(CViewContainer, CView)

	#if DEBUG
	virtual void dumpInfo ();
	virtual void dumpHierarchy ();
	#endif

protected:
	~CViewContainer ();
	virtual bool checkUpdateRect (CView* view, const CRect& rect);
	virtual bool hitTestSubViews (const CPoint& where, const CButtonState buttons = -1);
	VSTGUI_DEPRECATED(void drawBackToFront (CDrawContext* context, const CRect& rect);)

	enum {
		kAutosizeSubviews = 1 << (CView::kLastCViewFlag + 1)
	};

	/// @cond ignore
	std::list<SharedPointer<CView> > children;
	/// @endcond

	CDrawStyle backgroundColorDrawStyle;
	CColor backgroundColor;
	CPoint backgroundOffset;
	CRect lastDrawnFocus;

	CView* currentDragView;
	CView* mouseDownView;
};

/// @cond ignore
#ifndef FOREACHSUBVIEW
	#define FOREACHSUBVIEW ChildViewConstIterator it = children.begin (); while (it != children.end ()) { CView* pV = (*it); it++; {
#endif
#ifndef ENDFOREACHSUBVIEW
	#define ENDFOREACHSUBVIEW } }
#endif
/// @endcond

typedef CViewContainer::Iterator<false> ViewIterator;
typedef CViewContainer::Iterator<true> ReverseViewIterator;

} // namespace

#endif
