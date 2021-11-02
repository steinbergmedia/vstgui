// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32directcomposition.h"
#include "win32support.h"
#include "win32dll.h"

#ifdef _MSC_VER
#pragma comment(lib, "d3d11.lib")
#endif

//-----------------------------------------------------------------------------
namespace VSTGUI {

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

//-----------------------------------------------------------------------------
std::unique_ptr<DirectCompositionSurface> DirectCompositionSurface::create (
	HWND window, IDCompositionDesktopDevice* compDevice, ID2D1Device* d2dDevice)
{
	auto surface = std::make_unique<DirectCompositionSurface> ();
	if (!surface)
		return {};
	surface->window = window;
	auto hr =
		compDevice->CreateTargetForHwnd (window, false, surface->compositionTarget.adoptPtr ());
	if (FAILED (hr))
		return {};
	hr = compDevice->CreateVisual (surface->compositionVisual.adoptPtr ());
	if (FAILED (hr))
		return {};
	hr = compDevice->CreateSurfaceFactory (d2dDevice,
										   surface->compositionSurfaceFactory.adoptPtr ());
	if (FAILED (hr))
		return {};
	surface->size = surface->getWindowSize ();
	hr = surface->compositionSurfaceFactory->CreateVirtualSurface (
		surface->size.x, surface->size.y, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ALPHA_MODE_PREMULTIPLIED,
		surface->compositionSurface.adoptPtr ());
	if (FAILED (hr))
		return {};
	hr = surface->compositionVisual->SetContent (surface->compositionSurface.get ());
	if (FAILED (hr))
		return {};
	hr = surface->compositionTarget->SetRoot (surface->compositionVisual.get ());
	if (FAILED (hr))
		return {};
	hr = compDevice->Commit ();
	if (FAILED (hr))
		return {};
	return surface;
}

//-----------------------------------------------------------------------------
POINT DirectCompositionSurface::getWindowSize () const
{
	RECT clientRect {};
	if (!GetClientRect (window, &clientRect))
		return {};
	auto width = clientRect.right - clientRect.left;
	auto height = clientRect.bottom - clientRect.top;
	return {width, height};
}

//-----------------------------------------------------------------------------
void DirectCompositionSurface::onResize ()
{
	needResize = true;
}

//-----------------------------------------------------------------------------
bool DirectCompositionSurface::update (CRect inUpdateRect, const DrawCallback& drawCallback)
{
	if (!drawCallback)
		return false;
	if (needResize)
	{
		auto windowSize = getWindowSize ();
		if (size.x != windowSize.x || size.y != windowSize.y)
		{
			size = windowSize;
			compositionSurface->Resize (size.x, size.y);
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
	auto hr = compositionSurface->BeginDraw (
		&updateRect, __uuidof(ID2D1DeviceContext),
		reinterpret_cast<void**> (d2dDeviceContext.adoptPtr ()), &offset);
	if (FAILED (hr))
		return false;

	inUpdateRect = rectFromRECT (updateRect);
	drawCallback (d2dDeviceContext.get (), inUpdateRect, offset);
	hr = compositionSurface->EndDraw ();
	if (FAILED (hr))
		return false;

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
DirectCompositionSupport::~DirectCompositionSupport () noexcept {}

//-----------------------------------------------------------------------------
std::unique_ptr<DirectCompositionSupport>
	DirectCompositionSupport::create (ID2D1Factory* d2dFactory)
{
	auto obj = std::unique_ptr<DirectCompositionSupport> (new DirectCompositionSupport);
	if (obj->init (d2dFactory))
		return std::move (obj);
	return {};
}

//-----------------------------------------------------------------------------
bool DirectCompositionSupport::init (ID2D1Factory* _d2dFactory)
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
bool DirectCompositionSupport::createD3D11Device ()
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
IDCompositionDesktopDevice* DirectCompositionSupport::getCompositionDesktopDevice () const
{
	return compositionDesktopDevice.get ();
}

//-----------------------------------------------------------------------------
ID2D1Device* DirectCompositionSupport::getD2D1Device () const
{
	return d2dDevice.get ();
}

//-----------------------------------------------------------------------------
} // VSTGUI
