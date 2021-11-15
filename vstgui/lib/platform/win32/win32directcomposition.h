// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "comptr.h"
#include "../../crect.h"
#include <functional>
#include <memory>
#include <windows.h>
#include <combaseapi.h>

interface ID2D1DeviceContext;
interface IDCompositionDesktopDevice;
interface ID2D1Device;
interface ID2D1Factory;

//-----------------------------------------------------------------------------
namespace VSTGUI {
namespace DirectComposition {

//-----------------------------------------------------------------------------
struct Surface
{
public:
	using DrawCallback = std::function<void (ID2D1DeviceContext* deviceContext, CRect updateRect,
											 int32_t offsetX, int32_t offsetY)>;

	void resize (uint32_t width, uint32_t height);
	bool update (CRect updateRect, const DrawCallback& drawCallback);
	bool commit ();

	~Surface () noexcept;

private:
	friend struct Support;

	using DestroyCallback = std::function<void (Surface*)>;

	static std::unique_ptr<Surface> create (HWND window, IDCompositionDesktopDevice* compDevice,
											ID2D1Device* d2dDevice, DestroyCallback&& destroy);
	Surface ();
	bool enableVisualizeRedrawAreas (bool state);

	struct Impl;
	std::unique_ptr<Impl> impl;
};

//-----------------------------------------------------------------------------
struct Support
{
	static std::unique_ptr<Support> create (ID2D1Factory* d2dFactory);

	IDCompositionDesktopDevice* getCompositionDesktopDevice () const;
	ID2D1Device* getD2D1Device () const;
	bool enableVisualizeRedrawAreas (bool state);
	bool isVisualRedrawAreasEnabled () const;

	std::unique_ptr<Surface> createSurface (HWND hwnd);

	~Support () noexcept;

private:
	Support ();

	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // DirectComposition
} // VSTGUI
