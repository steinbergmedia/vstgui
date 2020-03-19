// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "highscorelist.h"
#include "vstgui/lib/cfont.h"
#include "vstgui/lib/idatabrowserdelegate.h"
#include "vstgui/uidescription/delegationcontroller.h"
#include <array>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Minesweeper {

//------------------------------------------------------------------------
class HighScoreViewController final : public DelegationController,
                                      public DataBrowserDelegateAdapter,
                                      public NonAtomicReferenceCounted
{
public:
	HighScoreViewController (IController* parent);

	void setHighScoreList (const std::shared_ptr<HighScoreList>& list);

	int32_t dbGetNumRows (CDataBrowser* browser) override;
	int32_t dbGetNumColumns (CDataBrowser* browser) override;
	CCoord dbGetRowHeight (CDataBrowser* browser) override;
	CCoord dbGetCurrentColumnWidth (int32_t index, CDataBrowser* browser) override;
	void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column,
	                 int32_t flags, CDataBrowser* browser) override;
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override;
	void dbAttached (CDataBrowser* browser) override;
	void dbRemoved (CDataBrowser* browser) override;
	bool dbGetLineWidthAndColor (CCoord& width, CColor& color, CDataBrowser* browser) override;

private:
	static constexpr const size_t NumCols = 4;

	CFontDesc font;
	CDataBrowser* dataBrowser {nullptr};
	std::shared_ptr<HighScoreList> list;
	std::array<CCoord, NumCols> columnWidths;
};

// temporarily:
void ShowHighscoreWindow (const std::shared_ptr<HighScoreList>& list);

//------------------------------------------------------------------------
} // Minesweeper
} // Standalone
} // VSTGUI
