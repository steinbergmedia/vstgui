// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"
#include "cviewcontainer.h"

namespace VSTGUI {
class CSplitViewSeparatorView;

//-----------------------------------------------------------------------------
/** @brief a split container view with separators between its child views
	@ingroup containerviews
	@ingroup new_in_4_0
*/
//-----------------------------------------------------------------------------
class CSplitView : public CViewContainer
{
public:
	enum Style {
		/** subviews will be horizontally arranged */
		kHorizontal,
		/** subviews will be vertically arranged */
		kVertical
	};

	/** Method how to resize the subviews if the size of the split view changes */
	enum ResizeMethod {
		/** only the first view will be resized */
		kResizeFirstView,
		/** only the second view will be resized */
		kResizeSecondView,
		/** only the last view will be resized */
		kResizeLastView,
		/** all views will be resized equally */
		kResizeAllViews
	};
	
	CSplitView (const CRect& size, Style style = kHorizontal, CCoord separatorWidth = 10., ISplitViewSeparatorDrawer* drawer = nullptr);
	~CSplitView () noexcept override = default;

	//-----------------------------------------------------------------------------
	/// @name CSplitView Methods
	//-----------------------------------------------------------------------------
	//@{
	/** set the style of the split view, see @ref CSplitView::Style */
	virtual void setStyle (Style s);
	/** get the style of the split view, see @ref CSplitView::Style */
	Style getStyle () const { return style; }

	/** set the resize method, see @ref CSplitView::ResizeMethod */
	virtual void setResizeMethod (ResizeMethod method);
	/** get the resize method, see @ref CSplitView::ResizeMethod */
	ResizeMethod getResizeMethod () const { return resizeMethod; }

	/** set the width of the separators */
	virtual void setSeparatorWidth (CCoord width);
	/** get the width of the separators */
	CCoord getSeparatorWidth () const { return separatorWidth; }

	ISplitViewSeparatorDrawer* getDrawer ();
	void storeViewSizes ();
	
	bool addViewToSeparator (int32_t sepIndex, CView* view);
	//@}
	
	// overrides
	bool addView (CView* pView, CView* pBefore = nullptr) override;
	bool removeView (CView* pView, bool withForget = true) override;
	bool removeAll (bool withForget = true) override;
	void setViewSize (const CRect& rect, bool invalid = true) override;
	bool sizeToFit () override;
	bool removed (CView* parent) override;
	bool attached (CView* parent) override;

	bool requestNewSeparatorSize (CSplitViewSeparatorView* separatorView, const CRect& newSize);
//-----------------------------------------------------------------------------
protected:
	void resizeFirstView (CPoint diff);
	void resizeSecondView (CPoint diff);
	void resizeLastView (CPoint diff);
	void resizeViewsEqual (CPoint diff);

	Style style;
	ResizeMethod resizeMethod;
	CCoord separatorWidth;
	ISplitViewSeparatorDrawer* separatorDrawer;
};

//-----------------------------------------------------------------------------
/** @brief Split View Controller

	controls the size of the subviews of the split view

	Extension to IController 
*/
//-----------------------------------------------------------------------------
class ISplitViewController
{
public:
	virtual ~ISplitViewController () noexcept = default;

	/** return the minimum and maximum size (width or height) of a view. */
	virtual bool getSplitViewSizeConstraint (int32_t index, CCoord& minSize, CCoord& maxSize, CSplitView* splitView) = 0;
	/** return the separator drawer. */
	virtual ISplitViewSeparatorDrawer* getSplitViewSeparatorDrawer (CSplitView* splitView) = 0;
	/** store the size of the view. */
	virtual bool storeViewSize (int32_t index, const CCoord& size, CSplitView* splitView) = 0;
	/** restore the size of the view. */
	virtual bool restoreViewSize (int32_t index, CCoord& size, CSplitView* splitView) = 0;
};

//-----------------------------------------------------------------------------
/** TODO: Doc 
*/
//-----------------------------------------------------------------------------
class ISplitViewSeparatorDrawer
{
public:
	virtual ~ISplitViewSeparatorDrawer () noexcept = default;

	enum Flags {
		kMouseOver = 1 << 0,
		kMouseDown = 1 << 1
	};
	/** TODO: Doc 
	*/
	virtual void drawSplitViewSeparator (CDrawContext* context, const CRect& size, int32_t flags, int32_t index, CSplitView* splitView) = 0;
};

} // VSTGUI
