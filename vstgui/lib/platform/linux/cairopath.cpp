// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../cgradient.h"
#include "../../cgraphicstransform.h"
#include "cairocontext.h"
#include "cairopath.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Cairo {

//-----------------------------------------------------------------------------
GraphicsPathFactory::GraphicsPathFactory (const ContextHandle& cr) : context (cr) {}

//-----------------------------------------------------------------------------
PlatformGraphicsPathPtr
	GraphicsPathFactory::createPath ([[maybe_unused]] PlatformGraphicsPathFillMode fillMode)
{
	return std::make_unique<GraphicsPath> (context);
}

//-----------------------------------------------------------------------------
PlatformGraphicsPathPtr GraphicsPathFactory::createTextPath (const PlatformFontPtr& font,
															 UTF8StringPtr text)
{
	return nullptr;
}

//------------------------------------------------------------------------
GraphicsPath::GraphicsPath (const ContextHandle& c) : context (c)
{
	cairo_save (context);
	cairo_new_path (context);
}

//------------------------------------------------------------------------
GraphicsPath::~GraphicsPath () noexcept
{
	cairo_path_destroy (path);
}

//------------------------------------------------------------------------
void GraphicsPath::finishBuilding ()
{
	path = cairo_copy_path (context);
	cairo_restore (context);
	cairo_new_path (context); // clear path in context
}

//------------------------------------------------------------------------
void GraphicsPath::addArc (const CRect& rect, double startAngle, double endAngle, bool clockwise)
{
	auto radiusX = (rect.right - rect.left) / 2.;
	auto radiusY = (rect.bottom - rect.top) / 2.;

	auto centerX = static_cast<double> (rect.left + radiusX);
	auto centerY = static_cast<double> (rect.top + radiusY);

	startAngle = radians (startAngle);
	endAngle = radians (endAngle);
	if (radiusX != radiusY)
	{
		startAngle = std::atan2 (std::sin (startAngle) * radiusX, std::cos (startAngle) * radiusY);
		endAngle = std::atan2 (std::sin (endAngle) * radiusX, std::cos (endAngle) * radiusY);
	}
	cairo_matrix_t matrix;
	cairo_get_matrix (context, &matrix);
	cairo_translate (context, centerX, centerY);
	cairo_scale (context, radiusX, radiusY);
	if (clockwise)
	{
		cairo_arc (context, 0, 0, 1, startAngle, endAngle);
	}
	else
	{
		cairo_arc_negative (context, 0, 0, 1, startAngle, endAngle);
	}
	cairo_set_matrix (context, &matrix);
}

//------------------------------------------------------------------------
void GraphicsPath::addEllipse (const CRect& rect) { addArc (rect, 0, 360, true); }

//------------------------------------------------------------------------
void GraphicsPath::addRect (const CRect& rect)
{
	cairo_rectangle (context, rect.left, rect.top, rect.getWidth (), rect.getHeight ());
}

//------------------------------------------------------------------------
void GraphicsPath::addLine (const CPoint& to)
{
	cairo_line_to (context, to.x, to.y);
}

//------------------------------------------------------------------------
void GraphicsPath::addBezierCurve (const CPoint& control1, const CPoint& control2,
                                   const CPoint& end)
{
	cairo_curve_to (context, control1.x, control1.y, control2.x, control2.y, end.x, end.y);
}

//------------------------------------------------------------------------
void GraphicsPath::beginSubpath (const CPoint& start)
{
	cairo_new_sub_path (context);
	cairo_move_to (context, start.x, start.y);
}

//------------------------------------------------------------------------
void GraphicsPath::closeSubpath ()
{
	cairo_close_path (context);
}

//------------------------------------------------------------------------
std::unique_ptr<GraphicsPath> GraphicsPath::copyPixelAlign (const CGraphicsTransform& tm)
{
	auto result = std::make_unique<GraphicsPath> (context);
	cairo_append_path (context, path);
	result->finishBuilding ();
	auto rpath = result->path;

	auto align = [] (_cairo_path_data_t* data, int index, const CGraphicsTransform& tm) {
		CPoint input (data[index].point.x, data[index].point.y);
		auto output = Cairo::pixelAlign (tm, input);
		data[index].point.x = output.x;
		data[index].point.y = output.y;
	};
	for (auto i = 0; i < rpath->num_data; i += rpath->data[i].header.length)
	{
		auto data = &rpath->data[i];
		switch (data->header.type)
		{
			case CAIRO_PATH_MOVE_TO:
			{
				align (data, 1, tm);
				break;
			}
			case CAIRO_PATH_LINE_TO:
			{
				align (data, 1, tm);
				break;
			}
			case CAIRO_PATH_CURVE_TO:
			{
				align (data, 1, tm);
				align (data, 2, tm);
				align (data, 3, tm);
				break;
			}
			case CAIRO_PATH_CLOSE_PATH: { break;
			}
		}
	}
	return result;
}

//------------------------------------------------------------------------
bool GraphicsPath::hitTest (const CPoint& p, bool evenOddFilled,
                            CGraphicsTransform* transform) const
{
	auto tp = p;
	if (transform)
		transform->transform (tp);
	cairo_save (context);
	cairo_new_path (context);
	cairo_append_path (context, path);
	cairo_set_fill_rule (context,
	                     evenOddFilled ? CAIRO_FILL_RULE_EVEN_ODD : CAIRO_FILL_RULE_WINDING);
	cairo_clip (context);
	auto result = cairo_in_clip (context, tp.x, tp.y);
	cairo_restore (context);
	return result;
}

//------------------------------------------------------------------------
CRect GraphicsPath::getBoundingBox () const
{
	CRect r;
	cairo_save (context);
	cairo_new_path (context);
	cairo_append_path (context, path);
	CPoint p1, p2;
	cairo_path_extents (context, &p1.x, &p1.y, &p2.x, &p2.y);
	cairo_restore (context);
	r.setTopLeft (p1);
	r.setBottomRight (p2);
	return r;
}

//------------------------------------------------------------------------
} // Cairo
} // VSTGUI
