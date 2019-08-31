// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "ccolor.h"
#include "cstring.h"
#include "algorithm.h"
#include <cmath>
#include <sstream>
#include <iomanip>

namespace VSTGUI {

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
uint8_t CColor::getLightness () const
{
	return (max3<uint8_t> (red, green, blue) / 2) + (min3<uint8_t>(red, green, blue) / 2);
}

//-----------------------------------------------------------------------------
void CColor::toHSL (double& hue, double& saturation, double& lightness) const
{
	double r = normRed<double> ();
	double g = normGreen<double> ();
	double b = normBlue<double> ();
	double M = max3<double> (r, g ,b);
	double m = min3<double> (r, g ,b);
	double C = M - m;
	lightness = (M + m) / 2.;
	if (C == 0.)
	{
		hue = saturation = 0.;
		return;
	}
	if (M == r)
	{
		hue = fmod (((g-b) / C), 6.);
	}
	else if (M == g)
	{
		hue = ((b - r) / C) + 2.;
	}
	else if (M == b)
	{
		hue = ((r - g) / C) + 4.;
	}
	hue *= 60.;
	if (hue < 0.0)
		hue += 360.0;
	if (lightness <= 0.5)
		saturation = C / (2. * lightness);
	else
		saturation = C / (2. - 2. * lightness);
}

//-----------------------------------------------------------------------------
void CColor::fromHSL (double hue, double saturation, double lightness)
{
	while (hue > 360.)
		hue -= 360.;
	while (hue < 0.)
		hue += 360.;
	double C = (1. - fabs (2 * lightness - 1)) * saturation;
	double H = hue / 60.;
	double X = C * (1. - fabs (fmod (H, 2) - 1.));
	double r,g,b;
	if (H >= 0 && H < 1.)
	{
		r = C;
		g = X;
		b = 0.;
	}
	else if (H >= 1. && H < 2.)
	{
		r = X;
		g = C;
		b = 0.;
	}
	else if (H >= 2. && H < 3.)
	{
		r = 0.;
		g = C;
		b = X;
	}
	else if (H >= 3. && H < 4.)
	{
		r = 0.;
		g = X;
		b = C;
	}
	else if (H >= 4. && H < 5.)
	{
		r = X;
		g = 0.;
		b = C;
	}
	else // if (H >= 5. && H <= 6.)
	{
		r = C;
		g = 0.;
		b = X;
	}
	double m = lightness - (C / 2.);
	setNormRed (clampNorm (r + m));
	setNormGreen (clampNorm (g + m));
	setNormBlue (clampNorm (b + m));
}

//-----------------------------------------------------------------------------
void CColor::toHSV (double& hue, double& saturation, double& value) const
{
	double rgbMax = (max3<uint8_t> (red, green, blue)) / 255.;
	value = rgbMax;
	if (value == 0)
	{
		hue = saturation = 0;
		return;
	}

	/* Normalize value to 1 */
	double r = normRed<double> () / value;
	double g = normGreen<double> () / value;
	double b = normBlue<double> () / value;
	double rgbMin = min3<double> (r, g, b);
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
		red = green = blue = static_cast<uint8_t> (value * 255.);
		return;
	}
	else if (saturation > 1.)
		saturation = 1.;
	while (hue > 360.)
		hue -= 360.;
	while (hue < 0.)
		hue += 360.;

	const double hf  = hue / 60.0;
	const int32_t i  = static_cast<int32_t> (floor (hf));
	const double f   = hf - i;
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
	setNormRed (clampNorm (r));
	setNormGreen (clampNorm (g));
	setNormBlue (clampNorm (b));
}

//-----------------------------------------------------------------------------
bool CColor::isColorRepresentation (UTF8StringPtr str)
{
	if (str && str[0] == '#' && strlen (str) == 9)
		return true;
	return false;
}

//-----------------------------------------------------------------------------
bool CColor::fromString (UTF8StringPtr str)
{
	if (!str)
		return false;
	if (!isColorRepresentation (str))
		return false;
	std::string rv (str + 1, 2);
	std::string gv (str + 3, 2);
	std::string bv (str + 5, 2);
	std::string av (str + 7, 2);
	red = (uint8_t)strtol (rv.data (), nullptr, 16);
	green = (uint8_t)strtol (gv.data (), nullptr, 16);
	blue = (uint8_t)strtol (bv.data (), nullptr, 16);
	alpha = (uint8_t)strtol (av.data (), nullptr, 16);
	return true;
}

//-----------------------------------------------------------------------------
UTF8String CColor::toString () const
{
	std::stringstream str;
	str << "#";
	str << std::hex << std::setw (2) << std::setfill ('0') << static_cast<int32_t> (red);
	str << std::hex << std::setw (2) << std::setfill ('0') << static_cast<int32_t> (green);
	str << std::hex << std::setw (2) << std::setfill ('0') << static_cast<int32_t> (blue);
	str << std::hex << std::setw (2) << std::setfill ('0') << static_cast<int32_t> (alpha);
	return UTF8String (str.str ());
}

} // VSTGUI
