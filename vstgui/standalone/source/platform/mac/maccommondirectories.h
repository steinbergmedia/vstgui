#pragma once

#include "../../../include/icommondirectories.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Mac {

//------------------------------------------------------------------------
class CommonDirectories : public ICommonDirectories
{
public:
	Optional<UTF8String> get (CommonDirectoryLocation location, const UTF8String& subDir,
	                          bool create = false) const override;
};

//------------------------------------------------------------------------
} // Mac
} // Platform
} // Standalone
} // VSTGUI
