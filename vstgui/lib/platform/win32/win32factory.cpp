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
#include "direct2d/d2dbitmap.h"
#include "direct2d/d2dfont.h"
#include "win32frame.h"
#include <list>
#include <memory>
#include <shlwapi.h>

//-----------------------------------------------------------------------------
namespace VSTGUI {

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
static SharedPointer<IPlatformBitmap> createFromIStream (IStream* stream)
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
	return IPlatformResourceInputStream::create (desc);
}

//-----------------------------------------------------------------------------
PlatformStringPtr Win32Factory::createString (UTF8StringPtr utf8String) const noexcept
{
	return IPlatformString::createWithUTF8String (utf8String);
}

//-----------------------------------------------------------------------------
PlatformTimerPtr Win32Factory::createTimer (IPlatformTimerCallback* callback) const noexcept
{
	return IPlatformTimer::create (callback);
}

//-----------------------------------------------------------------------------
} // VSTGUI
