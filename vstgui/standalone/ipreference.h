#pragma once

#include "fwd.h"
#include "interface.h"
#include "../lib/cstring.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
class IPreference : public Interface
{
public:
	virtual bool set (const UTF8String& key, const UTF8String& value) = 0;
	virtual UTF8String get (const UTF8String& key) = 0;
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
