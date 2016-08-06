#pragma once

#include "fwd.h"
#include "../lib/cstring.h"
#include "../lib/optional.h"
#include "interface.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
/** Preference interface
 *
 *	@ingroup standalone
 */
class IPreference : public Interface
{
public:
	virtual bool set (const UTF8String& key, const UTF8String& value) = 0;
	virtual Optional<UTF8String> get (const UTF8String& key) = 0;
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
