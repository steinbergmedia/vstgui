// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/platform/win32/win32support.h"
#include "../../../include/iappdelegate.h"
#include "../../../include/iapplication.h"
#include "win32commondirectories.h"
#include <array>
#include <shlobj.h>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Win32 {

//------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------
UTF8String GetKnownFolderPathStr (REFKNOWNFOLDERID folderID, bool create)
{
	UTF8String res;
	PWSTR path;
	if (SHGetKnownFolderPath (folderID, create ? KF_FLAG_CREATE : 0, nullptr, &path) == S_OK)
	{
		res = UTF8StringHelper (path).getUTF8String ();
		res += "\\";
		CoTaskMemFree (path);
	}
	return res;
}

//------------------------------------------------------------------------
bool createDirectoryRecursive (const UTF8String& path)
{
	UTF8StringHelper helper (path.data ());
	auto res = SHCreateDirectoryEx (nullptr, helper.getWideString (), nullptr);
	if (!(res == ERROR_SUCCESS || res == ERROR_ALREADY_EXISTS))
		return false;
	return true;
}

//------------------------------------------------------------------------
bool addSubDir (UTF8String& path, const UTF8String& subDir, bool create)
{
	if (!subDir.empty ())
	{
		path += subDir;
		path += "\\";
	}
	if (create && !createDirectoryRecursive (path))
	{
		return false;
	}
	return true;
}

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
CommonDirectories::CommonDirectories ()
{
	localAppDataPath = GetKnownFolderPathStr (FOLDERID_LocalAppData, true);
}

//------------------------------------------------------------------------
void CommonDirectories::setAppResourcePath (const UTF8String& path)
{
	appResourcePath = path;
}

//------------------------------------------------------------------------
Optional<UTF8String> CommonDirectories::getLocalAppDataPath (const UTF8String& dir,
                                                             const UTF8String& subDir,
                                                             bool create) const
{
	if (!localAppDataPath.empty ())
	{
		UTF8String result (localAppDataPath);
		result += IApplication::instance ().getDelegate ().getInfo ().uri;
		result += "\\";
		result += dir;
		result += "\\";
		if (!addSubDir (result, subDir, create))
			result = {};
		return result;
	}
	return {};
}

//------------------------------------------------------------------------
Optional<UTF8String> CommonDirectories::getAppPath () const
{
	UTF8String appPath;
	std::array<wchar_t, 1024> path;
	GetModuleFileName (GetModuleHandle (nullptr), path.data (), static_cast<DWORD> (path.size ()));
	appPath = UTF8StringHelper (path.data ()).getUTF8String ();
	return appPath;
}

//------------------------------------------------------------------------
Optional<UTF8String> CommonDirectories::get (CommonDirectoryLocation location,
                                             const UTF8String& subDir, bool create) const
{
	switch (location)
	{
		case CommonDirectoryLocation::AppPath: return getAppPath ();
		case CommonDirectoryLocation::AppResourcesPath:
		{
			UTF8String result (appResourcePath);
			if (!addSubDir (result, subDir, create))
				return {};
			return result;
		}
		case CommonDirectoryLocation::AppPreferencesPath:
			return getLocalAppDataPath ("Preferences", subDir, create);
		case CommonDirectoryLocation::AppCachesPath:
			return getLocalAppDataPath ("Caches", subDir, create);
		case CommonDirectoryLocation::UserDocumentsPath:
		{
			auto result = GetKnownFolderPathStr (FOLDERID_Documents, create);
			if (result.empty () || !addSubDir (result, subDir, create))
				return {};
			return result;
		}
	}
	return {};
}

//------------------------------------------------------------------------
} // Win32
} // Platform
} // Standalone
} // VSTGUI
