// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"
#include "cview.h"
#include "cdrawdefs.h"
#if VSTGUI_TOUCH_EVENT_HANDLING
#include "itouchevent.h"
#endif
#include <list>
#include <memory>

namespace VSTGUI {

/** Message of a view loosing focus (only CTextEdit and COptionMenu send this) */
extern IdStringPtr kMsgLooseFocus;

//-----------------------------------------------------------------------------
struct GetViewOptions
{
	enum {
		kNone					= 0,
		kDeep					= 1 << 0,
		kMouseEnabled			= 1 << 1,
		kIncludeViewContainer	= 1 << 2,
		kIncludeInvisible		= 1 << 3
	};

	explicit GetViewOptions (uint32_t options = kNone) : flags (options) {}

	GetViewOptions& deep (bool state = true) { setBit (flags, kDeep, state); return *this; }
	GetViewOptions& mouseEnabled (bool state = true) { setBit (flags, kMouseEnabled, state); return *this; }
	GetViewOptions& includeViewContainer (bool state = true) { setBit (flags, kIncludeViewContainer, state); return *this; }
	GetViewOptions& includeInvisible (bool state = true) { setBit (flags, kIncludeInvisible, state); return *this; }

	bool getDeep () const { return hasBit (flags, kDeep); }
	bool getMouseEnabled () const { return hasBit (flags, kMouseEnabled); }
	bool getIncludeViewContainer () const { return hasBit (flags, kIncludeViewContainer); }
	bool getIncludeInvisible () const { return hasBit (flags, kIncludeInvisible); }
private:
	uint32_t flags;
};

//-----------------------------------------------------------------------------
// CViewContainer Declaration
//! @brief Container Class of CView objects
/// @ingroup containerviews
//-----------------------------------------------------------------------------
class CViewContainer : public CView
{
public:
	using ViewList = std::list<SharedPointer<CView>>;

	explicit CViewContainer (const CRect& size);
	CViewContainer (const CViewContainer& viewContainer);

	//-----------------------------------------------------------------------------
	/// @name Sub View Methods
	//-----------------------------------------------------------------------------
	//@{
	/** add a child view */
	bool addView (CView* pView, const CRect& mouseableArea, bool mouseEnabled = true);
	/** add a child view before another view */
	virtual bool addView (CView* pView, CView* pBefore = nullptr);
	/** remove a child view */
	virtual bool removeView (CView* pView, bool withForget = true);
	/** remove all child views */
	virtual bool removeAll (bool withForget = true);
	/** check if pView is a child view of this container */
	bool isChild (CView* pView) const;
	/** check if pView is a child view of this container */
	virtual bool isChild (CView* pView, bool deep) const;
	/** check if container has child views */
	virtual bool hasChildren () const;
	/** get the number of child views */
	virtual uint32_t getNbViews () const;
	/** get the child view at index */
	virtual CView* getView (uint32_t index) const;
	/** get the view at point where */
	virtual CView* getViewAt (const CPoint& where, const GetViewOptions& options = GetViewOptions ()) const;
	/** get the container at point where */
	virtual CViewContainer* getContainerAt (const CPoint& where, const GetViewOptions& options = GetViewOptions ().deep ()) const;
	/** get all views at point where, top->down */
	virtual bool getViewsAt (const CPoint& where, ViewList& views, const GetViewOptions& options = GetViewOptions ().deep ()) const;
	/** change view z order position */
	virtual bool changeViewZOrder (CView* view, uint32_t newIndex);

	virtual bool hitTestSubViews (const CPoint& where, const Event& event);

	/** enable or disable autosizing subviews. Per default this is enabled. */
	virtual void setAutosizingEnabled (bool state);
	bool getAutosizingEnabled () const { return hasViewFlag (kAutosizeSubviews); }

	/** get child views of type ViewClass. ContainerClass must be a stdc++ container */
	template<class ViewClass, class ContainerClass>
	uint32_t getChildViewsOfType (ContainerClass& result, bool deep = false) const;

	/** execute proc for each child view */
	template<typename Proc>
	void forEachChild (Proc proc) const;

	//@}

	//-----------------------------------------------------------------------------
	/// @name Background Methods
	//-----------------------------------------------------------------------------
	//@{
	/** set the background color (will only be drawn if this container is not set to transparent and does not have a background bitmap) */
	virtual void setBackgroundColor (const CColor& color);
	/** get the background color */
	virtual CColor getBackgroundColor () const;
	/** set the offset of the background bitmap */
	virtual void setBackgroundOffset (const CPoint& p);
	/** get the offset of the background bitmap */
	virtual CPoint getBackgroundOffset () const;
	/** draw the background */
	virtual void drawBackgroundRect (CDrawContext* pContext, const CRect& _updateRect);
	
	virtual void setBackgroundColorDrawStyle (CDrawStyle style);
	CDrawStyle getBackgroundColorDrawStyle () const;
	//@}

	virtual bool advanceNextFocusView (CView* oldFocus, bool reverse = false);
	virtual bool invalidateDirtyViews ();
	virtual CRect getVisibleSize (const CRect& rect) const;

