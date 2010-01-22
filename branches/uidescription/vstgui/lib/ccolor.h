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

#ifndef __ccolor__
#define __ccolor__

#include "vstguibase.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
//! @brief RGBA Color structure
//-----------------------------------------------------------------------------
struct CColor
{
	CColor (unsigned char red = 255, unsigned char green = 255, unsigned char blue = 255, unsigned char alpha = 255)
	: red (red), green (green), blue (blue), alpha (alpha)
	{}

	CColor (const CColor& inColor)
	: red (inColor.red), green (inColor.green), blue (inColor.blue), alpha (inColor.alpha)
	{}

	//-----------------------------------------------------------------------------
	/// @name Operator Methods
	//-----------------------------------------------------------------------------
	//@{
	CColor& operator () (unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
	{
		this->red   = red;
		this->green = green;
		this->blue  = blue;
		this->alpha = alpha;
		return *this; 
	}

	CColor& operator = (const CColor& newColor)
	{
		red   = newColor.red;
		green = newColor.green;
		blue  = newColor.blue;
		alpha = newColor.alpha;
		return *this; 
	}
	
	CColor operator ~ ()
	{
		CColor c;
		c.red   = ~red;
		c.green = ~green;
		c.blue  = ~blue;
		c.alpha = ~alpha;
		return c;
	}

	bool operator != (const CColor &other) const 
	{ return (red != other.red || green != other.green || blue  != other.blue || alpha != other.alpha); }

	bool operator == (const CColor &other) const
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
	void toHSV (double& hue, double& saturation, double& value);
	/**
	 * @brief convert from hue, saturation and value
	 * @param hue in degree [0..360]
	 * @param saturation normalized [0..1]
	 * @param value normalized [0..1]
	 */
	void fromHSV (double hue, double saturation, double value);
	//@}
	
	unsigned char red;		///< red component [0..255]
	unsigned char green;	///< green component [0..255]
	unsigned char blue;		///< blue component [0..255]
	unsigned char alpha;	///< alpha component [0..255]
};

inline CColor MakeCColor (unsigned char red = 0, unsigned char green = 0, unsigned char blue = 0, unsigned char alpha = 255)
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
