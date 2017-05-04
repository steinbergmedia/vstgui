// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uicolor.h"

#if VSTGUI_LIVE_EDITING

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
IdStringPtr UIColor::kMsgChanged = "UIColor::kMsgChanged";
IdStringPtr UIColor::kMsgEditChange = "UIColor::kMsgEditChange";
IdStringPtr UIColor::kMsgBeginEditing = "UIColor::kMsgBeginEditing";
IdStringPtr UIColor::kMsgEndEditing = "UIColor::kMsgEndEditing";

//----------------------------------------------------------------------------------------------------
UIColor& UIColor::operator= (const CColor& c)
{
	red = c.red; green = c.green, blue = c.blue; alpha = c.alpha;
	r = red; g = green; b = blue;
	updateHSL (kTo);
	changed (kMsgChanged);
	return *this;
}

//----------------------------------------------------------------------------------------------------
void UIColor::updateHSL (HSLUpdateDirection direction)
{
	if (direction == kTo)
		toHSL (hue, saturation, lightness);
	else
	{
		fromHSL (hue, saturation, lightness);
		r = red; g = green; b = blue;
	}
	changed (kMsgEditChange);
}

//----------------------------------------------------------------------------------------------------
void UIColor::setHue (double h)
{
	if (hue != h)
	{
		hue = h;
		updateHSL (kFrom);
	}
}

//----------------------------------------------------------------------------------------------------
void UIColor::setSaturation (double s)
{
	if (saturation != s)
	{
		saturation = s;
		updateHSL (kFrom);
	}
}

//----------------------------------------------------------------------------------------------------
void UIColor::setLightness (double l)
{
	if (lightness != l)
	{
		lightness = l;
		updateHSL (kFrom);
	}
}

//----------------------------------------------------------------------------------------------------
void UIColor::setRed (double nr)
{
	if (r != nr)
	{
		r = nr;
		red = static_cast<uint8_t> (nr);
		updateHSL (kTo);
	}
}

//----------------------------------------------------------------------------------------------------
void UIColor::setGreen (double ng)
{
	if (g != ng)
	{
		g = ng;
		green = static_cast<uint8_t> (ng);
		updateHSL (kTo);
	}
}

//----------------------------------------------------------------------------------------------------
void UIColor::setBlue (double nb)
{
	if (b != nb)
	{
		b = nb;
		blue = static_cast<uint8_t> (nb);
		updateHSL (kTo);
	}
}

//----------------------------------------------------------------------------------------------------
void UIColor::setAlpha (double na)
{
	alpha = static_cast<uint8_t> (na);
	changed (kMsgEditChange);
}

//----------------------------------------------------------------------------------------------------
void UIColor::beginEdit ()
{
	changed (kMsgBeginEditing);
}

//----------------------------------------------------------------------------------------------------
void UIColor::endEdit ()
{
	changed (kMsgEndEditing);
}

}

#endif // VSTGUI_LIVE_EDITING

