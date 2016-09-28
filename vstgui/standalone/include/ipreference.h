#pragma once

#include "fwd.h"
#include "../../lib/cstring.h"
#include "../../lib/optional.h"
#include "interface.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
/** Preference interface
 *
 *	You get the preferences via IApplication::instance ().getPreferences ().
 *	@ingroup standalone
 */
class IPreference : public Interface
{
public:
	/** Set a preference value. */
	virtual bool set (const UTF8String& key, const UTF8String& value) = 0;
	/** Get a preference value */
	virtual Optional<UTF8String> get (const UTF8String& key) = 0;
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
