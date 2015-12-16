//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __csplitview__
#define __csplitview__

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
		kHorizontal,			///< subviews will be horizontally arranged
		kVertical				///< subviews will be vertically arranged
	};

	/** Method how to resize the subviews if the size of the split view changes */
	enum ResizeMethod {
		kResizeFirstView,		///< only the first view will be resized
		kResizeSecondView,		///< only the second view will be resized
		kResizeLastView,		///< only the last view will be resized
		kResizeAllViews			///< all views will be resized equally
	};
	
	CSplitView (const CRect& size, Style style = kHorizontal, CCoord separatorWidth = 10., ISplitViewSeparatorDrawer* drawer = 0);
	~CSplitView ();

	//-----------------------------------------------------------------------------
	/// @name CSplitView Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setStyle (Style s);								///< set the style of the split view, see @ref CSplitView::Style
	Style getStyle () const { return style; }						///< get the style of the split view, see @ref CSplitView::Style

	virtual void setResizeMethod (ResizeMethod method);				///< set the resize method, see @ref CSplitView::ResizeMethod
	ResizeMethod getResizeMethod () const { return resizeMethod; }	///< get the resize method, see @ref CSplitView::ResizeMethod

	virtual void setSeparatorWidth (CCoord width);					///< set the width of the separators
	CCoord getSeparatorWidth () const { return separatorWidth; }	///< get the width of the separators

	ISplitViewSeparatorDrawer* getDrawer ();
	void storeViewSizes ();
	
	bool addViewToSeparator (int32_t sepIndex, CView* view);
	//@}
	
	// overrides
	virtual bool addView (CView* pView) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool addView (CView* pView, const CRect& mouseableArea, bool mouseEnabled = true) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool addView (CView* pView, CView* pBefore) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool removeView (CView* pView, bool withForget = true) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool removeAll (bool withForget = true) VSTGUI_OVERRIDE_VMETHOD;
	virtual void setViewSize (const CRect& rect, bool invalid = true) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool sizeToFit () VSTGUI_OVERRIDE_VMETHOD;
	virtual bool removed (CView* parent) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool attached (CView* parent) VSTGUI_OVERRIDE_VMETHOD;

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
	enum Flags {
		kMouseOver = 1 << 0,
		kMouseDown = 1 << 1
	};
	/** TODO: Doc 
	*/
	virtual void drawSplitViewSeparator (CDrawContext* context, const CRect& size, int32_t flags, int32_t index, CSplitView* splitView) = 0;
};

}

#endif // __csplitview__
