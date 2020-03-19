// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstgui/standalone/include/icommand.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Minesweeper {

//------------------------------------------------------------------------
static constexpr IdStringPtr GameGroup = "Game";
static constexpr IdStringPtr ToggleHighscores = "Toggle Highscores";

static const Command NewGameCommand {GameGroup, "New Game"};
static const Command NewBeginnerGameCommand {GameGroup, "New Beginner Game"};
static const Command NewIntermediateGameCommand {GameGroup, "New Intermediate Game"};
static const Command NewExpertGameCommand {GameGroup, "New Expert Game"};
static const Command ToggleHighscoresCommand {GameGroup, ToggleHighscores};

//------------------------------------------------------------------------
static constexpr IdStringPtr MouseMode = "Use Mouse Mode";
static constexpr IdStringPtr TouchpadMode = "Use Touchpad Mode";

static const Command MouseModeCommand {GameGroup, MouseMode};
static const Command TouchpadModeCommand {GameGroup, TouchpadMode};

//------------------------------------------------------------------------
} // Minesweeper
} // Standalone
} // VSTGUI
