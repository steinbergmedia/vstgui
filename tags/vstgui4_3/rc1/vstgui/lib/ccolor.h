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

#ifndef __ccolor__
#define __ccolor__

#include "vstguibase.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
//! @brief RGBA Color structure
//-----------------------------------------------------------------------------
struct CColor
{
	CColor (uint8_t red = 255, uint8_t green = 255, uint8_t blue = 255, uint8_t alpha = 255)
	: red (red), green (green), blue (blue), alpha (alpha)
	{}

	CColor (const CColor& inColor)
	: red (inColor.red), green (inColor.green), blue (inColor.blue), alpha (inColor.alpha)
	{}

	//-----------------------------------------------------------------------------
	/// @name Operator Methods
	//-----------------------------------------------------------------------------
	//@{
	CColor& operator() (uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
	{
		this->red   = red;
		this->green = green;
		this->blue  = blue;
		this->alpha = alpha;
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
	
	VSTGUI_DEPRECATED (CColor operator~ ()
	{
		CColor c;
		c.red   = ~red;
		c.green = ~green;
		c.blue  = ~blue;
		c.alpha = ~alpha;
		return c;
	})

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
	
	uint8_t red;		///< red component [0..255]
	uint8_t green;		///< green component [0..255]
	uint8_t blue;		///< blue component [0..255]
	uint8_t alpha;		///< alpha component [0..255]
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
