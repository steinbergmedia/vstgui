// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#if VSTGUI_LIVE_EDITING

#include "uiselection.h"
#include "../uiviewfactory.h"
#include "../uidescription.h"
#include "../cstream.h"
#include "../uiattributes.h"
#include "../../lib/cviewcontainer.h"
#include "../../lib/cframe.h"
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
IdStringPtr UISelection::kMsgSelectionViewWillChange = "kMsgSelectionViewWillChange";

//----------------------------------------------------------------------------------------------------
void UISelection::setStyle (int32_t _style)
{
	style = _style;
}

//----------------------------------------------------------------------------------------------------
void UISelection::add (CView* view)
{
	vstgui_assert (view, "view cannot be nullptr");
	changed (kMsgSelectionWillChange);
	if (style == kSingleSelectionStyle)
		empty ();
	emplace_back (view);
	changed (kMsgSelectionChanged);
}

//----------------------------------------------------------------------------------------------------
void UISelection::remove (CView* view)
{
	vstgui_assert (view, "view cannot be nullptr");
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
	vstgui_assert (view, "view cannot be nullptr");
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
	return nullptr;
}

//----------------------------------------------------------------------------------------------------
CRect UISelection::getBounds () const
{
	CRect result;
	if (UISelectionViewList::empty ())
		return result;
	const_iterator it = begin ();
	result = getGlobalViewCoordinates (*it);
	++it;
	while (it != end ())
	{
		CRect vs = getGlobalViewCoordinates (*it);
		result.unite (vs);
		++it;
	}
	return result;
}

//----------------------------------------------------------------------------------------------------
CRect UISelection::getGlobalViewCoordinates (CView* view)
{
	CRect result = view->translateToGlobal (view->getViewSize ());
	return view->getFrame () ? view->getFrame ()->getTransform ().inverse ().transform (result) : result;
}

//----------------------------------------------------------------------------------------------------
void UISelection::moveBy (const CPoint& p)
{
	changed (kMsgSelectionViewWillChange);
	const_iterator it = begin ();
	while (it != end ())
	{
		if (!containsParent ((*it)))
		{
			CRect viewRect = (*it)->getViewSize ();
			viewRect.offset (p.x, p.y);
			(*it)->setViewSize (viewRect);
			(*it)->setMouseableArea (viewRect);
		}
		it++;
	}
	changed (kMsgSelectionViewChanged);
}

//----------------------------------------------------------------------------------------------------
void UISelection::invalidRects () const
{
	const_iterator it = begin ();
	while (it != end ())
	{
		if (!containsParent ((*it)))
		{
			(*it)->invalid ();
		}
		it++;
	}
}

//----------------------------------------------------------------------------------------------------
bool UISelection::store (OutputStream& stream, IUIDescription* uiDescription)
{
	UIDescription* desc = dynamic_cast<UIDescription*>(uiDescription);
	if (desc)
	{
		std::list<CView*> views;
		for (auto view : *this)
		{
			if (!containsParent (view))
			{
				views.emplace_back (view);
			}
		}
		
		auto attr = makeOwned<UIAttributes> ();
		attr->setPointAttribute ("selection-drag-offset", dragOffset);
		return desc->storeViews (views, stream, attr);
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
bool UISelection::restore (InputStream& stream, IUIDescription* uiDescription)
{
	empty ();
	UIDescription* desc = dynamic_cast<UIDescription*>(uiDescription);
	if (desc)
	{
		UIAttributes* attr = nullptr;
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
