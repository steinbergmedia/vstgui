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
class CScrollView : public CViewContainer, public IControlListener, public ViewListenerAdapter
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
	CScrollView (const CRect& size, const CRect& containerSize, int32_t style, CCoord scrollbarWidth = 16, CBitmap* pBackground = nullptr);
	CScrollView (const CScrollView& scrollView);

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
	int32_t getStyle () const { return style; }
	void setStyle (int32_t newStyle);
	
	int32_t getActiveScrollbars () const { return activeScrollbarStyle; }

	CCoord getScrollbarWidth () const { return scrollbarWidth; }
	void setScrollbarWidth (CCoord width);

	/** set the virtual size of this container */
	virtual void setContainerSize (const CRect& cs, bool keepVisibleArea = false);
	const CRect& getContainerSize () const { return containerSize; }
	/** get scroll offset */
	const CPoint& getScrollOffset () const;
	void resetScrollOffset ();

	/** get the vertical scrollbar */
	CScrollbar* getVerticalScrollbar () const { return vsb; }
	/** get the horizontal scrollbar */
	CScrollbar* getHorizontalScrollbar () const { return hsb; }

	/** set scrollview to show rect */
	virtual void makeRectVisible (const CRect& rect);
	//@}

	// overwrite
	bool addView (CView* pView, CView* pBefore = nullptr) override;
	bool removeView (CView* pView, bool withForget = true) override;
	bool removeAll (bool withForget = true) override;
	uint32_t getNbViews () const override;
	CView* getView (uint32_t index) const override;
	bool changeViewZOrder (CView* view, uint32_t newIndex) override;
	void drawBackgroundRect (CDrawContext* pContext, const CRect& _updateRect) override;
	bool onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons) override;
	void valueChanged (CControl* pControl) override;
	void setTransparency (bool val) override;
	void setBackgroundColor (const CColor& color) override;
	void setViewSize (const CRect& rect, bool invalid = true) override;
	void setAutosizeFlags (int32_t flags) override;
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;

	CLASS_METHODS(CScrollView, CViewContainer)
//-----------------------------------------------------------------------------
protected:
	~CScrollView () noexcept override = default;
	virtual void recalculateSubViews ();

	void viewSizeChanged (CView* view, const CRect& oldSize) override;
	void viewWillDelete (CView* view) override;

	CScrollContainer* sc;
	CScrollbar* vsb;
	CScrollbar* hsb;

	CRect containerSize;
	CCoord scrollbarWidth;
	int32_t style;
	int32_t activeScrollbarStyle;
	bool recalculateSubViewsRecursionGard {false};
	enum {
		kHSBTag,
		kVSBTag
	};
};

} // VSTGUI
