// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cairogradient.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Cairo {

//------------------------------------------------------------------------
Gradient::~Gradient () noexcept
{
	changed ();
}

//------------------------------------------------------------------------
void Gradient::changed ()
{
	linearGradient.reset ();
	radialGradient.reset ();
}

//------------------------------------------------------------------------
const PatternHandle& Gradient::getLinearGradient (CPoint start, CPoint end)
{
	if (!linearGradient || start != linearGradientStart || end != linearGradientEnd)
	{
		changed ();
		linearGradientStart = start;
		linearGradientEnd = end;
		linearGradient =
			PatternHandle (cairo_pattern_create_linear (start.x, start.y, end.x, end.y));
		for (auto& it : getColorStops ())
		{
			cairo_pattern_add_color_stop_rgba (
			    linearGradient, it.first, it.second.normRed<double> (),
			    it.second.normGreen<double> (), it.second.normBlue<double> (),
			    it.second.normAlpha<double> ());
		}
	}
	return linearGradient;
}

//------------------------------------------------------------------------
const PatternHandle& Gradient::getRadialGradient ()
{
	if (!radialGradient)
	{
		radialGradient = PatternHandle (cairo_pattern_create_radial (0, 0, 1, 0, 0, 1));
		for (auto& it : getColorStops ())
		{
			cairo_pattern_add_color_stop_rgba (
			    radialGradient, it.first, it.second.normRed<double> (),
			    it.second.normGreen<double> (), it.second.normBlue<double> (),
			    it.second.normAlpha<double> ());
		}
	}
	return radialGradient;
}

//------------------------------------------------------------------------
} // Cairo
} // VSTGUI
