// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cgradientview.h"
#include "cdrawcontext.h"
#include "cgraphicspath.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
CGradientView::CGradientView (const CRect& size)
: CView (size)
{
}

//------------------------------------------------------------------------
void CGradientView::attributeChanged ()
{
	invalid ();
}

//-----------------------------------------------------------------------------
void CGradientView::setGradientStyle (GradientStyle style)
{
	if (gradientStyle != style)
	{
		gradientStyle = style;
		attributeChanged ();
	}
}

//------------------------------------------------------------------------
void CGradientView::setGradient (CGradient* newGradient)
{
	if (gradient != newGradient)
	{
		gradient = newGradient;
		attributeChanged ();
	}
}

//-----------------------------------------------------------------------------
void CGradientView::setFrameColor (const CColor& newColor)
{
	if (newColor != frameColor)
	{
		frameColor = newColor;
		attributeChanged ();
	}
}

//-----------------------------------------------------------------------------
void CGradientView::setGradientAngle (double angle)
{
	if (angle != gradientAngle)
	{
		gradientAngle = angle;
		attributeChanged ();
	}
}

//-----------------------------------------------------------------------------
void CGradientView::setRoundRectRadius (CCoord radius)
{
	if (radius != roundRectRadius)
	{
		roundRectRadius = radius;
		path = nullptr;
		attributeChanged ();
	}
}

//-----------------------------------------------------------------------------
void CGradientView::setFrameWidth (CCoord width)
{
	if (width != frameWidth)
	{
		frameWidth = width;
		path = nullptr;
		attributeChanged ();
	}
}

//-----------------------------------------------------------------------------
void CGradientView::setDrawAntialiased (bool state)
{
	if (state != drawAntialiased)
	{
		drawAntialiased = state;
		attributeChanged ();
	}
}

//-----------------------------------------------------------------------------
void CGradientView::setRadialCenter (const CPoint& center)
{
	if (radialCenter != center)
	{
		radialCenter = center;
		attributeChanged ();
	}
}

//-----------------------------------------------------------------------------
void CGradientView::setRadialRadius (CCoord radius)
{
	if (radialRadius != radius)
	{
		radialRadius = radius;
		attributeChanged ();
	}
}

//-----------------------------------------------------------------------------
void CGradientView::setViewSize (const CRect& rect, bool invalid)
{
	if (rect != getViewSize ())
	{
		CView::setViewSize (rect, invalid);
		path = nullptr;
	}
}

//-----------------------------------------------------------------------------
void CGradientView::draw (CDrawContext* context)
{
	auto lineWidth = getFrameWidth ();
	if (lineWidth < 0.)
		lineWidth = context->getHairlineSize ();
	if (path == nullptr)
	{
		CRect r = getViewSize ();
		r.inset (lineWidth / 2., lineWidth / 2.);
		path = owned (context->createRoundRectGraphicsPath (r, roundRectRadius));
	}
	if (path && gradient)
	{
		context->setDrawMode (drawAntialiased ? kAntiAliasing : kAliasing);

		if (gradientStyle == kLinearGradient)
		{
			CPoint colorStartPoint (0, 0);
			colorStartPoint.x = getViewSize ().left + getViewSize ().getWidth () / 2 + cos (radians (gradientAngle-90)) * getViewSize ().getWidth () / 2;
			colorStartPoint.y = getViewSize ().top + getViewSize ().getHeight () / 2 + sin (radians (gradientAngle-90)) * getViewSize ().getHeight () / 2;
			CPoint colorEndPoint (0, getViewSize ().getHeight ());
			colorEndPoint.x = getViewSize ().left + getViewSize ().getWidth () / 2 + cos (radians (gradientAngle+90)) * getViewSize ().getWidth () / 2;
			colorEndPoint.y = getViewSize ().top + getViewSize ().getHeight () / 2 + sin (radians (gradientAngle+90)) * getViewSize ().getHeight () / 2;
			context->fillLinearGradient (path, *gradient, colorStartPoint, colorEndPoint, false);
		}
		else
		{
			CPoint center (radialCenter);
			center.x *= getViewSize ().getWidth ();
			center.y *= getViewSize ().getHeight ();
			center.offset (getViewSize ().left, getViewSize ().top);
			context->fillRadialGradient (path, *gradient, center, radialRadius * std::max (getViewSize ().getWidth (), getViewSize ().getHeight ()));
		}
		
		if (frameColor.alpha != 0 && lineWidth > 0.)
		{
			context->setDrawMode (drawAntialiased ? kAntiAliasing : kAliasing);
			context->setFrameColor (frameColor);
			context->setLineWidth (lineWidth);
			context->setLineStyle (kLineSolid);
			context->drawGraphicsPath (path, CDrawContext::kPathStroked);
		}
	}
}

} // VSTGUI
