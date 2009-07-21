/*
 *  cselection.h
 *
 *  Created by Arne Scheffler on 12/9/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __cselection__
#define __cselection__

#if VSTGUI_LIVE_EDITING

#include "../vstgui.h"
#include <list>

BEGIN_NAMESPACE_VSTGUI

//----------------------------------------------------------------------------------------------------
class CSelection : public CBaseObject, protected std::list<CView*>
//----------------------------------------------------------------------------------------------------
{
public:
	typedef std::list<CView*>::const_iterator const_iterator;
	
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

	const_iterator begin () const { return std::list<CView*>::begin (); }
	const_iterator end () const { return std::list<CView*>::end (); }

	CView* first () const;

	bool contains (CView* view) const;
	bool containsParent (CView* view) const;

	int total () const;
	CRect getBounds () const;
	static CRect getGlobalViewCoordinates (CView* view);

	void moveBy (const CPoint& p);

	void addDependent (CBaseObject* obj);
	void removeDependent (CBaseObject* obj);
	
	static const char* kMsgSelectionChanged;
	static const char* kMsgSelectionViewChanged;
	void changed (const char* what);
protected:

	std::list<CBaseObject*> dependencies;
	int style;
};

//----------------------------------------------------------------------------------------------------
#define FOREACH_IN_SELECTION(__selection, view) \
	CSelection::const_iterator __it = __selection->begin (); \
	while (__it != __selection->end ()) \
	{ \
		CView* view = (*__it);

//----------------------------------------------------------------------------------------------------
#define FOREACH_IN_SELECTION_END \
		__it++; \
	} \

END_NAMESPACE_VSTGUI

#endif // VSTGUI_LIVE_EDITING

#endif
