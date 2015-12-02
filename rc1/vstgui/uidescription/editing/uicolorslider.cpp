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

#include "uicolorslider.h"

#if VSTGUI_LIVE_EDITING

#include "uicolor.h"
#include "../../lib/coffscreencontext.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
UIColorSlider::UIColorSlider (UIColor* color, int32_t style)
: CSlider (CRect (0, 0, 0, 0), 0, 0, 0, 0, 0, 0)
, color (color)
, style (style)
{
	color->addDependency (this);
}

//----------------------------------------------------------------------------------------------------
UIColorSlider::~UIColorSlider ()
{
	color->removeDependency (this);
}

//----------------------------------------------------------------------------------------------------
void UIColorSlider::draw (CDrawContext* context)
{
	if (getHandle () == 0)
		updateHandle (context);
	if (getBackground () == 0)
		updateBackground (context);
	CSlider::draw (context);
}

//----------------------------------------------------------------------------------------------------
void UIColorSlider::setViewSize (const CRect& rect, bool invalid)
{
	bool widthDifferent = rect.getWidth () != getWidth ();
	bool heightDifferent = rect.getHeight () != getHeight ();
	CSlider::setViewSize (rect, invalid);
	if (widthDifferent)
		setBackground (0);
	if (heightDifferent)
		setHandle (0);
}

//----------------------------------------------------------------------------------------------------
CMessageResult UIColorSlider::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == UIColor::kMsgChanged || message == UIColor::kMsgEditChange)
	{
		setBackground (0);
		return kMessageNotified;
	}
	return CSlider::notify (sender, message);
}

//----------------------------------------------------------------------------------------------------
void UIColorSlider::updateBackground (CDrawContext* context)
{
	double scaleFactor = context->getScaleFactor ();
	SharedPointer<COffscreenContext> offscreen = owned (COffscreenContext::create (getFrame (), getWidth (), getHeight (), scaleFactor));
	if (offscreen)
	{
		const int32_t kNumPoints = (style <= kLightness) ? 360 : 256;
		CCoord width = std::floor (getWidth () + 0.5);
		offscreen->beginDraw ();
		offscreen->setDrawMode (kAliasing);
		CCoord minWidth = 1. / scaleFactor;
		CCoord widthPerColor = width / static_cast<double> (kNumPoints - 1);
		CRect r;
		r.setHeight (getHeight ());
		r.setWidth (widthPerColor < minWidth ? minWidth : (std::floor (widthPerColor * scaleFactor + 0.5) / scaleFactor));
		r.offset (-r.getWidth (), 0);
		offscreen->setLineWidth (minWidth);
		for (int32_t i = 0; i < kNumPoints; i++)
		{
			CCoord x = std::floor (widthPerColor * i * scaleFactor + 0.5) / scaleFactor;
			if (x > r.right || i == kNumPoints -1)
			{
				CColor c = color->base ();
				switch (style)
				{
					case kRed:
					{
						c.red = (uint8_t)i;
						break;
					}
					case kGreen:
					{
						c.green = (uint8_t)i;
						break;
					}
					case kBlue:
					{
						c.blue = (uint8_t)i;
						break;
					}
					case kAlpha:
					{
						c.alpha = (uint8_t)i;
						break;
					}
					case kHue:
					{
						double hue = (static_cast<double> (i) / static_cast<double> (kNumPoints)) * 360.;
						c.fromHSL (hue, color->getSaturation (), color->getLightness ());
						break;
					}
					case kSaturation:
					{
						double sat = (static_cast<double> (i) / static_cast<double> (kNumPoints));
						c.fromHSL (color->getHue (), sat, color->getLightness ());
						break;
					}
					case kLightness:
					{
						double light = (static_cast<double> (i) / static_cast<double> (kNumPoints));
						c.fromHSL (color->getHue (), color->getSaturation (), light);
						break;
					}
				}
				offscreen->setFrameColor (c);
				CCoord next = r.left + widthPerColor;
				while (r.left < next)
				{
					offscreen->drawLine (r.getTopLeft (), r.getBottomLeft ());
					r.offset (minWidth, 0);
				}
			}
		}
		offscreen->drawLine (r.getTopLeft (), r.getBottomLeft ());
		offscreen->endDraw ();
		setBackground (offscreen->getBitmap ());
	}
}

//----------------------------------------------------------------------------------------------------
void UIColorSlider::updateHandle (CDrawContext* context)
{
	SharedPointer<COffscreenContext> offscreen = owned (COffscreenContext::create (getFrame (), 7, getHeight (), context->getScaleFactor ()));
	if (offscreen)
	{
		offscreen->beginDraw ();
		offscreen->setFrameColor (kBlackCColor);
		offscreen->setLineWidth (1);
		offscreen->setDrawMode (kAliasing);
		CRect r (0, 0, 7, getHeight ());
		offscreen->drawRect (r, kDrawStroked);
		r.inset (1, 1);
		offscreen->setFrameColor (kWhiteCColor);
		offscreen->drawRect (r, kDrawStroked);
		offscreen->endDraw ();
		setHandle (offscreen->getBitmap ());
	}
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
