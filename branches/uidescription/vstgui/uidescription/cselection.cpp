/*
 *  cselection.cpp
 *
 *  Created by Arne Scheffler on 12/9/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#if VSTGUI_LIVE_EDITING

#include "cselection.h"

BEGIN_NAMESPACE_VSTGUI

//----------------------------------------------------------------------------------------------------
CSelection::CSelection (int style)
: style (style)
{
}

//----------------------------------------------------------------------------------------------------
CSelection::~CSelection ()
{
	empty ();
}

const char* CSelection::kMsgSelectionChanged = "kMsgSelectionChanged";
const char* CSelection::kMsgSelectionViewChanged = "kMsgSelectionViewChanged";

//----------------------------------------------------------------------------------------------------
void CSelection::addDependent (CBaseObject* obj)
{
	dependencies.push_back (obj);
}

//----------------------------------------------------------------------------------------------------
void CSelection::removeDependent (CBaseObject* obj)
{
	dependencies.remove (obj);
}

//----------------------------------------------------------------------------------------------------
void CSelection::changed (const char* what)
{
	std::list<CBaseObject*>::iterator it = dependencies.begin ();
	while (it != dependencies.end ())
	{
		(*it)->notify (this, what);
		it++;
	}
}

//----------------------------------------------------------------------------------------------------
void CSelection::setStyle (int _style)
{
	style = _style;
}

//----------------------------------------------------------------------------------------------------
void CSelection::add (CView* view)
{
	if (style == kSingleSelectionStyle)
		empty ();
	views.push_back (view);
	changed (kMsgSelectionChanged);
	view->remember ();
}

//----------------------------------------------------------------------------------------------------
void CSelection::remove (CView* view)
{
	if (contains (view))
	{
		views.remove (view);
		changed (kMsgSelectionChanged);
		view->forget ();
	}
}

//----------------------------------------------------------------------------------------------------
void CSelection::setExclusive (CView* view)
{
	empty ();
	add (view);
	changed (kMsgSelectionChanged);
}

//----------------------------------------------------------------------------------------------------
void CSelection::empty ()
{
	std::list<CView*>::const_iterator it = views.begin ();
	while (it != views.end ())
	{
		(*it)->forget ();
		it++;
	}
	views.erase (views.begin (), views.end ());
	changed (kMsgSelectionChanged);
}

//----------------------------------------------------------------------------------------------------
bool CSelection::contains (CView* view) const
{
	std::list<CView*>::const_iterator it = views.begin ();
	while (it != views.end ())
	{
		if (*it == view)
			return true;
		it++;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
bool CSelection::containsParent (CView* view) const
{
	CView* parent = view->getParentView ();
	if (parent)
	{
		std::list<CView*>::const_iterator it = views.begin ();
		while (it != views.end ())
		{
			if (*it == parent)
				return true;
			it++;
		}
		return containsParent (parent);
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
int CSelection::total () const
{
	return views.size ();
}

//----------------------------------------------------------------------------------------------------
CView* CSelection::getFirst () const
{
	return *views.begin ();
}

//----------------------------------------------------------------------------------------------------
CView* CSelection::getNext (CView* previous) const
{
	std::list<CView*>::const_iterator it = views.begin ();
	while (it != views.end ())
	{
		if ((*it) == previous)
		{
			it++;
			if (it != views.end ())
				return *it;
			else
				return 0;
		}
		it++;
	}
	return 0;
}

//----------------------------------------------------------------------------------------------------
CRect CSelection::getBounds () const
{
	CRect result (50000, 50000, 0, 0);
	std::list<CView*>::const_iterator it = views.begin ();
	while (it != views.end ())
	{
		CRect vs = getGlobalViewCoordinates (*it);
		if (result.left > vs.left)
			result.left = vs.left;
		if (result.right < vs.right)
			result.right = vs.right;
		if (result.top > vs.top)
			result.top = vs.top;
		if (result.bottom < vs.bottom)
			result.bottom = vs.bottom;
		it++;
	}
	return result;
}

//----------------------------------------------------------------------------------------------------
CRect CSelection::getGlobalViewCoordinates (CView* view)
{
	CRect r = view->getViewSize (r);
	CPoint p;
	if (dynamic_cast<CViewContainer*>(view))
		p.offset (-r.left, -r.top);
	view->localToFrame (p);
	r.offset (p.x, p.y);
	return r;
}

//----------------------------------------------------------------------------------------------------
void CSelection::moveBy (const CPoint& p)
{
	std::list<CView*>::const_iterator it = views.begin ();
	while (it != views.end ())
	{
		if (!containsParent ((*it)))
		{
			CRect viewRect = (*it)->getViewSize (viewRect);
			viewRect.offset (p.x, p.y);
			(*it)->setViewSize (viewRect);
			(*it)->setMouseableArea (viewRect);
		}
		it++;
	}
	changed (kMsgSelectionViewChanged);
}

END_NAMESPACE_VSTGUI

#endif // VSTGUI_LIVE_EDITING
