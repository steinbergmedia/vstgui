#ifndef __uicrosslines__
#define __uicrosslines__

#include "../../lib/vstguibase.h"

namespace VSTGUI {
class CView;
class UISelection;
class CDrawContext;

//----------------------------------------------------------------------------------------------------
class UICrossLines
{
public:
	enum {
		kSelectionStyle,
		kDragStyle
	};
	
	UICrossLines (CView* view, int32_t style, const CColor& background = kWhiteCColor, const CColor& foreground = kBlackCColor);
	~UICrossLines ();

	int32_t getStyle () const { return style; }

	void update (UISelection* selection);

	void update (const CPoint& point);

	void invalid ();
	
	void draw (CDrawContext* pContext);
protected:
	CView* view;
	CRect currentRect;
	int32_t style;
	
	CColor background;
	CColor foreground;
};

} // namespace

#endif // __uicrosslines__
