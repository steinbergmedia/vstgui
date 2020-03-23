// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "highscoreviewcontroller.h"
#include "keepchildviewscentered.h"
#include "vstgui/lib/cdatabrowser.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/uidescription/iuidescription.h"
#include "vstgui/uidescription/uiattributes.h"
#include <cassert>
#include <ctime>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Minesweeper {

//------------------------------------------------------------------------
HighScoreViewController::HighScoreViewController (IController* parent)
: DelegationController (parent)
{
	columnWidths[0] = 0.06;
	columnWidths[1] = 0.14;
	columnWidths[2] = 0.5;
	columnWidths[3] = 0.3;
	font = *kSystemFont;
}

//------------------------------------------------------------------------
void HighScoreViewController::setHighScoreList (const std::shared_ptr<HighScoreList>& l)
{
	list = l;
	if (dataBrowser)
		dataBrowser->recalculateLayout ();
}

//------------------------------------------------------------------------
bool HighScoreViewController::isVisible () const
{
	if (dataBrowser)
	{
		if (auto parent = dataBrowser->getParentView ())
			return parent->isVisible ();
	}
	return false;
}

//------------------------------------------------------------------------
void HighScoreViewController::show ()
{
	if (!dataBrowser)
		return;
	if (auto parent = dataBrowser->getParentView ())
	{
		parent->setMouseEnabled (true);
		parent->setAlphaValue (1.f);
	}
}

//------------------------------------------------------------------------
void HighScoreViewController::hide ()
{
	if (!dataBrowser)
		return;
	if (auto parent = dataBrowser->getParentView ())
	{
		parent->setAlphaValue (0.f);
		parent->setMouseEnabled (false);
	}
}

//------------------------------------------------------------------------
void HighScoreViewController::dbAttached (CDataBrowser* browser)
{
	dataBrowser = browser;
	if (auto parent = browser->getParentView ())
	{
		assert (parent->asViewContainer ());
		keepChildViewsCentered (parent->asViewContainer ());
	}
}

//------------------------------------------------------------------------
void HighScoreViewController::dbRemoved (CDataBrowser* browser)
{
	dataBrowser = nullptr;
}

//------------------------------------------------------------------------
int32_t HighScoreViewController::dbGetNumRows (CDataBrowser* browser)
{
	return HighScoreListModel::Size;
}

//------------------------------------------------------------------------
int32_t HighScoreViewController::dbGetNumColumns (CDataBrowser* browser)
{
	return NumCols;
};

//------------------------------------------------------------------------
CCoord HighScoreViewController::dbGetRowHeight (CDataBrowser* browser)
{
	auto height = browser->getHeight () / (HighScoreListModel::Size + 1) - 1;
	if (font.getSize () != height * 0.6)
		font.setSize (height * 0.6);
	return height;
}

//------------------------------------------------------------------------
CCoord HighScoreViewController::dbGetCurrentColumnWidth (int32_t index, CDataBrowser* browser)
{
	if (index >= 0 && index < NumCols)
		return browser->getWidth () * columnWidths[index];
	return 10;
}

//------------------------------------------------------------------------
bool HighScoreViewController::dbGetLineWidthAndColor (CCoord& width, CColor& color,
                                                      CDataBrowser* browser)
{
	width = 1.;
	color = kBlackCColor;
	return true;
}

//------------------------------------------------------------------------
void HighScoreViewController::dbDrawHeader (CDrawContext* context, const CRect& size,
                                            int32_t column, int32_t flags, CDataBrowser* browser)
{
	context->setFont (&font);
	context->setFontColor (fontColor);
	UTF8String text;
	switch (column)
	{
		case 1: text = "Sec"; break;
		case 2: text = "Name"; break;
		case 3: text = "Date"; break;
		default: break;
	}
	if (!text.empty ())
	{
		context->drawString (text, size);
	}
}

//------------------------------------------------------------------------
void HighScoreViewController::dbDrawCell (CDrawContext* context, const CRect& size, int32_t row,
                                          int32_t column, int32_t flags, CDataBrowser* browser)
{
	if (!list)
		return;

	auto entry = list->get ().begin ();
	std::advance (entry, row);
	if (entry == list->get ().end ())
		return;
	bool valid = entry->valid ();
	context->setFont (&font);
	context->setFontColor (fontColor);
	UTF8String text = "-";
	CHoriTxtAlign align = kCenterText;
	CRect r = size;
	switch (column)
	{
		case 0:
		{
			text = toString (row + 1) + ".";
			align = kRightText;
			break;
		}
		case 1:
		{
			if (valid)
				text = toString (entry->seconds);
			break;
		}
		case 2:
		{
			if (valid)
			{
				text = entry->name;
				align = kLeftText;
				r.left += 5.;
			}
			break;
		}
		case 3:
		{
			if (valid)
			{
				char mbstr[100];
				if (std::strftime (mbstr, sizeof (mbstr), "%F", std::localtime (&entry->date)))
				{
					text = mbstr;
				}
			}
			break;
		}
	}
	context->drawString (text, r, align);
}

//------------------------------------------------------------------------
CView* HighScoreViewController::createView (const UIAttributes& attributes,
                                            const IUIDescription* description)
{
	const auto attr = attributes.getAttributeValue (IUIDescription::kCustomViewName);
	if (attr && *attr == "DataBrowser")
	{
		if (auto f = description->getFont ("highscore"))
			font = *f;
		description->getColor ("highscore.font", fontColor);
		return new CDataBrowser ({}, this, CDataBrowser::kDrawHeader | CDataBrowser::kDrawRowLines,
		                         0.);
	}
	return nullptr;
}

//------------------------------------------------------------------------
} // Minesweeper
} // Standalone
} // VSTGUI
