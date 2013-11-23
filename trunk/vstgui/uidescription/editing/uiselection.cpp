//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#if VSTGUI_LIVE_EDITING

#include "uiselection.h"
#include "../uiviewfactory.h"
#include "../cstream.h"
#include "../../lib/cviewcontainer.h"
#include <sstream>
#include <algorithm>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
UISelection::UISelection (int32_t style)
: style (style)
{
}

//----------------------------------------------------------------------------------------------------
UISelection::~UISelection ()
{
	empty ();
}

IdStringPtr UISelection::kMsgSelectionWillChange = "kMsgSelectionWillChange";
IdStringPtr UISelection::kMsgSelectionChanged = "kMsgSelectionChanged";
IdStringPtr UISelection::kMsgSelectionViewChanged = "kMsgSelectionViewChanged";

//----------------------------------------------------------------------------------------------------
void UISelection::setStyle (int32_t _style)
{
	style = _style;
}

//----------------------------------------------------------------------------------------------------
void UISelection::add (CView* view)
{
	changed (kMsgSelectionWillChange);
	if (style == kSingleSelectionStyle)
		empty ();
	push_back (view);
	changed (kMsgSelectionChanged);
}

//----------------------------------------------------------------------------------------------------
void UISelection::remove (CView* view)
{
	if (contains (view))
	{
		changed (kMsgSelectionWillChange);
		std::list<SharedPointer<CView> >::remove (view);
		changed (kMsgSelectionChanged);
	}
}

//----------------------------------------------------------------------------------------------------
void UISelection::setExclusive (CView* view)
{
	changed (kMsgSelectionWillChange);
	DeferChanges dc (this);
	erase (std::list<SharedPointer<CView> >::begin (), std::list<SharedPointer<CView> >::end ());
	add (view);
}

//----------------------------------------------------------------------------------------------------
void UISelection::empty ()
{
	changed (kMsgSelectionWillChange);
	erase (std::list<SharedPointer<CView> >::begin (), std::list<SharedPointer<CView> >::end ());
	changed (kMsgSelectionChanged);
}

//----------------------------------------------------------------------------------------------------
bool UISelection::contains (CView* view) const
{
	return std::find (begin (), end (), view) != end ();
}

//----------------------------------------------------------------------------------------------------
bool UISelection::containsParent (CView* view) const
{
	CView* parent = view->getParentView ();
	if (parent)
	{
		if (contains (parent))
			return true;
		return containsParent (parent);
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
int32_t UISelection::total () const
{
	return (int32_t)size ();
}

//----------------------------------------------------------------------------------------------------
CView* UISelection::first () const
{
	if (size () > 0)
		return *begin ();
	return 0;
}

//----------------------------------------------------------------------------------------------------
CRect UISelection::getBounds () const
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
CRect UISelection::getGlobalViewCoordinates (CView* view)
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
void UISelection::moveBy (const CPoint& p)
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
bool UISelection::store (OutputStream& stream, UIViewFactory* viewFactory, IUIDescription* uiDescription)
{
	UIDescription* desc = dynamic_cast<UIDescription*>(uiDescription);
	if (desc)
	{
		std::list<CView*> views;
		FOREACH_IN_SELECTION(this, view)
			if (!containsParent (view))
			{
				views.push_back (view);
			}
		FOREACH_IN_SELECTION_END
		
		OwningPointer<UIAttributes> attr = new UIAttributes ();
		attr->setPointAttribute ("selection-drag-offset", dragOffset);
		return desc->storeViews (views, stream, attr);
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
bool UISelection::restore (InputStream& stream, UIViewFactory* viewFactory, IUIDescription* uiDescription)
{
	empty ();
	UIDescription* desc = dynamic_cast<UIDescription*>(uiDescription);
	if (desc)
	{
		UIAttributes* attr = 0;
		if (desc->restoreViews (stream, *this, &attr))
		{
			if (attr)
			{
				attr->getPointAttribute ("selection-drag-offset", dragOffset);
				attr->forget ();
			}
			return true;
		}
	}
	return false;
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
