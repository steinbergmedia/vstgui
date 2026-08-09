// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"
#include "cviewcontainer.h"
#include "iviewlistener.h"
#include "controls/icontrollistener.h"

namespace VSTGUI {
class CScrollContainer;

//-----------------------------------------------------------------------------
// CScrollView Declaration
//! @brief a scrollable container view with scrollbars
/// @ingroup containerviews
//-----------------------------------------------------------------------------
class CScrollView : public CViewContainer,
					public ControlListenerAdapter,
					public ViewListenerAdapter
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
	CScrollView (const CRect& size, const CRect& containerSize, int32_t style,
				 CCoord scrollbarWidth = 16, const SPtr<CBitmap>& background = {});

	/** Scroll View Style Flags */
	enum CScrollViewStyle
	{
		/** add a horizontal scrollbar */
		kHorizontalScrollbar = 1 << kHorizontalScrollbarFlag,
		/** add a vertical scrollbar */
		kVerticalScrollbar = 1 << kVerticalScrollbarFlag,
		/** don't draw frame */
		kDontDrawFrame = 1 << kDontDrawFrameFlag,
		/** automatic scrolling for drag moves */
		kAutoDragScrolling = 1 << kAutoDragScollingFlag,
		/** scrollbars are overlayed of the content */
		kOverlayScrollbars = 1 << kOverlayScrollbarsFlag,
		/** scroll to focus view when focus view changes */
		kFollowFocusView = 1 << kFollowFocusViewFlag,
		/** automatically hides the scrollbar if the container size is smaller than the size of the scrollview */
		kAutoHideScrollbars = 1 << kAutoHideScrollbarsFlag
	};

	//-----------------------------------------------------------------------------
	/// @name CScrollView Methods
	//-----------------------------------------------------------------------------
	//@{
	int32_t getStyle () const;
	void setStyle (int32_t newStyle);

	int32_t getActiveScrollbars () const;

	CCoord getScrollbarWidth () const;
	void setScrollbarWidth (CCoord width);

	/** set the virtual size of this container */
	virtual void setContainerSize (const CRect& cs, bool keepVisibleArea = false);
	const CRect& getContainerSize () const;
	/** get scroll offset */
	const CPoint& getScrollOffset () const;
	/** set scroll offset */
	void setScrollOffset (CPoint newOffset);
	void resetScrollOffset ();

	/** get the vertical scrollbar */
	SPtr<CScrollbar> getVerticalScrollbar () const;
	/** get the horizontal scrollbar */
	SPtr<CScrollbar> getHorizontalScrollbar () const;

	/** set scrollview to show rect */
	virtual void makeRectVisible (const CRect& rect);

	/** calculate the maximum rect where the scrollbars are inactive */
	CRect calculateOptimalContainerSize () const;

	/** returns the visible rect of the client */
	CRect getVisibleClientRect () const;

	enum class Edge
	{
		Top,
		Left,
	};
	/** set a static edge view
	 *
	 *	An edge view can be set for the top and left edge. The top edge view will have the same
	 *	width as the scroll view. The bottom of the left edge view will always be the bottom
	 *	of the scroll view and its top will either be the bottom of the top edge view or the top of
	 *	the scroll view itself.
	 *
	 *	@param edge the edge where to place the view
	 *	@param view the view to set (if this is a nullptr then the previously set view is removed)
	 *
	 *	@ingroup new_in_4_15
	 */
	void setEdgeView (Edge edge, const SPtr<CView>& view);
	SPtr<CView> getEdgeView (Edge edge) const;
	//@}

	// overwrite
	bool attached (CViewContainer& parent) override;
	bool insertSubview (const SPtr<CView>& view, const Optional<size_t>& position) override;
	bool removeSubview (const SPtr<CView>& view) override;
	Optional<size_t> indexOfSubview (const SPtr<CView>& view) const override;
	bool removeAll () override;
	uint32_t getNbViews () const override;
	SPtr<CView> getView (uint32_t index) const override;
	bool changeViewZOrder (const SPtr<CView>& view, uint32_t newIndex) override;
	void drawBackgroundRect (CDrawContext& pContext, const CRect& _updateRect) override;
	void onMouseWheelEvent (MouseWheelEvent& event) override;
	void valueChanged (CControl& pControl) override;
	void setTransparency (bool val) override;
	void setBackgroundColor (const CColor& color) override;
	void setViewSize (const CRect& rect, bool invalid = true) override;
	void setAutosizeFlags (int32_t flags) override;
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;

	//-----------------------------------------------------------------------------
protected:
	VSTGUI_SHAREDPTR_FRIEND (CScrollView)

	~CScrollView () noexcept override;

	void recalculateLayout ();
	void preLayouting () const;
	void syncScrollbars (bool keepVisibleArea);
	void syncHScrollbar (bool keepVisibleArea);
	void syncVScrollbar (bool keepVisibleArea);

	void viewSizeChanged (CView& view, const CRect& oldSize) override;
	void viewWillDelete (CView& view) override;

	static constexpr int32_t kHSBTag = 0;
	static constexpr int32_t kVSBTag = 1;

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

} // VSTGUI
