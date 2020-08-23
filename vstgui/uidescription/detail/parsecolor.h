// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../lib/ccolor.h"
#include <string>

namespace VSTGUI {
namespace Detail {

//-----------------------------------------------------------------------------
inline bool parseColor (const std::string& colorString, CColor& color)
{
	if (colorString.length () == 7)
	{
		if (colorString[0] == '#')
		{
			std::string rv (colorString.substr (1, 2));
			std::string gv (colorString.substr (3, 2));
			std::string bv (colorString.substr (5, 2));
			color.red = (uint8_t)strtol (rv.c_str (), nullptr, 16);
			color.green = (uint8_t)strtol (gv.c_str (), nullptr, 16);
			color.blue = (uint8_t)strtol (bv.c_str (), nullptr, 16);
			color.alpha = 255;
			return true;
		}
	}
	if (colorString.length () == 9)
	{
		if (colorString[0] == '#')
		{
			std::string rv (colorString.substr (1, 2));
			std::string gv (colorString.substr (3, 2));
			std::string bv (colorString.substr (5, 2));
			std::string av (colorString.substr (7, 2));
			color.red = (uint8_t)strtol (rv.c_str (), nullptr, 16);
			color.green = (uint8_t)strtol (gv.c_str (), nullptr, 16);
			color.blue = (uint8_t)strtol (bv.c_str (), nullptr, 16);
			color.alpha = (uint8_t)strtol (av.c_str (), nullptr, 16);
			return true;
		}
	}
	return false;
}


} // Detail
} // VSTGUI
