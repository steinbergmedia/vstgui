#ifndef __uisearchtextfield__
#define __uisearchtextfield__

#include "../../lib/controls/ctextedit.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UISearchTextField : public CTextEdit
{
public:
	UISearchTextField (const CRect& size, CControlListener* listener, int32_t tag, UTF8StringPtr txt = 0, CBitmap* background = 0, const int32_t style = 0);
	
	void draw (CDrawContext *pContext);
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);
protected:
	void drawClearMark (CDrawContext* context) const;
	CRect getClearMarkRect () const;

	CRect platformGetSize () const;
	CRect platformGetVisibleSize () const;
};

} // namespace

#endif // __uisearchtextfield__
