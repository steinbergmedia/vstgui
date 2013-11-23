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

#include "d2dgraphicspath.h"

#if WINDOWS && VSTGUI_DIRECT2D_SUPPORT

#include "../win32support.h"
#include "d2ddrawcontext.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
class D2DGradient : public CGradient
{
public:
	D2DGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2)
	: CGradient (color1Start, color2Start, color1, color2) {}
};


//-----------------------------------------------------------------------------
D2DGraphicsPath::D2DGraphicsPath ()
: path (0)
, currentPathFillMode (-1)
{
}

//-----------------------------------------------------------------------------
D2DGraphicsPath::~D2DGraphicsPath ()
{
	if (path)
	{
		path->Release ();
		path = 0;
	}
}

// CGraphicsPath
//-----------------------------------------------------------------------------
CGradient* D2DGraphicsPath::createGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2)
{
	return new D2DGradient (color1Start, color2Start, color1, color2);
}

//-----------------------------------------------------------------------------
CPoint D2DGraphicsPath::getCurrentPosition ()
{
	// TODO: D2DGraphicsPath::getCurrentPosition
	CPoint p;
#if DEBUG
	DebugPrint ("D2DGraphicsPath::getCurrentPosition not implemented\n");
#endif
	return p;
}

//-----------------------------------------------------------------------------
bool D2DGraphicsPath::hitTest (const CPoint& p, bool evenOddFilled, CGraphicsTransform* transform)
{
	ID2D1PathGeometry* _path = getPath (evenOddFilled ? D2D1_FILL_MODE_ALTERNATE : D2D1_FILL_MODE_WINDING);
	if (_path)
	{
		D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F::Identity ();
		if (transform)
		{
			matrix._11 = (FLOAT)transform->m11;
			matrix._12 = (FLOAT)transform->m12;
			matrix._21 = (FLOAT)transform->m21;
			matrix._22 = (FLOAT)transform->m22;
			matrix._31 = (FLOAT)transform->dx;
			matrix._32 = (FLOAT)transform->dy;
			
		}
		BOOL result;
		if (SUCCEEDED (_path->FillContainsPoint (makeD2DPoint (p), matrix, &result)))
		{
			return result ? true : false;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
CRect D2DGraphicsPath::getBoundingBox ()
{
	CRect r;
	ID2D1PathGeometry* _path = getPath (currentPathFillMode);
	if (_path)
	{
		D2D1_RECT_F bounds;
		if (SUCCEEDED (_path->GetBounds (0, &bounds)))
		{
			r.left = bounds.left;
			r.top = bounds.top;
			r.right = bounds.right;
			r.bottom = bounds.bottom;
		}
	}
	return r;
}

//-----------------------------------------------------------------------------
void D2DGraphicsPath::dirty ()
{
	if (path)
	{
		path->Release ();
		path = 0;
	}
}

//-----------------------------------------------------------------------------
ID2D1PathGeometry* D2DGraphicsPath::getPath (int32_t fillMode)
{
	if (path == 0 || fillMode != currentPathFillMode)
	{
		dirty ();
		if (!SUCCEEDED (getD2DFactory ()->CreatePathGeometry (&path)))
			return 0;
		if (fillMode == -1)
			fillMode = 0;
		currentPathFillMode = fillMode;
		
		ID2D1GeometrySink* sink = 0;
		if (!SUCCEEDED (path->Open (&sink)))
		{
			path->Release ();
			path = 0;
			return 0;
		}

		sink->SetFillMode ((D2D1_FILL_MODE)fillMode);

		bool figureOpen = false;
		CPoint lastPos;
		for (ElementList::const_iterator it = elements.begin (); it != elements.end (); it++)
		{
			Element e = (*it);
			switch (e.type)
			{
				case Element::kArc:
				{
					bool clockwise = e.instruction.arc.clockwise;
					double startAngle = e.instruction.arc.startAngle;
					double endAngle = e.instruction.arc.endAngle;
					CRect o_r (e.instruction.arc.rect.left, e.instruction.arc.rect.top, e.instruction.arc.rect.right, e.instruction.arc.rect.bottom);
					CRect r (o_r);
					o_r.originize ();
					CPoint center = o_r.getCenter ();
					CPoint start;
					start.x = r.left + center.x + center.x * cos (radians (startAngle));
					start.y = r.top + center.y + center.y * sin (radians (startAngle));
					if (!figureOpen)
					{
						sink->BeginFigure (makeD2DPoint (start), D2D1_FIGURE_BEGIN_FILLED);
						figureOpen = true;
					}
					else if (lastPos != start)
					{
						sink->AddLine (makeD2DPoint (start));
					}

					double sweepangle = endAngle - startAngle;
					if (clockwise) {
						// sweepangle positive
						while (sweepangle < 0.0)
							sweepangle += 360.0;
						while (sweepangle > 360.0)
							sweepangle -= 360.0;
					} else {
						// sweepangle negative
						while (sweepangle > 0.0)
							sweepangle -= 360.0;
						while (sweepangle < -360.0)
							sweepangle += 360.0;
					}

					CPoint endPoint;
					endPoint.x = r.left + center.x + center.x * cos (radians (endAngle));
					endPoint.y = r.top + center.y + center.y * sin (radians (endAngle));

					D2D1_ARC_SEGMENT arc;
					arc.size = makeD2DSize (r.getWidth ()/2., r.getHeight ()/2.);
					arc.rotationAngle = 0;
					arc.sweepDirection = clockwise ? D2D1_SWEEP_DIRECTION_CLOCKWISE : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
					arc.point = makeD2DPoint (endPoint);
					arc.arcSize = fabs(sweepangle) <= 180. ? D2D1_ARC_SIZE_SMALL : D2D1_ARC_SIZE_LARGE;
					sink->AddArc (arc);
					lastPos = endPoint;
					break;
				}
				case Element::kEllipse:
				{
					CRect r (e.instruction.rect.left, e.instruction.rect.top, e.instruction.rect.right, e.instruction.rect.bottom);
					CPoint top (r.getTopLeft ());
					top.x += r.getWidth () / 2.;
					CPoint bottom (r.getBottomLeft ());
					bottom.x += r.getWidth () / 2.;
					if (figureOpen && lastPos != CPoint (e.instruction.rect.left, e.instruction.rect.top))
					{
						sink->EndFigure (D2D1_FIGURE_END_OPEN);
						figureOpen = false;
					}
					if (!figureOpen)
					{
						sink->BeginFigure (makeD2DPoint (top), D2D1_FIGURE_BEGIN_FILLED);
						figureOpen = true;
					}
					D2D1_ARC_SEGMENT arc = D2D1::ArcSegment (makeD2DPoint (bottom), D2D1::SizeF ((FLOAT)r.getWidth ()/2.f, (FLOAT)r.getHeight ()/2.f), 180.f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL);
					sink->AddArc (arc);
					arc.point = makeD2DPoint (top);
					sink->AddArc (arc);
					lastPos = top;
					break;
				}
				case Element::kRect:
				{
					D2D1_POINT_2F points[4] = {
						{(FLOAT)e.instruction.rect.right, (FLOAT)e.instruction.rect.top},
						{(FLOAT)e.instruction.rect.right, (FLOAT)e.instruction.rect.bottom},
						{(FLOAT)e.instruction.rect.left, (FLOAT)e.instruction.rect.bottom},
						{(FLOAT)e.instruction.rect.left, (FLOAT)e.instruction.rect.top}
					};
					if (figureOpen && lastPos != CPoint (e.instruction.rect.left, e.instruction.rect.top))
					{
						sink->EndFigure (D2D1_FIGURE_END_OPEN);
						figureOpen = false;
					}
					if (figureOpen == false)
					{
						sink->BeginFigure (points[3], D2D1_FIGURE_BEGIN_FILLED);
						figureOpen = true;
					}
					sink->AddLine (points[0]);
					sink->AddLine (points[1]);
					sink->AddLine (points[2]);
					sink->AddLine (points[3]);
					lastPos = CPoint (e.instruction.rect.left, e.instruction.rect.top);
					break;
				}
				case Element::kLine:
				{
					if (figureOpen)
					{
						D2D1_POINT_2F end = {(FLOAT)e.instruction.point.x, (FLOAT)e.instruction.point.y};
						sink->AddLine (end);
						lastPos = CPoint (e.instruction.point.x, e.instruction.point.y);
					}
					break;
				}
				case Element::kBezierCurve:
				{
					if (figureOpen)
					{
						D2D1_POINT_2F control1 = {(FLOAT)e.instruction.curve.control1.x, (FLOAT)e.instruction.curve.control1.y};
						D2D1_POINT_2F control2 = {(FLOAT)e.instruction.curve.control2.x, (FLOAT)e.instruction.curve.control2.y};
						D2D1_POINT_2F end = {(FLOAT)e.instruction.curve.end.x, (FLOAT)e.instruction.curve.end.y};
						D2D1_BEZIER_SEGMENT bezier = D2D1::BezierSegment (control1, control2, end);
						sink->AddBezier (bezier);
						lastPos = CPoint (e.instruction.curve.end.x, e.instruction.curve.end.y);
					}
					break;
				}
				case Element::kBeginSubpath:
				{
					if (figureOpen)
						sink->EndFigure (D2D1_FIGURE_END_OPEN);
					D2D1_POINT_2F start = {(FLOAT)e.instruction.point.x, (FLOAT)e.instruction.point.y};
					sink->BeginFigure (start, D2D1_FIGURE_BEGIN_FILLED);
					figureOpen = true;
					lastPos = CPoint (e.instruction.point.x, e.instruction.point.y);
					break;
				}
				case Element::kCloseSubpath:
				{
					if (figureOpen)
					{
						sink->EndFigure (D2D1_FIGURE_END_CLOSED);
						figureOpen = false;
					}
					break;
				}
			}
		}
		if (figureOpen)
			sink->EndFigure (D2D1_FIGURE_END_OPEN);
		HRESULT res = sink->Close ();
		if (!SUCCEEDED (res))
		{
			path->Release ();
			path = 0;
		}
		sink->Release ();
	}
	return path;
}

} // namespace

#endif // WINDOWS
