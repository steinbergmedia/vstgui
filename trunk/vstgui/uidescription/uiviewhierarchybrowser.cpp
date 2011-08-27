//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2011, Steinberg Media Technologies, All Rights Reserved
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

#include "uiviewhierarchybrowser.h"
#include "uiviewfactory.h"
#include "uiviewcreator.h"
#include "uieditframe.h"
#include "editingcolordefs.h"
#include "../lib/cdatabrowser.h"
#include "../lib/vstkeycode.h"
#include "../lib/cfont.h"
#include "../lib/cdrawcontext.h"
#include "editing/uiactions.h"
#include <typeinfo>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class UIViewHierarchyData : public IDataBrowser
{
public:
	UIViewHierarchyData (UIViewHierarchyBrowser* parent, UIDescription* description, IActionOperator* actionOperator);

	int32_t dbGetNumRows (CDataBrowser* browser);
	int32_t dbGetNumColumns (CDataBrowser* browser);
	bool dbGetColumnDescription (int32_t index, CCoord& minWidth, CCoord& maxWidth, CDataBrowser* browser);
	CCoord dbGetCurrentColumnWidth (int32_t index, CDataBrowser* browser);
	void dbSetCurrentColumnWidth (int32_t index, const CCoord& width, CDataBrowser* browser);
	CCoord dbGetRowHeight (CDataBrowser* browser);
	bool dbGetLineWidthAndColor (CCoord& width, CColor& color, CDataBrowser* browser);
	void dbDrawHeader (CDrawContext* context, const CRect& size, int32_t column, int32_t flags, CDataBrowser* browser);
	void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser);
	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser);
	CMouseEventResult dbOnMouseMoved (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser);
	CMouseEventResult dbOnMouseUp (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser);
	void dbSelectionChanged (CDataBrowser* browser);
	void dbCellTextChanged (int32_t row, int32_t column, UTF8StringPtr newText, CDataBrowser* browser) {}
	void dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* textEditControl, CDataBrowser* browser) {}
	int32_t dbOnKeyDown (const VstKeyCode& key, CDataBrowser* browser);
protected:
	void doMoveOperation (int32_t row, bool up, CDataBrowser* browser);

	UIViewHierarchyBrowser* parent;
	UIDescription* description;
	IActionOperator* actionOperator;
};

//-----------------------------------------------------------------------------
UIViewHierarchyData::UIViewHierarchyData (UIViewHierarchyBrowser* parent, UIDescription* description, IActionOperator* actionOperator)
: parent (parent)
, description (description)
, actionOperator (actionOperator)
{
}

//-----------------------------------------------------------------------------
int32_t UIViewHierarchyData::dbGetNumRows (CDataBrowser* browser)
{
	if (parent->getCurrentView () == 0)
		return 0;
	return parent->getCurrentView ()->getNbViews ();
}

//-----------------------------------------------------------------------------
int32_t UIViewHierarchyData::dbGetNumColumns (CDataBrowser* browser)
{
	return 3;
}

//-----------------------------------------------------------------------------
bool UIViewHierarchyData::dbGetColumnDescription (int32_t index, CCoord& minWidth, CCoord& maxWidth, CDataBrowser* browser)
{
	return false;
}

//-----------------------------------------------------------------------------
CCoord UIViewHierarchyData::dbGetCurrentColumnWidth (int32_t index, CDataBrowser* browser)
{
	CCoord scrollbarWidth = 0;
	if (browser->getVerticalScrollbar ())
		scrollbarWidth = browser->getVerticalScrollbar ()->getWidth ();
	if (index == 0)
		return browser->getWidth () - scrollbarWidth - 50;
	return 25;
}

//-----------------------------------------------------------------------------
void UIViewHierarchyData::dbSetCurrentColumnWidth (int32_t index, const CCoord& width, CDataBrowser* browser)
{
}

//-----------------------------------------------------------------------------
CCoord UIViewHierarchyData::dbGetRowHeight (CDataBrowser* browser)
{
	return 20;
}

//-----------------------------------------------------------------------------
bool UIViewHierarchyData::dbGetLineWidthAndColor (CCoord& width, CColor& color, CDataBrowser* browser)
{
	width = 1;
	color = uidDataBrowserLineColor;
	return true;
}

//-----------------------------------------------------------------------------
void UIViewHierarchyData::dbDrawHeader (CDrawContext* context, const CRect& size, int32_t column, int32_t flags, CDataBrowser* browser)
{
}

