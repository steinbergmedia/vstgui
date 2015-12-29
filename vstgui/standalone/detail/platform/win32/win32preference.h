#pragma once

#include "../../../ipreference.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Win32 {

//------------------------------------------------------------------------
class Win32Preference : public IPreference
{
public:
	bool set (const UTF8String& key, const UTF8String& value) override;
	UTF8String get (const UTF8String& key) override;
};

//------------------------------------------------------------------------
} // Mac
} // Platform
} // Standalone
} // VSTGUI
