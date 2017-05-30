// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __cscrollview__
#define __cscrollview__

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
class CScrollView : public CViewContainer, public IControlListener, public IViewListenerAdapter
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
	bool addView (CView* pView) override;
	bool addView (CView* pView, const CRect& mouseableArea, bool mouseEnabled = true) override;
	bool addView (CView* pView, CView* pBefore) override;
	bool removeView (CView* pView, bool withForget = true) override;
	bool removeAll (bool withForget = true) override;
	uint32_t getNbViews () const override;
	CView* getView (uint32_t index) const override;
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

} // namespace

#endif
