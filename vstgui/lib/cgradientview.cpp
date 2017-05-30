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

//-----------------------------------------------------------------------------
void CGradientView::setGradientStyle (GradientStyle style)
{
	if (gradientStyle != style)
	{
		gradientStyle = style;
		invalid ();
	}
}

//------------------------------------------------------------------------
void CGradientView::setGradient (CGradient* newGradient)
{
	if (gradient != newGradient)
	{
		gradient = newGradient;
		invalid ();
	}
}

//-----------------------------------------------------------------------------
void CGradientView::setFrameColor (const CColor& newColor)
{
	if (newColor != frameColor)
	{
		frameColor = newColor;
		invalid ();
	}
}

//-----------------------------------------------------------------------------
void CGradientView::setGradientAngle (double angle)
{
	if (angle != gradientAngle)
	{
		gradientAngle = angle;
		invalid ();
	}
}

//-----------------------------------------------------------------------------
void CGradientView::setRoundRectRadius (CCoord radius)
{
	if (radius != roundRectRadius)
	{
		roundRectRadius = radius;
		path = nullptr;
		invalid ();
	}
}

//-----------------------------------------------------------------------------
void CGradientView::setFrameWidth (CCoord width)
{
	if (width != frameWidth)
	{
		frameWidth = width;
		path = nullptr;
		invalid ();
	}
}

//-----------------------------------------------------------------------------
void CGradientView::setDrawAntialiased (bool state)
{
	if (state != drawAntialiased)
	{
		drawAntialiased = state;
		invalid ();
	}
}

//-----------------------------------------------------------------------------
void CGradientView::setRadialCenter (const CPoint& center)
{
	if (radialCenter != center)
	{
		radialCenter = center;
		invalid ();
	}
}

//-----------------------------------------------------------------------------
void CGradientView::setRadialRadius (CCoord radius)
{
	if (radialRadius != radius)
	{
		radialRadius = radius;
		invalid ();
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
	if (path == nullptr)
	{
		CRect r = getViewSize ();
		r.inset (frameWidth / 2., frameWidth / 2.);
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
		
		if (frameColor.alpha != 0 && frameWidth > 0.)
		{
			context->setDrawMode (drawAntialiased ? kAntiAliasing : kAliasing);
			context->setFrameColor (frameColor);
			context->setLineWidth (frameWidth);
			context->setLineStyle (kLineSolid);
			context->drawGraphicsPath (path, CDrawContext::kPathStroked);
		}
	}
}

} // namespace