	void setTransform (const CGraphicsTransform& t);
	const CGraphicsTransform& getTransform () const;
	
	void registerViewContainerListener (IViewContainerListener* listener);
	void unregisterViewContainerListener (IViewContainerListener* listener);
	
	// CView
	void draw (CDrawContext* pContext) override;
	void drawRect (CDrawContext* pContext, const CRect& updateRect) override;
	void onMouseDownEvent (MouseDownEvent& event) override;
	void onMouseMoveEvent (MouseMoveEvent& event) override;
	void onMouseUpEvent (MouseUpEvent& event) override;
	void onMouseCancelEvent (MouseCancelEvent& event) override;
	void onMouseWheelEvent (MouseWheelEvent& event) override;
	void onZoomGestureEvent (ZoomGestureEvent& event) override;
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	bool onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance,
	              const CButtonState& buttons) final;
#endif
#if VSTGUI_TOUCH_EVENT_HANDLING
	virtual void onTouchEvent (ITouchEvent& event) override;
	virtual bool wantsMultiTouchEvents () const override { return true; }
	virtual bool findSingleTouchEventTarget (ITouchEvent::Touch& event);
#endif

	SharedPointer<IDropTarget> getDropTarget () override;

	void looseFocus () override;
	void takeFocus () override;

	bool isDirty () const override;

	void invalid () override;
	void invalidRect (const CRect& rect) override;
	
	void setViewSize (const CRect& rect, bool invalid = true) override;
	void parentSizeChanged () override;
	bool sizeToFit () override;

	bool removed (CView* parent) override;
	bool attached (CView* parent) override;
		
	CPoint& frameToLocal (CPoint& point) const override;
	CPoint& localToFrame (CPoint& point) const override;

	//-----------------------------------------------------------------------------
	using ChildViewConstIterator = ViewList::const_iterator;
	using ChildViewConstReverseIterator = ViewList::const_reverse_iterator;

	//-----------------------------------------------------------------------------
	template<bool reverse>
	class Iterator
	{
	public:
		using IteratorType = typename std::conditional<reverse, ChildViewConstReverseIterator,
													   ChildViewConstIterator>::type;

		explicit Iterator (const CViewContainer* container) : children (container->getChildren ())
		{
			if constexpr (reverse)
				iterator = children.rbegin ();
			else
				iterator = children.begin ();
		}

		explicit Iterator (const Iterator<reverse>& vi)
		: children (vi.children), iterator (vi.iterator)
		{
		}

		Iterator (Iterator<reverse>&& o) : children (o.children), iterator (std::move (o.iterator))
		{
		}

		Iterator<reverse>& operator++ ()
		{
			++iterator;
			return *this;
		}

		Iterator<reverse> operator++ (int)
		{
			Iterator<reverse> old (*this);
			++iterator;
			return old;
		}
		
		Iterator<reverse>& operator-- ()
		{
			--iterator;
			return *this;
		}
		
		CView* operator* () const
		{
			if constexpr (reverse)
				return (iterator == children.rend ()) ? nullptr : *iterator;
			else
				return (iterator == children.end ()) ? nullptr : *iterator;
		}
		
	protected:
		const ViewList& children;
		IteratorType iterator;
	};

	//-------------------------------------------
	CLASS_METHODS(CViewContainer, CView)

	#if DEBUG
	void dumpInfo () override;
	virtual void dumpHierarchy ();
	#endif

	CViewContainer* asViewContainer () final { return this; }
	const CViewContainer* asViewContainer () const final { return this; }

protected:
	enum {
		kAutosizeSubviews = 1 << (CView::kLastCViewFlag + 1)
	};
	
	~CViewContainer () noexcept override;
	void beforeDelete () override;
	
	virtual bool checkUpdateRect (CView* view, const CRect& rect);

	void setMouseDownView (CView* view);
	CView* getMouseDownView () const;
	
	const ViewList& getChildren () const;
private:
	void dispatchEventToSubViews (Event& event);
	
	void clearMouseDownView ();
	CRect getLastDrawnFocus () const;
	void setLastDrawnFocus (CRect r);

	struct Impl;
	std::unique_ptr<Impl> pImpl;
};

using ViewIterator = CViewContainer::Iterator<false>;
using ReverseViewIterator = CViewContainer::Iterator<true>;

//-----------------------------------------------------------------------------
template<class ViewClass, class ContainerClass>
inline uint32_t CViewContainer::getChildViewsOfType (ContainerClass& result, bool deep) const
{
	for (auto& child : getChildren ())
	{
		auto vObj = child.cast<ViewClass> ();
		if (vObj)
		{
			result.push_back (vObj);
		}
		if (deep)
		{
			if (auto container = child->asViewContainer ())
			{
				container->getChildViewsOfType<ViewClass, ContainerClass> (result);
			}
		}
	}
	return static_cast<uint32_t> (result.size ());
}

//-----------------------------------------------------------------------------
template <typename Proc>
inline void CViewContainer::forEachChild (Proc proc) const
{
	for (auto& child : getChildren ())
	{
		proc (child);
	}
}

} // VSTGUI
