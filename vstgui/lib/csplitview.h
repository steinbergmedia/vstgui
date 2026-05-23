// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"
#include "cviewcontainer.h"
#include "viewlayouter/baseviewlayouter.h"
#include "../uidescription/icontroller.h"

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

	CSplitView (const CRect& size, Style style = kHorizontal, CCoord separatorWidth = 10.,
				const SharedPointer<ISplitViewSeparatorDrawer>& drawer = {});
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

	SharedPointer<ISplitViewSeparatorDrawer> getDrawer ();
	void storeViewSizes ();

	bool addViewToSeparator (int32_t sepIndex, const SharedPointer<CView>& view);
	//@}
	
	// overrides
	bool insertSubview (const SharedPointer<CView>& view,
						const Optional<size_t>& position) override;
	bool removeSubview (const SharedPointer<CView>& view) override;
	bool removeAll () override;
	void setViewSize (const CRect& rect, bool invalid = true) override;
	bool sizeToFit () override;
	bool removed (CViewContainer& parent) override;
	bool attached (CViewContainer& parent) override;

	bool requestNewSeparatorSize (CSplitViewSeparatorView& separatorView, CRect newSize);
	//-----------------------------------------------------------------------------
protected:
	struct SplitViewLayouter;

	Style style;
	ResizeMethod resizeMethod;
	CCoord separatorWidth;
	SharedPointer<ISplitViewSeparatorDrawer> separatorDrawer;
};

//-----------------------------------------------------------------------------
/** @brief Split View Controller

	controls the size of the subviews of the split view

	Extension to IController 
*/
//-----------------------------------------------------------------------------
class ISplitViewController : public IControllerAddOn
{
public:
	virtual ~ISplitViewController () noexcept = default;

	/** return the minimum and maximum size (width or height) of a view. */
	virtual bool getSplitViewSizeConstraint (int32_t index, CCoord& minSize, CCoord& maxSize,
											 CSplitView& splitView) = 0;
	/** return the separator drawer. */
	virtual SharedPointer<ISplitViewSeparatorDrawer>
		getSplitViewSeparatorDrawer (CSplitView& splitView) = 0;
	/** store the size of the view. */
	virtual bool storeViewSize (int32_t index, const CCoord& size, CSplitView& splitView) = 0;
	/** restore the size of the view. */
	virtual bool restoreViewSize (int32_t index, CCoord& size, CSplitView& splitView) = 0;
};

//-----------------------------------------------------------------------------
/** TODO: Doc 
*/
//-----------------------------------------------------------------------------
class ISplitViewSeparatorDrawer : public virtual IReference
{
public:
	virtual ~ISplitViewSeparatorDrawer () noexcept = default;

	enum Flags {
		kMouseOver = 1 << 0,
		kMouseDown = 1 << 1
	};
	/** TODO: Doc 
	*/
	virtual void drawSplitViewSeparator (CDrawContext& context, const CRect& size, int32_t flags,
										 int32_t index, CSplitView& splitView) = 0;
};

} // VSTGUI
