// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "highscorelist.h"
#include "vstgui/lib/idatabrowserdelegate.h"
#include "vstgui/uidescription/delegationcontroller.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Minesweeper {

//------------------------------------------------------------------------
class HighScoreViewController : public DelegationController,
                                public DataBrowserDelegateAdapter,
                                public NonAtomicReferenceCounted
{
public:
	HighScoreViewController (const std::shared_ptr<HighScoreList>& list, IController* parent);

	int32_t dbGetNumRows (CDataBrowser* browser) override;
	int32_t dbGetNumColumns (CDataBrowser* browser) override;
	CCoord dbGetRowHeight (CDataBrowser* browser) override;
	CCoord dbGetCurrentColumnWidth (int32_t index, CDataBrowser* browser) override;
	void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column,
	                 int32_t flags, CDataBrowser* browser) override;
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override;

private:
	std::shared_ptr<HighScoreList> list;
};

//------------------------------------------------------------------------
} // Minesweeper
} // Standalone
} // VSTGUI
