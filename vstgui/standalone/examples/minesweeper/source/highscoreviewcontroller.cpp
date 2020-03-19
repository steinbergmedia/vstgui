// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "highscoreviewcontroller.h"
#include "vstgui/lib/cdatabrowser.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/uidescription/iuidescription.h"
#include "vstgui/uidescription/uiattributes.h"

#include "vstgui/standalone/include/iuidescwindow.h"
#include "vstgui/standalone/include/helpers/uidesc/customization.h"

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
	    [list] (const UTF8StringView& name, IController* parent, const IUIDescription* uiDesc)
	        -> IController* { return new HighScoreViewController (list, parent); });
	UIDesc::Config config;
	config.uiDescFileName = "Highscore.uidesc";
	config.viewName = "Window";
	config.customization = customization;
	config.windowConfig.title = "Minesweeper - Highscore";
	config.windowConfig.autoSaveFrameName = "MinesweeperHighscoreWindow";
	config.windowConfig.style.border ().close ().centered ();
	if (auto window = UIDesc::makeWindow (config))
		window->show ();
}

//------------------------------------------------------------------------
HighScoreViewController::HighScoreViewController (const std::shared_ptr<HighScoreList>& list,
                                                  IController* parent)
: DelegationController (parent), list (list)
{
}

//------------------------------------------------------------------------
int32_t HighScoreViewController::dbGetNumRows (CDataBrowser* browser)
{
	return HighScoreListModel::Size;
}

//------------------------------------------------------------------------
int32_t HighScoreViewController::dbGetNumColumns (CDataBrowser* browser)
{
	return 4;
};

//------------------------------------------------------------------------
CCoord HighScoreViewController::dbGetRowHeight (CDataBrowser* browser)
{
	return 15;
}

//------------------------------------------------------------------------
CCoord HighScoreViewController::dbGetCurrentColumnWidth (int32_t index, CDataBrowser* browser)
{
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
void HighScoreViewController::dbDrawCell (CDrawContext* context, const CRect& size, int32_t row,
                                          int32_t column, int32_t flags, CDataBrowser* browser)
{
	auto entry = list->get ().begin ();
	std::advance (entry, row);
	if (entry == list->get ().end ())
		return;
	bool valid = entry->valid ();
	switch (column)
	{
		case 0:
		{
			context->drawString (toString (row + 1), size);
			break;
		}
		case 1:
		{
			if (valid)
				context->drawString (toString (entry->seconds), size);
			break;
		}
		case 2:
		{
			if (valid)
				context->drawString (entry->name, size);
			break;
		}
		case 3:
		{
			if (valid)
			{
				char mbstr[100];
				if (std::strftime (mbstr, sizeof (mbstr), "%F", std::localtime (&entry->date)))
				{
					context->drawString (mbstr, size);
				}
			}
			break;
		}
	}
}

//------------------------------------------------------------------------
CView* HighScoreViewController::createView (const UIAttributes& attributes,
                                            const IUIDescription* description)
{
	const auto attr = attributes.getAttributeValue (IUIDescription::kCustomViewName);
	if (attr && *attr == "DataBrowser")
	{
		return new CDataBrowser ({}, this, 0, 0.);
	}
	return nullptr;
}

//------------------------------------------------------------------------
} // Minesweeper
} // Standalone
} // VSTGUI
