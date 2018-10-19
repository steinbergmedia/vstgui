// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32support.h"

#if WINDOWS

#include "../../vstkeycode.h"
#include "../common/fileresourceinputstream.h"
#include "../platform_win32.h"

#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>

#include <shlwapi.h>
#include "direct2d/d2ddrawcontext.h"
#include "direct2d/d2dbitmap.h"
#include "direct2d/d2dfont.h"

#ifdef _MSC_VER
#pragma comment (lib,"windowscodecs.lib")
#pragma comment (lib,"d2d1.lib")
#pragma comment (lib,"dwrite.lib")
#endif

extern void* hInstance;

namespace VSTGUI {

HINSTANCE GetInstance () { return (HINSTANCE)hInstance; }

//-----------------------------------------------------------------------------
class D2DFactory
{
public:
	D2DFactory ()
	{
	}

	~D2DFactory () noexcept
	{
		CFontDesc::cleanup ();
		releaseFactory ();
	}
	ID2D1Factory* getFactory () const
	{
		if (factory == nullptr)
		{
			D2D1_FACTORY_OPTIONS* options = 0;
		#if 0 //DEBUG
			D2D1_FACTORY_OPTIONS debugOptions;
			debugOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
			options = &debugOptions;
		#endif
			D2D1CreateFactory (D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory), options, (void**)&factory);
		}
		return factory;
	}
	
	IDWriteFactory* getWriteFactory ()
	{
		if (!writeFactory)
			DWriteCreateFactory (DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&writeFactory);
		return writeFactory;
	}

