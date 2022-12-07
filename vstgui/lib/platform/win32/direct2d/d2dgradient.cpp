// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "d2dgradient.h"

#if WINDOWS

#include "d2d.h"
#include <vector>

namespace VSTGUI {

//-----------------------------------------------------------------------------
ID2D1GradientStopCollection* D2DGradient::create (ID2D1RenderTarget* renderTarget,
												  float globalAlpha) const
{
	std::vector<D2D1_GRADIENT_STOP> gradientStops;
	gradientStops.resize (getColorStops ().size ());
	uint32_t index = 0;
	for (auto it = getColorStops ().begin (); it != getColorStops ().end (); ++it, ++index)
	{
		gradientStops[index].position = static_cast<FLOAT> (it->first);
		gradientStops[index].color = convert (it->second, globalAlpha);
	}
	ID2D1GradientStopCollection* collection = nullptr;
	auto hr = renderTarget->CreateGradientStopCollection (
		gradientStops.data (), static_cast<UINT32> (getColorStops ().size ()), &collection);
	if (SUCCEEDED (hr))
		return collection;
	return nullptr;
}

} // VSTGUI

#endif // WINDOWS
