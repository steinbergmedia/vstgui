// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32factory.h"
#include "win32directcomposition.h"
#include "../iplatformbitmap.h"
#include "../iplatformfont.h"
#include "../iplatformframe.h"
#include "../iplatformframecallback.h"
#include "../iplatformgraphicsdevice.h"
#include "../iplatformresourceinputstream.h"
#include "../iplatformstring.h"
#include "../iplatformtimer.h"
#include "../common/fileresourceinputstream.h"
#include "direct2d/d2dbitmap.h"
#include "direct2d/d2dbitmapcache.h"
#include "direct2d/d2dgraphicscontext.h"
#include "direct2d/d2dfont.h"
#include "direct2d/d2dgradient.h"
#include "win32frame.h"
#include "win32dragging.h"
#include "win32resourcestream.h"
#include "winfileselector.h"
#include "winstring.h"
#include "wintimer.h"
#include "comptr.h"
#include <cassert>
#include <list>
#include <memory>
#include <shlwapi.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <wincodec.h>

#ifdef _MSC_VER
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#endif

//-----------------------------------------------------------------------------
namespace VSTGUI {


//-----------------------------------------------------------------------------
struct Win32Factory::Impl
{
	HINSTANCE instance {nullptr};
	COM::Ptr<ID2D1Factory> d2dFactory;
	COM::Ptr<IDWriteFactory> directWriteFactory;
	COM::Ptr<IWICImagingFactory> wicImagingFactory;

	std::unique_ptr<DirectComposition::Factory> directCompositionFactory;
	D2DGraphicsDeviceFactory graphicsDeviceFactory;

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

	D2D1_FACTORY_OPTIONS* options = nullptr;
#if 0 // DEBUG
	D2D1_FACTORY_OPTIONS debugOptions;
	debugOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
	options = &debugOptions;
#endif
	D2D1CreateFactory (D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory), options,
					   (void**)impl->d2dFactory.adoptPtr ());
	DWriteCreateFactory (DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
						 (IUnknown**)impl->directWriteFactory.adoptPtr ());
#if _WIN32_WINNT > 0x601
// make sure when building with the Win 8.0 SDK we work on Win7
#define VSTGUI_WICImagingFactory CLSID_WICImagingFactory1
#else
#define VSTGUI_WICImagingFactory CLSID_WICImagingFactory
#endif
	CoCreateInstance (VSTGUI_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
					  IID_IWICImagingFactory, (void**)impl->wicImagingFactory.adoptPtr ());

	impl->directCompositionFactory = DirectComposition::Factory::create (impl->d2dFactory.get ());
	D2DBitmapCache::init ();
	if (impl->directCompositionFactory)
	{
		auto device = impl->directCompositionFactory->getDevice ();
		vstgui_assert (device, "if there's a direct composition factory it must have a device");
		auto d2dDevice = std::make_shared<D2DGraphicsDevice> (device);
		impl->graphicsDeviceFactory.addDevice (d2dDevice);
	}
}

//-----------------------------------------------------------------------------
Win32Factory::~Win32Factory () noexcept
{
	D2DBitmapCache::terminate ();
	D2DFont::terminate ();
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
ID2D1Factory* Win32Factory::getD2DFactory () const noexcept
{
	return impl->d2dFactory.get ();
}

//-----------------------------------------------------------------------------
IWICImagingFactory* Win32Factory::getWICImagingFactory () const noexcept
{
	return impl->wicImagingFactory.get ();
}

//-----------------------------------------------------------------------------
IDWriteFactory* Win32Factory::getDirectWriteFactory () const noexcept
{
	return impl->directWriteFactory.get ();
}

//-----------------------------------------------------------------------------
DirectComposition::Factory* Win32Factory::getDirectCompositionFactory () const noexcept
{
	return impl->directCompositionFactory.get ();
}

//-----------------------------------------------------------------------------
PlatformGraphicsDeviceContextPtr
	Win32Factory::createGraphicsDeviceContext (void* hwnd) const noexcept
{
	auto window = reinterpret_cast<HWND> (hwnd);
	auto renderTargetType = useD2DHardwareRenderer () ? D2D1_RENDER_TARGET_TYPE_HARDWARE
													  : D2D1_RENDER_TARGET_TYPE_SOFTWARE;
	RECT rc;
	GetClientRect (window, &rc);

	auto size = D2D1::SizeU (static_cast<UINT32> (rc.right - rc.left),
							 static_cast<UINT32> (rc.bottom - rc.top));
	COM::Ptr<ID2D1HwndRenderTarget> hwndRenderTarget;
	D2D1_PIXEL_FORMAT pixelFormat =
		D2D1::PixelFormat (DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED);
	HRESULT hr = getD2DFactory ()->CreateHwndRenderTarget (
		D2D1::RenderTargetProperties (renderTargetType, pixelFormat),
		D2D1::HwndRenderTargetProperties (window, size, D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS),
		hwndRenderTarget.adoptPtr ());
	if (FAILED (hr))
		return nullptr;
	hwndRenderTarget->SetDpi (96, 96);

	COM::Ptr<ID2D1DeviceContext> deviceContext;
	hr = hwndRenderTarget->QueryInterface (__uuidof (ID2D1DeviceContext),
										   reinterpret_cast<void**> (deviceContext.adoptPtr ()));
	if (FAILED (hr))
		return nullptr;

	ID2D1Device* d2ddevice {};
	deviceContext->GetDevice (&d2ddevice);
	auto device = impl->graphicsDeviceFactory.find (d2ddevice);
	if (!device)
	{
		impl->graphicsDeviceFactory.addDevice (std::make_shared<D2DGraphicsDevice> (d2ddevice));
		device = impl->graphicsDeviceFactory.find (d2ddevice);
		vstgui_assert (device);
	}

	return std::make_shared<D2DGraphicsDeviceContext> (
		*std::static_pointer_cast<D2DGraphicsDevice> (device).get (), deviceContext.get (),
		TransformMatrix {});
}

//-----------------------------------------------------------------------------
void Win32Factory::disableDirectComposition () const noexcept
{
	impl->directCompositionFactory.reset ();
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

//-----------------------------------------------------------------------------
PlatformGradientPtr Win32Factory::createGradient () const noexcept
{
	return std::make_unique<D2DGradient> ();
}

//-----------------------------------------------------------------------------
PlatformFileSelectorPtr Win32Factory::createFileSelector (PlatformFileSelectorStyle style,
														  IPlatformFrame* frame) const noexcept
{
	auto win32Frame = dynamic_cast<Win32Frame*> (frame);
	return createWinFileSelector (style, win32Frame ? win32Frame->getHWND () : nullptr);
}

//-----------------------------------------------------------------------------
const IPlatformGraphicsDeviceFactory& Win32Factory::getGraphicsDeviceFactory () const noexcept
{
	return impl->graphicsDeviceFactory;
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
