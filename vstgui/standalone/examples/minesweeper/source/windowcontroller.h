// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstgui/standalone/include/helpers/uidesc/customization.h"
#include "vstgui/standalone/include/helpers/uidesc/modelbinding.h"
#include "vstgui/standalone/include/helpers/windowcontroller.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Minesweeper {

class MinefieldViewController;
class EnterHighScoreViewController;

//------------------------------------------------------------------------
class WindowController final : public WindowControllerAdapter,
                               public UIDesc::Customization,
                               public UIDesc::IModelBinding,
                               public ICommandHandler
{
public:
	static constexpr auto valueRows = "Rows";
	static constexpr auto valueCols = "Cols";
	static constexpr auto valueMines = "Mines";
	static constexpr auto valueStart = "Start";
	static constexpr auto valueFlags = "Flags";
	static constexpr auto valueTime = "Time";
	static constexpr auto valueMouseMode = "MouseMode";
	static constexpr auto valueHighScoreOK = "HighScoreOK";
	static constexpr auto valueHighScoreName = "HighScoreName";

	WindowController ();
	~WindowController () noexcept override;

	void loadDefaults ();
	void storeDefaults ();

	bool canHandleCommand (const Command& command) override;
	bool handleCommand (const Command& command) override;

	void beforeShow (IWindow& w) override;
	void onShow (const IWindow& w) override;
	void onClosed (const IWindow& w) override;

	const ValueList& getValues () const override;

	std::tuple<uint32_t, uint32_t, uint32_t> getRowsColsMines ();
	void startNewGame ();
	void verifyNumMines ();
	void onWon (uint32_t secondsToWin);
	void showHighscoreListWindowDebug ();

private:
	UIDesc::ModelBindingCallbacks modelBinding;
	SharedPointer<MinefieldViewController> minefieldViewController;
	SharedPointer<EnterHighScoreViewController> enterHighscoreViewController;
	IWindow* window {nullptr};
};

//------------------------------------------------------------------------
} // Minesweeper
} // Standalone
} // VSTGUI
