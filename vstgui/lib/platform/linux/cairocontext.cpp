// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cairocontext.h"
#include "../../cbitmap.h"
#include "cairobitmap.h"
#include "cairogradient.h"
#include "cairopath.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Cairo {

//------------------------------------------------------------------------
namespace {

struct SaveCairoState
{
	SaveCairoState (ContextHandle& h) : h (h) { cairo_save (h); }
	~SaveCairoState () { cairo_restore (h); }
private:
	ContextHandle& h;
};

//------------------------------------------------------------------------
void checkCairoStatus (const ContextHandle& handle)
{
#if DEBUG
	auto status = cairo_status (handle);
	if (status != CAIRO_STATUS_SUCCESS)
	{
		auto msg = cairo_status_to_string (status);
		DebugPrint ("%s\n", msg);
	}
#endif
}

//------------------------------------------------------------------------
cairo_matrix_t convert (CGraphicsTransform& ct)
{
	return {ct.m11, ct.m21, ct.m12, ct.m22, ct.dx, ct.dy};
}

//-----------------------------------------------------------------------------
inline bool needPixelAlignment (CDrawMode mode)
{
	return (mode.integralMode () && mode.modeIgnoringIntegralMode () == kAntiAliasing);
}

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
DrawBlock::DrawBlock (Context& context) : context (context)
{
	auto ct = context.getCurrentTransform ();
	CRect clip;
	context.getClipRect (clip);
	ct.transform (clip);
	clip.bound (context.getSurfaceRect ());
	if (clip.isEmpty ())
	{
		clipIsEmpty = true;
	}
	else
	{
		cairo_save (context.getCairo ());
		cairo_rectangle (context.getCairo (), clip.left, clip.top, clip.getWidth (),
						 clip.getHeight ());
		cairo_clip (context.getCairo ());
		auto matrix = convert (ct);
		cairo_set_matrix (context.getCairo (), &matrix);
		auto antialiasMode = context.getDrawMode ().modeIgnoringIntegralMode () == kAntiAliasing ?
								 CAIRO_ANTIALIAS_BEST :
								 CAIRO_ANTIALIAS_NONE;
		cairo_set_antialias (context.getCairo (), antialiasMode);
	}
}

//------------------------------------------------------------------------
DrawBlock::~DrawBlock ()
{
	if (!clipIsEmpty)
	{
		cairo_restore (context.getCairo ());
	}
}

//------------------------------------------------------------------------
DrawBlock DrawBlock::begin (Context& context)
{
	return DrawBlock (context);
}

//-----------------------------------------------------------------------------
Context::Context (const CRect& rect, const SurfaceHandle& surface) : super (rect), surface (surface)
{
	init ();
}

//-----------------------------------------------------------------------------
Context::Context (Bitmap* bitmap) : super (new CBitmap (bitmap)), surface (bitmap->getSurface ())
{
	init ();
}

//-----------------------------------------------------------------------------
Context::Context (CRect r, cairo_t* context) : super (r)
{
	cr = ContextHandle {cairo_reference (context)};
	init ();
}

//-----------------------------------------------------------------------------
Context::~Context ()
{
}

//-----------------------------------------------------------------------------
void Context::init ()
{
	if (surface)
		cr.assign (cairo_create (surface));
	super::init ();
}

//-----------------------------------------------------------------------------
void Context::beginDraw ()
{
	super::beginDraw ();
	cairo_save (cr);
	checkCairoStatus (cr);
}

//-----------------------------------------------------------------------------
void Context::endDraw ()
{
	cairo_restore (cr);
	if (surface)
		cairo_surface_flush (surface);
	checkCairoStatus (cr);
	super::endDraw ();
}

//-----------------------------------------------------------------------------
void Context::saveGlobalState ()
{
	super::saveGlobalState ();
}

//-----------------------------------------------------------------------------
void Context::restoreGlobalState ()
{
	super::restoreGlobalState ();
}

//-----------------------------------------------------------------------------
void Context::setSourceColor (CColor color)
{
	auto alpha = color.normAlpha<double> ();
	alpha *= getGlobalAlpha ();
	cairo_set_source_rgba (cr, color.normRed<double> (), color.normGreen<double> (),
	                       color.normBlue<double> (), alpha);
	checkCairoStatus (cr);
}

//-----------------------------------------------------------------------------
void Context::setupCurrentStroke ()
{
	auto lineWidth = getLineWidth ();
	cairo_set_line_width (cr, lineWidth);
	const auto& style = getLineStyle ();
	if (!style.getDashLengths ().empty ())
	{
		auto lengths = style.getDashLengths ();
		for (auto& l : lengths)
			l *= lineWidth;
		cairo_set_dash (cr, lengths.data (), lengths.size (), style.getDashPhase ());
	}
	cairo_line_cap_t lineCap;
	switch (style.getLineCap ())
	{
		case CLineStyle::kLineCapButt:
		{
			lineCap = CAIRO_LINE_CAP_BUTT;
			break;
		}
		case CLineStyle::kLineCapRound:
		{
			lineCap = CAIRO_LINE_CAP_ROUND;
			break;
		}
		case CLineStyle::kLineCapSquare:
		{
			lineCap = CAIRO_LINE_CAP_SQUARE;
			break;
		}
	}
	cairo_set_line_cap (cr, lineCap);
	cairo_line_join_t lineJoin;
	switch (style.getLineJoin ())
	{
		case CLineStyle::kLineJoinBevel:
		{
			lineJoin = CAIRO_LINE_JOIN_BEVEL;
			break;
		}
		case CLineStyle::kLineJoinMiter:
		{
			lineJoin = CAIRO_LINE_JOIN_MITER;
			break;
		}
		case CLineStyle::kLineJoinRound:
		{
			lineJoin = CAIRO_LINE_JOIN_ROUND;
			break;
		}
	}

	cairo_set_line_join (cr, lineJoin);
}

//-----------------------------------------------------------------------------
void Context::draw (CDrawStyle drawStyle)
{
	switch (drawStyle)
	{
		case kDrawStroked:
		{
			setupCurrentStroke ();
			setSourceColor (getFrameColor ());
			cairo_stroke (cr);
			break;
		}
		case kDrawFilled:
		{
			setSourceColor (getFillColor ());
			cairo_fill (cr);
			break;
		}
		case kDrawFilledAndStroked:
		{
			setSourceColor (getFillColor ());
			cairo_fill_preserve (cr);
			setupCurrentStroke ();
			setSourceColor (getFrameColor ());
			cairo_stroke (cr);
			break;
		}
	}
	checkCairoStatus (cr);
}

//-----------------------------------------------------------------------------
void Context::drawLine (const CDrawContext::LinePair& line)
{
	if (auto cd = DrawBlock::begin (*this))
	{
		setupCurrentStroke ();
		setSourceColor (getFrameColor ());
		if (getDrawMode ().integralMode ())
		{
			CPoint start = pixelAlign (cr, line.first);
			CPoint end = pixelAlign (cr, line.second);
			cairo_move_to (cr, start.x + 0.5, start.y + 0.5);
			cairo_line_to (cr, end.x + 0.5, end.y + 0.5);
		}
		else
		{
			cairo_move_to (cr, line.first.x, line.first.y);
			cairo_line_to (cr, line.second.x, line.second.y);
		}
		cairo_stroke (cr);
	}
	checkCairoStatus (cr);
}

//-----------------------------------------------------------------------------
void Context::drawLines (const CDrawContext::LineList& lines)
{
	if (auto cd = DrawBlock::begin (*this))
	{
		setupCurrentStroke ();
		setSourceColor (getFrameColor ());
		if (getDrawMode ().integralMode ())
		{
			for (auto& line : lines)
			{
				CPoint start = pixelAlign (cr, line.first);
				CPoint end = pixelAlign (cr, line.second);
				cairo_move_to (cr, start.x + 0.5, start.y + 0.5);
				cairo_line_to (cr, end.x + 0.5, end.y + 0.5);
				cairo_stroke (cr);
			}
		}
		else
		{
			for (auto& line : lines)
			{
				cairo_move_to (cr, line.first.x, line.first.y);
				cairo_line_to (cr, line.second.x, line.second.y);
				cairo_stroke (cr);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void Context::drawPolygon (const CDrawContext::PointList& polygonPointList,
						   const CDrawStyle drawStyle)
{
	if (polygonPointList.size () < 2)
		return;

	if (auto cd = DrawBlock::begin (*this))
	{
		auto& last = polygonPointList.back ();
		double o = (drawStyle != kDrawFilled) ? 0.5 : 0.0;
		cairo_move_to (cr, last.x + o, last.y + o);
		for (auto& it : polygonPointList)
			cairo_line_to (cr, it.x + o, it.y + o);

		draw (drawStyle);
	}
}

//-----------------------------------------------------------------------------
void Context::drawRect (const CRect& rect, const CDrawStyle drawStyle)
{
	if (auto cd = DrawBlock::begin (*this))
	{
		CRect r (rect);
		if (drawStyle != kDrawFilled)
		{
			r.right -= 1.;
			r.bottom -= 1.;
		}

		double o = (drawStyle != kDrawFilled) ? 0.5 : 0.0;
		if (needPixelAlignment (getDrawMode ()))
		{
			r = pixelAlign (cr, r);
			cairo_rectangle (cr, r.left + o, r.top + o, r.getWidth (), r.getHeight ());
		}
		else
			cairo_rectangle (cr, r.left + o, r.top + o, r.getWidth (), r.getHeight ());
		draw (drawStyle);
	}
}

//-----------------------------------------------------------------------------
void Context::drawArc (const CRect& rect, const float startAngle1, const float endAngle2,
					   const CDrawStyle drawStyle)
{
	if (auto cd = DrawBlock::begin (*this))
	{
		CPoint center = rect.getCenter ();
		cairo_translate (cr, center.x, center.y);
		cairo_scale (cr, 2.0 / rect.getWidth (), 2.0 / rect.getHeight ());
		cairo_arc (cr, 0, 0, 1, startAngle1, endAngle2);
		draw (drawStyle);
	}
}

//-----------------------------------------------------------------------------
void Context::drawEllipse (const CRect& rect, const CDrawStyle drawStyle)
{
	if (auto cd = DrawBlock::begin (*this))
	{
		CPoint center = rect.getCenter ();
		cairo_translate (cr, center.x, center.y);
		cairo_scale (cr, 2.0 / rect.getWidth (), 2.0 / rect.getHeight ());
		cairo_arc (cr, 0, 0, 1, 0, 2 * M_PI);
		draw (drawStyle);
	}
}

//-----------------------------------------------------------------------------
void Context::drawPoint (const CPoint& point, const CColor& color)
{
	if (auto cd = DrawBlock::begin (*this))
	{
		setSourceColor (color);
		cairo_rectangle (cr, point.x + 0.5, point.y + 0.5, 1, 1);
		cairo_fill (cr);
	}
	checkCairoStatus (cr);
}

//-----------------------------------------------------------------------------
void Context::drawBitmap (CBitmap* bitmap, const CRect& dest, const CPoint& offset, float alpha)
{
	if (auto cd = DrawBlock::begin (*this))
	{
		double transformedScaleFactor = getScaleFactor();
		CGraphicsTransform t = getCurrentTransform ();
		if (t.m11 == t.m22 && t.m12 == 0 && t.m21 == 0)
			transformedScaleFactor *= t.m11;
                auto cairoBitmap = bitmap->getBestPlatformBitmapForScaleFactor (transformedScaleFactor).cast<Bitmap> ();
		if (cairoBitmap)
		{
			cairo_translate (cr, dest.left, dest.top);
			cairo_rectangle (cr, 0, 0, dest.getWidth (), dest.getHeight ());
			cairo_clip (cr);

			// Setup a pattern for scaling bitmaps and take it as source afterwards.
			auto pattern = cairo_pattern_create_for_surface (cairoBitmap->getSurface());
			cairo_matrix_t matrix;
			cairo_pattern_get_matrix (pattern, &matrix);
			cairo_matrix_init_scale (&matrix, cairoBitmap->getScaleFactor (), cairoBitmap->getScaleFactor ());
			cairo_matrix_translate (&matrix, offset.x, offset.y);
			cairo_pattern_set_matrix (pattern, &matrix);
			cairo_set_source (cr, pattern);

			cairo_rectangle (cr, -offset.x, -offset.y, dest.getWidth () + offset.x, dest.getHeight () + offset.y);
			alpha *= getGlobalAlpha ();
			if (alpha != 1.f)
			{
				cairo_paint_with_alpha (cr, alpha);
			}
			else
			{
				cairo_fill (cr);
			}

			cairo_pattern_destroy (pattern);
		}
	}
	checkCairoStatus (cr);
}

//-----------------------------------------------------------------------------
void Context::clearRect (const CRect& rect)
{
	if (auto cd = DrawBlock::begin (*this))
	{
		cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
		cairo_rectangle (cr, rect.left, rect.top, rect.getWidth (), rect.getHeight ());
		cairo_fill (cr);
	}
	checkCairoStatus (cr);
}

//-----------------------------------------------------------------------------
CGraphicsPath* Context::createGraphicsPath ()
{
	return new Path (cr);
}

//-----------------------------------------------------------------------------
CGraphicsPath* Context::createTextPath (const CFontRef font, UTF8StringPtr text)
{
#warning TODO: Implementation
	return nullptr;
}

//-----------------------------------------------------------------------------
void Context::drawGraphicsPath (CGraphicsPath* path, CDrawContext::PathDrawMode mode,
								CGraphicsTransform* transformation)
{
	if (auto cairoPath = dynamic_cast<Path*> (path))
	{
		if (auto cd = DrawBlock::begin (*this))
		{
			auto p = cairoPath->getPath (
				cr, needPixelAlignment (getDrawMode ()));
			if (transformation)
			{
				cairo_matrix_t currentMatrix;
				cairo_matrix_t resultMatrix;
				auto matrix = convert (*transformation);
				cairo_get_matrix (cr, &currentMatrix);
				cairo_matrix_multiply (&resultMatrix, &currentMatrix, &matrix);
				cairo_set_matrix (cr, &resultMatrix);
			}
			cairo_append_path (cr, p);
			switch (mode)
			{
				case PathDrawMode::kPathFilled:
				{
					setSourceColor (getFillColor ());
					cairo_fill (cr);
					break;
				}
				case PathDrawMode::kPathFilledEvenOdd:
				{
					setSourceColor (getFillColor ());
					cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
					cairo_fill (cr);
					break;
				}
				case PathDrawMode::kPathStroked:
				{
					setupCurrentStroke ();
					setSourceColor (getFrameColor ());
					cairo_stroke (cr);
					break;
				}
			}
		}
	}
	checkCairoStatus (cr);
}

//-----------------------------------------------------------------------------
void Context::fillLinearGradient (CGraphicsPath* path, const CGradient& gradient,
								  const CPoint& startPoint, const CPoint& endPoint, bool evenOdd,
								  CGraphicsTransform* transformation)
{
	if (auto cairoPath = dynamic_cast<Path*> (path))
	{
		if (auto cairoGradient = dynamic_cast<const Gradient*> (&gradient))
		{
			if (auto cd = DrawBlock::begin (*this))
			{
				auto p = cairoPath->getPath (cr);
				cairo_append_path (cr, p);
				cairo_set_source (cr, cairoGradient->getLinearGradient (startPoint, endPoint));
				if (evenOdd)
				{
					cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
					cairo_fill (cr);
				}
				else
				{
					cairo_fill (cr);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void Context::fillRadialGradient (CGraphicsPath* path, const CGradient& gradient,
								  const CPoint& center, CCoord radius, const CPoint& originOffset,
								  bool evenOdd, CGraphicsTransform* transformation)
{
#warning TODO: Implementation
	auto cd = DrawBlock::begin (*this);
	if (cd)
	{
	}
}

//-----------------------------------------------------------------------------
CPoint pixelAlign (const ContextHandle& handle, const CPoint& point)
{
    double x = point.x;
    double y = point.y;
    cairo_user_to_device(handle, &x, &y);
    return CPoint(std::round(x), std::round(y));
}

CRect pixelAlign (const ContextHandle& handle, const CRect& rect)
{
    double left = rect.left;
    double top = rect.top;
    double right = rect.right;
    double bottom = rect.bottom;
    cairo_user_to_device(handle, &left, &top);
    cairo_user_to_device(handle, &right, &bottom);
    return CRect(std::round(left), std::round(top), std::round(right), std::round(bottom));
}

//-----------------------------------------------------------------------------
} // Cairo
} // VSTGUI
