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
#include <array>

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
struct VisualSurfacePair
{
	COM::Ptr<IDCompositionVisual2> visual;
	COM::Ptr<IDCompositionVirtualSurface> surface;
	UINT left {};
	UINT top {};
	UINT width {};
	UINT height {};

	bool update (RECT updateRect, const IVisual::DrawCallback& callback);
	bool setOffset (uint32_t left, uint32_t top);
	bool setSize (uint32_t width, uint32_t height);
	bool setOpacity (float o);
};
using VisualSurfacePairPtr = std::shared_ptr<VisualSurfacePair>;

//-----------------------------------------------------------------------------
struct SurfaceRedrawArea : IPlatformTimerCallback
{
	using DoneCallback = std::function<void (SurfaceRedrawArea*)>;

	static constexpr uint32_t animationTime = 250;
	static constexpr float animationTimeSeconds = animationTime / 1000.f;
	static constexpr float startOpacity = 0.8f;

	static constexpr std::array<D2D1_COLOR_F, 5> colors = {{
		{1.f, 1.f, 0.f, 1.f},
		{1.f, 0.f, 0.f, 1.f},
		{1.f, 0.f, 1.f, 1.f},
		{0.f, 1.f, 1.f, 1.f},
		{0.f, 1.f, 0.f, 1.f},
	}};

	SurfaceRedrawArea (IDCompositionDesktopDevice* compDevice, const VisualSurfacePairPtr& s,
					   CRect r, uint32_t depth, DoneCallback&& cb)
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
		if (depth >= colors.size ())
			depth = static_cast<uint32_t> (colors.size () - 1u);

