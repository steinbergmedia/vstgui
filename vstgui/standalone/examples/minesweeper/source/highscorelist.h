// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstgui/lib/cstring.h"
#include "vstgui/lib/optional.h"
#include <array>
#include <ctime>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Minesweeper {

//------------------------------------------------------------------------
class HighScoreList
{
public:
//------------------------------------------------------------------------
	static constexpr size_t Size = 10u;
//------------------------------------------------------------------------
	struct Entry
	{
		uint32_t seconds {std::numeric_limits<uint32_t>::max ()};
		std::time_t date {};
		UTF8String name;

		bool valid () const { return seconds != std::numeric_limits<uint32_t>::max (); }
	};

	HighScoreList () { clear (); }

	void clear ();
	Optional<size_t> addHighscore (UTF8StringPtr name, uint32_t seconds, std::time_t date = 0);

	Optional<size_t> isHighScore (uint32_t seconds) const;

	using List = std::array<Entry, Size>;
	using ListIterator = List::iterator;
	using ListConstIterator = List::const_iterator;

	ListConstIterator begin () const;
	ListConstIterator end () const;
//------------------------------------------------------------------------
private:
	ListIterator highscorePosition (uint32_t seconds);
	ListConstIterator highscorePosition (uint32_t seconds) const;

	List list;
};

//------------------------------------------------------------------------
Optional<HighScoreList> LoadHighScoreList (const UTF8String& path);
bool SaveHighScoreList (const HighScoreList& list, const UTF8String& path);

//------------------------------------------------------------------------
} // Minesweeper
} // Standalone
} // VSTGUI
