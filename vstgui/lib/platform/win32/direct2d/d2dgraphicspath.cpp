// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "d2dgraphicspath.h"

#if WINDOWS

#include "../../../cgradient.h"
#include "../../../cstring.h"
#include "../win32support.h"
#include "d2ddrawcontext.h"
#include "d2dfont.h"
#include <dwrite.h>
#include <winnt.h>
#include <cassert>

#if defined(__GNUC__) && !defined(__clang__)
#define __maybenull
#define __out
#endif

namespace VSTGUI {

class D2DPathTextRenderer final : public IDWriteTextRenderer
{
public:
	D2DPathTextRenderer (ID2D1GeometrySink* sink) : sink (sink) {}

	STDMETHOD (DrawGlyphRun)
	(void* clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY,
	 DWRITE_MEASURING_MODE measuringMode, DWRITE_GLYPH_RUN const* glyphRun,
	 DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
	 IUnknown* clientDrawingEffect) override
	{
		return glyphRun->fontFace->GetGlyphRunOutline (
			glyphRun->fontEmSize, glyphRun->glyphIndices, glyphRun->glyphAdvances,
			glyphRun->glyphOffsets, glyphRun->glyphCount, glyphRun->isSideways,
			glyphRun->bidiLevel % 2, sink);
	}

	STDMETHOD (DrawUnderline)
	(void* clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY,
	 DWRITE_UNDERLINE const* underline, IUnknown* clientDrawingEffect) override
	{
		return S_FALSE;
	}

	STDMETHOD (DrawStrikethrough)
	(void* clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY,
	 DWRITE_STRIKETHROUGH const* strikethrough, IUnknown* clientDrawingEffect) override
	{
		return S_FALSE;
	}

	STDMETHOD (DrawInlineObject)
	(void* clientDrawingContext, FLOAT originX, FLOAT originY, IDWriteInlineObject* inlineObject,
	 BOOL isSideways, BOOL isRightToLeft, IUnknown* clientDrawingEffect) override
	{
		return S_FALSE;
	}

	STDMETHOD (IsPixelSnappingDisabled)
	(__maybenull void* clientDrawingContext, __out BOOL* isDisabled) override
	{
		if (isDisabled)
			*isDisabled = FALSE;
		return S_FALSE;
	}

	STDMETHOD (GetCurrentTransform)
	(__maybenull void* clientDrawingContext, __out DWRITE_MATRIX* transform) override
	{
		const DWRITE_MATRIX identityTransform = {1, 0, 0, 1, 0, 0};
		if (transform)
			*transform = identityTransform;
		return S_OK;
	}

	STDMETHOD (GetPixelsPerDip)
	(__maybenull void* clientDrawingContext, __out FLOAT* pixelsPerDip) override
	{
		if (pixelsPerDip)
			*pixelsPerDip = 96;
		return S_OK;
	}

