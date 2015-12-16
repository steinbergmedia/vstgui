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

#ifndef __ctabview__
#define __ctabview__

#include "vstguifwd.h"
#include "cviewcontainer.h"
#include "cfont.h"
#include "controls/icontrollistener.h"

namespace VSTGUI {

class CTabChildView;

//-----------------------------------------------------------------------------
class CTabView : public CViewContainer, public IControlListener
//! @brief a tab view
/// @ingroup containerviews
//-----------------------------------------------------------------------------
{
public:
	enum TabPosition {
		kPositionLeft = 0,
		kPositionRight,
		kPositionTop,
		kPositionBottom
	};

	enum TabAlignment {
		kAlignCenter = 0,
		kAlignLeft,
		kAlignRight,
		kAlignTop = kAlignLeft,
		kAlignBottom = kAlignRight
	};

	CTabView (const CRect& size, CBitmap* tabBitmap, CBitmap* background = 0, TabPosition tabPosition = kPositionTop, int32_t style = 0);
	CTabView (const CRect& size, const CRect& tabSize, CBitmap* background = 0, TabPosition tabPosition = kPositionTop, int32_t style = 0);

	VSTGUI_DEPRECATED(CTabView (const CRect& size, CFrame* parent, CBitmap* tabBitmap, CBitmap* background = 0, TabPosition tabPosition = kPositionTop, int32_t style = 0);)
	VSTGUI_DEPRECATED(CTabView (const CRect& size, CFrame* parent, const CRect& tabSize, CBitmap* background = 0, TabPosition tabPosition = kPositionTop, int32_t style = 0);)
	
	//-----------------------------------------------------------------------------
	/// @name Tab View Functions
	//-----------------------------------------------------------------------------
	//@{
	virtual bool addTab (CView* view, UTF8StringPtr name = 0, CBitmap* tabBitmap = 0);	///< add a tab
	virtual bool addTab (CView* view, CControl* button);	///< add a tab
	virtual bool removeTab (CView* view);	///< remove a tab
	virtual bool removeAllTabs ();			///< remove all tabs
	virtual bool selectTab (int32_t index);	///< select tab at index
	virtual int32_t getCurrentSelectedTab () const { return currentTab; } ///< get current index of selected tab

	virtual CRect& getTabViewSize (CRect& rect) const;	///< the size of one tab

	virtual void setTabFontStyle (const CFontRef font, CCoord fontSize = 12, CColor selectedColor = kBlackCColor, CColor deselectedColor = kWhiteCColor); ///< call this after the tabs are added. Tabs added after this call will have the default font style.

	virtual void alignTabs (TabAlignment alignment = kAlignCenter); ///< call this after you have added all tabs to align them according to alignment

	virtual void setTabViewInsets (const CPoint& inset);
	//@}

	virtual void drawBackgroundRect (CDrawContext *pContext, const CRect& _updateRect) VSTGUI_OVERRIDE_VMETHOD;
	virtual void valueChanged (CControl *pControl) VSTGUI_OVERRIDE_VMETHOD;
	virtual void setViewSize (const CRect &rect, bool invalid = true) VSTGUI_OVERRIDE_VMETHOD;
	virtual void setAutosizeFlags (int32_t flags) VSTGUI_OVERRIDE_VMETHOD;
//-----------------------------------------------------------------------------
	CLASS_METHODS (CTabView, CViewContainer)
protected:
	~CTabView ();
	void setCurrentChild (CTabChildView* childView);

	int32_t numberOfChilds;
	int32_t currentTab;
	TabPosition tabPosition;
	int32_t style;
	CRect tabSize;
	CPoint tabViewInset;
	CBitmap* tabBitmap;
	CTabChildView* firstChild;
	CTabChildView* lastChild;
	CTabChildView* currentChild;
};

} // namespace

#endif
