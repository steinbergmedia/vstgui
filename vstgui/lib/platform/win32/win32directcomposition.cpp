// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32directcomposition.h"
#include "win32support.h"
#include "win32dll.h"
#include "wintimer.h"
#include "win32factory.h"

#include <d3d11_4.h>
#include <dcomp.h>
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

//-----------------------------------------------------------------------------
struct RootVisual : IVisual
{
public:
	bool setPosition (uint32_t left, uint32_t top) override;
	bool resize (uint32_t width, uint32_t height) override;
	bool update (CRect updateRect, const DrawCallback& drawCallback) override;
	bool commit () override;

	~RootVisual () noexcept;

private:
	friend struct Factory;

	using DestroyCallback = std::function<void (RootVisual*)>;

	static std::unique_ptr<RootVisual> create (HWND window, IDCompositionDesktopDevice* compDevice,
											   ID2D1Device* d2dDevice, DestroyCallback&& destroy);
	RootVisual ();
	bool enableVisualizeRedrawAreas (bool state);

	struct Impl;
	std::unique_ptr<Impl> impl;
};

//-----------------------------------------------------------------------------
struct VisualSurfacePair
{
	COM::Ptr<IDCompositionVisual2> visual;
	COM::Ptr<IDCompositionVirtualSurface> surface;
	UINT width {};
	UINT height {};

	bool update (RECT updateRect, const IVisual::DrawCallback& callback);
	bool setSize (uint32_t width, uint32_t height);
};
using VisualSurfacePairPtr = std::shared_ptr<VisualSurfacePair>;

//-----------------------------------------------------------------------------
bool VisualSurfacePair::update (RECT updateRect, const IVisual::DrawCallback& callback)
{
	POINT offset {};
	COM::Ptr<ID2D1DeviceContext> d2dDeviceContext;
	auto hr = surface->BeginDraw (&updateRect, __uuidof(ID2D1DeviceContext),
								  reinterpret_cast<void**> (d2dDeviceContext.adoptPtr ()), &offset);
	if (FAILED (hr))
		return false;

	callback (d2dDeviceContext.get (), rectFromRECT (updateRect), offset.x, offset.y);
	hr = surface->EndDraw ();
	if (FAILED (hr))
		return false;
	return true;
}