	STDMETHOD (QueryInterface) (REFIID iid, void** ppvObject) override
	{
		if (iid == __uuidof(IUnknown) || iid == __uuidof(IDWriteTextRenderer))
		{
			*ppvObject = static_cast<IDWriteTextRenderer*> (this);
			AddRef ();
			return S_OK;
		}
		else
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
	HRESULT STDMETHODCALLTYPE QueryInterface (REFIID iid, void** ppvObject) override
	{
		if (iid == __uuidof(IUnknown) || iid == __uuidof(ID2D1SimplifiedGeometrySink))
		{
			*ppvObject = static_cast<ID2D1SimplifiedGeometrySink*> (this);
			AddRef ();
			return S_OK;
		}
		else
			return E_NOINTERFACE;
	}
	ULONG STDMETHODCALLTYPE AddRef () override { return 1; }
	ULONG STDMETHODCALLTYPE Release () override { return 1; }

	D2D1_POINT_2F alignPoint (const D2D1_POINT_2F& p)
	{
		CPoint point (p.x, p.y);
		if (context->getDrawMode ().antiAliasing ())
			point.offset (-0.5, -0.5);
		if (context)
			context->pixelAllign (point);
		return D2D1::Point2F (static_cast<FLOAT> (point.x), static_cast<FLOAT> (point.y));
	}

	STDMETHOD_ (void, AddBeziers) (const D2D1_BEZIER_SEGMENT* beziers, UINT beziersCount) override
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

	STDMETHOD_ (void, AddLines) (const D2D1_POINT_2F* points, UINT pointsCount) override
	{
		for (UINT i = 0; i < pointsCount; ++i)
		{
			D2D_POINT_2F point = alignPoint (points[i]);
			sink->AddLine (point);
		}
	}

	STDMETHOD_ (void, BeginFigure)
	(D2D1_POINT_2F startPoint, D2D1_FIGURE_BEGIN figureBegin) override
	{
		startPoint = alignPoint (startPoint);
		sink->BeginFigure (startPoint, figureBegin);
	}

	STDMETHOD_ (void, EndFigure) (D2D1_FIGURE_END figureEnd) override
	{
		sink->EndFigure (figureEnd);
	}

	STDMETHOD_ (void, SetFillMode) (D2D1_FILL_MODE fillMode) override
	{
		sink->SetFillMode (fillMode);
	}

	STDMETHOD_ (void, SetSegmentFlags) (D2D1_PATH_SEGMENT vertexFlags) override
	{
		sink->SetSegmentFlags (vertexFlags);
	}

	STDMETHOD (Close) () override
	{
		isClosed = true;
		return sink->Close ();
	}

	AlignPixelSink (D2DDrawContext* context)
	: path (nullptr), sink (nullptr), context (context), isClosed (true)
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
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
D2DGraphicsPath::D2DGraphicsPath (ID2D1PathGeometry* path, PlatformGraphicsPathFillMode fillMode)
: path (path), fillMode (fillMode)
{
}

//-----------------------------------------------------------------------------
D2DGraphicsPath::~D2DGraphicsPath () noexcept
{
	if (sinkInternal)
		sinkInternal->Release ();
	path->Release ();
}

//-----------------------------------------------------------------------------
ID2D1Geometry* D2DGraphicsPath::createTransformedGeometry (ID2D1Factory* factory,
														   const CGraphicsTransform& tm) const
{
	ID2D1TransformedGeometry* tg = nullptr;
	if (!SUCCEEDED (factory->CreateTransformedGeometry (path, convert (tm), &tg)))
		return nullptr;
	return tg;
	;
}

//-----------------------------------------------------------------------------
ID2D1Geometry* D2DGraphicsPath::createPixelAlignedGeometry (ID2D1Factory* factory,
															D2DDrawContext& context,
															const CGraphicsTransform* tm) const
{
	ID2D1Geometry* workingPath = path;
	workingPath->AddRef ();
	if (tm)
		workingPath = createTransformedGeometry (factory, *tm);

	AlignPixelSink alignSink (&context);
	if (alignSink.init () == false)
	{
		workingPath->Release ();
		return nullptr;
	}
	if (!SUCCEEDED (workingPath->Simplify (D2D1_GEOMETRY_SIMPLIFICATION_OPTION_CUBICS_AND_LINES,
										   nullptr, &alignSink)))
	{
		workingPath->Release ();
		return nullptr;
	}
	workingPath->Release ();

	return alignSink.get ();
}

//-----------------------------------------------------------------------------
ID2D1GeometrySink* D2DGraphicsPath::getSink ()
{
	if (!sinkInternal)
	{
		if (FAILED (path->Open (&sinkInternal)))
		{
			sinkInternal = nullptr;
		}
		else
		{
			D2D1_FILL_MODE mode = fillMode == PlatformGraphicsPathFillMode::Alternate
									  ? D2D1_FILL_MODE_ALTERNATE
									  : D2D1_FILL_MODE_WINDING;
			sinkInternal->SetFillMode (mode);
		}
	}
	return sinkInternal;
}

//-----------------------------------------------------------------------------
void D2DGraphicsPath::addArc (const CRect& rect, double startAngle, double endAngle, bool clockwise)
{
	if (auto sink = getSink ())
	{
		startAngle = radians (startAngle);
		endAngle = radians (endAngle);
		CRect o_r = rect;
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
		if (clockwise)
		{
			// sweepangle positive
			while (sweepangle < 0.0)
				sweepangle += 2 * M_PI;
			while (sweepangle > 2 * M_PI)
				sweepangle -= 2 * M_PI;
		}
		else
		{
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
		arc.size = makeD2DSize (r.getWidth () / 2., r.getHeight () / 2.);
		arc.rotationAngle = 0;
		arc.sweepDirection =
			clockwise ? D2D1_SWEEP_DIRECTION_CLOCKWISE : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
		arc.point = makeD2DPoint (endPoint);
		arc.arcSize = fabs (sweepangle) <= M_PI ? D2D1_ARC_SIZE_SMALL : D2D1_ARC_SIZE_LARGE;
		sink->AddArc (arc);
		lastPos = endPoint;
	}
}

//-----------------------------------------------------------------------------
void D2DGraphicsPath::addEllipse (const CRect& rect)
{
	if (auto sink = getSink ())
	{
		CPoint top (rect.getTopLeft ());
		top.x += rect.getWidth () / 2.;
		CPoint bottom (rect.getBottomLeft ());
		bottom.x += rect.getWidth () / 2.;
		if (figureOpen && lastPos != rect.getTopLeft ())
		{
			sink->EndFigure (D2D1_FIGURE_END_OPEN);
			figureOpen = false;
		}
		if (!figureOpen)
		{
			sink->BeginFigure (makeD2DPoint (top), D2D1_FIGURE_BEGIN_FILLED);
			figureOpen = true;
		}
		D2D1_ARC_SEGMENT arc =
		    D2D1::ArcSegment (makeD2DPoint (bottom),
		                      D2D1::SizeF (static_cast<FLOAT> (rect.getWidth () / 2.f),
		                                   static_cast<FLOAT> (rect.getHeight () / 2.f)),
		                      180.f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL);
		sink->AddArc (arc);
		arc.point = makeD2DPoint (top);
		sink->AddArc (arc);
		lastPos = top;
	}
}

//-----------------------------------------------------------------------------
void D2DGraphicsPath::addRect (const CRect& rect)
{
	if (auto sink = getSink ())
	{
		D2D1_POINT_2F points[4] = {{(FLOAT)rect.right, (FLOAT)rect.top},
								   {(FLOAT)rect.right, (FLOAT)rect.bottom},
								   {(FLOAT)rect.left, (FLOAT)rect.bottom},
								   {(FLOAT)rect.left, (FLOAT)rect.top}};
		if (figureOpen && lastPos != rect.getTopLeft ())
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
		lastPos = rect.getTopLeft ();
	}
}

//-----------------------------------------------------------------------------
void D2DGraphicsPath::addLine (const CPoint& to)
{
	if (auto sink = getSink ())
	{
		if (figureOpen)
		{
			D2D1_POINT_2F end = {(FLOAT)to.x, (FLOAT)to.y};
			sink->AddLine (end);
			lastPos = to;
		}
	}
}

//-----------------------------------------------------------------------------
void D2DGraphicsPath::addBezierCurve (const CPoint& control1, const CPoint& control2,
									  const CPoint& end)
{
	if (auto sink = getSink ())
	{
		if (figureOpen)
		{
			D2D1_POINT_2F d2dcontrol1 = {(FLOAT)control1.x, (FLOAT)control1.y};
			D2D1_POINT_2F d2dcontrol2 = {(FLOAT)control2.x, (FLOAT)control2.y};
			D2D1_POINT_2F d2dend = {(FLOAT)end.x, (FLOAT)end.y};
			D2D1_BEZIER_SEGMENT bezier = D2D1::BezierSegment (d2dcontrol1, d2dcontrol2, d2dend);
			sink->AddBezier (bezier);
			lastPos = end;
		}
	}
}

//-----------------------------------------------------------------------------
void D2DGraphicsPath::beginSubpath (const CPoint& start)
{
	if (auto sink = getSink ())
	{
		if (figureOpen)
			sink->EndFigure (D2D1_FIGURE_END_OPEN);
		D2D1_POINT_2F d2dstart = {(FLOAT)start.x, (FLOAT)start.y};
		sink->BeginFigure (d2dstart, D2D1_FIGURE_BEGIN_FILLED);
		figureOpen = true;
		lastPos = start;
	}
}

//-----------------------------------------------------------------------------
void D2DGraphicsPath::closeSubpath ()
{
	if (auto sink = getSink ())
	{
		if (figureOpen)
		{
			sink->EndFigure (D2D1_FIGURE_END_CLOSED);
			figureOpen = false;
		}
	}
}

//-----------------------------------------------------------------------------
void D2DGraphicsPath::finishBuilding ()
{
	if (auto sink = getSink ())
	{
		if (figureOpen)
			sink->EndFigure (D2D1_FIGURE_END_OPEN);
		HRESULT res = sink->Close ();
		assert (SUCCEEDED (res));
		sink->Release ();
		sinkInternal = nullptr;
		figureOpen = false;
	}
}

//-----------------------------------------------------------------------------
bool D2DGraphicsPath::hitTest (const CPoint& p, bool evenOddFilled,
							   CGraphicsTransform* transform) const
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
	path->FillContainsPoint (makeD2DPoint (p), matrix, &result);
	return result ? true : false;
}

//-----------------------------------------------------------------------------
CRect D2DGraphicsPath::getBoundingBox () const
{
	CRect r;
	D2D1_RECT_F bounds;
	if (SUCCEEDED (path->GetBounds (nullptr, &bounds)))
	{
		r.left = bounds.left;
		r.top = bounds.top;
		r.right = bounds.right;
		r.bottom = bounds.bottom;
	}
	return r;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PlatformGraphicsPathFactoryPtr D2DGraphicsPathFactory::instance ()
{
	static PlatformGraphicsPathFactoryPtr factory = std::make_shared<D2DGraphicsPathFactory> ();
	return factory;
}

//-----------------------------------------------------------------------------
PlatformGraphicsPathPtr D2DGraphicsPathFactory::createPath (PlatformGraphicsPathFillMode fillMode)
{
	ID2D1PathGeometry* path {nullptr};
	if (FAILED (getD2DFactory ()->CreatePathGeometry (&path)))
		return nullptr;
	return std::make_unique<D2DGraphicsPath> (path, fillMode);
}

//-----------------------------------------------------------------------------
PlatformGraphicsPathPtr D2DGraphicsPathFactory::createTextPath (const PlatformFontPtr& font,
																UTF8StringPtr text)
{
	auto d2dFont = font.cast<D2DFont> ();
	if (!d2dFont)
		return nullptr;
	ID2D1PathGeometry* localPath = nullptr;
	getD2DFactory ()->CreatePathGeometry (&localPath);
	if (localPath == nullptr)
		return nullptr;

	IDWriteTextLayout* layout = d2dFont->createTextLayout (UTF8String (text).getPlatformString ());
	if (layout == nullptr)
		return nullptr;

	ID2D1PathGeometry* textPath = nullptr;
	getD2DFactory ()->CreatePathGeometry (&textPath);
	if (textPath == nullptr)
	{
		layout->Release ();
		return nullptr;
	}

	ID2D1GeometrySink* sink = nullptr;
	if (!SUCCEEDED (textPath->Open (&sink)))
	{
		textPath->Release ();
		layout->Release ();
		return nullptr;
	}

	D2DPathTextRenderer renderer (sink);
	layout->Draw (nullptr, &renderer, 0, 0);

	sink->Close ();
	sink->Release ();
	layout->Release ();

	if (!SUCCEEDED (localPath->Open (&sink)))
	{
		textPath->Release ();
		return nullptr;
	}

	D2D1_RECT_F bounds = {};
	if (SUCCEEDED (textPath->GetBounds (nullptr, &bounds)))
	{
		textPath->Simplify (D2D1_GEOMETRY_SIMPLIFICATION_OPTION_CUBICS_AND_LINES,
							D2D1::Matrix3x2F::Translation (0, -bounds.top), sink);
	}

	textPath->Release ();
	sink->Close ();
	sink->Release ();
	return std::make_unique<D2DGraphicsPath> (localPath, PlatformGraphicsPathFillMode::Ignored);
}

} // VSTGUI

#endif // WINDOWS
