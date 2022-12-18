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
interface ID2D1Device;

//-----------------------------------------------------------------------------
namespace VSTGUI {
namespace DirectComposition {

//-----------------------------------------------------------------------------
struct IVisual
{
	using DrawCallback = std::function<void (ID2D1DeviceContext* deviceContext, CRect updateRect,
											 int32_t offsetX, int32_t offsetY)>;

	virtual bool setPosition (uint32_t left, uint32_t top) = 0;
	virtual bool resize (uint32_t width, uint32_t height) = 0;
	virtual bool update (CRect updateRect, const DrawCallback& drawCallback) = 0;
	virtual bool setOpacity (float opacity) = 0;
	virtual bool commit () = 0;
	virtual bool setZIndex (uint32_t zIndex) = 0;

	virtual ~IVisual () noexcept = default;
};

using VisualPtr = std::shared_ptr<IVisual>;

//-----------------------------------------------------------------------------
struct Factory
{
	static std::unique_ptr<Factory> create (IUnknown* d2dFactory);

	bool enableVisualizeRedrawAreas (bool state);
	bool isVisualRedrawAreasEnabled () const;

	VisualPtr createVisualForHWND (HWND hwnd);
	VisualPtr createChildVisual (const VisualPtr& parent, uint32_t width, uint32_t height);
	bool removeVisual (const VisualPtr& visual);

	ID2D1Device* getDevice () const;

	~Factory () noexcept;

	struct Impl;

private:
	Factory ();

	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // DirectComposition
} // VSTGUI
