//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#ifndef __win32support__
#define __win32support__

#include "../../vstguibase.h"

#if WINDOWS

#include "../../cbitmap.h"

#include <windows.h>

#include <objidl.h>
#include <gdiplus.h>

interface ID2D1Factory;
interface IDWriteFactory;

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
extern const OSVERSIONINFOEX& getSystemVersion ();
extern ID2D1Factory* getD2DFactory ();
extern void useD2D ();
extern void unuseD2D ();
extern IDWriteFactory* getDWriteFactory ();
extern CDrawContext* createDrawContext (HWND window, HDC device, const CRect& surfaceRect);
extern void useD2DHardwareRenderer (bool state);

/// @cond ignore
class GDIPlusGlobals : public CBaseObject
{
public:
	static void enter ();
	static void exit ();
protected:
	GDIPlusGlobals ();
	~GDIPlusGlobals ();

	static GDIPlusGlobals* gInstance;
	ULONG_PTR gdiplusToken;
};

class UTF8StringHelper
{
public:
	UTF8StringHelper (const char* utf8Str) : utf8Str (utf8Str), allocWideStr (0), allocStrIsWide (true) {}
	UTF8StringHelper (const WCHAR* wideStr) : wideStr (wideStr), allocUTF8Str (0), allocStrIsWide (false) {}
	~UTF8StringHelper ()
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
				allocWideStr = (WCHAR*)::std::malloc ((numChars+1)*2);
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
				allocUTF8Str = (char*)::std::malloc (allocSize+1);
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
