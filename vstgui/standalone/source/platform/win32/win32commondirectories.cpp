#include "win32commondirectories.h"
#include "../../../../lib/platform/win32/win32support.h"
#include "../../../include/iappdelegate.h"
#include "../../../include/iapplication.h"
#include <Shlobj.h>
#include <array>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Win32 {

//------------------------------------------------------------------------
CommonDirectories::CommonDirectories ()
{
	PWSTR path;
	if (SHGetKnownFolderPath (FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &path) == S_OK)
	{
		localAppDataPath = UTF8StringHelper (path).getUTF8String ();
		localAppDataPath += "\\";
		CoTaskMemFree (path);
	}
}

//------------------------------------------------------------------------
UTF8String CommonDirectories::get (Location location, const UTF8String& subDir, bool create) const
{
	switch (location)
	{
		case Location::AppPath:
		{
			UTF8String appPath;
			std::array<wchar_t, 1024> path;
			GetModuleFileName (GetModuleHandle (nullptr), path.data (),
			                   static_cast<DWORD> (path.size ()));
			appPath = UTF8StringHelper (path.data ()).getUTF8String ();
			return appPath;
		}
		case Location::AppPreferences:
		{
			// TODO:
			break;
		}
		case Location::AppCaches:
		{
			if (!localAppDataPath.empty ())
			{
				UTF8String result (localAppDataPath);
				result += IApplication::instance ().getDelegate ().getInfo ().uri;
				result += "\\Caches\\";
				if (!subDir.empty ())
				{
					result += subDir;
					result += "\\";
				}
				if (create)
				{
					UTF8StringHelper helper (result);
					auto res = SHCreateDirectoryEx (nullptr, helper.getWideString (), nullptr);
					if (!(res == ERROR_SUCCESS || res == ERROR_ALREADY_EXISTS))
						return {};
				}
				return result;
			}
			break;
		}
		case Location::UserDocuments:
		{
			// TODO:
			break;
		}
	}
	return {};
}

//------------------------------------------------------------------------
} // Win32
} // Platform
} // Standalone
} // VSTGUI
