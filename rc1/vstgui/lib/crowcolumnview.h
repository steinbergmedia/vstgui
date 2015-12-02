//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins :
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
	CAutoLayoutContainerView (const CRect& size);

	virtual void layoutViews () = 0;

	bool attached (CView* parent) VSTGUI_OVERRIDE_VMETHOD;
	void setViewSize (const CRect& rect, bool invalid = true) VSTGUI_OVERRIDE_VMETHOD;
	bool addView (CView* pView) VSTGUI_OVERRIDE_VMETHOD;
	bool addView (CView* pView, const CRect& mouseableArea, bool mouseEnabled = true) VSTGUI_OVERRIDE_VMETHOD;
	bool addView (CView* pView, CView* pBefore) VSTGUI_OVERRIDE_VMETHOD;
	bool removeView (CView* pView, bool withForget = true) VSTGUI_OVERRIDE_VMETHOD;
	bool changeViewZOrder (CView* view, uint32_t newIndex) VSTGUI_OVERRIDE_VMETHOD;

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

	bool isAnimateViewResizing () const { return animateViewResizing; }
	void setAnimateViewResizing (bool state) { animateViewResizing = state; }

	uint32_t getViewResizeAnimationTime () const { return viewResizeAnimationTime; }
	void setViewResizeAnimationTime (uint32_t ms) { viewResizeAnimationTime = ms; }

	LayoutStyle getLayoutStyle () const { return layoutStyle; }
	void setLayoutStyle (LayoutStyle style);

	void layoutViews () VSTGUI_OVERRIDE_VMETHOD;
	bool sizeToFit () VSTGUI_OVERRIDE_VMETHOD;
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD;

	CLASS_METHODS(CRowColumnView, CAutoLayoutContainerView)
protected:
	void getMaxChildViewSize (CPoint& maxSize);
	void layoutViewsEqualSize ();
	void resizeSubView (CView* view, const CRect& newSize);

	Style style;
	LayoutStyle layoutStyle;
	CCoord spacing;
	CRect margin;
	bool animateViewResizing;
	bool layoutGuard;
	uint32_t viewResizeAnimationTime;
};

} // namespace

#endif // __crowcolumnview__
