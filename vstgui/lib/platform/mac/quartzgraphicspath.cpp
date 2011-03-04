//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
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
: path (CGPathCreateMutable ())
{
}

//-----------------------------------------------------------------------------
QuartzGraphicsPath::~QuartzGraphicsPath ()
{
	CFRelease (path);
}

//-----------------------------------------------------------------------------
CGradient* QuartzGraphicsPath::createGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2)
{
#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_4
	return new QuartzGradient (color1Start, color2Start, color1, color2);
#else
	return 0;
#endif
}

//-----------------------------------------------------------------------------
void QuartzGraphicsPath::closeSubpath ()
{
	CGPathCloseSubpath (path);
}

//-----------------------------------------------------------------------------
void QuartzGraphicsPath::addArc (const CRect& rect, double startAngle, double endAngle, bool clockwise)
{
	CGFloat centerX = rect.left + rect.getWidth () / 2.;
	CGFloat centerY = rect.top + rect.getHeight () / 2.;

	CGAffineTransform transform = CGAffineTransformMakeTranslation (centerX, centerY);
	transform = CGAffineTransformScale (transform, rect.getWidth () / 2, rect.getHeight () / 2);
	
	if (CGPathIsEmpty (path))
		CGPathMoveToPoint (path, &transform, cos (radians (startAngle)), sin (radians (startAngle)));

	CGPathAddArc (path, &transform, 0, 0, 1, radians (startAngle), radians (endAngle), clockwise);
}

//-----------------------------------------------------------------------------
void QuartzGraphicsPath::addCurve (const CPoint& start, const CPoint& control1, const CPoint& control2, const CPoint& end)
{
	if (CGPathIsEmpty (path) || getCurrentPosition () != start)
		CGPathMoveToPoint (path, 0, start.x, start.y);
	CGPathAddCurveToPoint (path, 0, control1.x, control1.y, control2.x, control2.y, end.x, end.y);
}

//-----------------------------------------------------------------------------
void QuartzGraphicsPath::addEllipse (const CRect& rect)
{
	CGPathAddEllipseInRect (path, 0, CGRectMake (rect.left, rect.top, rect.getWidth (), rect.getHeight ()));
}

//-----------------------------------------------------------------------------
void QuartzGraphicsPath::addLine (const CPoint& start, const CPoint& end)
{
	if (CGPathIsEmpty (path) || getCurrentPosition () != start)
		CGPathMoveToPoint (path, 0, start.x, start.y);
	CGPathAddLineToPoint (path, 0, end.x, end.y);
}

//-----------------------------------------------------------------------------
void QuartzGraphicsPath::addRect (const CRect& rect)
{
	CGRect r = CGRectMake (rect.left, rect.top, rect.getWidth (), rect.getHeight ());
	CGPathAddRect (path, 0, r);
}

//-----------------------------------------------------------------------------
void QuartzGraphicsPath::addPath (const CGraphicsPath& inPath, CGraphicsTransform* t)
{
	if (t)
	{
		CGAffineTransform transform = createCGAfflineTransform (*t);
		if (&inPath == this)
		{
			CGPathRef pathCopy = CGPathCreateCopy (path);
			CGPathAddPath (path, &transform, pathCopy);
			CFRelease (pathCopy);
		}
		else
		{
			const QuartzGraphicsPath* qp = dynamic_cast<const QuartzGraphicsPath*> (&inPath);
			if (qp)
				CGPathAddPath (path, &transform, qp->getCGPathRef ());
		}
	}
	else
	{
		if (&inPath == this)
		{
			CGPathRef pathCopy = CGPathCreateCopy (path);
			CGPathAddPath (path, 0, pathCopy);
			CFRelease (pathCopy);
		}
		else
		{
			const QuartzGraphicsPath* qp = dynamic_cast<const QuartzGraphicsPath*> (&inPath);
			if (qp)
				CGPathAddPath (path, 0, qp->getCGPathRef ());
		}
	}
}

//-----------------------------------------------------------------------------
CPoint QuartzGraphicsPath::getCurrentPosition () const
{
	CPoint p (0, 0);

	if (!CGPathIsEmpty (path))
	{
		CGPoint cgPoint = CGPathGetCurrentPoint (path);
		p.x = cgPoint.x;
		p.y = cgPoint.y;
	}

	return p;
}

//-----------------------------------------------------------------------------
CRect QuartzGraphicsPath::getBoundingBox () const
{
	CRect r;

	CGRect cgRect = CGPathGetBoundingBox (path);
	r.left = cgRect.origin.x;
	r.top = cgRect.origin.y;
	r.setWidth (cgRect.size.width);
	r.setHeight (cgRect.size.height);
	
	return r;
}

} // namespace


#endif
