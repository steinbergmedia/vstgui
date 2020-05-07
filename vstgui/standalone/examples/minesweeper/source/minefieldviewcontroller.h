// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "model.h"
#include "vstgui/lib/ccolor.h"
#include "vstgui/lib/cfont.h"
#include "vstgui/lib/idatabrowserdelegate.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/standalone/include/ivalue.h"
#include "vstgui/uidescription/delegationcontroller.h"
#include <chrono>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Minesweeper {

static constexpr auto maxTimeInSeconds = 999;

//------------------------------------------------------------------------
class MinefieldViewController : public DelegationController,
                                public DataBrowserDelegateAdapter,
                                public NonAtomicReferenceCounted,
                                public ViewListenerAdapter,
                                public Model::IListener
{
public:
	static constexpr auto BombCharacter = "\xF0\x9F\x92\xA3";
	static constexpr auto FlagCharacter = "\xF0\x9F\x9A\xA9";
	static constexpr auto ExplosionCharacter = "\xF0\x9F\x92\xA5";
	static constexpr auto QuestionMarkCharacter = "\xE2\x9D\x93";

	using WonCallbackFunc = std::function<void (int32_t secondsToWin)>;

	MinefieldViewController (IValue& flagsValue, IValue& timeValue, IController* parent,
	                         WonCallbackFunc&& wonCallback);

	void startGame (uint32_t rows, uint32_t cols, uint32_t mines);
	void setMouseMode (bool state);

private:
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override;
	CView* verifyView (CView* view, const UIAttributes& attributes,
	                   const IUIDescription* description) override;
	void onCellChanged (uint32_t row, uint32_t col) override;
	int32_t dbGetNumRows (CDataBrowser* browser) override;
	int32_t dbGetNumColumns (CDataBrowser* browser) override;
	CCoord dbGetRowHeight (CDataBrowser* browser) override;
	CCoord dbGetCurrentColumnWidth (int32_t index, CDataBrowser* browser) override;
	bool dbGetLineWidthAndColor (CCoord& width, CColor& color, CDataBrowser* browser) override;
	void drawClosedCell (const CRect& r, CDrawContext& context) const;
	void drawOpenCell (const CRect& r, CDrawContext& context) const;
	void drawQuestionMark (const CRect& r, CDrawContext& context, CFontRef f) const;
	void drawQuestionMarkCell (const CRect& r, CDrawContext& context, CFontRef f) const;
	void drawFlag (const CRect& r, CDrawContext& context, CFontRef f) const;
	void drawFlaggedCell (const CRect& r, CDrawContext& context, CFontRef f) const;
	void drawMinedCell (const CRect& r, CDrawContext& context, CFontRef f) const;
	void drawExplosionCell (const CRect& r, CDrawContext& context, CFontRef f) const;
	void drawCellNeighbours (const CRect& r, CDrawContext& context, CFontRef f,
	                         uint32_t neighbours);
	void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column,
	                 int32_t flags, CDataBrowser* browser) override;
	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row,
	                                 int32_t column, CDataBrowser* browser) override;
	CMouseEventResult dbOnMouseMoved (const CPoint& where, const CButtonState& buttons, int32_t row,
	                                  int32_t column, CDataBrowser* browser) override;
	CMouseEventResult dbOnMouseUp (const CPoint& where, const CButtonState& buttons, int32_t row,
	                               int32_t column, CDataBrowser* browser) override;
	void checkGameOver ();
	void onGameLost ();
	void onGameWon ();
	void onTimer ();
	void viewSizeChanged (CView* view, const CRect& oldSize) override;
	void updateCellSize (CPoint newSize);

private:
	SharedPointer<CDataBrowser> dataBrowser;
	SharedPointer<CView> lostView;
	SharedPointer<CView> wonView;
	int32_t numRows {1};
	int32_t numCols {1};
	std::unique_ptr<Model> model;
	CColor closedFrameColor {kBlackCColor};
	CColor closedBackColor {kGreyCColor};
	CColor openedFrameColor {kGreyCColor};
	CColor openedBackColor {kTransparentCColor};
	CColor flagedFrameColor {kGreyCColor};
	CColor flagedBackColor {kTransparentCColor};
	CPoint cellSize {30, 30};
	CFontDesc font {*kSystemFont};
	CFontDesc smallEmojiFont {*kSymbolFont};
	CFontDesc emojiFont {*kSymbolFont};
	IValue& flagsValue;
	IValue& timeValue;
	WonCallbackFunc wonCallback;

	using Clock = std::chrono::steady_clock;
	using TimePoint = std::chrono::time_point<Clock>;
	TimePoint startTime;
	SharedPointer<CVSTGUITimer> gameTimer;
	SharedPointer<CVSTGUITimer> mouseDownTimer;
	bool ignoreMouseUp {false};
	bool mouseMode {true};
};

//------------------------------------------------------------------------
} // Minesweeper
} // Standalone
} // VSTGUI
