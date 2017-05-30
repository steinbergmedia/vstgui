// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __crowcolumnview__
#define __crowcolumnview__

#include "cviewcontainer.h"

namespace VSTGUI {

// a container view which automatically layout its child views
/** TODO: Doc 
*/
class CAutoLayoutContainerView : public CViewContainer
{
public:
	explicit CAutoLayoutContainerView (const CRect& size);

	virtual void layoutViews () = 0;

	bool attached (CView* parent) override;
	void setViewSize (const CRect& rect, bool invalid = true) override;
	bool addView (CView* pView) override;
	bool addView (CView* pView, const CRect& mouseableArea, bool mouseEnabled = true) override;
	bool addView (CView* pView, CView* pBefore) override;
	bool removeView (CView* pView, bool withForget = true) override;
	bool changeViewZOrder (CView* view, uint32_t newIndex) override;

	CLASS_METHODS_VIRTUAL(CAutoLayoutContainerView, CViewContainer)
};


//-----------------------------------------------------------------------------
// CRowColumnView Declaration
/// @brief a view container which layouts its subview as rows or columns
/// @ingroup containerviews
/// @ingroup new_in_4_1
//-----------------------------------------------------------------------------
class CRowColumnView : public CAutoLayoutContainerView
{
public:
	enum Style 
	{
		kRowStyle,				///< subviews are arranged as rows (top to bottom)
		kColumnStyle			///< subviews are arranged as columns (left to right)
	};

	enum LayoutStyle
	{
		kLeftTopEqualy,			///< subviews have the same left or top position (default)
		kCenterEqualy,			///< subviews are centered to each other
		kRightBottomEqualy,		///< subviews have the same right or bottom position
		kStretchEqualy			///< stretch subviews to the same width and height
	};

	CRowColumnView (const CRect& size, Style style = kRowStyle, LayoutStyle layoutStyle = kLeftTopEqualy, CCoord spacing = 0., const CRect& margin = CRect (0., 0., 0., 0.));

	Style getStyle () const { return style; }
	void setStyle (Style style);
	
	CCoord getSpacing () const { return spacing; }
	void setSpacing (CCoord spacing);

	const CRect& getMargin () const { return margin; }
	void setMargin (const CRect& margin);

	bool isAnimateViewResizing () const;
	void setAnimateViewResizing (bool state);

	uint32_t getViewResizeAnimationTime () const { return viewResizeAnimationTime; }
	void setViewResizeAnimationTime (uint32_t ms) { viewResizeAnimationTime = ms; }

	bool hideClippedSubviews () const;
	void setHideClippedSubviews (bool state);
	
	LayoutStyle getLayoutStyle () const { return layoutStyle; }
	void setLayoutStyle (LayoutStyle style);

	void layoutViews () override;
	bool sizeToFit () override;
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;

	CLASS_METHODS(CRowColumnView, CAutoLayoutContainerView)
protected:
	void getMaxChildViewSize (CPoint& maxSize);
	void layoutViewsEqualSize ();
	void resizeSubView (CView* view, const CRect& newSize);

	enum {
		kAnimateViewResizing = 1 << 0,
		kHideClippedSubViews = 1 << 1,
	};

	Style style;
	LayoutStyle layoutStyle;
	CCoord spacing;
	CRect margin;
	int32_t flags;
	bool layoutGuard;
	uint32_t viewResizeAnimationTime;
};

} // namespace

#endif // __crowcolumnview__
