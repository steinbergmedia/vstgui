// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "d2dgraphicspath.h"

#if WINDOWS

#include "../win32support.h"
#include "../../../cstring.h"
#include "../../../cgradient.h"
#include "d2ddrawcontext.h"
#include "d2dfont.h"
#include <dwrite.h>
#include <winnt.h>

#if defined (__GNUC__) && !defined (__clang__)
#define __maybenull
#define __out
#endif

namespace VSTGUI {

class D2DPathTextRenderer final : public IDWriteTextRenderer
{
public:
	D2DPathTextRenderer (ID2D1GeometrySink* sink) : sink (sink)
	{
	}

	STDMETHOD (DrawGlyphRun) (
		void* clientDrawingContext,
		FLOAT baselineOriginX,
		FLOAT baselineOriginY,
		DWRITE_MEASURING_MODE measuringMode,
		DWRITE_GLYPH_RUN const* glyphRun,
		DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
		IUnknown* clientDrawingEffect
		) override
	{
		return glyphRun->fontFace->GetGlyphRunOutline (
				glyphRun->fontEmSize,
				glyphRun->glyphIndices,
				glyphRun->glyphAdvances,
				glyphRun->glyphOffsets,
				glyphRun->glyphCount,
				glyphRun->isSideways,
				glyphRun->bidiLevel%2,
				sink
			);
	}

	STDMETHOD (DrawUnderline) (
		void* clientDrawingContext,
		FLOAT baselineOriginX,
		FLOAT baselineOriginY,
		DWRITE_UNDERLINE const* underline,
		IUnknown* clientDrawingEffect
		) override
	{
		return S_FALSE;
	}

	STDMETHOD (DrawStrikethrough) (
		void* clientDrawingContext,
		FLOAT baselineOriginX,
		FLOAT baselineOriginY,
		DWRITE_STRIKETHROUGH const* strikethrough,
		IUnknown* clientDrawingEffect
		) override
	{
		return S_FALSE;
	}

	STDMETHOD (DrawInlineObject) (
		void* clientDrawingContext,
		FLOAT originX,
		FLOAT originY,
		IDWriteInlineObject* inlineObject,
		BOOL isSideways,
		BOOL isRightToLeft,
		IUnknown* clientDrawingEffect
		) override
	{
		return S_FALSE;
	}

	STDMETHOD (IsPixelSnappingDisabled) (
		__maybenull void* clientDrawingContext,
		__out BOOL* isDisabled
		) override
	{
		if (isDisabled)
			*isDisabled = FALSE;
		return S_FALSE;
	}

	STDMETHOD (GetCurrentTransform) (
		__maybenull void* clientDrawingContext,
		__out DWRITE_MATRIX* transform
		) override
	{
		const DWRITE_MATRIX identityTransform =
		{
			1, 0,
			0, 1,
			0, 0
		};
		if (transform)
			*transform = identityTransform;
		return S_OK;
	}

	STDMETHOD (GetPixelsPerDip) (
		__maybenull void* clientDrawingContext,
		__out FLOAT* pixelsPerDip
		) override
	{
		if (pixelsPerDip)
			*pixelsPerDip = 96;
		return S_OK;
	}

	STDMETHOD (QueryInterface) (REFIID iid, void ** ppvObject) override
	{
		if (iid == __uuidof(IUnknown)
			|| iid == __uuidof(IDWriteTextRenderer))
		{
			*ppvObject = static_cast<IDWriteTextRenderer*>(this);
			AddRef();
			return S_OK;
		} else
			return E_NOINTERFACE;
	}
    STDMETHOD_ (ULONG, AddRef) () override { return 1; }
    STDMETHOD_ (ULONG, Release) () override { return 1; }

private:
	ID2D1GeometrySink* sink;
};

//-----------------------------------------------------------------------------
class AlignPixelSink final : public ID2D1SimplifiedGeometrySink
{
public:
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject) override
	{
		if (iid == __uuidof(IUnknown)
			|| iid == __uuidof(ID2D1SimplifiedGeometrySink))
		{
			*ppvObject = static_cast<ID2D1SimplifiedGeometrySink*>(this);
			AddRef();
			return S_OK;
		} else
			return E_NOINTERFACE;
	}
    ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
    ULONG STDMETHODCALLTYPE Release() override { return 1; }

