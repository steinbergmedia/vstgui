//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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

#include "quartzgraphicspath.h"

#if MAC

#include "cgdrawcontext.h"
#include "cfontmac.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
CGAffineTransform QuartzGraphicsPath::createCGAfflineTransform (const CGraphicsTransform& t)
{
	CGAffineTransform transform;
	transform.a = t.m11;
	transform.b = t.m12;
	transform.c = t.m21;
	transform.d = t.m22;
	transform.tx = t.dx;
	transform.ty = t.dy;
	return transform;
}

//-----------------------------------------------------------------------------
QuartzGraphicsPath::QuartzGraphicsPath ()
: path (0)
{
}

//-----------------------------------------------------------------------------
QuartzGraphicsPath::~QuartzGraphicsPath ()
{
	dirty ();
}

//-----------------------------------------------------------------------------
CGradient* QuartzGraphicsPath::createGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2)
{
	return new QuartzGradient (color1Start, color2Start, color1, color2);
}

//-----------------------------------------------------------------------------
CGPathRef QuartzGraphicsPath::getCGPathRef ()
{
	if (path == 0)
	{
		path = CGPathCreateMutable ();
		for (std::list<Element>::const_iterator it = elements.begin (); it != elements.end (); it++)
		{
			Element e = (*it);
			switch (e.type)
			{
				case Element::kArc:
				{
					CCoord radiusX = (e.instruction.arc.rect.right - e.instruction.arc.rect.left) / 2.;
					CCoord radiusY = (e.instruction.arc.rect.bottom - e.instruction.arc.rect.top) / 2.;
					
					CGFloat centerX = e.instruction.arc.rect.left + radiusX;
					CGFloat centerY = e.instruction.arc.rect.top + radiusY;

					CGAffineTransform transform = CGAffineTransformMakeTranslation (centerX, centerY);
					transform = CGAffineTransformScale (transform, radiusX, radiusY);
					
					if (CGPathIsEmpty (path))
						CGPathMoveToPoint (path, &transform, cos (radians (e.instruction.arc.startAngle)), sin (radians (e.instruction.arc.startAngle)));

					CGPathAddArc (path, &transform, 0, 0, 1, radians (e.instruction.arc.startAngle), radians (e.instruction.arc.endAngle), !e.instruction.arc.clockwise);
					break;
				}
				case Element::kEllipse:
				{
					CCoord width = e.instruction.rect.right - e.instruction.rect.left;
					CCoord height = e.instruction.rect.bottom - e.instruction.rect.top;
					CGPathAddEllipseInRect (path, 0, CGRectMake (e.instruction.rect.left, e.instruction.rect.top, width, height));
					break;
				}
				case Element::kRect:
				{
					CCoord width = e.instruction.rect.right - e.instruction.rect.left;
					CCoord height = e.instruction.rect.bottom - e.instruction.rect.top;
					CGPathAddRect (path, 0, CGRectMake (e.instruction.rect.left, e.instruction.rect.top, width, height));
					break;
				}
				case Element::kLine:
				{
					CGPathAddLineToPoint (path, 0, e.instruction.point.x, e.instruction.point.y);
					break;
				}
				case Element::kBezierCurve:
				{
					CGPathAddCurveToPoint (path, 0, e.instruction.curve.control1.x, e.instruction.curve.control1.y, e.instruction.curve.control2.x, e.instruction.curve.control2.y, e.instruction.curve.end.x, e.instruction.curve.end.y);
					break;
				}
				case Element::kBeginSubpath:
				{
					CGPathMoveToPoint (path, 0, e.instruction.point.x, e.instruction.point.y);
					break;
				}
				case Element::kCloseSubpath:
				{
					CGPathCloseSubpath (path);
					break;
				}
			}
		}
	}
	return path;
}

//-----------------------------------------------------------------------------
void QuartzGraphicsPath::dirty ()
{
	if (path)
	{
		CFRelease (path);
		path = 0;
	}
}

//-----------------------------------------------------------------------------
bool QuartzGraphicsPath::hitTest (const CPoint& p, bool evenOddFilled, CGraphicsTransform* transform)
{
	CGPathRef cgPath = getCGPathRef ();
	if (cgPath)
	{
		CGPoint cgPoint = CGPointMake (p.x, p.y);
		CGAffineTransform cgTransform;
		if (transform)
			cgTransform = createCGAfflineTransform (*transform);
		return CGPathContainsPoint (cgPath, transform ? &cgTransform : 0, cgPoint, evenOddFilled);
	}
	return false;
}

//-----------------------------------------------------------------------------
CPoint QuartzGraphicsPath::getCurrentPosition ()
{
	CPoint p (0, 0);

	CGPathRef cgPath = getCGPathRef ();
	if (cgPath && !CGPathIsEmpty (cgPath))
	{
		CGPoint cgPoint = CGPathGetCurrentPoint (cgPath);
		p.x = cgPoint.x;
		p.y = cgPoint.y;
	}

	return p;
}

//-----------------------------------------------------------------------------
CRect QuartzGraphicsPath::getBoundingBox ()
{
	CRect r;

	CGPathRef cgPath = getCGPathRef ();
	if (cgPath)
	{
		CGRect cgRect = CGPathGetBoundingBox (cgPath);
		r.left = cgRect.origin.x;
		r.top = cgRect.origin.y;
		r.setWidth (cgRect.size.width);
		r.setHeight (cgRect.size.height);
	}
	return r;
}

} // namespace


#endif
