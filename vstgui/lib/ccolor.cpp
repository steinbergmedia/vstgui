//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "ccolor.h"
#include <cmath>

namespace VSTGUI {

const CColor kTransparentCColor	= CColor (255, 255, 255,   0);
const CColor kBlackCColor		= CColor (  0,   0,   0, 255);
const CColor kWhiteCColor		= CColor (255, 255, 255, 255);
const CColor kGreyCColor		= CColor (127, 127, 127, 255);
const CColor kRedCColor			= CColor (255,   0,   0, 255);
const CColor kGreenCColor		= CColor (  0, 255,   0, 255);
const CColor kBlueCColor		= CColor (  0,   0, 255, 255);
const CColor kYellowCColor		= CColor (255, 255,   0, 255);
const CColor kMagentaCColor		= CColor (255,   0, 255, 255);
const CColor kCyanCColor		= CColor (  0, 255, 255, 255);

/// @cond ignore
//-----------------------------------------------------------------------------
template<typename _Tp>
inline const _Tp& min3 (const _Tp& v1, const _Tp& v2, const _Tp& v3)
{
	if (v1 <= v2)
		return (v1 <= v3) ? v1 : v3;
	return (v2 <= v3) ? v2 : v3;
}

//-----------------------------------------------------------------------------
template<typename _Tp>
inline const _Tp& max3 (const _Tp& v1, const _Tp& v2, const _Tp& v3)
{
	if (v1 >= v2)
		return (v1 >= v3) ? v1 : v3;
	return (v2 >= v3) ? v2 : v3;
}
/// @endcond

//-----------------------------------------------------------------------------
void CColor::toHSV (double& hue, double& saturation, double& value)
{
	double rgbMin = (min3<unsigned char> (red, green, blue)) / 255.;
	double rgbMax = (max3<unsigned char> (red, green, blue)) / 255.;
	value = rgbMax;
	if (value == 0)
	{
		hue = saturation = 0;
		return;
	}

	/* Normalize value to 1 */
	double r = (double)(red / 255.) / value;
	double g = (double)(green / 255.) / value;
	double b = (double)(blue / 255.) / value;
	rgbMin = min3<double> (r, g, b);
	rgbMax = max3<double> (r, g, b);

	saturation = rgbMax - rgbMin;
	if (saturation == 0)
	{
		hue = 0.;
		return;
	}

	/* Normalize saturation to 1 */
	r = (r - rgbMin)/(rgbMax - rgbMin);
	g = (g - rgbMin)/(rgbMax - rgbMin);
	b = (b - rgbMin)/(rgbMax - rgbMin);
	rgbMin = min3<double> (r, g, b);
	rgbMax = max3<double> (r, g, b);

	/* Compute hue */
	if (rgbMax == r)
	{
		hue = 0.0 + 60.0 * (g - b);
	}
	else if (rgbMax == g)
	{
		hue = 120.0 + 60.0 * (b - r);
	}
	else /* rgbMax == b */
	{
		hue = 240.0 + 60.0 * (r - g);
	}
	if (hue < 0.0)
	{
		hue += 360.0;
	}
}

//-----------------------------------------------------------------------------
void CColor::fromHSV (double hue, double saturation, double value)
{
	if (value <= 0.)
	{
		red = green = blue = 0;
		return;
	}
	else if (value > 1.)
		value = 1.;
	if (saturation <= 0.)
	{
		red = green = blue = (unsigned char) (value * 255.);
		return;
	}
	else if (saturation > 1.)
		saturation = 1.;
	while (hue > 360.)
		hue -= 360.;
	while (hue < 0.)
		hue += 360.;

	const double hf = hue / 60.0;
	const int    i  = (int) floor ( hf );
	const double f  = hf - i;
	const double pv  = value * ( 1 - saturation );
	const double qv  = value * ( 1 - saturation * f );
	const double tv  = value * ( 1 - saturation * ( 1 - f ) );
	
	double r = 0.;
	double g = 0.;
	double b = 0.;

	switch (i)
	{
		// red is dominant
		case 0:
		{
			r = value;
			g = tv;
			b = pv;
			break;
		}
		case 5:
		{
			r = value;
			g = pv;
			b = qv;
			break;
		}
		case 6:
		{
			r = value;
			g = tv;
			b = pv;
			break;
		}
		case -1:
		{
			r = value;
			g = pv;
			b = qv;
			break;
		}
		// green is dominant
		case 1:
		{
			r = qv;
			g = value;
			b = pv;
			break;
		}
		case 2:
		{
			r = pv;
			g = value;
			b = tv;
			break;
		}
		// blue is dominant
		case 3:
		{
			r = pv;
			g = qv;
			b = value;
			break;
		}
		case 4:
		{
			r = tv;
			g = pv;
			b = value;
			break;
		}
	}
	red = (unsigned char) floor (r * 255. + 0.5);
	green = (unsigned char) floor (g * 255. + 0.5);
	blue = (unsigned char) floor (b * 255. + 0.5);
}

#if 0 // MAC && DEBUG
__attribute__((__constructor__)) void testHSVtoRGBAndBack ()
{
	CColor color = kBlueCColor;
	CColor color2;
	double h,s,v;
	color.toHSV (h,s,v);
	double th,ts,tv;
	for (double i = 0; i < 360; i+= 0.01)
	{
		color.fromHSV (i, s, v);
		color.toHSV (th, ts, tv);
		color2.fromHSV (th, ts, tv);
		if (color != color2)
		{
			DebugPrint ("issue\n");
		}
	}
	for (double i = 0.; i < 1.; i += 0.0001)
	{
		color.fromHSV (h, i, v);
		color.toHSV (th, ts, tv);
		color2.fromHSV (th, ts, tv);
		if (color != color2)
		{
			DebugPrint ("issue\n");
		}
		color.fromHSV (h, s, i);
		color.toHSV (th, ts, tv);
		color2.fromHSV (th, ts, tv);
		if (color != color2)
		{
			DebugPrint ("issue\n");
		}
	}
}

#endif

} // namespace