//-----------------------------------------------------------------------------
void UIViewHierarchyData::dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser)
{
	if (parent->getCurrentView () == 0)
		return;
	if (browser->getSelectedRow () == row)
	{
		context->setFillColor (uidDataBrowserSelectionColor);
		context->drawRect (size, kDrawFilled);
	}
	if (column == 0)
	{
		CView* view = parent->getCurrentView ()->getView (row);
		if (view)
		{
			IdStringPtr viewname = 0;
			UIViewFactory* factory = description ? dynamic_cast<UIViewFactory*> (description->getViewFactory ()) : 0;
			if (factory)
				viewname = factory->getViewName (view);
			if (viewname == 0)
				viewname = typeid(*view).name ();
			if (dynamic_cast<CViewContainer*> (view))
			{
				context->setFontColor (kWhiteCColor);
			}
			else
				context->setFontColor (kGreyCColor);
			context->setFont (kNormalFont);
			context->drawString (viewname, size);
		}
	}
	else
	{
		CRect r (size);
		if (r.getWidth () > r.getHeight ())
		{
			CCoord diff = r.getWidth () - r.getHeight ();
			r.setWidth (r.getWidth () - diff);
			r.offset (diff/2, 0);
		}
		else if (r.getHeight () > r.getWidth ())
		{
			CCoord diff = r.getHeight () - r.getWidth ();
			r.setHeight (r.getHeight () - diff);
			r.offset (0, diff/2);
		}
		r.inset (6, 6);
		CPoint polygon[4];
		if (column == 1)
		{
			polygon[0] = CPoint (r.left, r.bottom);
			polygon[1] = CPoint (r.left + r.getWidth () / 2, r.top);
			polygon[2] = CPoint (r.right, r.bottom);
		}
		else
		{
			polygon[0] = CPoint (r.left, r.top);
			polygon[1] = CPoint (r.left + r.getWidth () / 2, r.bottom);
			polygon[2] = CPoint (r.right, r.top);
		}
		polygon[3] = polygon[0];
		context->setDrawMode (kAntiAliasing);
		if ((row == 0 && column == 1) || (row == dbGetNumRows (browser) -1 && column == 2))
			context->setFillColor (MakeCColor (255, 255, 255, 50));
		else
			context->setFillColor (kWhiteCColor);
		context->drawPolygon (polygon, 4, kDrawFilled);
	}
}

//-----------------------------------------------------------------------------
void UIViewHierarchyData::doMoveOperation (int32_t row, bool up, CDataBrowser* browser)
{
	if (parent->getCurrentView ())
	{
		if (!(row == 0 && up) && !(row == dbGetNumRows (browser)-1 && !up))
		{
			if (actionOperator)
				actionOperator->performAction (new HierarchyMoveViewOperation (parent->getCurrentView ()->getView (row), 0, up));
			browser->setSelectedRow (row + (up ? -1 : 1), true);
		}
	}
}

