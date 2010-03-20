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
, dirty (true)
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
void D2DGraphicsPath::addArc (const CRect& rect, double startAngle, double endAngle)
{
	Instruction in;
	in.type = Instruction::kArc;
	in.rect = rect;
	in.arc.startAngle = startAngle;
	in.arc.endAngle = endAngle;
	instructions.push_back (in);
	dirty = true;
}

//-----------------------------------------------------------------------------
void D2DGraphicsPath::addCurve (const CPoint& start, const CPoint& control1, const CPoint& control2, const CPoint& end)
{
	Instruction in;
	in.type = Instruction::kCurve;
	in.rect.setTopLeft (start);
	in.rect.setBottomRight (end);
	in.curve.control1 = control1;
	in.curve.control2 = control2;
	instructions.push_back (in);
	currentPosition = end;
	dirty = true;
}

//-----------------------------------------------------------------------------
void D2DGraphicsPath::addEllipse (const CRect& rect)
{
	Instruction in;
	in.type = Instruction::kEllipse;
	in.rect = rect;
	instructions.push_back (in);
	currentPosition = rect.getTopLeft ();
	currentPosition.x += rect.getWidth () / 2.;
	dirty = true;
}

//-----------------------------------------------------------------------------
void D2DGraphicsPath::addLine (const CPoint& start, const CPoint& end)
{
	Instruction in;
	in.type = Instruction::kLine;
	in.rect.setTopLeft (start);
	in.rect.setBottomRight (end);
	instructions.push_back (in);
	currentPosition = end;
	dirty = true;
}

//-----------------------------------------------------------------------------
void D2DGraphicsPath::addRect (const CRect& rect)
{
	Instruction in;
	in.type = Instruction::kRect;
	in.rect = rect;
	instructions.push_back (in);
	currentPosition = rect.getTopLeft ();
	dirty = true;
}

//-----------------------------------------------------------------------------
void D2DGraphicsPath::addPath (const CGraphicsPath& path, CGraphicsTransform* t)
{
	const D2DGraphicsPath* d2dGraphicsPath = dynamic_cast<const D2DGraphicsPath*> (&path);
	if (d2dGraphicsPath)
	{
		if (t)
		{
			std::list<Instruction>::const_iterator it = d2dGraphicsPath->instructions.begin ();
			while (it != d2dGraphicsPath->instructions.end ())
			{
				Instruction in = (*it);
				t->transform (in.curve.control1);
				t->transform (in.curve.control2);
				t->transform (in.rect);
				instructions.push_back (in);
				it++;
			}
		}
		else
		{
			instructions.insert (instructions.end (), d2dGraphicsPath->instructions.begin (), d2dGraphicsPath->instructions.end ());
		}
		dirty = true;
	}
}

//-----------------------------------------------------------------------------
void D2DGraphicsPath::addString (const char* utf8String, CFontRef font, const CPoint& position)
{
	// TODO: addString
//	dirty = true;
}

//-----------------------------------------------------------------------------
void D2DGraphicsPath::closeSubpath ()
{
	Instruction in;
	in.type = Instruction::kCloseSubpath;
	instructions.push_back (in);
	dirty = true;
}

//-----------------------------------------------------------------------------
void D2DGraphicsPath::draw (CDrawContext* context, PathDrawMode mode, CGraphicsTransform* t)
{
	D2DDrawContext* d2dContext = dynamic_cast<D2DDrawContext*> (context);
	if (!d2dContext)
		return;
	if (!buildPath (mode == kFilledEvenOdd ? D2D1_FILL_MODE_ALTERNATE : D2D1_FILL_MODE_WINDING))
		return;
	D2DDrawContext::D2DApplyClip ac (d2dContext);

	ID2D1Geometry* geometry = 0;
	if (t)
	{
		ID2D1TransformedGeometry* tg = 0;
		D2D1_MATRIX_3X2_F matrix;
		matrix._11 = (FLOAT)t->m11;
		matrix._12 = (FLOAT)t->m12;
		matrix._21 = (FLOAT)t->m21;
		matrix._22 = (FLOAT)t->m22;
		matrix._31 = (FLOAT)t->dx;
		matrix._32 = (FLOAT)t->dy;
		getD2DFactory ()->CreateTransformedGeometry (path, matrix, &tg);
		geometry = tg;
	}
	else
	{
		geometry = path;
		geometry->AddRef ();
	}

	d2dContext->getRenderTarget ()->SetTransform (D2D1::Matrix3x2F::Translation ((FLOAT)context->getOffset ().x, (FLOAT)context->getOffset ().y));

	if (mode == kFilled || mode == kFilledEvenOdd)
		d2dContext->getRenderTarget ()->FillGeometry (geometry, d2dContext->getFillBrush ());
	else if (mode == kStroked)
		d2dContext->getRenderTarget ()->DrawGeometry (geometry, d2dContext->getStrokeBrush (), (FLOAT)d2dContext->getLineWidth (), d2dContext->getStrokeStyle ());

	d2dContext->getRenderTarget ()->SetTransform (D2D1::Matrix3x2F::Identity ());

	geometry->Release ();
}

