#ifndef __pprimitivesviews__
#define __pprimitivesviews__

#ifndef __vstgui__
#include "vstgui.h"
#endif

class PLinesView : public CView
{
public:
	PLinesView (const CRect& size);
	
	virtual void draw (CDrawContext *pContext);
};

class PRectsView : public CView
{
public:
	PRectsView (const CRect& size);
	
	virtual void draw (CDrawContext *pContext);
};

#endif
