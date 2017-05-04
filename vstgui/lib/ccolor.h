// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __ccolor__
#define __ccolor__

#include "vstguibase.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
//! @brief RGBA Color structure
//-----------------------------------------------------------------------------
struct CColor
{
	CColor () = default;
	CColor (uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
	: red (red), green (green), blue (blue), alpha (alpha)
	{}

	CColor (const CColor& inColor)
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
	uint8_t getLuma () const { return (uint8_t)((float)red * 0.3f + (float)green * 0.59f + (float)blue * 0.11f); }
	/** get the lightness of the color */
	uint8_t getLightness () const;
	//@}
	
	uint8_t red {255};			///< red component [0..255]
	uint8_t green {255};		///< green component [0..255]
	uint8_t blue {255};			///< blue component [0..255]
	uint8_t alpha {255};		///< alpha component [0..255]
};

inline CColor MakeCColor (uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0, uint8_t alpha = 255)
{
	return CColor (red, green, blue, alpha);
}

// define some basic colors
extern const CColor kTransparentCColor;
extern const CColor kBlackCColor;
extern const CColor kWhiteCColor;
extern const CColor kGreyCColor;
extern const CColor kRedCColor;
extern const CColor kGreenCColor;
extern const CColor kBlueCColor;
extern const CColor kYellowCColor;
extern const CColor kCyanCColor;
extern const CColor kMagentaCColor;


} // namespace

#endif
