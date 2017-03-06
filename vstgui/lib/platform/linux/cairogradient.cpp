//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "cairogradient.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
CGradient* CGradient::create (const ColorStopMap& colorStopMap)
{
	return new Cairo::Gradient (colorStopMap);
}

//------------------------------------------------------------------------
namespace Cairo {

//------------------------------------------------------------------------
Gradient::Gradient (const ColorStopMap& colorStopMap) : CGradient (colorStopMap)
{
}

//------------------------------------------------------------------------
Gradient::~Gradient ()
{
	destroy ();
}

//------------------------------------------------------------------------
void Gradient::destroy () const
{
	linearGradient.reset ();
	radialGradient.reset ();
}

//------------------------------------------------------------------------
const PatternHandle& Gradient::getLinearGradient (CPoint start, CPoint end) const
{
	if (!linearGradient || start != linearGradientStart || end != linearGradientEnd)
	{
		destroy ();
		linearGradientStart = start;
		linearGradientEnd = end;
		linearGradient =
			PatternHandle (cairo_pattern_create_linear (start.x, start.y, end.x, end.y));
		for (auto& it : this->colorStops)
			cairo_pattern_add_color_stop_rgba (linearGradient, it.first, it.second.red / 255.,
											   it.second.green / 255., it.second.blue / 255.,
											   it.second.alpha);
	}
	return linearGradient;
}

//------------------------------------------------------------------------
const PatternHandle& Gradient::getRadialGradient () const
{
	if (!radialGradient)
	{
		radialGradient = PatternHandle (cairo_pattern_create_radial (0, 0, 1, 0, 0, 1));
		for (auto& it : this->colorStops)
			cairo_pattern_add_color_stop_rgba (radialGradient, it.first, it.second.red / 255.,
											   it.second.green / 255., it.second.blue / 255.,
											   it.second.alpha);
	}
	return radialGradient;
}

//------------------------------------------------------------------------
} // Cairo
} // VSTGUI
