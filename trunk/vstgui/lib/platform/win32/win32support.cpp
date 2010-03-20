//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "win32support.h"

#if WINDOWS

#if VSTGUI_DIRECT2D_SUPPORT
	#include <d2d1.h>
	#include <dwrite.h>
#endif

#include "cfontwin32.h"
#include "gdiplusbitmap.h"
#include "gdiplusdrawcontext.h"
#include "direct2d/d2ddrawcontext.h"
#include "direct2d/d2dbitmap.h"
#include "direct2d/d2dfont.h"

extern void* hInstance;

namespace VSTGUI {

HINSTANCE GetInstance () { return (HINSTANCE)hInstance; }

const OSVERSIONINFOEX& getSystemVersion ()
{
	static OSVERSIONINFOEX gSystemVersion = {0};
	static bool once = true;
	if (once)
	{
		memset (&gSystemVersion, 0, sizeof (gSystemVersion));
		gSystemVersion.dwOSVersionInfoSize = sizeof (gSystemVersion);
		GetVersionEx ((OSVERSIONINFO *)&gSystemVersion);
	}
	return gSystemVersion;
}

//-----------------------------------------------------------------------------
#if VSTGUI_DIRECT2D_SUPPORT
typedef HRESULT (WINAPI *D2D1CreateFactoryProc) (D2D1_FACTORY_TYPE type, REFIID riid, CONST D2D1_FACTORY_OPTIONS *pFactoryOptions, void** factory);
typedef HRESULT (WINAPI *DWriteCreateFactoryProc) (DWRITE_FACTORY_TYPE factoryType, REFIID iid, void** factory);

class D2DFactory
{
public:
	D2DFactory ()
	: factory (0)
	, writeFactory (0)
	{
		HMODULE d2d1Dll = LoadLibraryA ("d2d1.dll");
		if (d2d1Dll)
		{
			D2D1CreateFactoryProc _D2D1CreateFactory = (D2D1CreateFactoryProc)GetProcAddress (d2d1Dll, "D2D1CreateFactory");
			if (_D2D1CreateFactory)
			{
				HRESULT hr = _D2D1CreateFactory (D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), 0, (void**)&factory);
			}
		}
		HMODULE dwriteDll = LoadLibraryA ("dwrite.dll");
		if (dwriteDll)
		{
			DWriteCreateFactoryProc _DWriteCreateFactory = (DWriteCreateFactoryProc)GetProcAddress (dwriteDll, "DWriteCreateFactory");
			if (_DWriteCreateFactory)
			{
				HRESULT hr = _DWriteCreateFactory (DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (void**)&writeFactory);
			}
		}	
	}

	~D2DFactory ()
	{
		if (factory)
			factory->Release ();
		if (writeFactory)
			writeFactory->Release ();
	}
	ID2D1Factory* getFactory () const { return factory; }
	IDWriteFactory* getWriteFactory () const { return writeFactory; }
protected:
	ID2D1Factory* factory;
	IDWriteFactory* writeFactory;
};

static D2DFactory d2dFactory;

//-----------------------------------------------------------------------------
ID2D1Factory* getD2DFactory ()
{
	return d2dFactory.getFactory ();
}

IDWriteFactory* getDWriteFactory ()
{
	return d2dFactory.getWriteFactory ();
}
#endif

//-----------------------------------------------------------------------------
CDrawContext* createDrawContext (HWND window, HDC device, const CRect& surfaceRect)
{
#if VSTGUI_DIRECT2D_SUPPORT
	if (getD2DFactory ())
		return new D2DDrawContext (window, surfaceRect);
#endif
	return new GdiplusDrawContext (window, surfaceRect);
}

//-----------------------------------------------------------------------------
IPlatformBitmap* IPlatformBitmap::create (CPoint* size)
{
#if VSTGUI_DIRECT2D_SUPPORT
	if (getD2DFactory ())
	{
		if (size)
			return new D2DOffscreenBitmap (*size);
		else
			return new D2DBitmap ();
	}
#endif
	if (size)
		return new GdiplusBitmap (*size);
	return new GdiplusBitmap ();
}

//-----------------------------------------------------------------------------
IPlatformFont* IPlatformFont::create (const char* name, const CCoord& size, const long& style)
{
#if VSTGUI_DIRECT2D_SUPPORT
	if (getD2DFactory ())
	{
		return new D2DFont (name, size, style);
	}
#endif
	GdiPlusFont* font = new GdiPlusFont (name, size, style);
	if (font->getFont ())
		return font;
	font->forget ();
	return 0;
}

/// @cond ignore
//-----------------------------------------------------------------------------
GDIPlusGlobals* GDIPlusGlobals::gInstance = 0;

