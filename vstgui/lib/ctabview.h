// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __ctabview__
#define __ctabview__

#include "vstguifwd.h"
#include "cviewcontainer.h"
#include "cfont.h"
#include "ccolor.h"
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

	CTabView (const CRect& size, CBitmap* tabBitmap, CBitmap* background = nullptr, TabPosition tabPosition = kPositionTop, int32_t style = 0);
	CTabView (const CRect& size, const CRect& tabSize, CBitmap* background = nullptr, TabPosition tabPosition = kPositionTop, int32_t style = 0);

	//-----------------------------------------------------------------------------
	/// @name Tab View Functions
	//-----------------------------------------------------------------------------
	//@{
	virtual bool addTab (CView* view, UTF8StringPtr name = nullptr, CBitmap* tabBitmap = nullptr);	///< add a tab
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

	void drawBackgroundRect (CDrawContext *pContext, const CRect& _updateRect) override;
	void valueChanged (CControl *pControl) override;
	void setViewSize (const CRect &rect, bool invalid = true) override;
	void setAutosizeFlags (int32_t flags) override;
//-----------------------------------------------------------------------------
	CLASS_METHODS (CTabView, CViewContainer)
protected:
	~CTabView () noexcept override;
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