//-----------------------------------------------------------------------------
void D2DGraphicsPath::fillLinearGradient (CDrawContext* context, const CGradient& gradient, const CPoint& startPoint, const CPoint& endPoint, bool evenOdd, CGraphicsTransform* t)
{
	D2DDrawContext* d2dContext = dynamic_cast<D2DDrawContext*> (context);
	if (!d2dContext)
		return;
	if (!buildPath (evenOdd ? D2D1_FILL_MODE_ALTERNATE : D2D1_FILL_MODE_WINDING))
		return;
	D2DDrawContext::D2DApplyClip ac (d2dContext);

	ID2D1Geometry* geometry = 0;
	if (t)
	{
		ID2D1TransformedGeometry* tg = 0;
		D2D1_MATRIX_3X2_F matrix;
		matrix._11 = (FLOAT)t->m11;
		matrix._12 = (FLOAT)t->m12;
		matrix._21 = (FLOAT)t->m21;
		matrix._22 = (FLOAT)t->m22;
		matrix._31 = (FLOAT)t->dx;
		matrix._32 = (FLOAT)t->dy;
		getD2DFactory ()->CreateTransformedGeometry (path, matrix, &tg);
		geometry = tg;
	}
	else
	{
		geometry = path;
		geometry->AddRef ();
	}

	ID2D1GradientStopCollection* collection = 0;
	D2D1_GRADIENT_STOP gradientStops[2];
	gradientStops[0].position = (FLOAT)gradient.getColor1Start ();
	gradientStops[1].position = (FLOAT)gradient.getColor2Start ();
	gradientStops[0].color = D2D1::ColorF (gradient.getColor1 ().red/255.f, gradient.getColor1 ().green/255.f, gradient.getColor1 ().blue/255.f, gradient.getColor1 ().alpha/255.f);
	gradientStops[1].color = D2D1::ColorF (gradient.getColor2 ().red/255.f, gradient.getColor2 ().green/255.f, gradient.getColor2 ().blue/255.f, gradient.getColor2 ().alpha/255.f);

	if (SUCCEEDED (d2dContext->getRenderTarget ()->CreateGradientStopCollection (gradientStops, 2, &collection)))
	{
		ID2D1LinearGradientBrush* brush = 0;
		D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES properties;
		properties.startPoint = makeD2DPoint (startPoint);
		properties.endPoint = makeD2DPoint (endPoint);
		if (SUCCEEDED (d2dContext->getRenderTarget ()->CreateLinearGradientBrush (properties, collection, &brush)))
		{
			d2dContext->getRenderTarget ()->SetTransform (D2D1::Matrix3x2F::Translation ((FLOAT)context->getOffset ().x, (FLOAT)context->getOffset ().y));
			d2dContext->getRenderTarget ()->FillGeometry (geometry, brush);
			d2dContext->getRenderTarget ()->SetTransform (D2D1::Matrix3x2F::Identity ());
			brush->Release ();
		}
		collection->Release ();
	}

	geometry->Release ();
}

//-----------------------------------------------------------------------------
CPoint D2DGraphicsPath::getCurrentPosition () const
{
	return currentPosition;
}

