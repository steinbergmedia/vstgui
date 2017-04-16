#pragma once

#include "../ipreference.h"
#include "../iapplication.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
class Preferences
{
public:
	Preferences () : preferences (IApplication::instance ().getPreferences ()) {}
	
	inline bool set (const UTF8String& key, const UTF8String& value)
	{
		return preferences.set (key, value);
	}

	inline Optional<UTF8String> get (const UTF8String& key)
	{
		return preferences.get (key);
	}
	
	template<typename T>
	inline bool setNumber (const UTF8String& key, T value)
	{
		return set (key, toString (value));
	}

	template<typename T>
	inline Optional<T> getNumber (const UTF8String& key)
	{
		if (auto p = get (key))
		{
			return UTF8StringView (*p).toNumber<T> ();
		}
		return {};
	}

	inline bool setPoint (const UTF8String& key, CPoint p, uint32_t precision = 8)
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
	
	inline Optional<CPoint> getPoint (const UTF8String& key)
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
	
private:
	IPreference& preferences;
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