//-----------------------------------------------------------------------------
GDIPlusGlobals::GDIPlusGlobals ()
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup (&gdiplusToken, &gdiplusStartupInput, NULL);
}

//-----------------------------------------------------------------------------
GDIPlusGlobals::~GDIPlusGlobals ()
{
	CFontDesc::cleanup ();
	Gdiplus::GdiplusShutdown (gdiplusToken);
}

//-----------------------------------------------------------------------------
void GDIPlusGlobals::enter ()
{
	if (gInstance)
		gInstance->remember ();
	else
		gInstance = new GDIPlusGlobals;
}

//-----------------------------------------------------------------------------
void GDIPlusGlobals::exit ()
{
	if (gInstance)
	{
		bool destroyed = (gInstance->getNbReference () == 1);
		gInstance->forget ();
		if (destroyed)
			gInstance = 0;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ResourceStream::ResourceStream ()
: streamPos (0)
, resData (0)
, resSize (0)
, _refcount (1)
{
}

//-----------------------------------------------------------------------------
bool ResourceStream::open (const CResourceDescription& resourceDesc, const char* type)
{
	HRSRC rsrc = 0;
	if (resourceDesc.type == CResourceDescription::kIntegerType)
		rsrc = FindResourceA (GetInstance (), MAKEINTRESOURCEA (resourceDesc.u.id), type);
	else
		rsrc = FindResourceA (GetInstance (), resourceDesc.u.name, type);
	if (rsrc)
	{
		resSize = SizeofResource (GetInstance (), rsrc);
		HGLOBAL resDataLoad = LoadResource (GetInstance (), rsrc);
		if (resDataLoad)
		{
			resData = LockResource (resDataLoad);
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::Read (void *pv, ULONG cb, ULONG *pcbRead)
{
	int readSize = min (resSize - streamPos, cb);
	if (readSize > 0)
	{
		memcpy (pv, ((unsigned char*)resData+streamPos), readSize);
		streamPos += readSize;
		if (pcbRead)
			*pcbRead = readSize;
		return S_OK;
	}
	if (pcbRead)
		*pcbRead = 0;
	return S_FALSE;
}

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::Write (const void *pv, ULONG cb, ULONG *pcbWritten) { return E_NOTIMPL; }

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::Seek (LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
	switch(dwOrigin)
	{
		case STREAM_SEEK_SET:
		{
			if (dlibMove.QuadPart < resSize)
			{
				streamPos = (unsigned long)dlibMove.QuadPart;
				if (plibNewPosition)
					plibNewPosition->QuadPart = streamPos;
				return S_OK;
			}
			break;
		}
		case STREAM_SEEK_CUR:
		{
			if (streamPos + dlibMove.QuadPart < resSize && streamPos + dlibMove.QuadPart >= 0)
			{
				streamPos += (long)dlibMove.QuadPart;
				if (plibNewPosition)
					plibNewPosition->QuadPart = streamPos;
				return S_OK;
			}
			break;
		}
		case STREAM_SEEK_END:
		{
			break;
		}
		default:   
			return STG_E_INVALIDFUNCTION;
		break;
	}
	return S_FALSE;
}

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::SetSize (ULARGE_INTEGER libNewSize) { return E_NOTIMPL; }

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::CopyTo (IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten) { return E_NOTIMPL; }

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::Commit (DWORD grfCommitFlags) { return E_NOTIMPL; }

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::Revert (void) 
{ 
	streamPos = 0;
	return S_OK; 
}

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::LockRegion (ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) { return E_NOTIMPL; }

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::UnlockRegion (ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) { return E_NOTIMPL; }

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::Stat (STATSTG *pstatstg, DWORD grfStatFlag)
{
	pstatstg->cbSize.QuadPart = resSize;
	return S_OK;
}

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::Clone (IStream **ppstm) { return E_NOTIMPL; }

//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ResourceStream::QueryInterface(REFIID iid, void ** ppvObject)
{ 
    if (iid == __uuidof(IUnknown)
        || iid == __uuidof(IStream)
        || iid == __uuidof(ISequentialStream))
    {
        *ppvObject = static_cast<IStream*>(this);
        AddRef();
        return S_OK;
    } else
        return E_NOINTERFACE; 
}

//-----------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE ResourceStream::AddRef(void) 
{ 
    return (ULONG)InterlockedIncrement(&_refcount); 
}

//-----------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE ResourceStream::Release(void) 
{
    ULONG res = (ULONG) InterlockedDecrement(&_refcount);
    if (res == 0) 
        delete this;
    return res;
}

/// @endcond ignore

} // namespace

#endif // WINDOWS
