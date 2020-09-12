// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstgui/lib/cstring.h"
#include "vstgui/lib/optional.h"
#include <array>
#include <ctime>
#include <memory>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Minesweeper {

//------------------------------------------------------------------------
class HighScoreListModel
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

	HighScoreListModel () { clear (); }
	HighScoreListModel (HighScoreListModel&&) = default;
	HighScoreListModel& operator= (HighScoreListModel&&) = default;

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
Optional<HighScoreListModel> LoadHighScoreListModel (const UTF8String& path);
bool SaveHighScoreListModel (const HighScoreListModel& list, const UTF8String& path);

//------------------------------------------------------------------------
class HighScoreList
{
public:
	static std::shared_ptr<HighScoreList> make (const UTF8String& path)
	{
		auto instance = std::make_shared<HighScoreList> ();
		instance->path = path;
		if (auto l = LoadHighScoreListModel (path))
		{
			instance->list = std::move (*l);
		}
		return instance;
	}

	const UTF8String& getPath () const { return path; }
	const HighScoreListModel& get () const { return list; }
	HighScoreListModel& get () { return list; }

	Optional<size_t> addHighscore (UTF8StringPtr name, uint32_t seconds, std::time_t date = 0)
	{
		auto res = list.addHighscore (name, seconds, date);
		if (res)
			save ();
		return res;
	}
	Optional<size_t> isHighScore (uint32_t seconds) const { return list.isHighScore (seconds); }

private:
	void save () { SaveHighScoreListModel (list, path); }
	UTF8String path;
	HighScoreListModel list;
};

//------------------------------------------------------------------------
} // Minesweeper
} // Standalone
} // VSTGUI
