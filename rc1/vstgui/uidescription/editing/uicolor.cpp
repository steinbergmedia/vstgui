//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins
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

