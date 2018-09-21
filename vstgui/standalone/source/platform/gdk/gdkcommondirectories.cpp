// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "gdkcommondirectories.h"
#include "../../../include/iapplication.h"
#include "../../../include/iappdelegate.h"
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>

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
	UTF8String result;
	switch (location)
	{
		case CommonDirectoryLocation::AppPreferencesPath:
		{
			auto home = getenv ("HOME");
			if (home == nullptr)
				return {};
			result = home;
			result += "/.config/" + IApplication::instance ().getDelegate ().getInfo ().uri + "/";
			break;
		}
	}
	if (result.empty ())
		return {};
	if (!subDir.empty ())
		result += subDir + "/";
	if (create)
	{
		struct stat s {};
		if (stat (result.data (), &s) == 0)
		{
			if ((s.st_mode & S_IFMT) != S_IFDIR)
				return {};
		}
		else if (mkdir (result.data (), 0755) != 0)
			return {};
	}
	return Optional<UTF8String> (std::move (result));
}

//------------------------------------------------------------------------
} // GDK
} // Platform
} // Standalone
} // VSTGUI
