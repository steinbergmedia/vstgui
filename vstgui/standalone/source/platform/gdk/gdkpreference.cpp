// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "gdkpreference.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace GDK {

Preference::Preference () {}

Preference::~Preference () noexcept {}

bool Preference::set (const UTF8String& key, const UTF8String& value)
{
	return false;
}

Optional<UTF8String> Preference::get (const UTF8String& key)
{
	return {};
}

//------------------------------------------------------------------------
} // GDK
} // Platform
} // Standalone
} // VSTGUI
