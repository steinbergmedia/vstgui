//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
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

#include "cgraphicspath.h"
#include "cdrawcontext.h"
#include "cgraphicstransform.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
void CGraphicsPath::addRoundRect (const CRect& size, CCoord radius)
{
	if (radius == 0.)
	{
		addRect (size);
		return;
	}
	CRect rect2 (size);
	rect2.normalize ();
	const CCoord left = rect2.left;
	const CCoord right = rect2.right;
	const CCoord top = rect2.top;
	const CCoord bottom = rect2.bottom;

	beginSubpath (CPoint (right-radius, top));
	addArc (CRect (right - 2.0 * radius, top, right, top + 2.0 * radius), 270., 360., true);
	addArc (CRect (right - 2.0 * radius, bottom - 2.0 *radius, right, bottom), 0., 90., true);
	addArc (CRect (left, bottom - 2.0 * radius, left + 2.0 * radius, bottom), 90., 180., true);
	addArc (CRect (left, top, left + 2.0 * radius, top + 2.0 * radius), 180., 270., true);
	closeSubpath ();
}

//-----------------------------------------------------------------------------
void CGraphicsPath::addPath (const CGraphicsPath& path, CGraphicsTransform* transformation)
{
	for (ElementList::const_iterator it = path.elements.begin (); it != path.elements.end (); it++)
	{
		Element e = (*it);
		if (transformation)
		{
			switch (e.type)
			{
				case Element::kArc:
				{
					transformation->transform (e.instruction.arc.rect.left, e.instruction.arc.rect.right, e.instruction.arc.rect.top, e.instruction.arc.rect.bottom);
					break;
				}
				case Element::kEllipse:
				case Element::kRect:
				{
					transformation->transform (e.instruction.rect.left, e.instruction.rect.right, e.instruction.rect.top, e.instruction.rect.bottom);
					break;
				}
				case Element::kBeginSubpath:
				case Element::kLine:
				{
					transformation->transform (e.instruction.point.x, e.instruction.point.y);
					break;
				}
				case Element::kBezierCurve:
				{
					transformation->transform (e.instruction.curve.control1.x, e.instruction.curve.control1.y);
					transformation->transform (e.instruction.curve.control2.x, e.instruction.curve.control2.y);
					transformation->transform (e.instruction.curve.end.x, e.instruction.curve.end.y);
					break;
				}
				case Element::kCloseSubpath:
				{
					break;
				}
			}
		}
		elements.push_back (e);
	}
	dirty ();
}

//-----------------------------------------------------------------------------
void CGraphicsPath::addArc (const CRect& rect, double startAngle, double endAngle, bool clockwise)
{
	Element e;
	e.type = Element::kArc;
	CRect2Rect (rect, e.instruction.arc.rect);
	e.instruction.arc.startAngle = startAngle;
	e.instruction.arc.endAngle = endAngle;
	e.instruction.arc.clockwise = clockwise;
	elements.push_back (e);
	dirty ();
}

//-----------------------------------------------------------------------------
void CGraphicsPath::addEllipse (const CRect& rect)
{
	Element e;
	e.type = Element::kEllipse;
	CRect2Rect (rect, e.instruction.rect);
	elements.push_back (e);
	dirty ();
}

//-----------------------------------------------------------------------------
void CGraphicsPath::addRect (const CRect& rect)
{
	Element e;
	e.type = Element::kRect;
	CRect2Rect (rect, e.instruction.rect);
	elements.push_back (e);
	dirty ();
}

//-----------------------------------------------------------------------------
void CGraphicsPath::addLine (const CPoint& to)
{
	Element e;
	e.type = Element::kLine;
	CPoint2Point (to, e.instruction.point);
	elements.push_back (e);
	dirty ();
}

//-----------------------------------------------------------------------------
void CGraphicsPath::addBezierCurve (const CPoint& control1, const CPoint& control2, const CPoint& end)
{
	Element e;
	e.type = Element::kBezierCurve;
	CPoint2Point (control1, e.instruction.curve.control1);
	CPoint2Point (control2, e.instruction.curve.control2);
	CPoint2Point (end, e.instruction.curve.end);
	elements.push_back (e);
	dirty ();
}

//-----------------------------------------------------------------------------
void CGraphicsPath::beginSubpath (const CPoint& start)
{
	Element e;
	e.type = Element::kBeginSubpath;
	CPoint2Point (start, e.instruction.point);
	elements.push_back (e);
	dirty ();
}

//-----------------------------------------------------------------------------
void CGraphicsPath::closeSubpath ()
{
	Element e;
	e.type = Element::kCloseSubpath;
	elements.push_back (e);
	dirty ();
}

} // namespace
