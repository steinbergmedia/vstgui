// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __csearchtextedit__
#define __csearchtextedit__

#include "ctextedit.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
/** Search text edit field
 *	@ingroup new_in_4_5
 */
class CSearchTextEdit : public CTextEdit
{
public:
	CSearchTextEdit (const CRect& size, IControlListener* listener, int32_t tag, UTF8StringPtr txt = nullptr, CBitmap* background = nullptr, const int32_t style = 0);

	void setClearMarkInset (CPoint inset);
	CPoint getClearMarkInset () const;
	
	void draw (CDrawContext *pContext) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
protected:
	void drawClearMark (CDrawContext* context) const;
	CRect getClearMarkRect () const;
	CRect getTextRect () const;
	
	CRect platformGetSize () const override;
	CRect platformGetVisibleSize () const override;
	void platformTextDidChange () override;
	
	CPoint clearMarkInset {2., 2.};
};

} // namespace

#endif // __csearchtextedit__
