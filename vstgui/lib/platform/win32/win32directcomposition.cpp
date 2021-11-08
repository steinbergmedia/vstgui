// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32directcomposition.h"
#include "win32support.h"
#include "win32dll.h"

#include <vector>

#ifdef _MSC_VER
#pragma comment(lib, "d3d11.lib")
#endif

//-----------------------------------------------------------------------------
namespace VSTGUI {
namespace DirectComposition {
namespace {

//-----------------------------------------------------------------------------
struct DirectCompositionSupportDll : DllBase
{
	using DCompositionCreateDevice2Func = HRESULT (*) (IUnknown*, REFIID, void**);

	static DirectCompositionSupportDll& instance ()
	{
		static DirectCompositionSupportDll dll;
		return dll;
	}

	HRESULT createDevice2 (IUnknown* renderingDevice, REFIID iid, void** dcompositionDevice) const
	{
		if (!createDevice2Func)
			return S_FALSE;
		return createDevice2Func (renderingDevice, iid, dcompositionDevice);
	}

private:
	DirectCompositionSupportDll () : DllBase ("dcomp.dll")
	{
		createDevice2Func =
			getProcAddress<DCompositionCreateDevice2Func> ("DCompositionCreateDevice2");
	}

	DCompositionCreateDevice2Func createDevice2Func {nullptr};
};
//------------------------------------------------------------------------
} // anonymous

//-----------------------------------------------------------------------------
struct Surface::Impl
{
	HWND window {nullptr};
	IDCompositionDesktopDevice* compDevice {nullptr};
	COM::Ptr<IDCompositionTarget> compositionTarget;
	COM::Ptr<IDCompositionVisual2> compositionVisual;
	COM::Ptr<IDCompositionVirtualSurface> compositionSurface;
	COM::Ptr<IDCompositionSurfaceFactory> compositionSurfaceFactory;
	POINT size {};
	bool needResize {false};
};

//-----------------------------------------------------------------------------
Surface::Surface ()
{
	impl = std::make_unique<Impl> ();
}

//-----------------------------------------------------------------------------
Surface::~Surface () noexcept = default;

//-----------------------------------------------------------------------------
std::unique_ptr<Surface> Surface::create (HWND window, IDCompositionDesktopDevice* compDevice,
										  ID2D1Device* d2dDevice)
{
	auto surface = std::unique_ptr<Surface> (new Surface);
	if (!surface)
		return {};
	surface->impl->compDevice = compDevice;
	surface->impl->window = window;
	auto hr = compDevice->CreateTargetForHwnd (window, false,
											   surface->impl->compositionTarget.adoptPtr ());
	if (FAILED (hr))
		return {};
	hr = compDevice->CreateVisual (surface->impl->compositionVisual.adoptPtr ());
	if (FAILED (hr))
		return {};
	hr = compDevice->CreateSurfaceFactory (d2dDevice,
										   surface->impl->compositionSurfaceFactory.adoptPtr ());
	if (FAILED (hr))
		return {};
	surface->impl->size = surface->getWindowSize ();
	hr = surface->impl->compositionSurfaceFactory->CreateVirtualSurface (
		surface->impl->size.x, surface->impl->size.y, DXGI_FORMAT_B8G8R8A8_UNORM,
		DXGI_ALPHA_MODE_PREMULTIPLIED, surface->impl->compositionSurface.adoptPtr ());
	if (FAILED (hr))
		return {};
	hr = surface->impl->compositionVisual->SetContent (surface->impl->compositionSurface.get ());
	if (FAILED (hr))
		return {};
	hr = surface->impl->compositionTarget->SetRoot (surface->impl->compositionVisual.get ());
	if (FAILED (hr))
		return {};
	hr = compDevice->Commit ();
	if (FAILED (hr))
		return {};
	return surface;
}

//-----------------------------------------------------------------------------
POINT Surface::getWindowSize () const
{
	RECT clientRect {};
	if (!GetClientRect (impl->window, &clientRect))
		return {};
	auto width = clientRect.right - clientRect.left;
	auto height = clientRect.bottom - clientRect.top;
	return {width, height};
}

//-----------------------------------------------------------------------------
void Surface::onResize ()
{
	impl->needResize = true;
}

//-----------------------------------------------------------------------------
bool Surface::update (CRect inUpdateRect, const DrawCallback& drawCallback)
{
	if (!drawCallback)
		return false;
	if (impl->needResize)
	{
		auto windowSize = getWindowSize ();
		if (impl->size.x != windowSize.x || impl->size.y != windowSize.y)
		{
			impl->size = windowSize;
			impl->compositionSurface->Resize (impl->size.x, impl->size.y);
		}
	}

	RECT updateRect {};
	if (inUpdateRect.isEmpty ())
	{
		auto size = getWindowSize ();
		updateRect.right = size.x;
		updateRect.bottom = size.y;
	}
	else
	{
		updateRect = RECTfromRect (inUpdateRect);
	}
	POINT offset {};
	COM::Ptr<ID2D1DeviceContext> d2dDeviceContext;
	auto hr = impl->compositionSurface->BeginDraw (
		&updateRect, __uuidof(ID2D1DeviceContext),
		reinterpret_cast<void**> (d2dDeviceContext.adoptPtr ()), &offset);
	if (FAILED (hr))
		return false;

	inUpdateRect = rectFromRECT (updateRect);
	drawCallback (d2dDeviceContext.get (), inUpdateRect, offset);
	hr = impl->compositionSurface->EndDraw ();
	if (FAILED (hr))
		return false;

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Support::~Support () noexcept {}

//-----------------------------------------------------------------------------
std::unique_ptr<Support> Support::create (ID2D1Factory* d2dFactory)
{
	auto obj = std::unique_ptr<Support> (new Support);
	if (obj->init (d2dFactory))
		return std::move (obj);
	return {};
}

//-----------------------------------------------------------------------------
bool Support::init (ID2D1Factory* _d2dFactory)
{
	if (DirectCompositionSupportDll::instance ().loaded () == false)
		return false;

	COM::Ptr<ID2D1Factory1> d2dFactory;
	auto hr = _d2dFactory->QueryInterface (__uuidof(ID2D1Factory1),
										   reinterpret_cast<void**> (d2dFactory.adoptPtr ()));
	if (FAILED (hr))
		return false;

	if (!createD3D11Device ())
		return false;

	hr = d3dDevice->QueryInterface (__uuidof(IDXGIDevice),
									reinterpret_cast<void**> (dxgiDevice.adoptPtr ()));
	if (FAILED (hr))
		return false;

	hr = d2dFactory->CreateDevice (dxgiDevice.get (), d2dDevice.adoptPtr ());

	if (FAILED (hr))
		return false;

	hr = DirectCompositionSupportDll::instance ().createDevice2 (
		d2dDevice.get (), IID_PPV_ARGS (compositionDesktopDevice.adoptPtr ()));
	if (FAILED (hr))
		return false;

	return true;
}

//-----------------------------------------------------------------------------
bool Support::createD3D11Device ()
{
	D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
										 D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
										 D3D_FEATURE_LEVEL_9_3,	 D3D_FEATURE_LEVEL_9_2,
										 D3D_FEATURE_LEVEL_9_1};
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL featureLevel {};

	COM::Ptr<ID3D11Device> device;
	COM::Ptr<ID3D11DeviceContext> deviceContext;

	auto hr = D3D11CreateDevice (nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, creationFlags, featureLevels,
								 ARRAYSIZE (featureLevels), D3D11_SDK_VERSION, device.adoptPtr (),
								 &featureLevel, deviceContext.adoptPtr ());
	if (FAILED (hr))
		return false;
	hr = device->QueryInterface (__uuidof(ID3D11Device1),
								 reinterpret_cast<void**> (d3dDevice.adoptPtr ()));
	if (FAILED (hr))
		return false;
	hr = deviceContext->QueryInterface (__uuidof(ID3D11DeviceContext1),
										reinterpret_cast<void**> (d3dDeviceContext.adoptPtr ()));
	if (FAILED (hr))
		return false;

	return true;
}

//-----------------------------------------------------------------------------
IDCompositionDesktopDevice* Support::getCompositionDesktopDevice () const
{
	return compositionDesktopDevice.get ();
}

//-----------------------------------------------------------------------------
ID2D1Device* Support::getD2D1Device () const
{
	return d2dDevice.get ();
}

//------------------------------------------------------------------------
} // DirectComposition
} // VSTGUI
