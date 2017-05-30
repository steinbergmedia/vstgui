// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32preference.h"
#include "../../../../lib/platform/win32/win32support.h"
#include "../../../../lib/platform/win32/winstring.h"
#include "../../../include/iappdelegate.h"
#include "../../../include/iapplication.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Win32 {

//------------------------------------------------------------------------
Win32Preference::Win32Preference ()
{
	auto& appInfo = IApplication::instance ().getDelegate ().getInfo ();
	vstgui_assert (!appInfo.uri.empty (), "need uri for preferences");
	UTF8String path ("SOFTWARE\\" + appInfo.uri.getString ());
	auto winStr = dynamic_cast<WinString*> (path.getPlatformString ());
	vstgui_assert (winStr);
	DWORD dw;
	RegCreateKeyEx (HKEY_CURRENT_USER, winStr->getWideString (), 0, REG_NONE,
	                REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ, NULL, &hKey, &dw);
}

//------------------------------------------------------------------------
Win32Preference::~Win32Preference ()
{
	RegCloseKey (hKey);
}

//------------------------------------------------------------------------
bool Win32Preference::set (const UTF8String& key, const UTF8String& value)
{
	auto keyStr = dynamic_cast<WinString*> (key.getPlatformString ());
	auto valueStr = dynamic_cast<WinString*> (value.getPlatformString ());
	vstgui_assert (keyStr);
	auto res = RegSetValueEx (hKey, keyStr->getWideString (), NULL, REG_SZ,
	                          reinterpret_cast<const BYTE*> (valueStr->getWideString ()),
	                          static_cast<DWORD> (wcslen (valueStr->getWideString ()) * 2));

	return SUCCEEDED (res);
}

//------------------------------------------------------------------------
Optional<UTF8String> Win32Preference::get (const UTF8String& key)
{
	auto keyStr = dynamic_cast<WinString*> (key.getPlatformString ());
	vstgui_assert (keyStr);

	DWORD dwType {};
	DWORD dwCount {};
	if (SUCCEEDED (
	        RegQueryValueEx (hKey, keyStr->getWideString (), NULL, &dwType, nullptr, &dwCount)) &&
	    dwType == REG_SZ && dwCount > 0)
	{
		auto buffer = std::make_unique<uint8_t[]> (dwCount + 1);
		if (SUCCEEDED (RegQueryValueEx (hKey, keyStr->getWideString (), NULL, &dwType,
		                                buffer.get (), &dwCount)))
		{
			UTF8StringHelper helper (reinterpret_cast<const WCHAR*> (buffer.get ()));
			return Optional<UTF8String> (UTF8String (helper));
		}
	}

	return {};
}

//------------------------------------------------------------------------
} // Win32
} // Platform
} // Standalone
} // VSTGUI
