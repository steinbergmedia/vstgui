// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "highscores.h"
#include "vstgui/standalone/include/iapplication.h"
#include "vstgui/standalone/include/icommondirectories.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Minesweeper {

//------------------------------------------------------------------------
HighScores& HighScores::instance ()
{
	static HighScores gInstance;
	return gInstance;
}

//------------------------------------------------------------------------
std::shared_ptr<HighScoreList> HighScores::get (uint32_t rows, uint32_t cols, uint32_t mines)
{
	auto path = IApplication::instance ().getCommonDirectories ().get (
	    CommonDirectoryLocation::AppPreferencesPath, {}, true);
	if (!path)
		return nullptr;
	*path += getHighscoreListName (rows, cols, mines);

	auto it = lists.find (path->getString ());
	if (it != lists.end ())
	{
		return it->second;
	}

	auto list = HighScoreList::make (*path);
	lists.insert ({path->getString (), list});
	return list;
}

//------------------------------------------------------------------------
UTF8String HighScores::getHighscoreListName (uint32_t rows, uint32_t cols, uint32_t mines)
{
	return toString (rows) + "x" + toString (cols) + "x" + toString (mines) + ".highscore";
}

//------------------------------------------------------------------------
} // Minesweeper
} // Standalone
} // VSTGUI
