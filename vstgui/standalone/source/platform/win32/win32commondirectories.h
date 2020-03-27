// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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

	Optional<UTF8String> get (CommonDirectoryLocation location, const UTF8String& subDir,
	                          bool create = false) const override;

	void setAppResourcePath (const UTF8String& path);
	
private:
	Optional<UTF8String> getLocalAppDataPath (const UTF8String& dir, const UTF8String& subDir,
	                                          bool create) const;
	Optional<UTF8String> getAppPath () const;

	UTF8String localAppDataPath;
	UTF8String appResourcePath;
};

//------------------------------------------------------------------------
} // Win32
} // Platform
} // Standalone
} // VSTGUI
