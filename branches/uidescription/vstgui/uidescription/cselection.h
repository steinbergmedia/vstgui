/*
 *  cselection.h
 *
 *  Created by Arne Scheffler on 12/9/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __cselection__
#define __cselection__

#include "../vstgui.h"
#include <list>

BEGIN_NAMESPACE_VSTGUI

//----------------------------------------------------------------------------------------------------
class CSelection : public CBaseObject
//----------------------------------------------------------------------------------------------------
{
public:
	enum {
		kMultiSelectionStyle,
		kSingleSelectionStyle
	};
	
	CSelection (int style = kMultiSelectionStyle);
	~CSelection ();

	void setStyle (int style);

	void add (CView* view);
	void remove (CView* view);
	void setExclusive (CView* view);
	void empty ();

	CView* getFirst () const;
	CView* getNext (CView* previous) const;

	bool contains (CView* view) const;
	bool containsParent (CView* view) const;

	int total () const;
	CRect getBounds () const;
	static CRect getGlobalViewCoordinates (CView* view);

	void moveBy (const CPoint& p);
	
	void changed ();
protected:

	std::list<CView*> views;
	int style;
};

//----------------------------------------------------------------------------------------------------
#define FOREACH_IN_SELECTION(selection, view) \
{ \
	CView* _view = selection->getFirst (); \
	while (_view) \
	{ \
		CView* view = _view;

//----------------------------------------------------------------------------------------------------
#define FOREACH_IN_SELECTION_END \
		_view = selection->getNext (_view); \
	} \
} \

END_NAMESPACE_VSTGUI

#endif
