#pragma once

#include "../../lib/vstguifwd.h"
#include "../../lib/optional.h"
#include "interface.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
/** Shared UI resources interface
 *
 *	@ingroup standalone
 */
class ISharedUIResources : public Interface
{
public:
	/** get shared color. */
	virtual Optional<CColor> getColor (const UTF8String& name) const = 0;
	/** get shared bitmap. */
	virtual Optional<CBitmap*> getBitmap (const UTF8String& name) const = 0;
	/** get shared gradient. */
	virtual Optional<CGradient*> getGradient (const UTF8String& name) const = 0;
	/** get shared font. */
	virtual Optional<CFontDesc*> getFont (const UTF8String& name) const = 0;
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
