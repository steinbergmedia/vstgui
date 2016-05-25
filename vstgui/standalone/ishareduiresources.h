#pragma once

#include "interface.h"
#include "../lib/optional.h"
#include "../lib/vstguifwd.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
class ISharedUIResources : public Interface
{
public:
	virtual Optional<CColor> getColor (const UTF8String& name) const = 0;
	virtual Optional<CBitmap*> getBitmap (const UTF8String& name) const = 0;
	virtual Optional<CGradient*> getGradient (const UTF8String& name) const = 0;
	virtual Optional<CFontDesc*> getFont (const UTF8String& name) const = 0;
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