//-----------------------------------------------------------------------------
CMouseEventResult UIViewHierarchyData::dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser)
{
	if (parent->getCurrentView ())
	{
		if (column == 0)
		{
			if (buttons & kDoubleClick)
			{
				CViewContainer* view = dynamic_cast<CViewContainer*> (parent->getCurrentView ()->getView (row));
				if (view)
					parent->setCurrentView (view);
			}
		}
		else
		{
			doMoveOperation (row, column == 1, browser);
		}
	}
	return kMouseEventHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult UIViewHierarchyData::dbOnMouseMoved (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser)
{
	return kMouseEventHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult UIViewHierarchyData::dbOnMouseUp (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser)
{
	return kMouseEventHandled;
}

//-----------------------------------------------------------------------------
int32_t UIViewHierarchyData::dbOnKeyDown (const VstKeyCode& key, CDataBrowser* browser)
{
	if (parent->getCurrentView ())
	{
		if (key.virt == VKEY_RETURN && key.modifier == 0)
		{
			CViewContainer* view = dynamic_cast<CViewContainer*> (parent->getCurrentView ()->getView (browser->getSelectedRow ()));
			if (view)
			{
				parent->setCurrentView (view);
				return 1;
			}
		}
		else if (key.virt == VKEY_BACK && key.modifier == 0)
		{
			CViewContainer* view = dynamic_cast<CViewContainer*> (parent->getCurrentView ()->getParentView ());
			if (view)
			{
				CView* currentView = parent->getCurrentView ();
				parent->setCurrentView (view);
				int32_t i = 0;
				ViewIterator it (view);
				while (*it)
				{
					if (*it == currentView)
					{
						browser->setSelectedRow (i, true);
						break;
					}
					++it;
					++i;
				}
				return 1;
			}
		}
		else if (key.virt == VKEY_UP && key.modifier == MODIFIER_CONTROL)
		{
			doMoveOperation (browser->getSelectedRow (), true, browser);
			return 1;
		}
		else if (key.virt == VKEY_DOWN && key.modifier == MODIFIER_CONTROL)
		{
			doMoveOperation (browser->getSelectedRow (), false, browser);
			return 1;
		}
	}
	return -1;
}

//-----------------------------------------------------------------------------
void UIViewHierarchyData::dbSelectionChanged (CDataBrowser* browser)
{
	if (actionOperator && parent->getCurrentView ())
	{
		actionOperator->makeSelection (parent->getCurrentView ()->getView (browser->getSelectedRow ()));
	}
}

//-----------------------------------------------------------------------------
class ViewHierarchyPathView : public CView
{
public:
	ViewHierarchyPathView (const CRect& size, UIViewHierarchyBrowser* browser, UIViewFactory* viewFactory);
	~ViewHierarchyPathView ();

	CMouseEventResult onMouseDown (CPoint &where, const CButtonState& buttons);
	void draw (CDrawContext* context);
	void setViewSize (const CRect &rect, bool invalid) { CView::setViewSize (rect, invalid); needCompute = true; }

	void setHierarchyDirty () { needCompute = true; invalid (); }
protected:
	class PathElement
	{
	public:
		PathElement (const PathElement& pe)
		: view (pe.view), name (pe.name), nameWidth (pe.nameWidth), drawWidth (pe.drawWidth) {}
		
		PathElement (UIViewFactory* factory, CViewContainer* view, CDrawContext* context)
		: view (view)
		{
			IdStringPtr viewname = 0;
			if (factory)
				viewname = factory->getViewName (view);
			if (viewname == 0)
				viewname = typeid(*view).name ();
			name = viewname;
			drawWidth = nameWidth = context->getStringWidth (name.c_str ());
		}
		bool operator==(const PathElement& pe) const { return pe.view == view; }
		void setDrawWidth (CCoord w) { drawWidth = w; }

		IdStringPtr getName () const { return name.c_str (); }
		CViewContainer* getView () const { return view; }
		CCoord getNameWidth () const { return nameWidth; }
		CCoord getDrawWidth () const { return drawWidth; }
	protected:
		CViewContainer* view;
		std::string name;
		CCoord nameWidth;
		CCoord drawWidth;
	};

	void compute (CDrawContext* context);
	void drawPathElement (const CRect& size, const PathElement& element, CDrawContext* context, bool isLast);

	UIViewHierarchyBrowser* browser;
	UIViewFactory* viewFactory;
	std::list<PathElement> elements;
	CCoord margin;
	bool needCompute;

	typedef std::list<PathElement>::iterator elements_iterator;
	typedef std::list<PathElement>::const_iterator const_elements_iterator;
};

//-----------------------------------------------------------------------------
ViewHierarchyPathView::ViewHierarchyPathView (const CRect& size, UIViewHierarchyBrowser* browser, UIViewFactory* viewFactory)
: CView (size)
, browser (browser)
, viewFactory (viewFactory)
, margin (10)
, needCompute (true)
{
}

//-----------------------------------------------------------------------------
ViewHierarchyPathView::~ViewHierarchyPathView ()
{
}

//-----------------------------------------------------------------------------
void ViewHierarchyPathView::compute (CDrawContext* context)
{
	elements.clear ();
	elements.push_back (PathElement (viewFactory, browser->getCurrentView (), context));
	CViewContainer* container = browser->getCurrentView ();
	while (container != browser->getBaseView ())
	{
		container = dynamic_cast<CViewContainer*> (container->getParentView ());
		if (container)
			elements.push_back (PathElement (viewFactory, container, context));
		else
			return; // should never happen
	}
	CCoord totalWidth = 0;
	const_elements_iterator it = elements.begin ();
	while (it != elements.end ())
	{
		totalWidth += (*it).getNameWidth () + margin;
		it++;
	}
	if (totalWidth > getWidth ())
	{
		CCoord lessen = (totalWidth - getWidth ()) / (elements.size () - 1);
		elements_iterator it = elements.begin ();
		it++;
		while (it != elements.end ())
		{
			(*it).setDrawWidth ((*it).getDrawWidth () - lessen);
			it++;
		}
	}
	elements.reverse ();
}

//-----------------------------------------------------------------------------
void ViewHierarchyPathView::drawPathElement (const CRect& size, const PathElement& element, CDrawContext* context, bool isLast)
{
	CRect r (size);
	CPoint polygon[6];
	if (isLast)
	{
		CCoord right = r.right - margin;
		right += 4;
		polygon[0] = CPoint (r.left, r.top);
		polygon[1] = CPoint (right, r.top);
		polygon[2] = CPoint (right, r.bottom);
		polygon[3] = CPoint (r.left, r.bottom);
		polygon[4] = polygon[0];
	}
	else
	{
		polygon[0] = CPoint (r.left, r.top);
		polygon[1] = CPoint (r.right - margin, r.top);
		polygon[2] = CPoint (r.right, r.top + r.getHeight () / 2);
		polygon[3] = CPoint (r.right - margin, r.bottom);
		polygon[4] = CPoint (r.left, r.bottom);
		polygon[5] = polygon[0];
	}
	context->drawPolygon (polygon, isLast ? 5 : 6, kDrawFilledAndStroked);
	r.right -= margin;
	r.offset (2, 0);
	context->drawString (element.getName (), r);
}

//-----------------------------------------------------------------------------
void ViewHierarchyPathView::draw (CDrawContext* context)
{
	setDirty (false);

	if (!browser->getCurrentView ())
		return;

	context->setFrameColor (MakeCColor (255, 255, 255, 40));
	context->setFillColor (MakeCColor (255, 255, 255, 20));
	context->setFontColor (kWhiteCColor);
	context->setFont (kNormalFont);
	context->setDrawMode (kAntiAliasing);
	context->setLineWidth (1);

	if (needCompute)
		compute (context);

	CRect r (getViewSize ());
	const_elements_iterator it = elements.begin ();
	while (it != elements.end ())
	{
		r.setWidth ((*it).getDrawWidth () + margin);
		context->setClipRect (r);
		drawPathElement (r, (*it), context, (*it) == elements.back ());
		r.offset (r.getWidth (), 0);
		it++;
	}
}

//-----------------------------------------------------------------------------
CMouseEventResult ViewHierarchyPathView::onMouseDown (CPoint &where, const CButtonState& buttons)
{
	CRect r (getViewSize ());
	const_elements_iterator it = elements.begin ();
	while (it != elements.end ())
	{
		r.setWidth ((*it).getDrawWidth () + margin);
		if (r.pointInside (where))
		{
			browser->setCurrentView ((*it).getView ());
			break;
		}
		r.offset (r.getWidth (), 0);
		it++;
	}
	
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UIViewHierarchyBrowser::UIViewHierarchyBrowser (const CRect& rect, CViewContainer* baseView, UIDescription* description, IActionOperator* actionOperator)
: CViewContainer (rect, 0)
, baseView (baseView)
, currentView (baseView)
, browser (0)
, data (0)
, pathView (0)
{
	setTransparency (true);
	data = new UIViewHierarchyData (this, description, actionOperator);
	CRect r2 (rect);
	r2.offset (-r2.left, -r2.top);
	r2.setHeight (25);
	r2.inset (1, 4);
	pathView = new ViewHierarchyPathView (r2, this, description ? dynamic_cast<UIViewFactory*>(description->getViewFactory ()) : 0);
	pathView->setAutosizeFlags (kAutosizeLeft|kAutosizeRight|kAutosizeTop);
	addView (pathView);

	CRect r (rect);
	r.offset (-r.left, -r.top);
	r.top += 25;
	browser = new CDataBrowser (r, 0, data, CScrollView::kVerticalScrollbar|CScrollView::kDontDrawFrame|CDataBrowser::kDrawRowLines|CDataBrowser::kDrawColumnLines, 10);
	browser->setTransparency (true);
	browser->setAutosizeFlags (kAutosizeAll);
	CScrollbar* bar = browser->getVerticalScrollbar ();
	bar->setScrollerColor (uidScrollerColor);
	bar->setBackgroundColor (kTransparentCColor);
	bar->setFrameColor (kTransparentCColor);
	addView (browser);
	setCurrentView (baseView);
}

//-----------------------------------------------------------------------------
UIViewHierarchyBrowser::~UIViewHierarchyBrowser ()
{
}

//-----------------------------------------------------------------------------
void UIViewHierarchyBrowser::setCurrentView (CViewContainer* newView)
{
	if ((newView && newView->isChild (baseView, true)) || newView == currentView)
		return;
	currentView = newView;
	browser->recalculateLayout (false);
	browser->setSelectedRow (0, true);
	pathView->setHierarchyDirty ();
}

//-----------------------------------------------------------------------------
void UIViewHierarchyBrowser::changeBaseView (CViewContainer* newBaseView)
{
	baseView = currentView = newBaseView;
	browser->recalculateLayout (false);
	browser->setSelectedRow (0, true);
	pathView->setHierarchyDirty ();
}

//-----------------------------------------------------------------------------
void UIViewHierarchyBrowser::notifyHierarchyChange (CView* view, bool wasRemoved)
{
	CViewContainer* parent = dynamic_cast<CViewContainer*> (view->getParentView ());
	if (parent == currentView)
		browser->recalculateLayout (true);
	else if (view == currentView)
	{
		if (wasRemoved)
			setCurrentView (dynamic_cast<CViewContainer*> (view->getParentView ()));
		else
			browser->recalculateLayout (true);
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UIViewHierarchyBrowserWindow::UIViewHierarchyBrowserWindow (CViewContainer* baseView, CBaseObject* owner, UIDescription* description, void* parentPlatformWindow)
: UIPanelBase (owner, parentPlatformWindow)
, browser (0)
, description (description)
{
	CRect size (0, 0, 300, 500);
	if (init (size, "VSTGUI Hierarchy Browser", PlatformWindow::kClosable|PlatformWindow::kResizable))
	{
		browser->changeBaseView (baseView);
		platformWindow->center ();
		UIAttributes* customAttributes = description->getCustomAttributes ("UIViewHierarchyBrowser");
		if (customAttributes)
		{
			CRect windowSize;
			if (customAttributes->getRectAttribute ("windowSize", windowSize))
				platformWindow->setSize (windowSize);
		}
		platformWindow->show ();
	}
}

//-----------------------------------------------------------------------------
CFrame* UIViewHierarchyBrowserWindow::createFrame (void* platformWindow, const CCoord& width, const CCoord& height)
{
	CRect size (0, 0, width, height);
	CFrame* frame = new CFrame (size, platformWindow, this);
	frame->setBackgroundColor (uidPanelBackgroundColor);

	const CCoord kMargin = 12;
	size.left += kMargin;
	size.right -= kMargin;
	size.bottom -= kMargin;
	browser = new UIViewHierarchyBrowser (size, 0, description, dynamic_cast<IActionOperator*> (owner));
	browser->setAutosizeFlags (kAutosizeAll);
	frame->addView (browser);
	return frame;
}

//-----------------------------------------------------------------------------
UIViewHierarchyBrowserWindow::~UIViewHierarchyBrowserWindow ()
{
}

//-----------------------------------------------------------------------------
void UIViewHierarchyBrowserWindow::changeBaseView (CViewContainer* newBaseView)
{
	if (browser)
		browser->changeBaseView (newBaseView);
}

//-----------------------------------------------------------------------------
void UIViewHierarchyBrowserWindow::notifyHierarchyChange (CView* view, bool wasRemoved)
{
	if (browser)
		browser->notifyHierarchyChange (view, wasRemoved);
}

//-----------------------------------------------------------------------------
void UIViewHierarchyBrowserWindow::windowClosed (PlatformWindow* _platformWindow)
{
	if (_platformWindow == platformWindow)
	{
		UIAttributes* customAttributes = description->getCustomAttributes ("UIViewHierarchyBrowser");
		if (!customAttributes)
			customAttributes = new UIAttributes;
		CRect windowSize (platformWindow->getSize ());
		customAttributes->setRectAttribute ("windowSize", windowSize);
		description->setCustomAttributes ("UIViewHierarchyBrowser", customAttributes);
	}
	UIPanelBase::windowClosed (_platformWindow);
}

//-----------------------------------------------------------------------------
void UIViewHierarchyBrowserWindow::checkWindowSizeConstraints (CPoint& size, PlatformWindow* platformWindow)
{
	if (size.x < 300)
		size.x = 300;
	if (size.y < 200)
		size.y = 200;
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
