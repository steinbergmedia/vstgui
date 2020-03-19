// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "highscoreviewcontroller.h"
#include "vstgui/lib/cdatabrowser.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/uidescription/iuidescription.h"
#include "vstgui/uidescription/uiattributes.h"

#include "vstgui/standalone/include/helpers/uidesc/customization.h"
#include "vstgui/standalone/include/iuidescwindow.h"

#include <ctime>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Minesweeper {

//------------------------------------------------------------------------
void ShowHighscoreWindow (const std::shared_ptr<HighScoreList>& list)
{
	auto customization = UIDesc::Customization::make ();
	customization->addCreateViewControllerFunc (
	    "DataBrowserController",
	    [list] (const UTF8StringView& name, IController* parent,
	            const IUIDescription* uiDesc) -> IController* {
		    auto ctrler = new HighScoreViewController (parent);
		    ctrler->setHighScoreList (list);
		    return ctrler;
	    });
	UIDesc::Config config;
	config.uiDescFileName = "Highscore.uidesc";
	config.viewName = "Window";
	config.customization = customization;
	config.windowConfig.title = "Minesweeper - Highscore";
	config.windowConfig.autoSaveFrameName = "MinesweeperHighscoreWindow";
	config.windowConfig.style.border ().close ().centered ().size ();
	if (auto window = UIDesc::makeWindow (config))
		window->show ();
}

//------------------------------------------------------------------------
HighScoreViewController::HighScoreViewController (IController* parent)
: DelegationController (parent)
{
	columnWidths[0] = 0.05;
	columnWidths[1] = 0.15;
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
void HighScoreViewController::dbAttached (CDataBrowser* browser)
{
	dataBrowser = browser;
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
	auto height = browser->getHeight () / (HighScoreListModel::Size)-1;
	if (font.getSize () != height * 0.6)
		font.setSize (height * 0.6);
	return height;
}

//------------------------------------------------------------------------
CCoord HighScoreViewController::dbGetCurrentColumnWidth (int32_t index, CDataBrowser* browser)
{
	if (index >= 0 && index < NumCols)
		return browser->getWidth () * columnWidths[index];
	switch (index)
	{
		case 0: return 20;
		case 1: return 40;
		case 2: return 100;
		case 3: return 140;
	}
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
void HighScoreViewController::dbDrawCell (CDrawContext* context, const CRect& size, int32_t row,
                                          int32_t column, int32_t flags, CDataBrowser* browser)
{
	auto entry = list->get ().begin ();
	std::advance (entry, row);
	if (entry == list->get ().end ())
		return;
	bool valid = entry->valid ();
	context->setFont (&font);
	UTF8String text = "-";
	switch (column)
	{
		case 0:
		{
			text = toString (row + 1);
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
				text = entry->name;
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
	context->drawString (text, size);
}

//------------------------------------------------------------------------
CView* HighScoreViewController::createView (const UIAttributes& attributes,
                                            const IUIDescription* description)
{
	const auto attr = attributes.getAttributeValue (IUIDescription::kCustomViewName);
	if (attr && *attr == "DataBrowser")
	{
		return new CDataBrowser ({}, this,
		                         CDataBrowser::kDrawRowLines | CDataBrowser::kDrawColumnLines, 0.);
	}
	return nullptr;
}

//------------------------------------------------------------------------
} // Minesweeper
} // Standalone
} // VSTGUI
