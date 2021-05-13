// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cdatabrowser.h"
#include "cdrawcontext.h"
#include "cframe.h"
#include "cvstguitimer.h"
#include "genericstringlistdatabrowsersource.h"
#include "platform/iplatformfont.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
GenericStringListDataBrowserSource::GenericStringListDataBrowserSource (
    const StringVector* stringList, GenericStringListDataBrowserSourceSelectionChanged* delegate)
: stringList (stringList)
, rowHeight (-1)
, fontColor (kWhiteCColor)
, selectionColor (kBlueCColor)
, rowlineColor (kGreyCColor)
, rowBackColor (kTransparentCColor)
, rowAlternateBackColor (kTransparentCColor)
, textInset (2., 0.)
, textAlignment (kLeftText)
, drawFont (kSystemFont)
, dataBrowser (nullptr)
, delegate (delegate)
{
}

//-----------------------------------------------------------------------------
GenericStringListDataBrowserSource::~GenericStringListDataBrowserSource () noexcept = default;

//-----------------------------------------------------------------------------
void GenericStringListDataBrowserSource::dbAttached (CDataBrowser* browser)
{
	dataBrowser = browser;
}

//-----------------------------------------------------------------------------
void GenericStringListDataBrowserSource::dbRemoved (CDataBrowser* browser)
{
	dataBrowser = nullptr;
}

//-----------------------------------------------------------------------------
void GenericStringListDataBrowserSource::setStringList (const StringVector* inStringList)
{
	stringList = inStringList;
	if (dataBrowser)
		dataBrowser->recalculateLayout (true);
}

//-----------------------------------------------------------------------------
void GenericStringListDataBrowserSource::setupUI (
    const CColor& _selectionColor, const CColor& _fontColor, const CColor& _rowlineColor,
    const CColor& _rowBackColor, const CColor& _rowAlternateBackColor, CFontRef _font,
    int32_t _rowHeight, CCoord _textInset)
{
	if (_font)
		drawFont = _font;
	textInset = CPoint (_textInset, 0);
	rowHeight = _rowHeight;
	selectionColor = _selectionColor;
	fontColor = _fontColor;
	rowlineColor = _rowlineColor;
	rowBackColor = _rowBackColor;
	rowAlternateBackColor = _rowAlternateBackColor;
	if (dataBrowser)
		dataBrowser->recalculateLayout (true);
}

//-----------------------------------------------------------------------------
void GenericStringListDataBrowserSource::dbSelectionChanged (CDataBrowser* browser)
{
	if (delegate)
		delegate->dbSelectionChanged (browser->getSelectedRow (), this);
}

//-----------------------------------------------------------------------------
int32_t GenericStringListDataBrowserSource::dbGetNumRows (CDataBrowser* browser)
{
	return stringList ? (int32_t)stringList->size () : 0;
}

//-----------------------------------------------------------------------------
CCoord GenericStringListDataBrowserSource::dbGetCurrentColumnWidth (int32_t index,
                                                                    CDataBrowser* browser)
{
	return browser->getWidth () -
	       ((browser->getStyle () & CScrollView::kOverlayScrollbars ||
	         (browser->getActiveScrollbars () & CScrollView::kVerticalScrollbar) == 0) ?
	            0 :
	            browser->getScrollbarWidth ());
}

//-----------------------------------------------------------------------------
bool GenericStringListDataBrowserSource::dbGetLineWidthAndColor (CCoord& width, CColor& color,
                                                                 CDataBrowser* browser)
{
	width = 1.;
	color = rowlineColor;
	return true;
}

//-----------------------------------------------------------------------------
CCoord GenericStringListDataBrowserSource::dbGetRowHeight (CDataBrowser* browser)
{
	if (rowHeight < 0)
	{
		if (drawFont->getPlatformFont ())
		{
			CCoord height = drawFont->getPlatformFont ()->getAscent ();
			height += drawFont->getPlatformFont ()->getDescent ();
			height += drawFont->getPlatformFont ()->getLeading ();
			return std::floor (height + 2.5);
		}
		return drawFont->getSize () + 2.;
	}
	return rowHeight;
}

//-----------------------------------------------------------------------------
void GenericStringListDataBrowserSource::dbDrawHeader (CDrawContext* context, const CRect& size,
                                                       int32_t column, int32_t flags,
                                                       CDataBrowser* browser)
{
}

