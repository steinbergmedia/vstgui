// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "highscorelist.h"
#include <unordered_map>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Minesweeper {

//------------------------------------------------------------------------
class HighScores
{
public:
	static HighScores& instance ();
	static UTF8String getHighscoreListName (uint32_t rows, uint32_t cols, uint32_t mines);

	std::shared_ptr<HighScoreList> get (uint32_t rows, uint32_t cols, uint32_t mines);

private:
	std::unordered_map<std::string, std::shared_ptr<HighScoreList>> lists;
};

//------------------------------------------------------------------------
} // Minesweeper
} // Standalone
} // VSTGUI
