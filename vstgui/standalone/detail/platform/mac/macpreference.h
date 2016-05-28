#pragma once

#import "../../../ipreference.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Mac {

//------------------------------------------------------------------------
class MacPreference : public IPreference
{
public:
	bool set (const UTF8String& key, const UTF8String& value) override;
	Optional<UTF8String> get (const UTF8String& key) override;
};

//------------------------------------------------------------------------
} // Mac
} // Platform
} // Standalone
} // VSTGUI