//-----------------------------------------------------------------------------
void GenericStringListDataBrowserSource::drawRowBackground (CDrawContext* context,
                                                            const CRect& size, int32_t row,
                                                            int32_t flags,
                                                            CDataBrowser* browser) const
{
	vstgui_assert (row >= 0 && static_cast<size_t> (row) < stringList->size ());

	context->setDrawMode (kAliasing);
	context->setLineWidth (1.);
	context->setFillColor ((row % 2) ? rowBackColor : rowAlternateBackColor);
	context->drawRect (size, kDrawFilled);
	if (flags & kRowSelected)
	{
		CColor color (selectionColor);
		CView* focusView = browser->getFrame ()->getFocusView ();
		if (!(focusView && browser->isChild (focusView, true)))
		{
			double hue, saturation, value;
			color.toHSV (hue, saturation, value);
			if (saturation > 0.)
			{
				saturation *= 0.5;
				color.fromHSV (hue, saturation, value);
			}
			else
				color.alpha /= 2;
		}
		context->setFillColor (color);
		context->drawRect (size, kDrawFilled);
	}
}

//-----------------------------------------------------------------------------
void GenericStringListDataBrowserSource::drawRowString (CDrawContext* context, const CRect& size,
                                                        int32_t row, int32_t flags,
                                                        CDataBrowser* browser) const
{
	vstgui_assert (row >= 0 && static_cast<size_t> (row) < stringList->size ());

	context->saveGlobalState ();
	CRect stringSize (size);
	stringSize.inset (textInset.x, textInset.y);
	context->setFont (drawFont);
	context->setFontColor (fontColor);
	ConcatClip cc (*context, stringSize);
	context->drawString ((*stringList)[static_cast<size_t> (row)].getPlatformString (), stringSize,
	                     textAlignment);
	context->restoreGlobalState ();
}

//-----------------------------------------------------------------------------
void GenericStringListDataBrowserSource::dbDrawCell (CDrawContext* context, const CRect& size,
                                                     int32_t row, int32_t column, int32_t flags,
                                                     CDataBrowser* browser)
{
	vstgui_assert (row >= 0 && static_cast<size_t> (row) < stringList->size ());
	vstgui_assert (column == 0);

	drawRowBackground (context, size, row, flags, browser);
	drawRowString (context, size, row, flags, browser);
}

//-----------------------------------------------------------------------------
void GenericStringListDataBrowserSource::dbOnKeyboardEvent (KeyboardEvent& event, CDataBrowser* browser)
{
	if (event.type != EventType::KeyDown)
		return;

	if (event.virt == VirtualKey::Space)
	{
		event.virt = VirtualKey::None;
		event.character = 0x20;
	}
	if (dataBrowser && event.virt == VirtualKey::None && event.modifiers.empty ())
	{
		if (timer == nullptr)
		{
			timer = makeOwned<CVSTGUITimer> (this, 1000);
			timer->start ();
		}
		else
		{
			timer->stop ();
			timer->start ();
		}
		keyDownFindString += static_cast<char> (toupper (event.character));
		StringVector::const_iterator it = stringList->begin ();
		int32_t row = 0;
		while (it != stringList->end ())
		{
			std::string str ((*it).getString (), 0, keyDownFindString.length ());
			std::transform (str.begin (), str.end (), str.begin (), ::toupper);
			if (str == keyDownFindString)
			{
				dataBrowser->setSelectedRow (row, true);
				event.consumed = true;
				return;
			}
			row++;
			++it;
		}
	}
}

//-----------------------------------------------------------------------------
CMouseEventResult GenericStringListDataBrowserSource::dbOnMouseDown (const CPoint& where,
                                                                     const CButtonState& buttons,
                                                                     int32_t row, int32_t column,
                                                                     CDataBrowser* browser)
{
	if (delegate && buttons.isDoubleClick ())
		delegate->dbRowDoubleClick (row, this);
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//-----------------------------------------------------------------------------
CMessageResult GenericStringListDataBrowserSource::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == CVSTGUITimer::kMsgTimer)
	{
		keyDownFindString = "";
		timer = nullptr;
		return kMessageNotified;
	}
	return kMessageUnknown;
}

//------------------------------------------------------------------------
} // VSTGUI
