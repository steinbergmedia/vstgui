// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../ipreference.h"
#include "../iapplication.h"
#include <sstream>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

static constexpr const char* DefaultPreferencesGroupSeparator = "::";

//------------------------------------------------------------------------
class Preferences
{
public:
	Preferences (Preferences&&) = default;
	Preferences& operator= (Preferences&&) = default;

	Preferences (const std::initializer_list<const char*>& groups,
	             const char* groupSeparator = DefaultPreferencesGroupSeparator)
	: groupSeparator (groupSeparator)
	{
		for (auto& g : groups)
			groupKey += UTF8String (g) + groupSeparator;
	}

	Preferences (const UTF8String& inGroupKey = "",
	             const char* groupSeparator = DefaultPreferencesGroupSeparator)
	: groupKey (inGroupKey), groupSeparator (groupSeparator)
	{
		if (!groupKey.empty ())
			groupKey += groupSeparator;
	}

	inline Preferences subGroupPreferences (const UTF8String& subGroup) const
	{
		return Preferences (groupKey + subGroup, groupSeparator);
	}

	inline bool set (const UTF8String& key, const UTF8String& value) const
	{
		if (!groupKey.empty ())
			return preferences->set (groupKey + key, value);
		return preferences->set (key, value);
	}

	inline Optional<UTF8String> get (const UTF8String& key) const
	{
		if (!groupKey.empty ())
			return preferences->get (groupKey + key);
		return preferences->get (key);
	}

	template <typename T>
	inline bool setNumber (const UTF8String& key, T value) const
	{
		return set (key, toString (value));
	}

	template <typename T>
	inline bool setFloat (const UTF8String& key, T value, uint32_t precision = 8) const
	{
		std::ostringstream sstream;
		sstream.imbue (std::locale::classic ());
		sstream.precision (static_cast<std::streamsize> (precision));
		sstream << value;
		return set (key, UTF8String (sstream.str ()));
	}

	template <typename T>
	inline Optional<T> getNumber (const UTF8String& key) const
	{
		if (auto p = get (key))
		{
			if constexpr (std::is_floating_point<T>::value)
				return UTF8StringView (*p).toFloat ();
			return UTF8StringView (*p).toNumber<T> ();
		}
		return {};
	}

	inline bool setPoint (const UTF8String& key, CPoint p, uint32_t precision = 8) const
	{
		std::ostringstream sstream;
		sstream.imbue (std::locale::classic ());
		sstream.precision (static_cast<std::streamsize> (precision));
		sstream << '{';
		sstream << p.x;
		sstream << ';';
		sstream << p.y;
		sstream << '}';
		return set (key, UTF8String (sstream.str ()));
	}

	inline Optional<CPoint> getPoint (const UTF8String& key) const
	{
		if (auto p = get (key))
		{
			std::istringstream sstream (p->getString ());
			sstream.imbue (std::locale::classic ());
			uint8_t c;
			sstream >> c;
			if (sstream.fail () || c != '{')
				return {};
			CCoord x;
			sstream >> x;
			sstream >> c;
			if (sstream.fail () || c != ';')
				return {};
			CCoord y;
			sstream >> y;
			sstream >> c;
			if (sstream.fail () || c != '}')
				return {};
			return {CPoint (x, y)};
		}
		return {};
	}

	inline const UTF8String& getGroupKey () const { return groupKey; }
	inline const UTF8String& getGroupSeparator () const { return groupSeparator; }

private:
	Preferences (const Preferences&) = default;
	Preferences& operator= (const Preferences&) = default;

	IPreference* preferences {&IApplication::instance ().getPreferences ()};
	UTF8String groupKey;
	UTF8String groupSeparator;
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
