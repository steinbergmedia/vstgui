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

using namespace std::literals;

//-----------------------------------------------------------------------------
static constexpr std::array<CSSNamedColor, 148> namedColors = {
	{{"transparent"sv, {0, 0, 0, 0}},
	 {"aliceblue"sv, {240, 248, 255, 255}},
	 {"antiquewhite"sv, {250, 235, 215, 255}},
	 {"aqua"sv, {0, 255, 255, 255}},
	 {"aquamarine"sv, {127, 255, 212, 255}},
	 {"azure"sv, {240, 255, 255, 255}},
	 {"beige"sv, {245, 245, 220, 255}},
	 {"bisque"sv, {255, 228, 196, 255}},
	 {"black"sv, {0, 0, 0, 255}},
	 {"blanchedalmond"sv, {255, 235, 205, 255}},
	 {"blue"sv, {0, 0, 255, 255}},
	 {"blueviolet"sv, {138, 43, 226, 255}},
	 {"brown"sv, {165, 42, 42, 255}},
	 {"burlywood"sv, {222, 184, 135, 255}},
	 {"cadetblue"sv, {95, 158, 160, 255}},
	 {"chartreuse"sv, {127, 255, 0, 255}},
	 {"chocolate"sv, {210, 105, 30, 255}},
	 {"coral"sv, {255, 127, 80, 255}},
	 {"cornflowerblue"sv, {100, 149, 237, 255}},
	 {"cornsilk"sv, {255, 248, 220, 255}},
	 {"crimson"sv, {220, 20, 60, 255}},
	 {"cyan"sv, {0, 255, 255, 255}},
	 {"darkblue"sv, {0, 0, 139, 255}},
	 {"darkcyan"sv, {0, 139, 139, 255}},
	 {"darkgoldenrod"sv, {184, 134, 11, 255}},
	 {"darkgray"sv, {169, 169, 169, 255}},
	 {"darkgreen"sv, {0, 100, 0, 255}},
	 {"darkgrey"sv, {169, 169, 169, 255}},
	 {"darkkhaki"sv, {189, 183, 107, 255}},
	 {"darkmagenta"sv, {139, 0, 139, 255}},
	 {"darkolivegreen"sv, {85, 107, 47, 255}},
	 {"darkorange"sv, {255, 140, 0, 255}},
	 {"darkorchid"sv, {153, 50, 204, 255}},
	 {"darkred"sv, {139, 0, 0, 255}},
	 {"darksalmon"sv, {233, 150, 122, 255}},
	 {"darkseagreen"sv, {143, 188, 143, 255}},
	 {"darkslateblue"sv, {72, 61, 139, 255}},
	 {"darkslategray"sv, {47, 79, 79, 255}},
	 {"darkslategrey"sv, {47, 79, 79, 255}},
	 {"darkturquoise"sv, {0, 206, 209, 255}},
	 {"darkviolet"sv, {148, 0, 211, 255}},
	 {"deeppink"sv, {255, 20, 147, 255}},
	 {"deepskyblue"sv, {0, 191, 255, 255}},
	 {"dimgray"sv, {105, 105, 105, 255}},
	 {"dimgrey"sv, {105, 105, 105, 255}},
	 {"dodgerblue"sv, {30, 144, 255, 255}},
	 {"firebrick"sv, {178, 34, 34, 255}},
	 {"floralwhite"sv, {255, 250, 240, 255}},
	 {"forestgreen"sv, {34, 139, 34, 255}},
	 {"fuchsia"sv, {255, 0, 255, 255}},
	 {"gainsboro"sv, {220, 220, 220, 255}},
	 {"ghostwhite"sv, {248, 248, 255, 255}},
	 {"gold"sv, {255, 215, 0, 255}},
	 {"goldenrod"sv, {218, 165, 32, 255}},
	 {"gray"sv, {128, 128, 128, 255}},
	 {"green"sv, {0, 128, 0, 255}},
	 {"greenyellow"sv, {173, 255, 47, 255}},
	 {"grey"sv, {128, 128, 128, 255}},
	 {"honeydew"sv, {240, 255, 240, 255}},
	 {"hotpink"sv, {255, 105, 180, 255}},
	 {"indianred"sv, {205, 92, 92, 255}},
	 {"indigo"sv, {75, 0, 130, 255}},
	 {"ivory"sv, {255, 255, 240, 255}},
	 {"khaki"sv, {240, 230, 140, 255}},
	 {"lavender"sv, {230, 230, 250, 255}},
	 {"lavenderblush"sv, {255, 240, 245, 255}},
	 {"lawngreen"sv, {124, 252, 0, 255}},
	 {"lemonchiffon"sv, {255, 250, 205, 255}},
	 {"lightblue"sv, {173, 216, 230, 255}},
	 {"lightcoral"sv, {240, 128, 128, 255}},
	 {"lightcyan"sv, {224, 255, 255, 255}},
	 {"lightgoldenrodyellow"sv, {250, 250, 210, 255}},
	 {"lightgray"sv, {211, 211, 211, 255}},
	 {"lightgreen"sv, {144, 238, 144, 255}},
	 {"lightgrey"sv, {211, 211, 211, 255}},
	 {"lightpink"sv, {255, 182, 193, 255}},
	 {"lightsalmon"sv, {255, 160, 122, 255}},
	 {"lightseagreen"sv, {32, 178, 170, 255}},
	 {"lightskyblue"sv, {135, 206, 250, 255}},
	 {"lightslategray"sv, {119, 136, 153, 255}},
	 {"lightslategrey"sv, {119, 136, 153, 255}},
	 {"lightsteelblue"sv, {176, 196, 222, 255}},
	 {"lightyellow"sv, {255, 255, 224, 255}},
	 {"lime"sv, {0, 255, 0, 255}},
	 {"limegreen"sv, {50, 205, 50, 255}},
	 {"linen"sv, {250, 240, 230, 255}},
	 {"magenta"sv, {255, 0, 255, 255}},
	 {"maroon"sv, {128, 0, 0, 255}},
	 {"mediumaquamarine"sv, {102, 205, 170, 255}},
	 {"mediumblue"sv, {0, 0, 205, 255}},
	 {"mediumorchid"sv, {186, 85, 211, 255}},
	 {"mediumpurple"sv, {147, 112, 219, 255}},
	 {"mediumseagreen"sv, {60, 179, 113, 255}},
	 {"mediumslateblue"sv, {123, 104, 238, 255}},
	 {"mediumspringgreen"sv, {0, 250, 154, 255}},
	 {"mediumturquoise"sv, {72, 209, 204, 255}},
	 {"mediumvioletred"sv, {199, 21, 133, 255}},
	 {"midnightblue"sv, {25, 25, 112, 255}},
	 {"mintcream"sv, {245, 255, 250, 255}},
	 {"mistyrose"sv, {255, 228, 225, 255}},
	 {"moccasin"sv, {255, 228, 181, 255}},
	 {"navajowhite"sv, {255, 222, 173, 255}},
	 {"navy"sv, {0, 0, 128, 255}},
	 {"oldlace"sv, {253, 245, 230, 255}},
	 {"olive"sv, {128, 128, 0, 255}},
	 {"olivedrab"sv, {107, 142, 35, 255}},
	 {"orange"sv, {255, 165, 0, 255}},
	 {"orangered"sv, {255, 69, 0, 255}},
	 {"orchid"sv, {218, 112, 214, 255}},
	 {"palegoldenrod"sv, {238, 232, 170, 255}},
	 {"palegreen"sv, {152, 251, 152, 255}},
	 {"paleturquoise"sv, {175, 238, 238, 255}},
	 {"palevioletred"sv, {219, 112, 147, 255}},
	 {"papayawhip"sv, {255, 239, 213, 255}},
	 {"peachpuff"sv, {255, 218, 185, 255}},
	 {"peru"sv, {205, 133, 63, 255}},
	 {"pink"sv, {255, 192, 203, 255}},
	 {"plum"sv, {221, 160, 221, 255}},
	 {"powderblue"sv, {176, 224, 230, 255}},
	 {"purple"sv, {128, 0, 128, 255}},
	 {"red"sv, {255, 0, 0, 255}},
	 {"rosybrown"sv, {188, 143, 143, 255}},
	 {"royalblue"sv, {65, 105, 225, 255}},
	 {"saddlebrown"sv, {139, 69, 19, 255}},
	 {"salmon"sv, {250, 128, 114, 255}},
	 {"sandybrown"sv, {244, 164, 96, 255}},
	 {"seagreen"sv, {46, 139, 87, 255}},
	 {"seashell"sv, {255, 245, 238, 255}},
	 {"sienna"sv, {160, 82, 45, 255}},
	 {"silver"sv, {192, 192, 192, 255}},
	 {"skyblue"sv, {135, 206, 235, 255}},
	 {"slateblue"sv, {106, 90, 205, 255}},
	 {"slategray"sv, {112, 128, 144, 255}},
	 {"slategrey"sv, {112, 128, 144, 255}},
	 {"snow"sv, {255, 250, 250, 255}},
	 {"springgreen"sv, {0, 255, 127, 255}},
	 {"steelblue"sv, {70, 130, 180, 255}},
	 {"tan"sv, {210, 180, 140, 255}},
	 {"teal"sv, {0, 128, 128, 255}},
	 {"thistle"sv, {216, 191, 216, 255}},
	 {"tomato"sv, {255, 99, 71, 255}},
	 {"turquoise"sv, {64, 224, 208, 255}},
	 {"violet"sv, {238, 130, 238, 255}},
	 {"wheat"sv, {245, 222, 179, 255}},
	 {"white"sv, {255, 255, 255, 255}},
	 {"whitesmoke"sv, {245, 245, 245, 255}},
	 {"yellow"sv, {255, 255, 0, 255}},
	 {"yellowgreen"sv, {154, 205, 50, 255}}}};

//-----------------------------------------------------------------------------
const CSSNamedColorArray& getCSSNamedColors () { return namedColors; }

} // VSTGUI
