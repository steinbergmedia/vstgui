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
#include "../../lib/ccolor.h"
#include "../../lib/coffscreencontext.h"
#include "../../lib/clayeredviewcontainer.h"
#include "../../lib/cbitmap.h"
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
	clear ();
}

//----------------------------------------------------------------------------------------------------
auto UISelection::begin () const -> const_iterator
{
	return viewList.begin ();
}

//----------------------------------------------------------------------------------------------------
auto  UISelection::end () const -> const_iterator
{
	return viewList.end ();
}

//----------------------------------------------------------------------------------------------------
auto UISelection::rbegin () const -> const_reverse_iterator
{
	return viewList.rbegin ();
}

//----------------------------------------------------------------------------------------------------
auto UISelection::rend () const -> const_reverse_iterator
{
	return viewList.rend ();
}

//----------------------------------------------------------------------------------------------------
void UISelection::setStyle (int32_t _style)
{
	style = _style;
}

//----------------------------------------------------------------------------------------------------
void UISelection::add (CView* view)
{
	vstgui_assert (view, "view cannot be nullptr");
	willChange ();
	if (style == kSingleSelectionStyle)
		clear ();
	viewList.emplace_back (view);
	didChange ();
}

//----------------------------------------------------------------------------------------------------
void UISelection::remove (CView* view)
{
	vstgui_assert (view, "view cannot be nullptr");
	if (contains (view))
	{
		willChange ();
		viewList.remove (view);
		didChange ();
	}
}

//----------------------------------------------------------------------------------------------------
void UISelection::setExclusive (CView* view)
{
	vstgui_assert (view, "view cannot be nullptr");
	if (viewList.size () == 1 && viewList.front () == view)
		return;
	UISelection::DeferChange dc (*this);
	viewList.clear ();
	add (view);
}

//----------------------------------------------------------------------------------------------------
void UISelection::clear ()
{
	willChange ();
	viewList.clear ();
	didChange ();
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
	return (int32_t)viewList.size ();
}

//----------------------------------------------------------------------------------------------------
CView* UISelection::first () const
{
	if (!viewList.empty ())
		return *begin ();
	return nullptr;
}

//----------------------------------------------------------------------------------------------------
CRect UISelection::getBounds () const
{
	CRect result;
	if (viewList.empty ())
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
	if (auto frame = view->getFrame ())
		return frame->getTransform ().inverse ().transform (result);
	return result;
}

//----------------------------------------------------------------------------------------------------
void UISelection::moveBy (const CPoint& p)
{
	viewsWillChange ();
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
	viewsDidChange ();
}

//----------------------------------------------------------------------------------------------------
void UISelection::sizeBy (const CRect& r)
{
	viewsWillChange ();
	for (auto view : *this)
	{
		auto viewSize = view->getViewSize ();
		viewSize.left += r.left;
		viewSize.top += r.top;
		viewSize.right += r.right;
		viewSize.bottom += r.bottom;
		view->setViewSize (viewSize);
		view->setMouseableArea (viewSize);
	}
	viewsDidChange ();
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
void UISelection::willChange ()
{
	if (++inChange == 1)
		forEachListener ([this] (IUISelectionListener* l) { l->selectionWillChange (this); });
}

//----------------------------------------------------------------------------------------------------
void UISelection::didChange ()
{
	if (--inChange == 0)
		forEachListener ([this] (IUISelectionListener* l) { l->selectionDidChange (this); });
}

//----------------------------------------------------------------------------------------------------
void UISelection::viewsWillChange ()
{
	if (++inViewsChange == 1)
	{
		invalidRects ();
		forEachListener ([this] (IUISelectionListener* l) { l->selectionViewsWillChange (this); });
	}
}

//----------------------------------------------------------------------------------------------------
void UISelection::viewsDidChange ()
{
	if (--inViewsChange == 0)
	{
		invalidRects ();
		forEachListener ([this] (IUISelectionListener* l) { l->selectionViewsDidChange (this); });
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
	clear ();
	UIDescription* desc = dynamic_cast<UIDescription*>(uiDescription);
	if (desc)
	{
		UIAttributes* attr = nullptr;
		if (desc->restoreViews (stream, viewList, &attr))
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

//----------------------------------------------------------------------------------------------------
SharedPointer<CBitmap> createBitmapFromSelection (UISelection* selection, CFrame* frame,
                                                  CViewContainer* anchorView)
{
	CRect selectionRect = selection->getBounds ();
	auto scaleFactor = frame->getScaleFactor ();
	auto bitmap = renderBitmapOffscreen (selectionRect.getSize (), scaleFactor, [&] (auto& context) {
		CGraphicsTransform tm;
		CGraphicsTransform invTm;
		if (anchorView)
		{
			tm = anchorView->getTransform ();
			invTm = tm.inverse ();
			invTm.transform (selectionRect);
			tm = tm * CGraphicsTransform ().translate (-selectionRect.left, -selectionRect.top);
		}
		double originalZoom = 1.;
		if (anchorView && anchorView->isAttached ())
		{
			// reset frame zoom
			originalZoom = anchorView->getFrame ()->getZoom ();
			anchorView->getFrame ()->setZoom (1.);
		}
		CDrawContext::Transform tr (context, tm);
		for (auto view : *selection)
		{
			if (!selection->containsParent (view))
			{
				CPoint p;
				p = view->translateToGlobal (p);
				if (anchorView)
					invTm.transform (p);
				CDrawContext::Transform transform (context,
				                                   CGraphicsTransform ().translate (p.x, p.y));
				context.setClipRect (view->getViewSize ());
				if (IPlatformViewLayerDelegate* layer = view.cast<IPlatformViewLayerDelegate> ())
				{
					CRect r (view->getViewSize ());
					r.originize ();
					layer->drawViewLayer (&context, r);
				}
				else
				{
					view->drawRect (&context, view->getViewSize ());
				}
			}
		}
		if (anchorView && anchorView->isAttached ())
		{
			anchorView->getFrame ()->setZoom (originalZoom);
		}
	});
	return bitmap;
}

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
