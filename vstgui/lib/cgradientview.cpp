//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins :
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

#include "cgradientview.h"
#include "cdrawcontext.h"
#include "cgraphicspath.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
CGradientView::CGradientView (const CRect& size)
: CView (size)
, gradientStyle (kLinearGradient)
, frameColor (kBlackCColor)
, gradientAngle (0.)
, roundRectRadius (5.)
, frameWidth (1.)
, drawAntialiased (true)
, radialRadius (1.)
, radialCenter (CPoint (0.5, 0.5))
{
}

//-----------------------------------------------------------------------------
CGradientView::~CGradientView ()
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
		path = 0;
		invalid ();
	}
}

//-----------------------------------------------------------------------------
void CGradientView::setFrameWidth (CCoord width)
{
	if (width != frameWidth)
	{
		frameWidth = width;
		path = 0;
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
		path = 0;
	}
}

//-----------------------------------------------------------------------------
void CGradientView::draw (CDrawContext* context)
{
	if (path == 0)
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
