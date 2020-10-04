// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32factory.h"
#include "../iplatformbitmap.h"
#include "../iplatformfont.h"
#include "../iplatformframe.h"
#include "../iplatformframecallback.h"
#include "../iplatformresourceinputstream.h"
#include "../iplatformstring.h"
#include "../iplatformtimer.h"
#include "../common/fileresourceinputstream.h"
#include "direct2d/d2dbitmap.h"
#include "direct2d/d2dfont.h"
#include "win32frame.h"
#include "win32resourcestream.h"
#include "winstring.h"
#include "wintimer.h"
#include <list>
#include <memory>
#include <shlwapi.h>

//-----------------------------------------------------------------------------
namespace VSTGUI {
namespace Win32FactoryPrivate {

//------------------------------------------------------------------------
static UTF8String gWinResourceBasePath;

//------------------------------------------------------------------------
static std::function<PlatformResourceInputStreamPtr (const CResourceDescription& desc)>
	gCreateResourceInputStream =
	    [] (const CResourceDescription& desc) { return WinResourceInputStream::create (desc); };

//------------------------------------------------------------------------
void setBasePath (const UTF8String& path)
{
	gWinResourceBasePath = path;
	if (!UTF8StringView (gWinResourceBasePath).endsWith ("\\"))
		gWinResourceBasePath += "\\";
	gCreateResourceInputStream = [] (const CResourceDescription& desc) {
		PlatformResourceInputStreamPtr result = nullptr;
		if (desc.type == CResourceDescription::kStringType && !gWinResourceBasePath.empty ())
		{
			auto path = gWinResourceBasePath;
			path += desc.u.name;
			result = FileResourceInputStream::create (path.getString ());
		}
		return result;
	};
}

//------------------------------------------------------------------------
Optional<UTF8String> getBasePath ()
{
	if (gWinResourceBasePath.empty ())
		return {};
	return Optional<UTF8String>{gWinResourceBasePath};
}

//------------------------------------------------------------------------
} // Win32FactoryPrivate

//-----------------------------------------------------------------------------
uint64_t Win32Factory::getTicks () const noexcept
{
	return static_cast<uint64_t> (GetTickCount64 ());
}

//-----------------------------------------------------------------------------
PlatformFramePtr Win32Factory::createFrame (IPlatformFrameCallback* frame, const CRect& size,
                                            void* parent, PlatformType parentType,
                                            IPlatformFrameConfig* config) const noexcept
{
	return makeOwned<Win32Frame> (frame, size, static_cast<HWND> (parent), parentType);
}

//-----------------------------------------------------------------------------
PlatformFontPtr Win32Factory::createFont (const UTF8String& name, const CCoord& size,
                                          const int32_t& style) const noexcept
{
	return makeOwned<D2DFont> (name, size, style);
}

//-----------------------------------------------------------------------------
bool Win32Factory::getAllFontFamilies (const FontFamilyCallback& callback) const noexcept
{
	return D2DFont::getAllFontFamilies (callback);
}

//-----------------------------------------------------------------------------
PlatformBitmapPtr Win32Factory::createBitmap (const CPoint& size) const noexcept
{
	return makeOwned<D2DBitmap> (size);
}

//------------------------------------------------------------------------
static PlatformBitmapPtr createFromIStream (IStream* stream)
{
	auto bitmap = makeOwned<D2DBitmap> ();
	if (bitmap->loadFromStream (stream))
		return bitmap;
	return nullptr;
}

//-----------------------------------------------------------------------------
PlatformBitmapPtr Win32Factory::createBitmap (const CResourceDescription& desc) const noexcept
{
	auto bitmap = makeOwned<D2DBitmap> ();
	if (bitmap->load (desc))
		return bitmap;
	return nullptr;
}

//-----------------------------------------------------------------------------
PlatformBitmapPtr Win32Factory::createBitmapFromPath (UTF8StringPtr absolutePath) const noexcept
{
	UTF8StringHelper path (absolutePath);
	IStream* stream = nullptr;
	if (SUCCEEDED (SHCreateStreamOnFileEx (path, STGM_READ|STGM_SHARE_DENY_WRITE, 0, false, nullptr, &stream)))
	{
		auto result = createFromIStream (stream);
		stream->Release ();
		return result;
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
PlatformBitmapPtr Win32Factory::createBitmapFromMemory (const void* ptr, uint32_t memSize) const
    noexcept
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
	return nullptr;
}

//-----------------------------------------------------------------------------
PNGBitmapBuffer Win32Factory::createBitmapMemoryPNGRepresentation (
    const PlatformBitmapPtr& bitmap) const noexcept
{
	if (auto bitmapBase = bitmap.cast<Win32BitmapBase> ())
		return bitmapBase->createMemoryPNGRepresentation ();
	return {};
}

//-----------------------------------------------------------------------------
PlatformResourceInputStreamPtr Win32Factory::createResourceInputStream (
    const CResourceDescription& desc) const noexcept
{
	return Win32FactoryPrivate::gCreateResourceInputStream (desc);
}

//-----------------------------------------------------------------------------
PlatformStringPtr Win32Factory::createString (UTF8StringPtr utf8String) const noexcept
{
	return makeOwned<WinString> (utf8String);
}

//-----------------------------------------------------------------------------
PlatformTimerPtr Win32Factory::createTimer (IPlatformTimerCallback* callback) const noexcept
{
	return makeOwned<WinTimer> (callback);
}

//-----------------------------------------------------------------------------
void Win32Factory::setResourceBasePath (const UTF8String& path) const
{
	Win32FactoryPrivate::setBasePath (path);
}

//-----------------------------------------------------------------------------
Optional<UTF8String> Win32Factory::getResourceBasePath () const
{
	return Win32FactoryPrivate::getBasePath ();
}

//------------------------------------------------------------------------
void IWin32PlatformFrame::setResourceBasePath (const UTF8String& path)
{
	Win32FactoryPrivate::setBasePath (path);
}

//-----------------------------------------------------------------------------
} // VSTGUI
