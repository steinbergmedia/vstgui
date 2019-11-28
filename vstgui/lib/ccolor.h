// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguibase.h"
#include "vstguifwd.h"
#include <cmath>

namespace VSTGUI {

//-----------------------------------------------------------------------------
//! @brief RGBA Color structure
//-----------------------------------------------------------------------------
struct CColor
{
	constexpr CColor () = default;
	constexpr CColor (uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
	: red (red), green (green), blue (blue), alpha (alpha)
	{}

	constexpr CColor (const CColor& inColor)
	: red (inColor.red), green (inColor.green), blue (inColor.blue), alpha (inColor.alpha)
	{}

	//-----------------------------------------------------------------------------
	/// @name Operator Methods
	//-----------------------------------------------------------------------------
	//@{
	CColor& operator() (uint8_t _red, uint8_t _green, uint8_t _blue, uint8_t _alpha)
	{
		red   = _red;
		green = _green;
		blue  = _blue;
		alpha = _alpha;
		return *this; 
	}

	CColor& operator= (const CColor& newColor)
	{
		red   = newColor.red;
		green = newColor.green;
		blue  = newColor.blue;
		alpha = newColor.alpha;
		return *this; 
	}
	
	bool operator!= (const CColor &other) const 
	{ return (red != other.red || green != other.green || blue  != other.blue || alpha != other.alpha); }

	bool operator== (const CColor &other) const
	{ return (red == other.red && green == other.green && blue  == other.blue && alpha == other.alpha); }
	//@}

	//-----------------------------------------------------------------------------
	/// @name Convert Methods
	//-----------------------------------------------------------------------------
	//@{
	/**
	 * @brief convert to hue, saturation and value
	 * @param hue in degree [0..360]
	 * @param saturation normalized [0..1]
	 * @param value normalized [0..1]
	 */
	void toHSV (double& hue, double& saturation, double& value) const;
	/**
	 * @brief convert from hue, saturation and value
	 * @param hue in degree [0..360]
	 * @param saturation normalized [0..1]
	 * @param value normalized [0..1]
	 */
	void fromHSV (double hue, double saturation, double value);

	/**
	 * @brief convert to hue, saturation and lightness
	 * @param hue in degree [0..360]
	 * @param saturation normalized [0..1]
	 * @param lightness normalized [0..1]
	 */
	void toHSL (double& hue, double& saturation, double& lightness) const;
	/**
	 * @brief convert from hue, saturation and lightness
	 * @param hue in degree [0..360]
	 * @param saturation normalized [0..1]
	 * @param lightness normalized [0..1]
	 */
	void fromHSL (double hue, double saturation, double lightness);
	
	/** get the luma of the color */
	inline constexpr uint8_t getLuma () const;
	/** get the lightness of the color */
	uint8_t getLightness () const;

	/** get the normalized red value */
	template<typename T>
	constexpr T normRed () const;
	/** get the normalized green value */
	template<typename T>
	constexpr T normGreen () const;
	/** get the normalized blue value */
	template<typename T>
	constexpr T normBlue () const;
	/** get the normalized alpha value */
	template<typename T>
	constexpr T normAlpha () const;

	/** set the red value normalized */
	template<typename T>
	void setNormRed (T v);
	/** set the green value normalized */
	template<typename T>
	void setNormGreen (T v);
	/** set the blue value normalized */
	template<typename T>
	void setNormBlue (T v);
	/** set the alpha value normalized */
	template<typename T>
	void setNormAlpha (T v);
	//@}
	
	bool fromString (UTF8StringPtr str);
	UTF8String toString () const;
	static bool isColorRepresentation (UTF8StringPtr str);

	/** red component [0..255] */
	uint8_t red {255};
	/** green component [0..255] */
	uint8_t green {255};
	/** blue component [0..255] */
	uint8_t blue {255};
	/** alpha component [0..255] */
	uint8_t alpha {255};
};

//-----------------------------------------------------------------------------
inline constexpr CColor MakeCColor (uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0,
                                    uint8_t alpha = 255)
{
	return CColor (red, green, blue, alpha);
}

//-----------------------------------------------------------------------------
// define some basic colors
constexpr const CColor kTransparentCColor	= CColor (255, 255, 255,   0);
constexpr const CColor kBlackCColor			= CColor (  0,   0,   0, 255);
constexpr const CColor kWhiteCColor			= CColor (255, 255, 255, 255);
constexpr const CColor kGreyCColor			= CColor (127, 127, 127, 255);
constexpr const CColor kRedCColor			= CColor (255,   0,   0, 255);
constexpr const CColor kGreenCColor			= CColor (  0, 255,   0, 255);
constexpr const CColor kBlueCColor			= CColor (  0,   0, 255, 255);
constexpr const CColor kYellowCColor		= CColor (255, 255,   0, 255);
constexpr const CColor kMagentaCColor		= CColor (255,   0, 255, 255);
constexpr const CColor kCyanCColor			= CColor (  0, 255, 255, 255);

//-----------------------------------------------------------------------------
inline constexpr uint8_t CColor::getLuma () const
{
	return static_cast<uint8_t> (static_cast<float> (red) * 0.3f +
	                             static_cast<float> (green) * 0.59f +
	                             static_cast<float> (blue) * 0.11f);
}

//-----------------------------------------------------------------------------
template <typename T>
constexpr T CColor::normRed () const
{
	return static_cast<T> (red) / static_cast<T> (255.);
}

//-----------------------------------------------------------------------------
template <typename T>
constexpr T CColor::normGreen () const
{
	return static_cast<T> (green) / static_cast<T> (255.);
}

//-----------------------------------------------------------------------------
template <typename T>
constexpr T CColor::normBlue () const
{
	return static_cast<T> (blue) / static_cast<T> (255.);
}

//-----------------------------------------------------------------------------
template <typename T>
constexpr T CColor::normAlpha () const
{
	return static_cast<T> (alpha) / static_cast<T> (255.);
}

//-----------------------------------------------------------------------------
template <typename T>
void CColor::setNormRed (T v)
{
	vstgui_assert (v >= 0. && v <= 1.);
	red = static_cast<uint8_t> (std::round (v * 255.));
}

//-----------------------------------------------------------------------------
template <typename T>
void CColor::setNormGreen (T v)
{
	vstgui_assert (v >= 0. && v <= 1.);
	green = static_cast<uint8_t> (std::round (v * 255.));
}

//-----------------------------------------------------------------------------
template <typename T>
void CColor::setNormBlue (T v)
{
	vstgui_assert (v >= 0. && v <= 1.);
	blue = static_cast<uint8_t> (std::round (v * 255.));
}

//-----------------------------------------------------------------------------
template <typename T>
void CColor::setNormAlpha (T v)
{
	vstgui_assert (v >= 0. && v <= 1.);
	alpha = static_cast<uint8_t> (std::round (v * 255.));
}

} // VSTGUI