	D2D1_POINT_2F alignPoint (const D2D1_POINT_2F& p)
	{
		CPoint point (p.x, p.y);
		if (context->getDrawMode ().antiAliasing ())
			point.offset (-0.5, -0.5);
		if (context)
			context->pixelAllign (point);
		return D2D1::Point2F (static_cast<FLOAT> (point.x), static_cast<FLOAT> (point.y));
	}
	
	STDMETHOD_(void, AddBeziers)(const D2D1_BEZIER_SEGMENT * beziers, UINT beziersCount) override
	{
		for (UINT i = 0; i < beziersCount; ++i)
		{
			D2D1_BEZIER_SEGMENT segment = {};
			segment.point1 = alignPoint (beziers[i].point1);
			segment.point2 = alignPoint (beziers[i].point2);
			segment.point3 = alignPoint (beziers[i].point3);
			sink->AddBezier (segment);
		}
	}

	STDMETHOD_(void, AddLines)(const D2D1_POINT_2F *points, UINT pointsCount) override
	{
		for (UINT i = 0; i < pointsCount; ++i)
		{
			D2D_POINT_2F point = alignPoint (points[i]);
			sink->AddLine (point);
		}
	}

	STDMETHOD_(void, BeginFigure)(D2D1_POINT_2F startPoint,
								D2D1_FIGURE_BEGIN figureBegin) override
	{
		startPoint = alignPoint (startPoint);
		sink->BeginFigure (startPoint, figureBegin);
	}

	STDMETHOD_(void, EndFigure)(D2D1_FIGURE_END figureEnd) override
	{
		sink->EndFigure (figureEnd);
	}

	STDMETHOD_(void, SetFillMode)(D2D1_FILL_MODE fillMode) override
	{
		sink->SetFillMode (fillMode);
	}

	STDMETHOD_(void, SetSegmentFlags)(D2D1_PATH_SEGMENT vertexFlags) override
	{
		sink->SetSegmentFlags (vertexFlags);
	}

	STDMETHOD(Close)() override
	{
		isClosed = true;
		return sink->Close ();
	}

	AlignPixelSink (D2DDrawContext* context)
	: path (nullptr)
	, sink (nullptr)
	, context (context)
	, isClosed (true)
	{
	}

	~AlignPixelSink ()
	{
		if (sink)
			sink->Release ();
		if (path)
			path->Release ();
	}
	
	bool init ()
	{
		getD2DFactory ()->CreatePathGeometry (&path);
		if (path == nullptr)
			return false;
		if (!SUCCEEDED (path->Open (&sink)))
			return false;
		isClosed = false;
		return true;
	}

	ID2D1PathGeometry* get ()
	{
		if (path)
		{
			if (sink)
			{
				if (!isClosed)
					sink->Close ();
				sink->Release ();
				sink = nullptr;
			}
			ID2D1PathGeometry* result = path;
			path = nullptr;
			return result;
		}
		return nullptr;
	}
private:
	ID2D1PathGeometry* path;
	ID2D1GeometrySink* sink;
	D2DDrawContext* context;
	bool isClosed;
};

//-----------------------------------------------------------------------------
D2DGraphicsPath::D2DGraphicsPath ()
: path (nullptr)
, currentPathFillMode (-1)
{
}

