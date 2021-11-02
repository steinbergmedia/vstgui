// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "comptr.h"
#include "../../crect.h"
#include <functional>
#include <memory>
#include <d3d11_4.h>
#include <dcomp.h>

//-----------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
struct DirectCompositionSurface
{
public:
	using DrawCallback =
		std::function<void (ID2D1DeviceContext* deviceContext, CRect updateRect, POINT offset)>;

	static std::unique_ptr<DirectCompositionSurface> create (HWND window,
															 IDCompositionDesktopDevice* compDevice,
															 ID2D1Device* d2dDevice);
	void onResize ();
	bool update (CRect updateRect, const DrawCallback& drawCallback);

private:
	POINT getWindowSize () const;

	HWND window {nullptr};
	COM::Ptr<IDCompositionTarget> compositionTarget;
	COM::Ptr<IDCompositionVisual2> compositionVisual;
	COM::Ptr<IDCompositionVirtualSurface> compositionSurface;
	COM::Ptr<IDCompositionSurfaceFactory> compositionSurfaceFactory;
	POINT size {};
	bool needResize {false};
};

//-----------------------------------------------------------------------------
struct DirectCompositionSupport
{
	static std::unique_ptr<DirectCompositionSupport> create (ID2D1Factory* d2dFactory);

	IDCompositionDesktopDevice* getCompositionDesktopDevice () const;
	ID2D1Device* getD2D1Device () const;

	~DirectCompositionSupport () noexcept;

private:
	DirectCompositionSupport () = default;

	bool init (ID2D1Factory* _d2dFactory);
	bool createD3D11Device ();

	COM::Ptr<ID3D11Device1> d3dDevice;
	COM::Ptr<ID3D11DeviceContext1> d3dDeviceContext;
	COM::Ptr<IDXGIDevice> dxgiDevice;
	COM::Ptr<ID2D1Device> d2dDevice;
	COM::Ptr<IDCompositionDesktopDevice> compositionDesktopDevice;
};

//-----------------------------------------------------------------------------
} // VSTGUI
