// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uicolor.h"

#if VSTGUI_LIVE_EDITING

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
UIColor& UIColor::operator= (const CColor& c)
{
	red = c.red; green = c.green; blue = c.blue; alpha = c.alpha;
	r = red; g = green; b = blue;
	updateHSL (kTo);
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
	editChange ();
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
	editChange ();
}

//----------------------------------------------------------------------------------------------------
void UIColor::beginEdit ()
{
	forEachListener ([this] (IUIColorListener* l) { l->uiColorBeginEditing (this); });
}

//----------------------------------------------------------------------------------------------------
void UIColor::endEdit ()
{
	forEachListener ([this] (IUIColorListener* l) { l->uiColorEndEditing (this); });
}

//----------------------------------------------------------------------------------------------------
void UIColor::editChange ()
{
	forEachListener ([this] (IUIColorListener* l) { l->uiColorChanged (this); });
}

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
