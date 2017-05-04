// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __uicrosslines__
#define __uicrosslines__

#include "../../lib/vstguifwd.h"

#if VSTGUI_LIVE_EDITING

#include "../../lib/crect.h"
#include "../../lib/ccolor.h"
#include "../../lib/cview.h"
#include "../../lib/iviewlistener.h"

namespace VSTGUI {
class UISelection;

//----------------------------------------------------------------------------------------------------
class UICrossLines : public CView, public IViewListenerAdapter
{
public:
	enum {
		kSelectionStyle,
		kDragStyle
	};
	
	UICrossLines (CViewContainer* editView, int32_t style, const CColor& background = kWhiteCColor, const CColor& foreground = kBlackCColor);
	~UICrossLines () override;

	int32_t getStyle () const { return style; }

	void update (UISelection* selection);
	void update (const CPoint& point);
	void invalid () override;
	void draw (CDrawContext* pContext) override;
protected:
	void drawLines (CDrawContext* pContext, const CRect& size, const CRect& selectionSize);
	void viewSizeChanged (CView* view, const CRect& oldSize) override;

	CViewContainer* editView;
	CRect currentRect;
	int32_t style;
	
	CColor background;
	CColor foreground;
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uicrosslines__