	IWICImagingFactory* getImagingFactory ()
	{
		if (imagingFactory == nullptr)
		{
#if _WIN32_WINNT > 0x601
// make sure when building with the Win 8.0 SDK we work on Win7
#define VSTGUI_WICImagingFactory CLSID_WICImagingFactory1
#else
#define VSTGUI_WICImagingFactory CLSID_WICImagingFactory
#endif
			CoCreateInstance (VSTGUI_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (void**)&imagingFactory);
		}
		return imagingFactory;
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

private:
	void releaseFactory ()
	{
		if (writeFactory)
			writeFactory->Release ();
		writeFactory = nullptr;
		if (imagingFactory)
			imagingFactory->Release ();
		imagingFactory = nullptr;
		if (factory)
			factory->Release ();
		factory = nullptr;
	}

	ID2D1Factory* factory {nullptr};
	IDWriteFactory* writeFactory {nullptr};
	IWICImagingFactory* imagingFactory {nullptr};
	int32_t useCount {0};
};

//-----------------------------------------------------------------------------
D2DFactory& getD2DFactoryInstance ()
{
	static D2DFactory d2dFactory;
	return d2dFactory;
}

//-----------------------------------------------------------------------------
ID2D1Factory* getD2DFactory ()
{
	return getD2DFactoryInstance ().getFactory ();
}

//-----------------------------------------------------------------------------
IWICImagingFactory* getWICImageingFactory ()
{
	return getD2DFactoryInstance ().getImagingFactory ();
}

//-----------------------------------------------------------------------------
void useD2D ()
{
	getD2DFactoryInstance ().use ();
}

//-----------------------------------------------------------------------------
void unuseD2D ()
{
	getD2DFactoryInstance ().unuse ();
}

//-----------------------------------------------------------------------------
IDWriteFactory* getDWriteFactory ()
{
	return getD2DFactoryInstance ().getWriteFactory ();
}

//-----------------------------------------------------------------------------
CDrawContext* createDrawContext (HWND window, HDC device, const CRect& surfaceRect)
{
	return new D2DDrawContext (window, surfaceRect);
}

//-----------------------------------------------------------------------------
SharedPointer<IPlatformBitmap> IPlatformBitmap::create (CPoint* size)
{
	if (size)
		return owned<IPlatformBitmap> (new D2DBitmap (*size));
	return owned<IPlatformBitmap> (new D2DBitmap ());
}

//------------------------------------------------------------------------
static SharedPointer<IPlatformBitmap> createFromIStream (IStream* stream)
{
	auto result = owned (new D2DBitmap ());
	if (result->loadFromStream (stream))
	{
		return shared<IPlatformBitmap> (result);
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
SharedPointer<IPlatformBitmap> IPlatformBitmap::createFromPath (UTF8StringPtr absolutePath)
{
	UTF8StringHelper path (absolutePath);
	IStream* stream = 0;
	if (SUCCEEDED (SHCreateStreamOnFileEx (path, STGM_READ|STGM_SHARE_DENY_WRITE, 0, false, 0, &stream)))
	{
		auto result = createFromIStream (stream);
		stream->Release ();
		return result;
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
SharedPointer<IPlatformBitmap> IPlatformBitmap::createFromMemory (const void* ptr, uint32_t memSize)
{
#ifdef __GNUC__
	using SHCreateMemStreamProc = IStream* (*) (const BYTE* pInit, UINT cbInit);
	HMODULE shlwDll = LoadLibraryA ("shlwapi.dll");
	SHCreateMemStreamProc proc = reinterpret_cast<SHCreateMemStreamProc> (GetProcAddress (shlwDll, MAKEINTRESOURCEA (12)));
	IStream* stream = proc (static_cast<const BYTE*> (ptr), memSize);
#else
	IStream* stream = SHCreateMemStream ((const BYTE*)ptr, memSize);
#endif
	if (stream)
	{
		auto result = createFromIStream (stream);
		stream->Release ();
		return result;
	}
#ifdef __GNUC__
	FreeLibrary (shlwDll);
#endif
	return 0;
}

//-----------------------------------------------------------------------------
PNGBitmapBuffer IPlatformBitmap::createMemoryPNGRepresentation (const SharedPointer<IPlatformBitmap>& bitmap)
{
	if (auto bitmapBase = bitmap.cast<Win32BitmapBase> ())
		return bitmapBase->createMemoryPNGRepresentation ();
	return {};
}

//-----------------------------------------------------------------------------
SharedPointer<IPlatformFont> IPlatformFont::create (const UTF8String& name, const CCoord& size, const int32_t& style)
{
	return owned<IPlatformFont> (new D2DFont (name, size, style));
}

//-----------------------------------------------------------------------------
bool IPlatformFont::getAllPlatformFontFamilies (std::list<std::string>& fontFamilyNames)
{
	return D2DFont::getAllPlatformFontFamilies (fontFamilyNames);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ResourceStream::ResourceStream ()
: resData (0)
, streamPos (0)
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
	int readSize = std::min<int> (static_cast<int> (resSize - streamPos), static_cast<int> (cb));
	if (readSize > 0)
	{
		memcpy (pv, ((uint8_t*)resData+streamPos), static_cast<size_t> (readSize));
		streamPos += static_cast<uint32_t> (readSize);
		if (pcbRead)
			*pcbRead = static_cast<ULONG> (readSize);
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
				streamPos += static_cast<uint32_t> (dlibMove.QuadPart);
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

//-----------------------------------------------------------------------------
Optional<VstKeyCode> keyMessageToKeyCode (WPARAM wParam, LPARAM lParam)
{
	static std::map<WPARAM, VstVirtualKey> vMap = {
		{VK_BACK, VKEY_BACK}, 
		{VK_TAB, VKEY_TAB}, 
		{VK_CLEAR, VKEY_CLEAR}, 
		{VK_RETURN, VKEY_RETURN}, 
		{VK_PAUSE, VKEY_PAUSE}, 
		{VK_ESCAPE, VKEY_ESCAPE}, 
		{VK_SPACE, VKEY_SPACE}, 
		{VK_NEXT, VKEY_NEXT}, 
		{VK_END, VKEY_END}, 
		{VK_HOME, VKEY_HOME}, 
		{VK_LEFT, VKEY_LEFT}, 
		{VK_UP, VKEY_UP}, 
		{VK_RIGHT, VKEY_RIGHT}, 
		{VK_DOWN, VKEY_DOWN}, 
		// {VK_PAGEUP, VKEY_PAGEUP}, 
		// {VK_PAGEDOWN, VKEY_PAGEDOWN}, 
		{VK_SELECT, VKEY_SELECT}, 
		{VK_PRINT, VKEY_PRINT}, 
		// {VK_ENTER, VKEY_ENTER}, 
		{VK_SNAPSHOT, VKEY_SNAPSHOT}, 
		{VK_INSERT, VKEY_INSERT}, 
		{VK_DELETE, VKEY_DELETE}, 
		{VK_HELP, VKEY_HELP}, 
		{VK_NUMPAD0, VKEY_NUMPAD0}, 
		{VK_NUMPAD1, VKEY_NUMPAD1}, 
		{VK_NUMPAD2, VKEY_NUMPAD2}, 
		{VK_NUMPAD3, VKEY_NUMPAD3}, 
		{VK_NUMPAD4, VKEY_NUMPAD4}, 
		{VK_NUMPAD5, VKEY_NUMPAD5}, 
		{VK_NUMPAD6, VKEY_NUMPAD6}, 
		{VK_NUMPAD7, VKEY_NUMPAD7}, 
		{VK_NUMPAD8, VKEY_NUMPAD8}, 
		{VK_NUMPAD9, VKEY_NUMPAD9}, 
		{VK_MULTIPLY, VKEY_MULTIPLY}, 
		{VK_ADD, VKEY_ADD}, 
		{VK_SEPARATOR, VKEY_SEPARATOR}, 
		{VK_SUBTRACT, VKEY_SUBTRACT}, 
		{VK_DECIMAL, VKEY_DECIMAL}, 
		{VK_DIVIDE, VKEY_DIVIDE}, 
		{VK_F1, VKEY_F1}, 
		{VK_F2, VKEY_F2}, 
		{VK_F3, VKEY_F3}, 
		{VK_F4, VKEY_F4}, 
		{VK_F5, VKEY_F5}, 
		{VK_F6, VKEY_F6}, 
		{VK_F7, VKEY_F7}, 
		{VK_F8, VKEY_F8}, 
		{VK_F9, VKEY_F9}, 
		{VK_F10, VKEY_F10}, 
		{VK_F11, VKEY_F11}, 
		{VK_F12, VKEY_F12}, 
		{VK_NUMLOCK, VKEY_NUMLOCK}, 
		{VK_SCROLL, VKEY_SCROLL},
		{VK_SHIFT, VKEY_SHIFT},
		{VK_CONTROL, VKEY_CONTROL},
		// {VK_ALT, VKEY_ALT},
		{VK_OEM_NEC_EQUAL, VKEY_EQUALS} // TODO: verify
	};
	auto it = vMap.find (wParam);
	if (it != vMap.end ())
	{
		VstKeyCode res {};
		res.virt = it->second;
		return Optional<VstKeyCode> (res);
	}
	return {};
}

//-----------------------------------------------------------------------------
auto WinResourceInputStream::create (const CResourceDescription& desc) -> Ptr
{
	auto stream = ResourceStreamPtr (new ResourceStream ());
	if (stream->open (desc, "DATA"))
	{
		return Ptr (new WinResourceInputStream (std::move (stream)));
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
WinResourceInputStream::WinResourceInputStream (ResourceStreamPtr&& stream)
: stream (std::move (stream))
{
}

//-----------------------------------------------------------------------------
uint32_t WinResourceInputStream::readRaw (void* buffer, uint32_t size)
{
	ULONG read = 0;
	if (stream->Read (buffer, size, &read) == S_OK)
		return read;
	return kStreamIOError;
}

//-----------------------------------------------------------------------------
int64_t WinResourceInputStream::seek (int64_t pos, SeekMode mode)
{
	DWORD dwOrigin;
	switch (mode)
	{
		case SeekMode::Set: dwOrigin = STREAM_SEEK_SET; break;
		case SeekMode::Current: dwOrigin = STREAM_SEEK_CUR; break;
		case SeekMode::End: dwOrigin = STREAM_SEEK_END; break;
	}
	LARGE_INTEGER li;
	li.QuadPart = pos;
	if (stream->Seek (li, dwOrigin, 0) == S_OK)
		return tell ();
	return kStreamSeekError;
}

//-----------------------------------------------------------------------------
int64_t WinResourceInputStream::tell ()
{
	ULARGE_INTEGER pos;
	LARGE_INTEGER dummy = {};
	if (stream->Seek (dummy, STREAM_SEEK_CUR, &pos) == S_OK)
		return static_cast<int64_t> (pos.QuadPart);
	return kStreamSeekError;
}

//-----------------------------------------------------------------------------
static UTF8String gWinResourceBasePath;

//-----------------------------------------------------------------------------
Optional<UTF8String> WinResourceInputStream::getBasePath ()
{
	if (!gWinResourceBasePath.empty ())
		return {UTF8String (gWinResourceBasePath)};
	return {};
}

//-----------------------------------------------------------------------------
static std::function<IPlatformResourceInputStream::Ptr (const CResourceDescription& desc)>
    gCreateResourceInputStream =
        [] (const CResourceDescription& desc) { return WinResourceInputStream::create (desc); };

//-----------------------------------------------------------------------------
void IWin32PlatformFrame::setResourceBasePath (const UTF8String& _basePath)
{
	gWinResourceBasePath = _basePath;
	if (!UTF8StringView (gWinResourceBasePath).endsWith ("\\"))
		gWinResourceBasePath += "\\";
	gCreateResourceInputStream = [] (const CResourceDescription& desc) {
		IPlatformResourceInputStream::Ptr result = nullptr;
		if (desc.type == CResourceDescription::kStringType)
		{
			if (auto path = WinResourceInputStream::getBasePath ())
			{
				*path += desc.u.name;
				result = FileResourceInputStream::create (path->getString ());
			}
		}
		return result;
	};
}

//-----------------------------------------------------------------------------
auto IPlatformResourceInputStream::create (const CResourceDescription& desc) -> Ptr
{
	return gCreateResourceInputStream (desc);
}


/// @endcond ignore

} // namespace

#endif // WINDOWS
