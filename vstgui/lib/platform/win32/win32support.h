// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../vstguibase.h"

#if WINDOWS

struct IUnknown;

#include "../../cbitmap.h"
#include "../../optional.h"
#include "../iplatformresourceinputstream.h"
#include <algorithm>
#include <windows.h>
#include <combaseapi.h>

interface ID2D1Factory;
interface IDWriteFactory;
interface IWICImagingFactory;

struct VstKeyCode;

namespace VSTGUI {

#define VSTGUI_STRCMP	wcscmp
#define VSTGUI_STRCPY	wcscpy
#define VSTGUI_SPRINTF	wsprintf
#define VSTGUI_STRRCHR	wcschr
#define VSTGUI_STRICMP	_wcsicmp
#define VSTGUI_STRLEN	wcslen
#define VSTGUI_STRCAT	wcscat

class CDrawContext;

extern HINSTANCE GetInstance ();
extern ID2D1Factory* getD2DFactory ();
extern IWICImagingFactory* getWICImageingFactory ();
extern void useD2D ();
extern void unuseD2D ();
extern IDWriteFactory* getDWriteFactory ();
extern CDrawContext* createDrawContext (HWND window, HDC device, const CRect& surfaceRect);
extern Optional<VstKeyCode> keyMessageToKeyCode (WPARAM wParam, LPARAM lParam);

class UTF8StringHelper
{
public:
	UTF8StringHelper (const char* utf8Str, int numChars = -1) : utf8Str (utf8Str), allocWideStr (nullptr), allocStrIsWide (true), numCharacters (numChars) {}
	UTF8StringHelper (const WCHAR* wideStr, int numChars = -1) : wideStr (wideStr), allocUTF8Str (nullptr), allocStrIsWide (false), numCharacters (numChars) {}
	~UTF8StringHelper () noexcept
	{
		if (allocUTF8Str)
			std::free (allocUTF8Str);
	}

	operator const char* () { return getUTF8String (); }
	operator const WCHAR*() { return getWideString (); }

	const WCHAR* getWideString ()
	{
		if (!allocStrIsWide)
			return wideStr;
		else
		{
			if (!allocWideStr && utf8Str)
			{
				int numChars = MultiByteToWideChar (CP_UTF8, 0, utf8Str, numCharacters, nullptr, 0);
				allocWideStr = (WCHAR*)::std::malloc ((static_cast<size_t> (numChars)+1)*sizeof (WCHAR));
				if (allocWideStr)
				{
					if (MultiByteToWideChar (CP_UTF8, 0, utf8Str, numCharacters, allocWideStr,
											 numChars) == 0)
						allocWideStr[0] = 0;
					else
						allocWideStr[numChars] = 0;
				}
			}
			return allocWideStr;
		}
	}
	const char* getUTF8String ()
	{
		if (allocStrIsWide)
			return utf8Str;
		else
		{
			if (!allocUTF8Str && wideStr)
			{
				int allocSize = WideCharToMultiByte (CP_UTF8, 0, wideStr, numCharacters, nullptr, 0, nullptr, nullptr);
				allocUTF8Str = (char*)::std::malloc (static_cast<size_t> (allocSize)+1);
				if (allocUTF8Str)
				{
					if (WideCharToMultiByte (CP_UTF8, 0, wideStr, numCharacters, allocUTF8Str,
											 allocSize, nullptr, nullptr) == 0)
						allocUTF8Str[0] = 0;
					else
						allocUTF8Str[allocSize] = 0;
				}
			}
			return allocUTF8Str;
		}
	}
protected:
	union {
		const char* utf8Str;
		const WCHAR* wideStr;
	};
	union {
		WCHAR* allocWideStr;
		char* allocUTF8Str;
	};

	bool allocStrIsWide;
	int numCharacters {-1};
};

/// @endcond

} // VSTGUI

#endif // WINDOWS
