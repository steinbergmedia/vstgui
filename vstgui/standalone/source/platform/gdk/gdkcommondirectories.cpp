// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "gdkcommondirectories.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace GDK {

//------------------------------------------------------------------------
Optional<UTF8String> CommonDirectories::get (CommonDirectoryLocation location,
											 const UTF8String& subDir,
											 bool create) const
{
	// TODO:
	return {};
}

//------------------------------------------------------------------------
} // GDK
} // Platform
} // Standalone
} // VSTGUI
