// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstgui/lib/cstring.h"
#include "vstgui/lib/optional.h"
#include <array>

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
		UTF8String name;
	};

	using List = std::array<Entry, Size>;
	using ListIterator = List::iterator;
	using ListConstIterator = List::const_iterator;

	HighScoreList () { clear (); }
	void clear ();
	bool addHighscore (UTF8StringPtr name, uint32_t seconds);

	bool isHighScore (uint32_t seconds) const;

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