//-----------------------------------------------------------------------------
D2DGraphicsPath::D2DGraphicsPath (const D2DFont* font, UTF8StringPtr text)
: path (nullptr)
{
	ID2D1PathGeometry* localPath = nullptr;
	getD2DFactory ()->CreatePathGeometry (&localPath);
	if (localPath == nullptr)
		return;

	IDWriteTextLayout* layout = font->createTextLayout (UTF8String (text).getPlatformString ());
	if (layout == nullptr)
		return;
	
	ID2D1PathGeometry* textPath = nullptr;
	getD2DFactory ()->CreatePathGeometry (&textPath);
	if (textPath == nullptr)
	{
		layout->Release ();
		return;
	}
	
	ID2D1GeometrySink* sink = nullptr;
	if (!SUCCEEDED (textPath->Open (&sink)))
	{
		textPath->Release ();
		layout->Release ();
		return;
	}
	
	D2DPathTextRenderer renderer (sink);
	layout->Draw (nullptr, &renderer, 0, 0);
	
	sink->Close ();
	sink->Release ();
	layout->Release ();
	
	if (!SUCCEEDED (localPath->Open (&sink)))
	{
		textPath->Release ();
		return;
	}
	
	D2D1_RECT_F bounds = {};
	if (SUCCEEDED (textPath->GetBounds (nullptr, &bounds)))
	{
		textPath->Simplify (D2D1_GEOMETRY_SIMPLIFICATION_OPTION_CUBICS_AND_LINES, D2D1::Matrix3x2F::Translation (0, -bounds.top), sink);
	}
	
	textPath->Release ();
	sink->Close ();
	sink->Release ();
	path = localPath;
}

//-----------------------------------------------------------------------------
D2DGraphicsPath::~D2DGraphicsPath ()
{
	if (path)
	{
		path->Release ();
		path = nullptr;
	}
}

// CGraphicsPath
//-----------------------------------------------------------------------------
CGradient* D2DGraphicsPath::createGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2)
{
	return CGradient::create (color1Start, color2Start, color1, color2);
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
	ID2D1Geometry* _path = createPath (evenOddFilled ? D2D1_FILL_MODE_ALTERNATE : D2D1_FILL_MODE_WINDING);
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
		BOOL result = false;
		_path->FillContainsPoint (makeD2DPoint (p), matrix, &result);
		_path->Release ();
		return result ? true : false;
	}
	return false;
}

//-----------------------------------------------------------------------------
CRect D2DGraphicsPath::getBoundingBox ()
{
	CRect r;
	ID2D1Geometry* _path = createPath (currentPathFillMode);
	if (_path)
	{
		D2D1_RECT_F bounds;
		if (SUCCEEDED (_path->GetBounds (nullptr, &bounds)))
		{
			r.left = bounds.left;
			r.top = bounds.top;
			r.right = bounds.right;
			r.bottom = bounds.bottom;
		}
		_path->Release ();
	}
	return r;
}

//-----------------------------------------------------------------------------
void D2DGraphicsPath::dirty ()
{
	if (path)
	{
		path->Release ();
		path = nullptr;
	}
}