//-----------------------------------------------------------------------------
bool VisualSurfacePair::setSize (uint32_t w, uint32_t h)
{
	if (w != width || h != height)
	{
		if (SUCCEEDED (surface->Resize (w, h)))
		{
			width = w;
			height = h;
		}
		else
		{
			return false;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
struct SurfaceRedrawArea : IPlatformTimerCallback
{
	using DoneCallback = std::function<void (SurfaceRedrawArea*)>;

	static constexpr uint32_t animationTime = 250;
	static constexpr float animationTimeSeconds = animationTime / 1000.f;
	static constexpr float startOpacity = 0.8f;

	SurfaceRedrawArea (IDCompositionDesktopDevice* compDevice, const VisualSurfacePairPtr& s,
					   CRect r, DoneCallback&& cb)
	: surface (s), callback (std::move (cb)), rect (r)
	{
		surface->visual->SetOffsetX (static_cast<float> (r.left));
		surface->visual->SetOffsetY (static_cast<float> (r.top));
		auto tm = D2D1::Matrix3x2F::Scale (static_cast<float> (r.getWidth ()),
										   static_cast<float> (r.getHeight ()));
		surface->visual->SetTransform (tm);
		r.setSize ({1., 1.});
		surface->surface->Resize (static_cast<UINT> (r.getWidth ()),
								  static_cast<UINT> (r.getHeight ()));
		r.originize ();
		s->update (RECTfromRect (r),
				   [] (auto deviceContext, auto r, int32_t offsetX, int32_t offsetY) {
					   COM::Ptr<ID2D1SolidColorBrush> brush;
					   D2D1_COLOR_F color = {1.f, 1.f, 0.f, 1.f};
					   deviceContext->CreateSolidColorBrush (&color, nullptr, brush.adoptPtr ());
					   D2D1_RECT_F rect;
					   rect.left = static_cast<FLOAT> (r.left) + offsetX;
					   rect.top = static_cast<FLOAT> (r.top) + offsetY;
					   rect.right = static_cast<FLOAT> (r.right) + offsetX;
					   rect.bottom = static_cast<FLOAT> (r.bottom) + offsetY;
					   deviceContext->FillRectangle (&rect, brush.get ());
				   });
		compDevice->CreateAnimation (animation.adoptPtr ());

		restart ();
	}

	~SurfaceRedrawArea () noexcept { fire (); }

	void restart ()
	{
		COM::Ptr<IDCompositionVisual3> vis3;
		auto hr = surface->visual->QueryInterface (__uuidof(IDCompositionVisual3),
												   reinterpret_cast<void**> (vis3.adoptPtr ()));
		if (SUCCEEDED (hr) && vis3 && animation)
		{
			animation->Reset ();
			hr = animation->AddCubic (0., startOpacity, -startOpacity / animationTimeSeconds, 0.f,
									  0.f);
			hr = animation->End (animationTimeSeconds, 0.0f);
			vis3->SetOpacity (animation.get ());
		}
		timer.stop ();
		timer.start (animationTime);
	}

	void fire () override
	{
		timer.stop ();
		if (callback)
		{
			auto cb = std::move (callback);
			callback = nullptr;
			cb (this);
		}
	}

	IDCompositionVisual2* getVisual () const { return surface->visual.get (); }
	const CRect& getRect () const { return rect; }

private:
	VisualSurfacePairPtr surface {};
	DoneCallback callback {};
	COM::Ptr<IDCompositionAnimation> animation {};
	WinTimer timer {this};
	CRect rect;
};
using SurfaceRedrawAreaPtr = std::shared_ptr<SurfaceRedrawArea>;

//-----------------------------------------------------------------------------
struct RootVisual::Impl
{
	using Children = std::vector<VisualSurfacePairPtr>;
	using RedrawAreaVector = std::vector<SurfaceRedrawAreaPtr>;

	DestroyCallback destroyCallback;
	HWND window {nullptr};
	IDCompositionDesktopDevice* compDevice {nullptr};
	COM::Ptr<IDCompositionTarget> compositionTarget;
	COM::Ptr<IDCompositionSurfaceFactory> compositionSurfaceFactory;
	VisualSurfacePair rootPlane;
	VisualSurfacePairPtr redrawAreaPlane;
	Children children;
	RedrawAreaVector redrawAreas;
	bool showUpdateRects {true};

	POINT getWindowSize () const;
	VisualSurfacePairPtr createVisualSurfacePair (uint32_t width, uint32_t height) const;
	void addChild (const VisualSurfacePairPtr& child);
	void removeChild (const VisualSurfacePairPtr& child);

	bool enableVisualizeRedrawAreas (bool state);
	void addRedrawArea (CRect r);
};

//-----------------------------------------------------------------------------
POINT RootVisual::Impl::getWindowSize () const
{
	RECT clientRect {};
	if (!GetClientRect (window, &clientRect))
		return {};
	auto width = clientRect.right - clientRect.left;
	auto height = clientRect.bottom - clientRect.top;
	return {width, height};
}

//-----------------------------------------------------------------------------
VisualSurfacePairPtr RootVisual::Impl::createVisualSurfacePair (uint32_t width,
																uint32_t height) const
{
	auto child = std::make_shared<VisualSurfacePair> ();
	auto hr = compDevice->CreateVisual (child->visual.adoptPtr ());
	if (FAILED (hr))
		return {};
	hr = compositionSurfaceFactory->CreateVirtualSurface (width, height, DXGI_FORMAT_B8G8R8A8_UNORM,
														  DXGI_ALPHA_MODE_PREMULTIPLIED,
														  child->surface.adoptPtr ());
	if (FAILED (hr))
		return {};
	child->visual->SetContent (child->surface.get ());
	child->width = width;
	child->height = height;
	return child;
}

//-----------------------------------------------------------------------------
void RootVisual::Impl::addChild (const VisualSurfacePairPtr& child)
{
	rootPlane.visual->AddVisual (child->visual.get (), TRUE, nullptr);
	children.push_back (child);
}

//-----------------------------------------------------------------------------
void RootVisual::Impl::removeChild (const VisualSurfacePairPtr& child)
{
	auto it = std::find_if (children.begin (), children.end (), [&] (const auto& el) {
		return el->visual.get () == child->visual.get ();
	});
	if (it == children.end ())
		return;
	rootPlane.visual->RemoveVisual (child->visual.get ());
	children.erase (it);
}

//-----------------------------------------------------------------------------
bool RootVisual::Impl::enableVisualizeRedrawAreas (bool state)
{
	if (state)
	{
		if (redrawAreaPlane)
			return true;

		redrawAreaPlane = createVisualSurfacePair (rootPlane.width, rootPlane.height);
		rootPlane.visual->AddVisual (redrawAreaPlane->visual.get (), TRUE, nullptr);
	}
	else
	{
		if (!redrawAreaPlane)
			return true;

		redrawAreas.clear ();
		rootPlane.visual->RemoveVisual (redrawAreaPlane->visual.get ());
		redrawAreaPlane = nullptr;
	}

	return false;
}

//-----------------------------------------------------------------------------
void RootVisual::Impl::addRedrawArea (CRect r)
{
	if (!redrawAreaPlane)
		return;

	for (auto& area : redrawAreas)
	{
		if (area->getRect () == r)
		{
			area->restart ();
			return;
		}
	}

	auto vis = createVisualSurfacePair (static_cast<uint32_t> (r.getWidth ()),
										static_cast<uint32_t> (r.getHeight ()));
	auto redrawArea = std::make_shared<SurfaceRedrawArea> (compDevice, vis, r, [this] (auto area) {
		auto it = std::find_if (redrawAreas.begin (), redrawAreas.end (),
								[&] (const auto& el) { return el.get () == area; });
		if (it != redrawAreas.end ())
		{
			redrawAreaPlane->visual->RemoveVisual (area->getVisual ());
			redrawAreas.erase (it);
			compDevice->Commit ();
		}
	});
	redrawAreaPlane->visual->AddVisual (vis->visual.get (), TRUE, nullptr);
	redrawAreas.emplace_back (std::move (redrawArea));
}

//-----------------------------------------------------------------------------
RootVisual::RootVisual ()
{
	impl = std::make_unique<Impl> ();
}

//-----------------------------------------------------------------------------
RootVisual::~RootVisual () noexcept
{
	impl->enableVisualizeRedrawAreas (false);
	vstgui_assert (impl->children.empty ());
	impl->destroyCallback (this);
}

//-----------------------------------------------------------------------------
std::unique_ptr<RootVisual> RootVisual::create (HWND window, IDCompositionDesktopDevice* compDevice,
												ID2D1Device* d2dDevice, DestroyCallback&& destroy)
{
	auto surface = std::unique_ptr<RootVisual> (new RootVisual);
	if (!surface)
		return {};
	surface->impl->compDevice = compDevice;
	surface->impl->window = window;
	auto hr = compDevice->CreateTargetForHwnd (window, false,
											   surface->impl->compositionTarget.adoptPtr ());
	if (FAILED (hr))
		return {};
	hr = compDevice->CreateVisual (surface->impl->rootPlane.visual.adoptPtr ());
	if (FAILED (hr))
		return {};
	hr = compDevice->CreateSurfaceFactory (d2dDevice,
										   surface->impl->compositionSurfaceFactory.adoptPtr ());
	if (FAILED (hr))
		return {};
	auto windowSize = surface->impl->getWindowSize ();
	surface->impl->rootPlane.width = windowSize.x;
	surface->impl->rootPlane.height = windowSize.y;

	hr = surface->impl->compositionSurfaceFactory->CreateVirtualSurface (
		surface->impl->rootPlane.width, surface->impl->rootPlane.height, DXGI_FORMAT_B8G8R8A8_UNORM,
		DXGI_ALPHA_MODE_PREMULTIPLIED, surface->impl->rootPlane.surface.adoptPtr ());
	if (FAILED (hr))
		return {};
	hr = surface->impl->rootPlane.visual->SetContent (surface->impl->rootPlane.surface.get ());
	if (FAILED (hr))
		return {};
	hr = surface->impl->compositionTarget->SetRoot (surface->impl->rootPlane.visual.get ());
	if (FAILED (hr))
		return {};
	hr = compDevice->Commit ();
	if (FAILED (hr))
		return {};
	surface->impl->destroyCallback = std::move (destroy);
	return surface;
}

//-----------------------------------------------------------------------------
bool RootVisual::setPosition (uint32_t left, uint32_t top)
{
	// the root visual is always at (0, 0)
	return false;
}

//-----------------------------------------------------------------------------
bool RootVisual::resize (uint32_t width, uint32_t height)
{
	return impl->rootPlane.setSize (width, height);
}

//-----------------------------------------------------------------------------
bool RootVisual::update (CRect inUpdateRect, const DrawCallback& drawCallback)
{
	if (!drawCallback)
		return false;

	RECT updateRect {};
	if (inUpdateRect.isEmpty ())
	{
		auto size = impl->getWindowSize ();
		updateRect.right = size.x;
		updateRect.bottom = size.y;
	}
	else
	{
		updateRect = RECTfromRect (inUpdateRect);
	}

	impl->rootPlane.update (updateRect, drawCallback);

	impl->addRedrawArea (rectFromRECT (updateRect));

	return true;
}

//-----------------------------------------------------------------------------
bool RootVisual::commit ()
{
	if (impl->compDevice)
		return SUCCEEDED (impl->compDevice->Commit ());
	return false;
}

//-----------------------------------------------------------------------------
bool RootVisual::enableVisualizeRedrawAreas (bool state)
{
	return impl->enableVisualizeRedrawAreas (state);
}

//------------------------------------------------------------------------
} // anonymous

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
struct Factory::Impl
{
	COM::Ptr<ID3D11Device1> d3dDevice;
	COM::Ptr<ID3D11DeviceContext1> d3dDeviceContext;
	COM::Ptr<IDXGIDevice> dxgiDevice;
	COM::Ptr<ID2D1Device> d2dDevice;
	COM::Ptr<IDCompositionDesktopDevice> compositionDesktopDevice;
	std::vector<RootVisual*> surfaces;
	bool visualizeRedrawAreas {false};

	bool init (IUnknown* _d2dFactory);
	bool createD3D11Device ();
};

//-----------------------------------------------------------------------------
Factory::Factory ()
{
	impl = std::make_unique<Impl> ();
}

//-----------------------------------------------------------------------------
Factory::~Factory () noexcept
{
	vstgui_assert (impl->surfaces.empty ());
}

//-----------------------------------------------------------------------------
std::unique_ptr<Factory> Factory::create (IUnknown* d2dFactory)
{
	auto obj = std::unique_ptr<Factory> (new Factory);
	if (obj->impl->init (d2dFactory))
		return std::move (obj);
	return {};
}

//-----------------------------------------------------------------------------
bool Factory::Impl::init (IUnknown* _d2dFactory)
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
bool Factory::Impl::createD3D11Device ()
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
bool Factory::enableVisualizeRedrawAreas (bool state)
{
	if (impl->visualizeRedrawAreas == state)
		return true;

	impl->visualizeRedrawAreas = state;
	for (auto& s : impl->surfaces)
	{
		s->enableVisualizeRedrawAreas (state);
	}
	return true;
}

//-----------------------------------------------------------------------------
bool Factory::isVisualRedrawAreasEnabled () const
{
	return impl->visualizeRedrawAreas;
}

//-----------------------------------------------------------------------------
VisualPtr Factory::createVisualForHWND (HWND hwnd)
{
	if (auto surface = RootVisual::create (hwnd, impl->compositionDesktopDevice.get (),
										   impl->d2dDevice.get (), [this] (auto surface) {
											   auto it = std::find (impl->surfaces.begin (),
																	impl->surfaces.end (), surface);
											   vstgui_assert (it != impl->surfaces.end ());
											   impl->surfaces.erase (it);
										   }))
	{
		surface->enableVisualizeRedrawAreas (impl->visualizeRedrawAreas);
		impl->surfaces.push_back (surface.get ());
		return surface;
	}
	return nullptr;
}

//------------------------------------------------------------------------
} // DirectComposition
} // VSTGUI
