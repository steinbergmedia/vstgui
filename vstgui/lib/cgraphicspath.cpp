// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cdrawcontext.h"
#include "cgradient.h"
#include "cgraphicspath.h"
#include "cgraphicstransform.h"
#include "platform/iplatformgraphicspath.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
void CGraphicsPath::addRoundRect (const CRect& size, CCoord radius)
{
	if (radius <= 0.)
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

	beginSubpath (CPoint (right - radius, top));
	addArc (CRect (right - 2.0 * radius, top, right, top + 2.0 * radius), 270., 360., true);
	addArc (CRect (right - 2.0 * radius, bottom - 2.0 * radius, right, bottom), 0., 90., true);
	addArc (CRect (left, bottom - 2.0 * radius, left + 2.0 * radius, bottom), 90., 180., true);
	addArc (CRect (left, top, left + 2.0 * radius, top + 2.0 * radius), 180., 270., true);
	closeSubpath ();
}

//-----------------------------------------------------------------------------
void CGraphicsPath::addPath (const CGraphicsPath& inPath, CGraphicsTransform* transformation)
{
	for (auto e : inPath.elements)
	{
		if (transformation)
		{
			switch (e.type)
			{
				case Element::kArc:
				{
					transformation->transform (
						e.instruction.arc.rect.left, e.instruction.arc.rect.right,
						e.instruction.arc.rect.top, e.instruction.arc.rect.bottom);
					break;
				}
				case Element::kEllipse:
				case Element::kRect:
				{
					transformation->transform (e.instruction.rect.left, e.instruction.rect.right,
											   e.instruction.rect.top, e.instruction.rect.bottom);
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
					transformation->transform (e.instruction.curve.control1.x,
											   e.instruction.curve.control1.y);
					transformation->transform (e.instruction.curve.control2.x,
											   e.instruction.curve.control2.y);
					transformation->transform (e.instruction.curve.end.x,
											   e.instruction.curve.end.y);
					break;
				}
				case Element::kCloseSubpath:
				{
					break;
				}
			}
		}
		elements.emplace_back (e);
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
	elements.emplace_back (e);
	dirty ();
}

//-----------------------------------------------------------------------------
void CGraphicsPath::addEllipse (const CRect& rect)
{
	Element e;
	e.type = Element::kEllipse;
	CRect2Rect (rect, e.instruction.rect);
	elements.emplace_back (e);
	dirty ();
}

//-----------------------------------------------------------------------------
void CGraphicsPath::addRect (const CRect& rect)
{
	Element e;
	e.type = Element::kRect;
	CRect2Rect (rect, e.instruction.rect);
	elements.emplace_back (e);
	dirty ();
}

//-----------------------------------------------------------------------------
void CGraphicsPath::addLine (const CPoint& to)
{
	Element e;
	e.type = Element::kLine;
	CPoint2Point (to, e.instruction.point);
	elements.emplace_back (e);
	dirty ();
}

//-----------------------------------------------------------------------------
void CGraphicsPath::addBezierCurve (const CPoint& control1, const CPoint& control2,
									const CPoint& end)
{
	Element e;
	e.type = Element::kBezierCurve;
	CPoint2Point (control1, e.instruction.curve.control1);
	CPoint2Point (control2, e.instruction.curve.control2);
	CPoint2Point (end, e.instruction.curve.end);
	elements.emplace_back (e);
	dirty ();
}

//-----------------------------------------------------------------------------
void CGraphicsPath::beginSubpath (const CPoint& start)
{
	Element e;
	e.type = Element::kBeginSubpath;
	CPoint2Point (start, e.instruction.point);
	elements.emplace_back (e);
	dirty ();
}

//-----------------------------------------------------------------------------
void CGraphicsPath::closeSubpath ()
{
	Element e;
	e.type = Element::kCloseSubpath;
	elements.emplace_back (e);
	dirty ();
}

//-----------------------------------------------------------------------------
CGraphicsPath::CGraphicsPath (const PlatformGraphicsPathFactoryPtr& factory,
							  PlatformGraphicsPathPtr&& path)
: factory (factory), path (std::move (path))
{
}

//-----------------------------------------------------------------------------
CGraphicsPath::~CGraphicsPath () noexcept {}

//-----------------------------------------------------------------------------
CGradient* CGraphicsPath::createGradient (double color1Start, double color2Start,
										  const CColor& color1, const CColor& color2)
{
	return CGradient::create (color1Start, color2Start, color1, color2);
}

//-----------------------------------------------------------------------------
void CGraphicsPath::makePlatformGraphicsPath (PlatformGraphicsPathFillMode fillMode)
{
	if (!factory)
		return;
	path = factory->createPath (fillMode);
	if (!path)
		return;
	for (const auto& e : elements)
	{
		switch (e.type)
		{
			case Element::kArc:
			{
				path->addArc (rect2CRect (e.instruction.arc.rect), e.instruction.arc.startAngle,
							  e.instruction.arc.endAngle, e.instruction.arc.clockwise);
				break;
			}
			case Element::kEllipse:
			{
				path->addEllipse (rect2CRect (e.instruction.rect));
				break;
			}
			case Element::kRect:
			{
				path->addRect (rect2CRect (e.instruction.rect));
				break;
			}
			case Element::kLine:
			{
				path->addLine (point2CPoint (e.instruction.point));
				break;
			}
			case Element::kBezierCurve:
			{
				path->addBezierCurve (point2CPoint (e.instruction.curve.control1),
									  point2CPoint (e.instruction.curve.control2),
									  point2CPoint (e.instruction.curve.end));
				break;
			}
			case Element::kBeginSubpath:
			{
				path->beginSubpath (point2CPoint (e.instruction.point));
				break;
			}
			case Element::kCloseSubpath:
			{
				path->closeSubpath ();
				break;
			}
		}
	}
	path->finishBuilding ();
}

//-----------------------------------------------------------------------------
bool CGraphicsPath::ensurePlatformGraphicsPathValid (PlatformGraphicsPathFillMode fillMode)
{
	if (path == nullptr || (path->getFillMode () != PlatformGraphicsPathFillMode::Ignored &&
							path->getFillMode () != fillMode))
	{
		makePlatformGraphicsPath (fillMode);
	}
	return path != nullptr;
}

//-----------------------------------------------------------------------------
void CGraphicsPath::dirty ()
{
	path = nullptr;
}

//-----------------------------------------------------------------------------
bool CGraphicsPath::hitTest (const CPoint& p, bool evenOddFilled, CGraphicsTransform* transform)
{
	ensurePlatformGraphicsPathValid (evenOddFilled ? PlatformGraphicsPathFillMode::Alternate
												   : PlatformGraphicsPathFillMode::Winding);
	return path ? path->hitTest (p, evenOddFilled, transform) : false;
}

//-----------------------------------------------------------------------------
CPoint CGraphicsPath::getCurrentPosition ()
{
	CPoint res;
	if (!elements.empty ())
	{
		const auto& e = elements.back ();
		switch (e.type)
		{
			case Element::kBeginSubpath:
			{
				res = point2CPoint (e.instruction.point);
				break;
			}
			case Element::kCloseSubpath:
			{
				// TODO: find opening point
				break;
			}
			case Element::kArc:
			{
				// TODO: calculate end point
				break;
			}
			case Element::kEllipse:
			{
				res = {e.instruction.rect.left +
						   (e.instruction.rect.right - e.instruction.rect.left) / 2.,
					   e.instruction.rect.bottom};
				break;
			}
			case Element::kRect:
			{
				res = rect2CRect (e.instruction.rect).getTopLeft ();
				break;
			}
			case Element::kLine:
			{
				res = point2CPoint (e.instruction.point);
				break;
			}
			case Element::kBezierCurve:
			{
				res = point2CPoint (e.instruction.curve.end);
				break;
			}
		}
	}
	return res;
}

//-----------------------------------------------------------------------------
CRect CGraphicsPath::getBoundingBox ()
{
	ensurePlatformGraphicsPathValid (path ? path->getFillMode ()
										  : PlatformGraphicsPathFillMode::Winding);
	return path ? path->getBoundingBox () : CRect ();
}

//-----------------------------------------------------------------------------
const PlatformGraphicsPathPtr&
	CGraphicsPath::getPlatformPath (PlatformGraphicsPathFillMode fillMode)
{
	ensurePlatformGraphicsPathValid (fillMode);
	return path;
}

} // VSTGUI
