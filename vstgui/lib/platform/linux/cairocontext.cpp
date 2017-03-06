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
Context::Context (CRect& rect, const SurfaceHandle& surface) : super (rect), surface (surface)
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
	auto alpha = color.alpha / 255.;
	alpha *= getGlobalAlpha ();
	cairo_set_source_rgba (cr, color.red / 255., color.green / 255., color.blue / 255., alpha);
	checkCairoStatus (cr);
}

//-----------------------------------------------------------------------------
void Context::setupCurrentStroke ()
{
	cairo_set_line_width (cr, getLineWidth ());
	const auto& style = getLineStyle ();
	if (!style.getDashLengths ().empty ())
	{
		cairo_set_dash (cr, style.getDashLengths ().data (), style.getDashLengths ().size (),
						style.getDashPhase ());
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
			CPoint start = pixelAlign (getCurrentTransform (), line.first);
			CPoint end = pixelAlign (getCurrentTransform (), line.second);
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
				CPoint start = pixelAlign (getCurrentTransform (), line.first);
				CPoint end = pixelAlign (getCurrentTransform (), line.second);
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
		cairo_move_to (cr, last.x, last.y);
		for (auto& it : polygonPointList)
			cairo_line_to (cr, it.x, it.y);

		draw (drawStyle);
	}
}

//-----------------------------------------------------------------------------
void Context::drawRect (const CRect& rect, const CDrawStyle drawStyle)
{
	if (auto cd = DrawBlock::begin (*this))
	{
		CRect r (rect);
		if (needPixelAlignment (getDrawMode ()))
		{
			r = pixelAlign (getCurrentTransform (), r);
			cairo_rectangle (cr, r.left + 0.5, r.top + 0.5, r.getWidth (), r.getHeight ());
		}
		else
			cairo_rectangle (cr, r.left + 0.5, r.top + 0.5, r.getWidth () - 0.5,
							 r.getHeight () - 0.5);
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
		if (auto cairoBitmap = dynamic_cast<Bitmap*> (bitmap->getPlatformBitmap ()))
		{
			cairo_translate (cr, dest.left, dest.top);
			cairo_rectangle (cr, 0, 0, dest.getWidth (), dest.getHeight ());
			cairo_clip (cr);
			cairo_set_source_surface (cr, cairoBitmap->getSurface (), -offset.x, -offset.y);
			cairo_rectangle (cr, -offset.x, -offset.y, dest.getWidth () + offset.x,
							 dest.getHeight () + offset.y);
			alpha *= getGlobalAlpha ();
			if (alpha != 1.f)
			{
				cairo_paint_with_alpha (cr, alpha);
			}
			else
			{
				cairo_fill (cr);
			}
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
				cr, needPixelAlignment (getDrawMode ()) ? &getCurrentTransform () : nullptr);
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
} // Cairo
} // VSTGUI
