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

#include "win32support.h"

#if WINDOWS

#if VSTGUI_DIRECT2D_SUPPORT
	#include <d2d1.h>
	#include <dwrite.h>
#endif

#include <shlwapi.h>
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
	, d2d1Dll (0)
	, dwriteDll (0)
	, useCount (0)
	{
		d2d1Dll = LoadLibraryA ("d2d1.dll");
		if (d2d1Dll)
		{
			HRESULT hr = S_OK;
			_D2D1CreateFactory = (D2D1CreateFactoryProc)GetProcAddress (d2d1Dll, "D2D1CreateFactory");
			getFactory ();
			dwriteDll = LoadLibraryA ("dwrite.dll");
			if (dwriteDll)
			{
				DWriteCreateFactoryProc _DWriteCreateFactory = (DWriteCreateFactoryProc)GetProcAddress (dwriteDll, "DWriteCreateFactory");
				if (_DWriteCreateFactory)
				{
					hr = _DWriteCreateFactory (DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (void**)&writeFactory);
				}
			}	
		}
	}

	~D2DFactory ()
	{
		CFontDesc::cleanup ();
		if (writeFactory)
			writeFactory->Release ();
		if (dwriteDll)
			FreeLibrary (dwriteDll);
		if (d2d1Dll)
			FreeLibrary (d2d1Dll);
	}
	ID2D1Factory* getFactory () const
	{
		if (_D2D1CreateFactory && !factory)
		{
			D2D1_FACTORY_OPTIONS* options = 0;
		#if 0 //DEBUG
			D2D1_FACTORY_OPTIONS debugOptions;
			debugOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
			options = &debugOptions;
		#endif
			_D2D1CreateFactory (D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory), options, (void**)&factory);
		}
		return factory;
	}
	
	void use ()
	{
		++useCount;
	}

	void unuse ()
	{
		vstgui_assert (useCount > 0);
		if (--useCount == 0)
			releaseFactory ();
	}

	void releaseFactory ()
	{
		if (factory)
			factory->Release ();
		factory = 0;
	}
	IDWriteFactory* getWriteFactory () const { return writeFactory; }
protected:
	D2D1CreateFactoryProc _D2D1CreateFactory;
	ID2D1Factory* factory;
	IDWriteFactory* writeFactory;
	HMODULE d2d1Dll;
	HMODULE dwriteDll;
	int32_t useCount;
};

//-----------------------------------------------------------------------------
D2DFactory& getD2DFactoryInstance ()
{
	static D2DFactory d2dFactory;
	return d2dFactory;
}
#endif

//-----------------------------------------------------------------------------
ID2D1Factory* getD2DFactory ()
{
#if VSTGUI_DIRECT2D_SUPPORT
	return getD2DFactoryInstance ().getFactory ();
#else
	return 0;
#endif
}

//-----------------------------------------------------------------------------
void useD2D ()
{
#if VSTGUI_DIRECT2D_SUPPORT
	getD2DFactoryInstance ().use ();
#endif
}

//-----------------------------------------------------------------------------
void unuseD2D ()
{
#if VSTGUI_DIRECT2D_SUPPORT
	getD2DFactoryInstance ().unuse ();
#endif
}

//-----------------------------------------------------------------------------
IDWriteFactory* getDWriteFactory ()
{
#if VSTGUI_DIRECT2D_SUPPORT
	return getD2DFactoryInstance ().getWriteFactory ();
#else
	return 0;
#endif
}

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
			return new D2DBitmap (*size);
		return new D2DBitmap ();
	}
#endif
	if (size)
		return new GdiplusBitmap (*size);
	return new GdiplusBitmap ();
}

//-----------------------------------------------------------------------------
IPlatformBitmap* IPlatformBitmap::createFromPath (UTF8StringPtr absolutePath)
{
	UTF8StringHelper path (absolutePath);
	IStream* stream = 0;
	if (SUCCEEDED (SHCreateStreamOnFileEx (path, STGM_READ|STGM_SHARE_DENY_WRITE, 0, false, 0, &stream)))
	{
#if VSTGUI_DIRECT2D_SUPPORT
		if (getD2DFactory ())
		{
			D2DBitmap* result = new D2DBitmap ();
			if (result->loadFromStream (stream))
			{
				stream->Release ();
				return result;
			}
			stream->Release ();
			result->forget ();
			return 0;
		}
#endif
		GdiplusBitmap* bitmap = new GdiplusBitmap ();
		if (bitmap->loadFromStream (stream))
		{
			stream->Release ();
			return bitmap;
		}
		bitmap->forget ();
		stream->Release ();
	}
	return 0;
}

//-----------------------------------------------------------------------------
IPlatformBitmap* IPlatformBitmap::createFromMemory (const void* ptr, uint32_t memSize)
{
#ifdef __GNUC__
	typedef IStream* (*SHCreateMemStreamProc) (const BYTE* pInit, UINT cbInit);
	HMODULE shlwDll = LoadLibraryA ("shlwapi.dll");
	SHCreateMemStreamProc proc = reinterpret_cast<SHCreateMemStreamProc> (GetProcAddress (shlwDll, MAKEINTRESOURCEA (12)));
	IStream* stream = proc (static_cast<const BYTE*> (ptr), memSize);
#else
	IStream* stream = SHCreateMemStream ((const BYTE*)ptr, memSize);
#endif
	if (stream)
	{
#if VSTGUI_DIRECT2D_SUPPORT
		if (getD2DFactory ())
		{
			D2DBitmap* result = new D2DBitmap ();
			if (result->loadFromStream (stream))
			{
				stream->Release ();
				return result;
			}
			stream->Release ();
			result->forget ();
			return 0;
		}
#endif
		GdiplusBitmap* bitmap = new GdiplusBitmap ();
		if (bitmap->loadFromStream (stream))
		{
			stream->Release ();
			return bitmap;
		}
		bitmap->forget ();
		stream->Release ();
	}
#ifdef __GNUC__
	FreeLibrary (shlwDll);
#endif
	return 0;
}

//-----------------------------------------------------------------------------
bool IPlatformBitmap::createMemoryPNGRepresentation (IPlatformBitmap* bitmap, void** ptr, uint32_t& size)
{
	Win32BitmapBase* bitmapBase = dynamic_cast<Win32BitmapBase*> (bitmap);
	if (bitmapBase)
		return bitmapBase->createMemoryPNGRepresentation (ptr, size);
	return false;
}

//-----------------------------------------------------------------------------
IPlatformFont* IPlatformFont::create (const char* name, const CCoord& size, const int32_t& style)
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

//-----------------------------------------------------------------------------
bool IPlatformFont::getAllPlatformFontFamilies (std::list<std::string>& fontFamilyNames)
{
#if VSTGUI_DIRECT2D_SUPPORT
	if (getD2DFactory ())
	{
		return D2DFont::getAllPlatformFontFamilies (fontFamilyNames);
	}
#endif
	return GdiPlusFont::getAllPlatformFontFamilies (fontFamilyNames);
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
	int readSize = std::min<int> (resSize - streamPos, cb);
	if (readSize > 0)
	{
		memcpy (pv, ((uint8_t*)resData+streamPos), readSize);
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
				streamPos = (uint32_t)dlibMove.QuadPart;
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
				streamPos += (int32_t)dlibMove.QuadPart;
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
	memset (pstatstg, 0, sizeof (STATSTG));
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