//-----------------------------------------------------------------------------
CRect D2DGraphicsPath::getBoundingBox () const
{
	CRect r;
	if (const_cast<D2DGraphicsPath*> (this)->buildPath ())
	{
		D2D1_RECT_F bounds;
		if (SUCCEEDED (path->GetBounds (0, &bounds)))
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
bool D2DGraphicsPath::buildPath (long fillMode)
{
	if (path)
	{
		if (!dirty && fillMode == currentPathFillMode)
			return true;
		path->Release ();
		path = 0;
		currentPathFillMode = -1;
	}
	if (!SUCCEEDED (getD2DFactory ()->CreatePathGeometry (&path)))
		return false;

	ID2D1GeometrySink* sink = 0;
	if (!SUCCEEDED (path->Open (&sink)))
		return false;

	sink->SetFillMode ((D2D1_FILL_MODE)fillMode);
	currentPathFillMode = fillMode;

	bool result = true;
	bool figureOpen = false;
	CPoint pos;

	std::list<Instruction>::const_iterator it = instructions.begin ();
	while (it != instructions.end () && result)
	{
		switch ((*it).type)
		{
			case Instruction::kLine:
			{
				CPoint start = (*it).rect.getTopLeft ();
				CPoint end = (*it).rect.getBottomRight ();
				if (start != pos && figureOpen)
				{
					sink->EndFigure (D2D1_FIGURE_END_OPEN);
					figureOpen = false;
				}
				if (!figureOpen)
				{
					sink->BeginFigure (makeD2DPoint (start), D2D1_FIGURE_BEGIN_FILLED);
					figureOpen = true;
				}
				sink->AddLine (makeD2DPoint (end));
				pos = end;
				break;
			}
			case Instruction::kRect:
			{
				const CRect& r = (*it).rect;
				if (r.getTopLeft () != pos && figureOpen)
				{
					sink->EndFigure (D2D1_FIGURE_END_OPEN);
					figureOpen = false;
				}
				if (!figureOpen)
				{
					sink->BeginFigure (makeD2DPoint (r.getTopLeft ()), D2D1_FIGURE_BEGIN_FILLED);
					figureOpen = true;
				}
				sink->AddLine (makeD2DPoint (r.getTopRight ()));
				sink->AddLine (makeD2DPoint (r.getBottomRight ()));
				sink->AddLine (makeD2DPoint (r.getBottomLeft ()));
				sink->AddLine (makeD2DPoint (r.getTopLeft ()));
				pos = r.getTopLeft ();
				break;
			}
			case Instruction::kEllipse:
			{
				const CRect& r = (*it).rect;
				CPoint top (r.getTopLeft ());
				top.x += r.getWidth () / 2.;
				CPoint bottom (r.getBottomLeft ());
				bottom.x += r.getWidth () / 2.;
				if (top != pos && figureOpen)
				{
					sink->EndFigure (D2D1_FIGURE_END_OPEN);
					figureOpen = false;
				}
				if (!figureOpen)
				{
					sink->BeginFigure (makeD2DPoint (top), D2D1_FIGURE_BEGIN_FILLED);
					figureOpen = true;
				}
				D2D1_ARC_SEGMENT arc = D2D1::ArcSegment (makeD2DPoint (bottom), D2D1::SizeF ((FLOAT)r.getWidth ()/2.f, (FLOAT)r.getHeight ()/2.f), 180.f, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE_SMALL);
				sink->AddArc (arc);
				arc.point = makeD2DPoint (top);
				sink->AddArc (arc);
				pos = top;
				break;
			}
			case Instruction::kCurve:
			{
				CPoint start = (*it).rect.getTopLeft ();
				CPoint end = (*it).rect.getBottomRight ();
				if (start != pos && figureOpen)
				{
					sink->EndFigure (D2D1_FIGURE_END_OPEN);
					figureOpen = false;
				}
				if (!figureOpen)
				{
					sink->BeginFigure (makeD2DPoint (start), D2D1_FIGURE_BEGIN_FILLED);
					figureOpen = true;
				}
				D2D1_BEZIER_SEGMENT bezier = D2D1::BezierSegment (makeD2DPoint ((*it).curve.control1), makeD2DPoint ((*it).curve.control2), makeD2DPoint (end));
				sink->AddBezier (bezier);
				pos = end;
				break;
			}
			case Instruction::kArc:
			{
//				D2D1_ARC_SEGMENT arc;
//				sink->AddArc (arc);
				// TODO: AddArc
				break;
			}
			case Instruction::kCloseSubpath:
			{
				if (figureOpen)
				{
					sink->EndFigure (D2D1_FIGURE_END_CLOSED);
					figureOpen = false;
				}
				break;
			}
		}
		it++;
	}
	if (figureOpen)
		sink->EndFigure (D2D1_FIGURE_END_OPEN);
	sink->Close ();
	sink->Release ();

	return result;
}

} // namespace

#endif // WINDOWS