		s->update (RECTfromRect (r),
				   [depth] (auto deviceContext, auto r, int32_t offsetX, int32_t offsetY) {
					   COM::Ptr<ID2D1SolidColorBrush> brush;
					   D2D1_COLOR_F color = colors[depth]; //{1.f, 1.f, 0.f, 1.f};
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

struct RootVisual;
//-----------------------------------------------------------------------------
struct Visual : IVisual
{
	Visual () = default;
	Visual (VisualSurfacePairPtr vsPair, const std::shared_ptr<Visual>& parent);
	~Visual () noexcept;

	bool setPosition (uint32_t left, uint32_t top) override;
	bool resize (uint32_t width, uint32_t height) override;
	bool update (CRect updateRect, const DrawCallback& drawCallback) override;
	bool setOpacity (float opacity) override;
	bool commit () override;
	bool setZIndex (uint32_t index) override;

	void addChild (Visual* child);
	bool removeChild (Visual* child);
	void onZIndexChanged (Visual* child);

	uint32_t getZIndex () const { return zIndex; }
	Visual* getParent () const { return parent.get (); }
	virtual bool removeFromParent ();
	virtual RootVisual* getRootVisual ();
	virtual void addRedrawArea (CRect r, uint32_t depth = 0);
	virtual IDCompositionVisual* getTopVisual () const { return nullptr; }

protected:
	using Children = std::vector<Visual*>;

	std::shared_ptr<Visual> parent;
	VisualSurfacePairPtr root;
	Children children;
	uint32_t zIndex {0};
};

//-----------------------------------------------------------------------------
struct RootVisual : Visual
{
	bool setPosition (uint32_t left, uint32_t top) override;
	bool update (CRect updateRect, const DrawCallback& drawCallback) override;
	bool commit () override;
	bool setZIndex (uint32_t zIndex) override { return false; }

	~RootVisual () noexcept;

	bool removeFromParent () final { return true; }
	RootVisual* getRootVisual () final { return this; }
	VisualSurfacePairPtr createVisualSurfacePair (uint32_t width, uint32_t height) const;

private:
	friend struct Factory;

	using DestroyCallback = std::function<void (RootVisual*)>;
	using Children = std::vector<VisualSurfacePairPtr>;
	using RedrawAreaVector = std::vector<SurfaceRedrawAreaPtr>;

	static std::shared_ptr<RootVisual> create (HWND window, IDCompositionDesktopDevice* compDevice,
											   ID2D1Device* d2dDevice, DestroyCallback&& destroy);
	RootVisual ();
	bool enableVisualizeRedrawAreas (bool state);
	POINT getWindowSize () const;
	void addRedrawArea (CRect r, uint32_t depth = 0) final;
	IDCompositionVisual* getTopVisual () const final;

	DestroyCallback destroyCallback;
	HWND window {nullptr};
	IDCompositionDesktopDevice* compDevice {nullptr};
	COM::Ptr<IDCompositionTarget> compositionTarget;
	COM::Ptr<IDCompositionSurfaceFactory> compositionSurfaceFactory;
	VisualSurfacePairPtr redrawAreaPlane;
	RedrawAreaVector redrawAreas;
	bool showUpdateRects {true};
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Visual::Visual (VisualSurfacePairPtr vsPair, const std::shared_ptr<Visual>& inParent)
{
	root = vsPair;
	parent = inParent;
	parent->addChild (this);
}

//-----------------------------------------------------------------------------
Visual::~Visual () noexcept
{
	vstgui_assert (children.empty ());
	vstgui_assert (parent == nullptr);
}

//-----------------------------------------------------------------------------
void Visual::addChild (Visual* child)
{
	IDCompositionVisual* referenceVis = getTopVisual ();
	if (!children.empty ())
	{
		auto it = std::find_if (
			children.begin (), children.end (),
			[zIndex = child->getZIndex ()] (const auto& c) { return c->getZIndex () >= zIndex; });
		if (it != children.end ())
			referenceVis = (*it)->root->visual.get ();
	}
	root->visual->AddVisual (child->root->visual.get (), FALSE, referenceVis);
	children.push_back (child);
}

//-----------------------------------------------------------------------------
bool Visual::removeChild (Visual* child)
{
	auto it = std::find (children.begin (), children.end (), child);
	if (it == children.end ())
		return true;
	root->visual->RemoveVisual (child->root->visual.get ());
	children.erase (it);
	return true;
}

//-----------------------------------------------------------------------------
void Visual::onZIndexChanged (Visual* child)
{
	if (children.size () == 1)
		return;
	if (!removeChild (child))
		return;
	addChild (child);
	commit ();
}

//-----------------------------------------------------------------------------
bool Visual::resize (uint32_t width, uint32_t height)
{
	return root->setSize (width, height);
}

//-----------------------------------------------------------------------------
bool Visual::setPosition (uint32_t left, uint32_t top)
{
	return root->setOffset (left, top);
}

//-----------------------------------------------------------------------------
bool Visual::setOpacity (float opacity)
{
	return root->setOpacity (opacity);
}

//-----------------------------------------------------------------------------
bool Visual::update (CRect inUpdateRect, const DrawCallback& drawCallback)
{
	if (!drawCallback || inUpdateRect.isEmpty ())
		return false;

	RECT updateRect = RECTfromRect (inUpdateRect);
	root->update (updateRect, drawCallback);
	addRedrawArea (inUpdateRect);
	return true;
}

//-----------------------------------------------------------------------------
void Visual::addRedrawArea (CRect r, uint32_t depth)
{
	r.offset (root->left, root->top);
	getParent ()->addRedrawArea (r, ++depth);
}

//-----------------------------------------------------------------------------
bool Visual::commit ()
{
	if (parent)
		return parent->commit ();
	return false;
}

//-----------------------------------------------------------------------------
bool Visual::setZIndex (uint32_t index)
{
	if (zIndex == index)
		return true;
	zIndex = index;
	vstgui_assert (getParent ());
	getParent ()->onZIndexChanged (this);
	return true;
}

//-----------------------------------------------------------------------------
RootVisual* Visual::getRootVisual ()
{
	if (parent)
		return parent->getRootVisual ();
	return nullptr;
}

//-----------------------------------------------------------------------------
bool Visual::removeFromParent ()
{
	vstgui_assert (parent);
	if (parent)
	{
		parent->removeChild (this);
		parent = nullptr;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
RootVisual::RootVisual () {}

//-----------------------------------------------------------------------------
RootVisual::~RootVisual () noexcept
{
	enableVisualizeRedrawAreas (false);
	vstgui_assert (children.empty ());
	destroyCallback (this);
}

//-----------------------------------------------------------------------------
std::shared_ptr<RootVisual> RootVisual::create (HWND window, IDCompositionDesktopDevice* compDevice,
												ID2D1Device* d2dDevice, DestroyCallback&& destroy)
{
	auto surface = std::shared_ptr<RootVisual> (new RootVisual);
	if (!surface)
		return {};
	surface->compDevice = compDevice;
	surface->window = window;
	auto hr =
		compDevice->CreateTargetForHwnd (window, false, surface->compositionTarget.adoptPtr ());
	if (FAILED (hr))
		return {};
	surface->root = std::make_shared<VisualSurfacePair> ();
	hr = compDevice->CreateVisual (surface->root->visual.adoptPtr ());
	if (FAILED (hr))
		return {};
	hr = compDevice->CreateSurfaceFactory (d2dDevice,
										   surface->compositionSurfaceFactory.adoptPtr ());
	if (FAILED (hr))
		return {};
	auto windowSize = surface->getWindowSize ();
	surface->root->width = windowSize.x;
	surface->root->height = windowSize.y;

	hr = surface->compositionSurfaceFactory->CreateVirtualSurface (
		surface->root->width, surface->root->height, DXGI_FORMAT_B8G8R8A8_UNORM,
		DXGI_ALPHA_MODE_PREMULTIPLIED, surface->root->surface.adoptPtr ());
	if (FAILED (hr))
		return {};
	hr = surface->root->visual->SetContent (surface->root->surface.get ());
	if (FAILED (hr))
		return {};
	hr = surface->compositionTarget->SetRoot (surface->root->visual.get ());
	if (FAILED (hr))
		return {};
	hr = compDevice->Commit ();
	if (FAILED (hr))
		return {};
	surface->destroyCallback = std::move (destroy);
	return surface;
}

//-----------------------------------------------------------------------------
POINT RootVisual::getWindowSize () const
{
	RECT clientRect {};
	if (!GetClientRect (window, &clientRect))
		return {};
	auto width = clientRect.right - clientRect.left;
	auto height = clientRect.bottom - clientRect.top;
	return {width, height};
}

//-----------------------------------------------------------------------------
VisualSurfacePairPtr RootVisual::createVisualSurfacePair (uint32_t width, uint32_t height) const
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
bool RootVisual::setPosition (uint32_t left, uint32_t top)
{
	// the root visual is always at (0, 0)
	return false;
}

//-----------------------------------------------------------------------------
bool RootVisual::update (CRect inUpdateRect, const DrawCallback& drawCallback)
{
	if (!drawCallback)
		return false;

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

	root->update (updateRect, drawCallback);

	addRedrawArea (rectFromRECT (updateRect));

	return true;
}

//-----------------------------------------------------------------------------
bool RootVisual::commit ()
{
	if (compDevice)
	{
		auto hr = compDevice->Commit ();
		return SUCCEEDED (hr);
	}
	return false;
}

//-----------------------------------------------------------------------------
bool RootVisual::enableVisualizeRedrawAreas (bool state)
{
	if (state)
	{
		if (redrawAreaPlane)
			return true;

		redrawAreaPlane = createVisualSurfacePair (root->width, root->height);
		root->visual->AddVisual (redrawAreaPlane->visual.get (), FALSE, nullptr);
	}
	else
	{
		if (!redrawAreaPlane)
			return true;

		redrawAreas.clear ();
		root->visual->RemoveVisual (redrawAreaPlane->visual.get ());
		redrawAreaPlane = nullptr;
	}

	return false;
}

//-----------------------------------------------------------------------------
void RootVisual::addRedrawArea (CRect r, uint32_t depth)
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
	auto redrawArea =
		std::make_shared<SurfaceRedrawArea> (compDevice, vis, r, depth, [this] (auto area) {
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

//------------------------------------------------------------------------
IDCompositionVisual* RootVisual::getTopVisual () const
{
	if (redrawAreaPlane)
		return redrawAreaPlane->visual.get ();
	return nullptr;
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
VisualPtr Factory::createChildVisual (const VisualPtr& parent, uint32_t width, uint32_t height)
{
	if (auto visual = std::dynamic_pointer_cast<Visual> (parent))
	{
		if (auto root = visual->getRootVisual ())
		{
			auto vsPair = root->createVisualSurfacePair (width, height);
			auto childVisual = std::make_shared<Visual> (vsPair, visual);
			return childVisual;
		}
	}

	return nullptr;
}

//------------------------------------------------------------------------
bool Factory::removeVisual (const VisualPtr& visualPtr)
{
	if (auto visual = dynamic_cast<Visual*> (visualPtr.get ()))
	{
		return visual->removeFromParent ();
	}
	return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
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
bool VisualSurfacePair::setOffset (uint32_t inLeft, uint32_t inTop)
{
	if (left == inLeft && top == inTop)
		return true;
	left = inLeft;
	top = inTop;
	auto hr = visual->SetOffsetX (static_cast<float> (left));
	if (FAILED (hr))
		return false;
	hr = visual->SetOffsetY (static_cast<float> (top));
	return SUCCEEDED (hr);
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
bool VisualSurfacePair::setOpacity (float o)
{
	COM::Ptr<IDCompositionVisual3> vis3;
	auto hr = visual->QueryInterface (__uuidof(IDCompositionVisual3),
									  reinterpret_cast<void**> (vis3.adoptPtr ()));
	if (SUCCEEDED (hr) && vis3)
	{
		hr = vis3->SetOpacity (o);
	}
	return SUCCEEDED (hr);
}

//------------------------------------------------------------------------
} // DirectComposition
} // VSTGUI
