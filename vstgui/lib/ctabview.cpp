// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "ctabview.h"
#include "cfont.h"
#include "cbitmap.h"
#include "cdrawcontext.h"
#include "controls/cbuttons.h"
#include "cgraphicspath.h"

namespace VSTGUI {

#define  kTabButtonTagStart	20000

/// @cond ignore
//-----------------------------------------------------------------------------
class CTabButton : public COnOffButton
//-----------------------------------------------------------------------------
{
public:
	CTabButton (const CRect &size, IControlListener *listener, int32_t tag, CBitmap *background, UTF8StringPtr inName)
	: COnOffButton (size, listener, tag, background)
	, name (inName)
	{
		activeTextColor = kBlackCColor;
		inactiveTextColor (90, 90, 90, 255);
		textFont = kSystemFont; textFont->remember ();
	}

	~CTabButton () noexcept override
	{
		if (textFont)
			textFont->forget ();
	}

	void draw (CDrawContext *pContext) override
	{
		COnOffButton::draw (pContext);
		if (!name.empty ())
		{
			pContext->setFont (textFont);
			pContext->setFontColor (value > 0.f ? activeTextColor : inactiveTextColor);
			pContext->drawString (name, getViewSize ());
		}
	}

	CMouseEventResult onMouseDown (CPoint &where, const CButtonState& button) override
	{
		value = ((int32_t)value) ? 0.f : 1.f;
		
		valueChanged ();

		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}

	void onDragEnter (IDataPackage* drag, const CPoint& where) override
	{
		if (value == 0.f)
		{
			value = 1.f;
			valueChanged ();
		}
	}

	void setTextFont (CFontRef font) { if (textFont) textFont->forget (); textFont = font; textFont->remember ();}
	void setActiveTextColor (const CColor& color) { activeTextColor = color; }
	void setInactiveTextColor (const CColor& color) { inactiveTextColor = color; }

	CLASS_METHODS (CTabButton, COnOffButton)
protected:
	UTF8String name;
	CFontRef textFont;
	CColor activeTextColor;
	CColor inactiveTextColor;
};

//-----------------------------------------------------------------------------
class CTabChildView : public NonAtomicReferenceCounted
//-----------------------------------------------------------------------------
{
public:
	explicit CTabChildView (CView* view)
	: view (view)
	, previous (nullptr)
	, next (nullptr)
	, button (nullptr)
	{
	}

	~CTabChildView () noexcept override
	{
		view->forget ();
	}

	CView* view;
	CTabChildView* previous;
	CTabChildView* next;
	CControl* button;
};
/// @endcond

//-----------------------------------------------------------------------------
CTabView::CTabView (const CRect& size, CBitmap* tabBitmap, CBitmap* background, TabPosition tabPosition, int32_t style)
: CViewContainer (size)
, numberOfChilds (0)
, tabPosition (tabPosition)
, style (style)
, tabSize (CRect (0, 0, 0, 0))
, tabBitmap (tabBitmap)
, firstChild (nullptr)
, lastChild (nullptr)
, currentChild (nullptr)
{
	setBackground (background);
	if (tabBitmap)
	{
		tabBitmap->remember ();
		tabSize.right = tabBitmap->getWidth ();
		tabSize.bottom = tabBitmap->getHeight ();
	}
	setTransparency (true);
}

//-----------------------------------------------------------------------------
CTabView::CTabView (const CRect& size, const CRect& tabSize, CBitmap* background, TabPosition tabPosition, int32_t style)
: CViewContainer (size)
, numberOfChilds (0)
, currentTab (-1)
, tabPosition (tabPosition)
, style (style)
, tabSize (tabSize)
, tabBitmap (nullptr)
, firstChild (nullptr)
, lastChild (nullptr)
, currentChild (nullptr)
{
	setBackground (background);
	setTransparency (true);
}

//-----------------------------------------------------------------------------
CTabView::~CTabView () noexcept
{
	setParentView (nullptr);
	setParentFrame (nullptr);
	removeAllTabs ();
	if (tabBitmap)
		tabBitmap->forget ();
}

//-----------------------------------------------------------------------------
void CTabView::setAutosizeFlags (int32_t flags)
{
	CViewContainer::setAutosizeFlags (flags);
}

//-----------------------------------------------------------------------------
bool CTabView::addTab (CView* view, UTF8StringPtr name, CBitmap* tabBitmap)
{
	if (!view)
		return false;
	if (tabBitmap == nullptr)
		tabBitmap = this->tabBitmap;

	CTabButton* b = new CTabButton (CRect (0, 0, 0, 0), nullptr, 0, tabBitmap, name);
	b->setTransparency (true);

	return addTab (view, b);
}

//-----------------------------------------------------------------------------
bool CTabView::addTab (CView* view, CControl* button)
{
	if (!view || !button)
		return false;

	CViewContainer* tabContainer = hasChildren () ? getView (0)->asViewContainer () : nullptr;
	if (tabContainer == nullptr)
	{
		int32_t asf = kAutosizeLeft | kAutosizeTop | kAutosizeRight | kAutosizeColumn;
		CRect tsc (0, 0, getViewSize ().getWidth (), tabSize.getHeight () / 2);
		switch (tabPosition)
		{
			case kPositionBottom:
			{
				asf = kAutosizeLeft | kAutosizeBottom | kAutosizeRight | kAutosizeColumn;
				tsc.offset (0, getViewSize ().getHeight () - tabSize.getHeight () / 2);
				break;
			}
			case kPositionLeft:
			{
				asf = kAutosizeLeft | kAutosizeTop | kAutosizeBottom | kAutosizeRow;
				tsc.setWidth (tabSize.getWidth ());
				tsc.setHeight (getViewSize ().getHeight ());
				break;
			}
			case kPositionRight:
			{
				asf = kAutosizeRight | kAutosizeTop | kAutosizeBottom | kAutosizeRow;
				tsc.setWidth (getViewSize ().getWidth ());
				tsc.left = tsc.right - tabSize.getWidth ();
				tsc.setHeight (getViewSize ().getHeight ());
				break;
			}
			case kPositionTop:
			{
				break;
			}
		}
		tabContainer = new CViewContainer (tsc);
		tabContainer->setTransparency (true);
		tabContainer->setAutosizeFlags (asf);
		addView (tabContainer);
	}
	CRect ts (tabSize.left, tabSize.top, tabSize.getWidth (), tabSize.getHeight () / 2);
	switch (tabPosition)
	{
		case kPositionTop:
			ts.offset (tabSize.getWidth () * numberOfChilds, 0); break;
		case kPositionBottom:
			ts.offset (tabSize.getWidth () * numberOfChilds, 0); break;
		case kPositionLeft:
			ts.offset (0, tabSize.getHeight () / 2 * numberOfChilds); break;
		case kPositionRight:
			ts.offset (0, tabSize.getHeight () / 2 * numberOfChilds); break;
	}
	button->setViewSize (ts, false);
	button->setMouseableArea (ts);
	button->setListener (this);
	button->setTag (numberOfChilds + kTabButtonTagStart);
	tabContainer->addView (button);

	CRect tabViewSize;
	getTabViewSize (tabViewSize);
	view->setViewSize (tabViewSize);
	view->setMouseableArea (tabViewSize);

	CTabChildView* v = new CTabChildView (view);
	v->button = button;
	if (lastChild)
	{
		lastChild->next = v;
		v->previous = lastChild;
		lastChild = v;
	}
	else
	{
		firstChild = lastChild = v;
		setCurrentChild (v);
	}
	numberOfChilds++;
	return true;
}

//-----------------------------------------------------------------------------
bool CTabView::removeTab (CView* view)
{
	if (!view)
		return false;
	
	CViewContainer* tabContainer = hasChildren () ? getView (0)->asViewContainer () : nullptr;
	if (!tabContainer)
		return false;
	CTabChildView* v = firstChild;
	while (v)
	{
		if (v->view == view)
		{
			if (v->previous)
				v->previous->next = v->next;
			if (v->next)
				v->next->previous = v->previous;
			if (v == currentChild)
			{
				setCurrentChild (v->previous ? v->previous : v->next);
				if (v->previous == nullptr && v->next == nullptr)
					currentTab = -1;
			}
			tabContainer->removeView (v->button, true);
			v->forget ();
			numberOfChilds--;
			return true;
		}
		v = v->next;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool CTabView::removeAllTabs ()
{
	setCurrentChild (nullptr);
	CTabChildView* v = lastChild;
	while (v)
	{
		CTabChildView* next = v->previous;
		removeTab (v->view);
		v = next;
	}
	firstChild = nullptr;
	lastChild = nullptr;
	numberOfChilds = 0;
	currentTab = -1;
	return true;
}

//-----------------------------------------------------------------------------
bool CTabView::selectTab (int32_t index)
{
	if (index > numberOfChilds)
		return false;
	CTabChildView* v = firstChild;
	int32_t i = 0;
	while (v)
	{
		if (index == i)
			break;
		v = v->next;
		i++;
	}
	if (v)
	{
		setCurrentChild (v);
		currentTab = i;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void CTabView::setCurrentChild (CTabChildView* childView)
{
	if (currentChild)
	{
		if (childView == currentChild)
		{
			if (currentChild->button)
				currentChild->button->setValue (1.f);
			return;
		}
		if (currentChild->button)
			currentChild->button->setValue (0.f);
		removeView (currentChild->view, false);
	}
	currentChild = childView;
	if (currentChild)
	{
		addView (currentChild->view);
		if (currentChild->button)
			currentChild->button->setValue (1.f);
	}
	invalid ();
}

//-----------------------------------------------------------------------------
void CTabView::drawBackgroundRect (CDrawContext *pContext, const CRect& _updateRect)
{
	CRect oldClip = pContext->getClipRect (oldClip);
	CRect updateRect (_updateRect);
	CViewContainer* tabContainer = hasChildren () ? getView (0)->asViewContainer () : nullptr;
	if (tabContainer)
	{
		CRect tcRect = tabContainer->getViewSize ();
		if (updateRect.top < tcRect.bottom)
			updateRect.top = tcRect.bottom;
	}
	pContext->setClipRect (updateRect);
	CViewContainer::drawBackgroundRect (pContext, updateRect);
	pContext->setClipRect (oldClip);
}

//-----------------------------------------------------------------------------
void CTabView::valueChanged (CControl *pControl)
{
	selectTab (pControl->getTag () - kTabButtonTagStart);
}

//-----------------------------------------------------------------------------
void CTabView::setTabViewInsets (const CPoint& inset)
{
	tabViewInset = inset;
}

//-----------------------------------------------------------------------------
CRect& CTabView::getTabViewSize (CRect& rect) const
{
	rect = getViewSize ();
	rect.originize ();
	switch (tabPosition)
	{
		case kPositionTop:
			rect.top += tabSize.getHeight () / 2; break;
		case kPositionBottom:
			rect.bottom -= tabSize.getHeight () / 2; break;
		case kPositionLeft:
			rect.left += tabSize.getWidth (); break;
		case kPositionRight:
			rect.right -= tabSize.getWidth (); break;
	}
	rect.inset (tabViewInset.x, tabViewInset.y);
	return rect;
}

//-----------------------------------------------------------------------------
void CTabView::setTabFontStyle (const CFontRef font, CCoord fontSize, CColor selectedColor, CColor deselectedColor)
{
	auto tabFont = makeOwned<CFontDesc> (*font);
	tabFont->setSize (fontSize);
	CTabChildView* v = firstChild;
	while (v)
	{
		CTabButton* button = dynamic_cast<CTabButton*>(v->button);
		if (button)
		{
			button->setTextFont (tabFont);
			button->setActiveTextColor (selectedColor);
			button->setInactiveTextColor (deselectedColor);
		}
		v = v->next;
	}
}

//-----------------------------------------------------------------------------
void CTabView::alignTabs (TabAlignment alignment)
{
	CCoord allTabsWidth;
	CCoord viewWidth;
	CCoord offset = 0;
	CRect ts (tabSize.left, tabSize.top, tabSize.getWidth (), tabSize.getHeight () / 2);
	if (tabPosition == kPositionTop || tabPosition == kPositionBottom)
	{
		allTabsWidth = tabSize.getWidth () * numberOfChilds;
		viewWidth = getViewSize ().getWidth ();
	}
	else
	{
		allTabsWidth = (tabSize.getHeight () / 2) * numberOfChilds;
		viewWidth = getViewSize ().getHeight ();
	}
	if (alignment == kAlignCenter)
		offset = (viewWidth - allTabsWidth) / 2;
	else if (alignment == kAlignLeft)
		offset = 0;
	else if (alignment == kAlignRight)
		offset = viewWidth - allTabsWidth;
	if (tabPosition == kPositionTop)
		ts.offset (offset, 0);
	else if (tabPosition == kPositionBottom)
		ts.offset (offset, 0);
	else if (tabPosition == kPositionLeft)
		ts.offset (0, offset);
	else if (tabPosition == kPositionRight)
		ts.offset (0, offset);
	CTabChildView* v = firstChild;
	while (v)
	{
		v->button->setViewSize (ts);
		v->button->setMouseableArea (ts);
		if (tabPosition == kPositionTop || tabPosition == kPositionBottom)
			ts.offset (tabSize.getWidth (), 0);
		else
			ts.offset (0, tabSize.getHeight () / 2);
		v = v->next;
	}
	setDirty (true);
	invalid ();
}

//-----------------------------------------------------------------------------
void CTabView::setViewSize (const CRect &rect, bool invalid)
{
	if (rect == getViewSize ())
		return;

	CRect oldSize (getViewSize ());

	CCoord widthDelta = rect.getWidth () - oldSize.getWidth ();
	CCoord heightDelta = rect.getHeight () - oldSize.getHeight ();

	if (widthDelta != 0 || heightDelta != 0)
	{
		uint32_t numSubviews = getNbViews();
		int32_t counter = 1;
		bool treatAsColumn = (getAutosizeFlags () & kAutosizeColumn) ? true : false;
		bool treatAsRow = (getAutosizeFlags () & kAutosizeRow) ? true : false;
		CTabChildView* v = firstChild;
		while (v)
		{
			if (v != currentChild)
			{
				CView* pV = v->view;
				int32_t autosize = pV->getAutosizeFlags ();
				CRect viewSize (pV->getViewSize ());
				CRect mouseSize (pV->getMouseableArea ());
				if (treatAsColumn)
				{
					if (counter)
					{
						viewSize.offset (counter * (widthDelta / (numSubviews)), 0);
						mouseSize.offset (counter * (widthDelta / (numSubviews)), 0);
					}
					viewSize.setWidth (viewSize.getWidth () + (widthDelta / (numSubviews)));
					mouseSize.setWidth (mouseSize.getWidth () + (widthDelta / (numSubviews)));
				}
				else if (widthDelta != 0 && autosize & kAutosizeRight)
				{
					viewSize.right += widthDelta;
					mouseSize.right += widthDelta;
					if (!(autosize & kAutosizeLeft))
					{
						viewSize.left += widthDelta;
						mouseSize.left += widthDelta;
					}
				}
				if (treatAsRow)
				{
					if (counter)
					{
						viewSize.offset (0, counter * (heightDelta / (numSubviews)));
						mouseSize.offset (0, counter * (heightDelta / (numSubviews)));
					}
					viewSize.setHeight (viewSize.getHeight () + (heightDelta / (numSubviews)));
					mouseSize.setHeight (mouseSize.getHeight () + (heightDelta / (numSubviews)));
				}
				else if (heightDelta != 0 && autosize & kAutosizeBottom)
				{
					viewSize.bottom += heightDelta;
					mouseSize.bottom += heightDelta;
					if (!(autosize & kAutosizeTop))
					{
						viewSize.top += heightDelta;
						mouseSize.top += heightDelta;
					}
				}
				if (viewSize != pV->getViewSize ())
				{
					pV->setViewSize (viewSize);
					pV->setMouseableArea (mouseSize);
				}
			}
			v = v->next;
//			counter++;
		}
	}
	
	CViewContainer::setViewSize (rect, invalid);
}


} // namespace
