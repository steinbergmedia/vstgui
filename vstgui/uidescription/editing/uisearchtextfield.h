#ifndef __uisearchtextfield__
#define __uisearchtextfield__

#include "../../lib/controls/ctextedit.h"

#if VSTGUI_LIVE_EDITING

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UISearchTextField : public CTextEdit
{
public:
	UISearchTextField (const CRect& size, CControlListener* listener, int32_t tag, UTF8StringPtr txt = 0, CBitmap* background = 0, const int32_t style = 0);
	
	void draw (CDrawContext *pContext) VSTGUI_OVERRIDE_VMETHOD;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
protected:
	void drawClearMark (CDrawContext* context) const;
	CRect getClearMarkRect () const;

	CRect platformGetSize () const VSTGUI_OVERRIDE_VMETHOD;
	CRect platformGetVisibleSize () const VSTGUI_OVERRIDE_VMETHOD;
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uisearchtextfield__