//-----------------------------------------------------------------------------
ID2D1Geometry* D2DGraphicsPath::createPath (int32_t fillMode, D2DDrawContext* context, CGraphicsTransform* transform)
{
	if (!elements.empty () && (path == nullptr || fillMode != currentPathFillMode))
	{
		dirty ();
		ID2D1PathGeometry* localPath = nullptr;
		if (!SUCCEEDED (getD2DFactory ()->CreatePathGeometry (&localPath)))
			return nullptr;
		if (fillMode == -1)
			fillMode = 0;
		currentPathFillMode = fillMode;
		
		ID2D1GeometrySink* sink = nullptr;
		if (!SUCCEEDED (localPath->Open (&sink)))
		{
			localPath->Release ();
			return nullptr;
		}

		path = localPath;

		sink->SetFillMode ((D2D1_FILL_MODE)fillMode);

		bool figureOpen = false;
		CPoint lastPos;
		for (const CGraphicsPath::Element& e : elements)
		{
			switch (e.type)
			{
				case Element::kArc:
				{
					bool clockwise = e.instruction.arc.clockwise;
					double startAngle = radians (e.instruction.arc.startAngle);
					double endAngle = radians (e.instruction.arc.endAngle);
					CRect o_r (e.instruction.arc.rect.left, e.instruction.arc.rect.top, e.instruction.arc.rect.right, e.instruction.arc.rect.bottom);
					CRect r (o_r);
					o_r.originize ();
					CPoint center = o_r.getCenter ();
					if (center.x != center.y)
					{
						startAngle = atan2 (sin (startAngle) * center.x, cos (startAngle) * center.y);
						endAngle = atan2 (sin (endAngle) * center.x, cos (endAngle) * center.y);
					}
					CPoint start;
					start.x = r.left + center.x + center.x * cos (startAngle);
					start.y = r.top + center.y + center.y * sin (startAngle);
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
							sweepangle += 2 * M_PI;
						while (sweepangle > 2 * M_PI)
							sweepangle -= 2 * M_PI;
					} else {
						// sweepangle negative
						while (sweepangle > 0.0)
							sweepangle -= 2 * M_PI;
						while (sweepangle < -2 * M_PI)
							sweepangle += 2 * M_PI;
					}

					CPoint endPoint;
					endPoint.x = r.left + center.x + center.x * cos (endAngle);
					endPoint.y = r.top + center.y + center.y * sin (endAngle);

					D2D1_ARC_SEGMENT arc;
					arc.size = makeD2DSize (r.getWidth ()/2., r.getHeight ()/2.);
					arc.rotationAngle = 0;
					arc.sweepDirection = clockwise ? D2D1_SWEEP_DIRECTION_CLOCKWISE : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
					arc.point = makeD2DPoint (endPoint);
					arc.arcSize = fabs(sweepangle) <= M_PI ? D2D1_ARC_SIZE_SMALL : D2D1_ARC_SIZE_LARGE;
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
					CRect r (e.instruction.rect.left, e.instruction.rect.top, e.instruction.rect.right, e.instruction.rect.bottom);
					D2D1_POINT_2F points[4] = {
						{(FLOAT)r.right, (FLOAT)r.top},
						{(FLOAT)r.right, (FLOAT)r.bottom},
						{(FLOAT)r.left, (FLOAT)r.bottom},
						{(FLOAT)r.left, (FLOAT)r.top}
					};
					if (figureOpen && lastPos != CPoint (r.left, r.top))
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
					lastPos = CPoint (r.left, r.top);
					break;
				}
				case Element::kLine:
				{
					if (figureOpen)
					{
						CPoint p (e.instruction.point.x, e.instruction.point.y);
						D2D1_POINT_2F end = {(FLOAT)p.x, (FLOAT)p.y};
						sink->AddLine (end);
						lastPos = p;
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
					CPoint p (e.instruction.point.x, e.instruction.point.y);
					D2D1_POINT_2F start = {(FLOAT)p.x, (FLOAT)p.y};
					sink->BeginFigure (start, D2D1_FIGURE_BEGIN_FILLED);
					figureOpen = true;
					lastPos = p;
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
			path = nullptr;
		}
		sink->Release ();
	}
	if (path && (transform || context))
	{
		ID2D1Geometry* geometry = path;
		if (transform)
		{
			ID2D1TransformedGeometry* tg = nullptr;
			if (!SUCCEEDED (getD2DFactory ()->CreateTransformedGeometry (geometry, convert (*transform), &tg)))
				return nullptr;
			geometry = tg;
		}
		if (context)
		{
			AlignPixelSink sink (context);
			if (sink.init () == false)
			{
				if (transform)
					geometry->Release ();
				return nullptr;
			}
			if (!SUCCEEDED (geometry->Simplify (D2D1_GEOMETRY_SIMPLIFICATION_OPTION_CUBICS_AND_LINES, nullptr, &sink)))
			{
				if (transform)
					geometry->Release ();
				return nullptr;
			}
			
			ID2D1PathGeometry* result = sink.get ();
			if (transform)
				geometry->Release ();
			return result;
		}
		return geometry;
	}
	if (path)
		path->AddRef ();
	return path;
}

} // VSTGUI

#endif // WINDOWS
