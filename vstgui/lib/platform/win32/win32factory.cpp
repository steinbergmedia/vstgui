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
#include "direct2d/d2ddrawcontext.h"
#include "direct2d/d2dfont.h"
#include "win32frame.h"
#include "win32dragging.h"
#include "win32resourcestream.h"
#include "winstring.h"
#include "wintimer.h"
#include <cassert>
#include <list>
#include <memory>
#include <shlwapi.h>

//-----------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
struct Win32Factory::Impl
{
	HINSTANCE instance {nullptr};
	UTF8String resourceBasePath;
	bool useD2DHardwareRenderer {false};
	bool useGenericTextEdit {false};

	PlatformResourceInputStreamPtr createResourceInputStream (const CResourceDescription& desc)
	{
		if (resourceBasePath.empty ())
			return WinResourceInputStream::create (desc);
		if (desc.type == CResourceDescription::kStringType)
		{
			auto path = resourceBasePath;
			path += desc.u.name;
			return FileResourceInputStream::create (path.getString ());
		}
		return {};
	}

	void setBasePath (const UTF8String& path)
	{
		resourceBasePath = path;
		if (!path.empty () && !UTF8StringView (resourceBasePath).endsWith ("\\"))
			resourceBasePath += "\\";
	}

	Optional<UTF8String> getBasePath () const
	{
		if (resourceBasePath.empty ())
			return {};
		return Optional<UTF8String> {resourceBasePath};
	}
};

//-----------------------------------------------------------------------------
Win32Factory::Win32Factory (HINSTANCE instance)
{
	impl = std::unique_ptr<Impl> (new Impl);
	impl->instance = instance;
}

//-----------------------------------------------------------------------------
HINSTANCE Win32Factory::getInstance () const noexcept
{
	assert (impl->instance); // missing instance handle means VSTGUI was not initialized correctly
	return impl->instance;
}

//-----------------------------------------------------------------------------
void Win32Factory::setResourceBasePath (const UTF8String& path) const noexcept
{
	impl->setBasePath (path);
}

//-----------------------------------------------------------------------------
Optional<UTF8String> Win32Factory::getResourceBasePath () const noexcept
{
	return impl->getBasePath ();
}

//-----------------------------------------------------------------------------
void Win32Factory::useD2DHardwareRenderer (bool state) const noexcept
{
	impl->useD2DHardwareRenderer = state;
}

//-----------------------------------------------------------------------------
bool Win32Factory::useD2DHardwareRenderer () const noexcept
{
	return impl->useD2DHardwareRenderer;
}

//-----------------------------------------------------------------------------
void Win32Factory::useGenericTextEdit (bool state) const noexcept
{
	impl->useGenericTextEdit = state;
}

//-----------------------------------------------------------------------------
bool Win32Factory::useGenericTextEdit () const noexcept
{
	return impl->useGenericTextEdit;
}

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
	if (SUCCEEDED (SHCreateStreamOnFileEx (path, STGM_READ | STGM_SHARE_DENY_WRITE, 0, false,
										   nullptr, &stream)))
	{
		auto result = createFromIStream (stream);
		stream->Release ();
		return result;
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
PlatformBitmapPtr Win32Factory::createBitmapFromMemory (const void* ptr,
														uint32_t memSize) const noexcept
{
#ifdef __GNUC__
	using SHCreateMemStreamProc = IStream* (*)(const BYTE* pInit, UINT cbInit);
	HMODULE shlwDll = LoadLibraryA ("shlwapi.dll");
	SHCreateMemStreamProc proc =
		reinterpret_cast<SHCreateMemStreamProc> (GetProcAddress (shlwDll, MAKEINTRESOURCEA (12)));
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
PlatformResourceInputStreamPtr
	Win32Factory::createResourceInputStream (const CResourceDescription& desc) const noexcept
{
	return impl->createResourceInputStream (desc);
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

//------------------------------------------------------------------------
bool Win32Factory::setClipboard (const DataPackagePtr& data) const noexcept
{
	auto dataObject = makeOwned<Win32DataObject> (data);
	auto hr = OleSetClipboard (dataObject);
	return hr == S_OK;
}

//------------------------------------------------------------------------
auto Win32Factory::getClipboard () const noexcept -> DataPackagePtr
{
	IDataObject* dataObject = nullptr;
	if (OleGetClipboard (&dataObject) != S_OK)
		return nullptr;
	return makeOwned<Win32DataPackage> (dataObject);
}

//------------------------------------------------------------------------
auto Win32Factory::createOffscreenContext (const CPoint& size, double scaleFactor) const noexcept
	-> COffscreenContextPtr
{
	if (auto bitmap = makeOwned<D2DBitmap> (size * scaleFactor))
	{
		bitmap->setScaleFactor (scaleFactor);
		return owned<COffscreenContext> (new D2DDrawContext (bitmap));
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
const LinuxFactory* Win32Factory::asLinuxFactory () const noexcept
{
	return nullptr;
}

//-----------------------------------------------------------------------------
const MacFactory* Win32Factory::asMacFactory () const noexcept
{
	return nullptr;
}

//-----------------------------------------------------------------------------
const Win32Factory* Win32Factory::asWin32Factory () const noexcept
{
	return this;
}

//-----------------------------------------------------------------------------
} // VSTGUI
