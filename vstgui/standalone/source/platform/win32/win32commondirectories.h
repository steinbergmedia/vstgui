#pragma once

#include "../../../include/icommondirectories.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Win32 {

//------------------------------------------------------------------------
class CommonDirectories : public ICommonDirectories
{
public:
	CommonDirectories ();

	UTF8String get (CommonDirectoryLocation location, const UTF8String& subDir, bool create = false) const override;
private:
	UTF8String getLocalAppDataPath (const UTF8String& dir, const UTF8String& subDir, bool create) const;
	UTF8String getAppPath () const;

	UTF8String localAppDataPath;
};

//------------------------------------------------------------------------
} // Win32
} // Platform
} // Standalone
} // VSTGUI
