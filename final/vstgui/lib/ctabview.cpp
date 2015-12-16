//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
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
	, name (0)
	{
		name = String::newWithString (inName);
		activeTextColor = kBlackCColor;
		inactiveTextColor (90, 90, 90, 255);
		textFont = kSystemFont; textFont->remember ();
	}

	virtual ~CTabButton ()
	{
		if (textFont)
			textFont->forget ();
		String::free (name);
	}	

	virtual void draw (CDrawContext *pContext) VSTGUI_OVERRIDE_VMETHOD
	{
		COnOffButton::draw (pContext);
		if (name)
		{
			pContext->setFont (textFont);
			pContext->setFontColor (value > 0.f ? activeTextColor : inactiveTextColor);
			pContext->drawString (name, getViewSize ());
		}
	}

	CMouseEventResult onMouseDown (CPoint &where, const CButtonState& button) VSTGUI_OVERRIDE_VMETHOD
	{
		value = ((int32_t)value) ? 0.f : 1.f;
		
		valueChanged ();

		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}

	virtual void onDragEnter (IDataPackage* drag, const CPoint& where) VSTGUI_OVERRIDE_VMETHOD
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
	UTF8StringBuffer name;
	CFontRef textFont;
	CColor activeTextColor;
	CColor inactiveTextColor;
};

//-----------------------------------------------------------------------------
class CTabChildView : public CBaseObject
//-----------------------------------------------------------------------------
{
public:
	CTabChildView (CView* view)
	: view (view)
	, previous (0)
	, next (0)
	, button (0)
	{
	}

	virtual ~CTabChildView ()
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
, firstChild (0)
, lastChild (0)
, currentChild (0)
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
, tabBitmap (0)
, firstChild (0)
, lastChild (0)
, currentChild (0)
{
	setBackground (background);
	setTransparency (true);
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
CTabView::CTabView (const CRect& size, CFrame* parent, CBitmap* tabBitmap, CBitmap* background, TabPosition tabPosition, int32_t style)
: CViewContainer (size)
, numberOfChilds (0)
, tabPosition (tabPosition)
, style (style)
, tabSize (CRect (0, 0, 0, 0))
, tabBitmap (tabBitmap)
, firstChild (0)
, lastChild (0)
, currentChild (0)
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
CTabView::CTabView (const CRect& size, CFrame* parent, const CRect& tabSize, CBitmap* background, TabPosition tabPosition, int32_t style)
: CViewContainer (size)
, numberOfChilds (0)
, currentTab (-1)
, tabPosition (tabPosition)
, style (style)
, tabSize (tabSize)
, tabBitmap (0)
, firstChild (0)
, lastChild (0)
, currentChild (0)
{
	setBackground (background);
	setTransparency (true);
}
#endif

//-----------------------------------------------------------------------------
CTabView::~CTabView ()
{
	pParentView = 0;
	pParentFrame = 0;
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
	if (tabBitmap == 0)
		tabBitmap = this->tabBitmap;

	CTabButton* b = new CTabButton (CRect (0, 0, 0, 0), 0, 0, tabBitmap, name);
	b->setTransparency (true);

	return addTab (view, b);
}

//-----------------------------------------------------------------------------
bool CTabView::addTab (CView* view, CControl* button)
{
	if (!view || !button)
		return false;

	CViewContainer* tabContainer = dynamic_cast<CViewContainer*>(getView (0));
	if (tabContainer == 0)
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
	
	CViewContainer* tabContainer = dynamic_cast<CViewContainer*>(getView (0));
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
				if (v->previous == 0 && v->next == 0)
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
	setCurrentChild (0);
	CTabChildView* v = lastChild;
	while (v)
	{
		CTabChildView* next = v->previous;
		removeTab (v->view);
		v = next;
	}
	firstChild = 0;
	lastChild = 0;
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
	CViewContainer* tabContainer = dynamic_cast<CViewContainer*>(getView (0));
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
	CFontRef tabFont = (CFontRef)font->newCopy ();
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
	tabFont->forget ();
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
