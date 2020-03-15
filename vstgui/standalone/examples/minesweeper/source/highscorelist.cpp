// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "highscorelist.h"
#include "vstgui/uidescription/cstream.h"
#include <algorithm>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Minesweeper {
namespace {

//------------------------------------------------------------------------
UTF8String readUntil (char separator, CFileStream& stream)
{
	std::string str;
	std::string::value_type character;
	while (stream.readRaw (&character, sizeof (character)) == sizeof (character))
	{
		if (character == separator)
			break;
		str += character;
	}
	return {std::move (str)};
}

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
Optional<HighScoreList> LoadHighScoreList (const UTF8String& path)
{
	CFileStream stream;
	if (!stream.open (path.data (), CFileStream::kReadMode))
		return {};

	HighScoreList list;
	while (!stream.isEndOfFile ())
	{
		auto secondsStr = readUntil (':', stream);
		if (auto seconds = UTF8StringView (secondsStr).toNumber<uint32_t> ())
		{
			auto name = readUntil (':', stream);
			auto dateStr = readUntil ('\n', stream);
			if (auto date = UTF8StringView (dateStr).toNumber<uint32_t> ())
			{
				list.addHighscore (name, *seconds, *date);
			}
		}
		else if (!stream.isEndOfFile ())
			return {};
	}
	return {std::move (list)};
}

//------------------------------------------------------------------------
bool SaveHighScoreList (const HighScoreList& list, const UTF8String& path)
{
	CFileStream stream;
	if (!stream.open (path.data (), CFileStream::kWriteMode | CFileStream::kTruncateMode))
		return false;

	for (auto& element : list)
	{
		stream << std::to_string (element.seconds);
		stream << std::string (1, ':');
		stream << element.name.getString ();
		stream << std::string (1, ':');
		stream << std::to_string (element.date);
		stream << std::string (1, '\n');
	}
	return true;
}

//------------------------------------------------------------------------
void HighScoreList::clear ()
{
	std::for_each (list.begin (), list.end (), [] (auto& sel) { sel = {}; });
}

//------------------------------------------------------------------------
auto HighScoreList::highscorePosition (uint32_t seconds) -> ListIterator
{
	return std::find_if (list.begin (), list.end (),
	                     [seconds] (const auto& entry) { return entry.seconds > seconds; });
}

//------------------------------------------------------------------------
auto HighScoreList::highscorePosition (uint32_t seconds) const -> ListConstIterator
{
	return std::find_if (list.begin (), list.end (),
	                     [seconds] (const auto& entry) { return entry.seconds > seconds; });
}

//------------------------------------------------------------------------
Optional<size_t> HighScoreList::isHighScore (uint32_t seconds) const
{
	auto pos = highscorePosition (seconds);
	if (pos == list.end ())
		return {};
	return {std::distance (list.begin (), pos)};
}

//------------------------------------------------------------------------
Optional<size_t> HighScoreList::addHighscore (UTF8StringPtr name, uint32_t seconds,
                                              std::time_t date)
{
	auto it = highscorePosition (seconds);
	if (it == list.end ())
		return {};

	// TODO: Check name for not allowed characters: ':' and '\n'
	auto pos = std::distance (list.begin (), it);
	auto end = list.begin ();
	std::advance (end, list.size () - 1);
	std::move_backward (it, end, list.end ());
	list[pos].name = name;
	list[pos].seconds = seconds;
	list[pos].date = (date == 0) ? std::time (nullptr) : date;
	return {pos};
}

//------------------------------------------------------------------------
auto HighScoreList::begin () const -> ListConstIterator
{
	return list.begin ();
}

//------------------------------------------------------------------------
auto HighScoreList::end () const -> ListConstIterator
{
	return list.end ();
}

//------------------------------------------------------------------------
} // Minesweeper
} // Standalone
} // VSTGUI
