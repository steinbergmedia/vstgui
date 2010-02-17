//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#if VSTGUI_LIVE_EDITING

#include "cselection.h"
#include "viewfactory.h"
#include "cstream.h"
#include "../lib/cviewcontainer.h"
#include <sstream>

namespace VSTGUI {

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
	push_back (view);
	changed (kMsgSelectionChanged);
	view->remember ();
}

//----------------------------------------------------------------------------------------------------
void CSelection::remove (CView* view)
{
	if (contains (view))
	{
		std::list<CView*>::remove (view);
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
	const_iterator it = begin ();
	while (it != end ())
	{
		(*it)->forget ();
		it++;
	}
	erase (std::list<CView*>::begin (), std::list<CView*>::end ());
	changed (kMsgSelectionChanged);
}

//----------------------------------------------------------------------------------------------------
bool CSelection::contains (CView* view) const
{
	const_iterator it = begin ();
	while (it != end ())
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
		const_iterator it = begin ();
		while (it != end ())
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
	return size ();
}

//----------------------------------------------------------------------------------------------------
CView* CSelection::first () const
{
	if (size () > 0)
		return *begin ();
	return 0;
}

//----------------------------------------------------------------------------------------------------
CRect CSelection::getBounds () const
{
	CRect result (50000, 50000, 0, 0);
	const_iterator it = begin ();
	while (it != end ())
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
	const_iterator it = begin ();
	while (it != end ())
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

//----------------------------------------------------------------------------------------------------
bool CSelection::storeAttributesForView (OutputStream& stream, ViewFactory* viewFactory, IUIDescription* uiDescription, CView* view)
{
	UIAttributes attr;
	if (viewFactory->getAttributesForView (view, uiDescription, attr))
	{
		if (!(stream << 'view')) return false;
		if (!attr.store (stream))
			return false;
		CViewContainer* container = dynamic_cast<CViewContainer*> (view);
		if (container)
		{
			long subViews = container->getNbViews ();
			if (!(stream << (int)subViews)) return false;
			for (long i = 0; i < subViews; i++)
			{
				storeAttributesForView (stream, viewFactory, uiDescription, container->getView (i));
			}
		}
		else
			if (!(stream << (int)0)) return false;
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
CView* CSelection::createView (InputStream& stream, ViewFactory* viewFactory, IUIDescription* uiDescription)
{
	int identifier;
	if (!(stream >> identifier)) return 0;
	if (identifier != 'view') return 0;
	UIAttributes attr;
	if (!attr.restore (stream)) return 0;
	CView* view = 0; 
	if (uiDescription->getController ())
		view = uiDescription->getController ()->createView (attr, uiDescription);
	if (view == 0)
		view = viewFactory->createView (attr, uiDescription);
	if (view && uiDescription->getController ())
		view = uiDescription->getController ()->verifyView (view, attr, uiDescription);
	int subViews;
	if (!(stream >> subViews)) return view;
	CViewContainer* container = view ? dynamic_cast<CViewContainer*> (view) : 0;
	for (int i = 0; i < subViews; i++)
	{
		CView* subView = createView (stream, viewFactory, uiDescription);
		if (subView)
		{
			if (container)
				container->addView (subView);
			else
				subView->forget ();
		}
	}
	return view;
}

//----------------------------------------------------------------------------------------------------
bool CSelection::store (OutputStream& stream, ViewFactory* viewFactory, IUIDescription* uiDescription)
{
	if (!(stream << 'CSEL')) return false;
	FOREACH_IN_SELECTION(this, view)
		if (!containsParent (view))
		{
			if (!(stream << 'selv')) return false;
			if (!storeAttributesForView (stream, viewFactory, uiDescription, view))
				return false;
		}
	FOREACH_IN_SELECTION_END
	if (!(stream << 'ende')) return false;
	if (!(stream << dragOffset.x)) return false;
	if (!(stream << dragOffset.y)) return false;
	return true;
}

//----------------------------------------------------------------------------------------------------
bool CSelection::restore (InputStream& stream, ViewFactory* viewFactory, IUIDescription* uiDescription)
{
	empty ();
	int identifier = 0;
	if (!(stream >> identifier)) return false;
	if (identifier == 'CSEL')
	{
		while (true)
		{
			if (!(stream >> identifier)) return false;
			if (identifier == 'ende')
				break;
			if (identifier == 'selv')
			{
				CView* view = createView (stream, viewFactory, uiDescription);
				if (view)
				{
					add (view);
					view->forget ();
				}
			}
			else
				return false;
		}
		if (!(stream >> dragOffset.x)) return false;
		if (!(stream >> dragOffset.y)) return false;
		return true;
	}
	return false;
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
