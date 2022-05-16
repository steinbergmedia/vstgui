// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uicolorslider.h"

#if VSTGUI_LIVE_EDITING

#include "../../lib/coffscreencontext.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
UIColorSlider::UIColorSlider (UIColor* color, int32_t style)
: CSlider (CRect (0, 0, 0, 0), nullptr, 0, 0, 0, nullptr, nullptr)
, color (color)
, style (style)
{
	color->registerListener (this);
}

//----------------------------------------------------------------------------------------------------
UIColorSlider::~UIColorSlider ()
{
	color->unregisterListener (this);
}

//----------------------------------------------------------------------------------------------------
void UIColorSlider::draw (CDrawContext* context)
{
	if (getHandle () == nullptr)
		updateHandle (context);
	if (getBackground () == nullptr)
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
		setBackground (nullptr);
	if (heightDifferent)
		setHandle (nullptr);
}

//----------------------------------------------------------------------------------------------------
void UIColorSlider::uiColorChanged (UIColor* c)
{
	setBackground (nullptr);
}

//----------------------------------------------------------------------------------------------------
void UIColorSlider::updateBackground (CDrawContext* context)
{
	double scaleFactor = context->getScaleFactor ();
	if (auto offscreen = COffscreenContext::create ({getWidth (), getHeight ()}, scaleFactor))
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

		auto maxLines = static_cast<size_t> (std::ceil (widthPerColor / minWidth));
		CDrawContext::LineList lines;
		lines.reserve (maxLines);

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
					lines.emplace_back (r.getTopLeft (), r.getBottomLeft ());
					r.offset (minWidth, 0);
				}
				if (!lines.empty ())
				{
					offscreen->drawLines (lines);
					lines.clear ();
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
	if (auto offscreen = COffscreenContext::create ({7., getHeight ()}, context->getScaleFactor ()))
	{
		auto lineWidth = 1.;
		offscreen->beginDraw ();
		offscreen->setFrameColor (kBlackCColor);
		offscreen->setLineWidth (lineWidth);
		offscreen->setDrawMode (kAliasing);
		CRect r (0, 0, 7, getHeight ());
		offscreen->drawRect (r, kDrawStroked);
		r.inset (lineWidth, lineWidth);
		offscreen->setFrameColor (kWhiteCColor);
		offscreen->drawRect (r, kDrawStroked);
		offscreen->endDraw ();
		setHandle (offscreen->getBitmap ());
	}
}

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
