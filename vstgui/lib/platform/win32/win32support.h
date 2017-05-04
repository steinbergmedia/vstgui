// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __win32support__
#define __win32support__

#include "../../vstguibase.h"

#if WINDOWS

#include "../../cbitmap.h"
#include "../../optional.h"

#include <algorithm>
using std::min;
using std::max;
#include <windows.h>
#if defined (WINAPI_FAMILY_SYSTEM)
#include <VersionHelpers.h>
#endif

#include <objidl.h>
#include <gdiplus.h>

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
#ifndef VERSIONHELPERAPI
extern const OSVERSIONINFOEX& getSystemVersion();
extern const bool IsWindowsVistaOrGreater();
#endif
extern ID2D1Factory* getD2DFactory ();
extern IWICImagingFactory* getWICImageingFactory ();
extern void useD2D ();
extern void unuseD2D ();
extern IDWriteFactory* getDWriteFactory ();
extern CDrawContext* createDrawContext (HWND window, HDC device, const CRect& surfaceRect);
extern void useD2DHardwareRenderer (bool state);
extern Optional<VstKeyCode> keyMessageToKeyCode (WPARAM wParam, LPARAM lParam);

/// @cond ignore
class GDIPlusGlobals : public AtomicReferenceCounted
{
public:
	static void enter ();
	static void exit ();
protected:
	GDIPlusGlobals ();
	~GDIPlusGlobals () noexcept;

	static GDIPlusGlobals* gInstance;
	ULONG_PTR gdiplusToken;
};

class UTF8StringHelper
{
public:
	UTF8StringHelper (const char* utf8Str) : utf8Str (utf8Str), allocWideStr (0), allocStrIsWide (true) {}
	UTF8StringHelper (const WCHAR* wideStr) : wideStr (wideStr), allocUTF8Str (0), allocStrIsWide (false) {}
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
				int numChars = MultiByteToWideChar (CP_UTF8, 0, utf8Str, -1, 0, 0);
				allocWideStr = (WCHAR*)::std::malloc ((static_cast<size_t> (numChars)+1)*2);
				if (MultiByteToWideChar (CP_UTF8, 0, utf8Str, -1, allocWideStr, numChars) == 0)
				{
					allocWideStr[0] = 0;
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
				int allocSize = WideCharToMultiByte (CP_UTF8, 0, wideStr, -1, 0, 0, 0, 0);
				allocUTF8Str = (char*)::std::malloc (static_cast<size_t> (allocSize)+1);
				if (WideCharToMultiByte (CP_UTF8, 0, wideStr, -1, allocUTF8Str, allocSize, 0, 0) == 0)
				{
					allocUTF8Str[0] = 0;
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
};

class ResourceStream : public IStream
{
public:
	ResourceStream ();
	~ResourceStream () noexcept = default;

	bool open (const CResourceDescription& resourceDesc, const char* type);

	virtual HRESULT STDMETHODCALLTYPE Read (void *pv, ULONG cb, ULONG *pcbRead);
    virtual HRESULT STDMETHODCALLTYPE Write (const void *pv, ULONG cb, ULONG *pcbWritten);
    virtual HRESULT STDMETHODCALLTYPE Seek (LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);    
    virtual HRESULT STDMETHODCALLTYPE SetSize (ULARGE_INTEGER libNewSize);
    virtual HRESULT STDMETHODCALLTYPE CopyTo (IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
    virtual HRESULT STDMETHODCALLTYPE Commit (DWORD grfCommitFlags);
    virtual HRESULT STDMETHODCALLTYPE Revert (void);
    virtual HRESULT STDMETHODCALLTYPE LockRegion (ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    virtual HRESULT STDMETHODCALLTYPE UnlockRegion (ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    virtual HRESULT STDMETHODCALLTYPE Stat (STATSTG *pstatstg, DWORD grfStatFlag);
    virtual HRESULT STDMETHODCALLTYPE Clone (IStream **ppstm);
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);

protected:
	HGLOBAL resData;
	uint32_t streamPos;
	uint32_t resSize;
	LONG _refcount;
};

/// @endcond

} // namespace

#endif // WINDOWS

#endif
