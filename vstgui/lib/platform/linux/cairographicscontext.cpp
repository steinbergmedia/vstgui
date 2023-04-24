// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cairographicscontext.h"
#include "cairobitmap.h"
#include "cairopath.h"
#include "cairogradient.h"
#include "../../crect.h"
#include "../../cgraphicstransform.h"
#include "../../ccolor.h"
#include "../../cdrawdefs.h"
#include "../../clinestyle.h"

#include <pango/pangocairo.h>
#include <stack>

//------------------------------------------------------------------------
namespace VSTGUI {

using TransformMatrix = CGraphicsTransform;

//------------------------------------------------------------------------
inline void checkCairoStatus (const Cairo::ContextHandle& handle)
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
inline cairo_matrix_t convert (const TransformMatrix& ct)
{
	return {ct.m11, ct.m21, ct.m12, ct.m22, ct.dx, ct.dy};
}

//-----------------------------------------------------------------------------
struct CairoGraphicsDeviceFactory::Impl
{
	std::vector<std::shared_ptr<CairoGraphicsDevice>> devices;
};

//-----------------------------------------------------------------------------
CairoGraphicsDeviceFactory::CairoGraphicsDeviceFactory ()
{
	impl = std::make_unique<Impl> ();
}

//-----------------------------------------------------------------------------
CairoGraphicsDeviceFactory::~CairoGraphicsDeviceFactory () noexcept = default;

//-----------------------------------------------------------------------------
PlatformGraphicsDevicePtr CairoGraphicsDeviceFactory::getDeviceForScreen (ScreenInfo::Identifier screen) const
{
	if (impl->devices.empty ())
	{
		// just create a dummy device as we don't really need the cairo device at the moment
		impl->devices.push_back (std::make_shared<CairoGraphicsDevice> (nullptr));
	}
	return impl->devices.front ();
}

//-----------------------------------------------------------------------------
PlatformGraphicsDevicePtr CairoGraphicsDeviceFactory::addDevice (cairo_device_t* device)
{
	for (auto& dev : impl->devices)
	{
		if (dev->get () == device)
			return dev;
	}
	impl->devices.push_back (std::make_shared<CairoGraphicsDevice> (device));
	return impl->devices.back ();
}

//-----------------------------------------------------------------------------
struct CairoGraphicsDevice::Impl
{
	cairo_device_t* device;
};

//-----------------------------------------------------------------------------
CairoGraphicsDevice::CairoGraphicsDevice (cairo_device_t* device)
{
	impl = std::make_unique<Impl> ();
	impl->device = device;
	if (device)
		cairo_device_reference (device);
}

//-----------------------------------------------------------------------------
CairoGraphicsDevice::~CairoGraphicsDevice () noexcept
{
	if (impl->device)
		cairo_device_destroy (impl->device);
}

//-----------------------------------------------------------------------------
cairo_device_t* CairoGraphicsDevice::get () const { return impl->device; }

//-----------------------------------------------------------------------------
PlatformGraphicsDeviceContextPtr
	CairoGraphicsDevice::createBitmapContext (const PlatformBitmapPtr& bitmap) const
{
	auto cairoBitmap = bitmap.cast<Cairo::Bitmap> ();
	if (cairoBitmap)
	{
		return std::make_shared<CairoGraphicsDeviceContext> (*this, cairoBitmap->getSurface ());
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
inline CPoint pixelAlign (const CGraphicsTransform& tm, const CPoint& p)
{
	auto obj = p;
	tm.transform (obj);
	obj.x = std::round (obj.x);
	obj.y = std::round (obj.y);
	tm.inverse ().transform (obj);
	return obj;
}

//-----------------------------------------------------------------------------
inline CRect pixelAlign (const CGraphicsTransform& tm, const CRect& r)
{
	auto obj = r;
	tm.transform (obj);
	obj.left = std::round (obj.left);
	obj.right = std::round (obj.right);
	obj.top = std::round (obj.top);
	obj.bottom = std::round (obj.bottom);
	tm.inverse ().transform (obj);
	return obj;
}

//------------------------------------------------------------------------
struct CairoGraphicsDeviceContext::Impl
{
	const CairoGraphicsDevice& device;
	Cairo::ContextHandle context;
	Cairo::SurfaceHandle surface;

	Impl (const CairoGraphicsDevice& device, const Cairo::SurfaceHandle& surface)
	: device (device), surface (surface)
	{
		context.assign (cairo_create (surface));
	}

	template<typename Proc>
	void doInContext (Proc p)
	{
		auto ct = state.tm;
		CRect clip = state.clip;
		if (clip.isEmpty ())
			return;
		cairo_save (context);
		cairo_rectangle (context, clip.left, clip.top, clip.getWidth (), clip.getHeight ());
		cairo_clip (context);
		auto matrix = convert (ct);
		cairo_set_matrix (context, &matrix);
		auto antialiasMode = state.drawMode.modeIgnoringIntegralMode () == kAntiAliasing
								 ? CAIRO_ANTIALIAS_BEST
								 : CAIRO_ANTIALIAS_NONE;
		cairo_set_antialias (context, antialiasMode);
		p ();
		checkCairoStatus (context);
		cairo_restore (context);
	}

	void applyLineWidthCTM ()
	{
		auto p = calcLineTranslate ();
		cairo_translate (context, p.x, p.y);
	}

	CPoint calcLineTranslate () const
	{
		CPoint p {};
		int32_t lineWidthInt = static_cast<int32_t> (state.lineWidth);
		if (static_cast<CCoord> (lineWidthInt) == state.lineWidth && lineWidthInt % 2)
			p.x = p.y = 0.5;
		return p;
	}

	void applyLineStyle ()
	{
		auto lineWidth = state.lineWidth;
		cairo_set_line_width (context, lineWidth);
		const auto& style = state.lineStyle;
		if (!style.getDashLengths ().empty ())
		{
			auto lengths = style.getDashLengths ();
			for (auto& l : lengths)
				l *= lineWidth;
			cairo_set_dash (context, lengths.data (), lengths.size (), style.getDashPhase ());
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
		cairo_set_line_cap (context, lineCap);
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

		cairo_set_line_join (context, lineJoin);
	}

	void setupSourceColor (CColor color)
	{
		auto alpha = color.normAlpha<double> ();
		alpha *= state.globalAlpha;
		cairo_set_source_rgba (context, color.normRed<double> (), color.normGreen<double> (),
							   color.normBlue<double> (), alpha);
		checkCairoStatus (context);
	}
	void applyFillColor () { setupSourceColor (state.fillColor); }
	void applyFrameColor () { setupSourceColor (state.frameColor); }
	void applyFontColor (CColor color) { setupSourceColor (color); }

	void draw (PlatformGraphicsDrawStyle drawStyle)
	{
		switch (drawStyle)
		{
			case PlatformGraphicsDrawStyle::Stroked:
			{
				applyLineStyle ();
				applyFrameColor ();
				cairo_stroke (context);
				break;
			}
			case PlatformGraphicsDrawStyle::Filled:
			{
				applyFillColor ();
				cairo_fill (context);
				break;
			}
			case PlatformGraphicsDrawStyle::FilledAndStroked:
			{
				applyFillColor ();
				cairo_fill_preserve (context);
				applyLineStyle ();
				applyFrameColor ();
				cairo_stroke (context);
				break;
			}
		}
		checkCairoStatus (context);
	}

	struct State
	{
		CRect clip {};
		CLineStyle lineStyle {kLineSolid};
		CDrawMode drawMode {};
		CColor fillColor {kTransparentCColor};
		CColor frameColor {kTransparentCColor};
		CCoord lineWidth {1.};
		double globalAlpha {1.};
		TransformMatrix tm {};
	};
	State state;
	std::stack<State> stateStack;
	double scaleFactor {1.};

	PlatformGraphicsPathFactoryPtr pathFactory;
};

//------------------------------------------------------------------------
CairoGraphicsDeviceContext::CairoGraphicsDeviceContext (const CairoGraphicsDevice& device,
														const Cairo::SurfaceHandle& handle)
{
	impl = std::make_unique<Impl> (device, handle);
}

//------------------------------------------------------------------------
CairoGraphicsDeviceContext::~CairoGraphicsDeviceContext () noexcept {}

//------------------------------------------------------------------------
const IPlatformGraphicsDevice& CairoGraphicsDeviceContext::getDevice () const
{
	return impl->device;
}

//------------------------------------------------------------------------
PlatformGraphicsPathFactoryPtr CairoGraphicsDeviceContext::getGraphicsPathFactory () const
{
	if (!impl->pathFactory)
		impl->pathFactory = std::make_shared<Cairo::GraphicsPathFactory> (impl->context);
	return impl->pathFactory;
}

//------------------------------------------------------------------------
bool CairoGraphicsDeviceContext::beginDraw () const
{
	if (impl->context)
		cairo_save (impl->context);
	return true;
}

//------------------------------------------------------------------------
bool CairoGraphicsDeviceContext::endDraw () const
{
	if (impl->context)
		cairo_restore (impl->context);
	if (impl->surface)
		cairo_surface_flush (impl->surface);
	return true;
}

//------------------------------------------------------------------------
bool CairoGraphicsDeviceContext::drawLine (LinePair line) const
{
	impl->doInContext ([&] () {
		impl->applyLineStyle ();
		impl->applyFrameColor ();
		if (impl->state.drawMode.integralMode ())
		{
			CPoint start = pixelAlign (impl->state.tm, line.first);
			CPoint end = pixelAlign (impl->state.tm, line.second);
			impl->applyLineWidthCTM ();
			cairo_move_to (impl->context, start.x, start.y);
			cairo_line_to (impl->context, end.x, end.y);
		}
		else
		{
			cairo_move_to (impl->context, line.first.x, line.first.y);
			cairo_line_to (impl->context, line.second.x, line.second.y);
		}
		cairo_stroke (impl->context);
	});
	return true;
}

//------------------------------------------------------------------------
bool CairoGraphicsDeviceContext::drawLines (const LineList& lines) const
{
	impl->doInContext ([&] () {
		impl->applyLineStyle ();
		impl->applyFrameColor ();
		if (impl->state.drawMode.integralMode ())
		{
			auto pt = impl->calcLineTranslate ();
			for (auto& line : lines)
			{
				CPoint start = pixelAlign (impl->state.tm, line.first);
				CPoint end = pixelAlign (impl->state.tm, line.second);
				cairo_move_to (impl->context, start.x + pt.x, start.y + pt.y);
				cairo_line_to (impl->context, end.x + pt.x, end.y + pt.y);
				cairo_stroke (impl->context);
			}
		}
		else
		{
			for (auto& line : lines)
			{
				cairo_move_to (impl->context, line.first.x, line.first.y);
				cairo_line_to (impl->context, line.second.x, line.second.y);
				cairo_stroke (impl->context);
			}
		}
	});
	return true;
}

//------------------------------------------------------------------------
bool CairoGraphicsDeviceContext::drawPolygon (const PointList& polygonPointList,
											  PlatformGraphicsDrawStyle drawStyle) const
{
	vstgui_assert (polygonPointList.empty () == false);
	impl->doInContext ([&] () {
		bool doPixelAlign = impl->state.drawMode.integralMode ();
		auto last = polygonPointList.back ();
		if (doPixelAlign)
			last = pixelAlign (impl->state.tm, last);
		cairo_move_to (impl->context, last.x, last.y);
		for (auto p : polygonPointList)
		{
			if (doPixelAlign)
				p = pixelAlign (impl->state.tm, p);
			cairo_line_to (impl->context, p.x, p.y);
		}
		impl->draw (drawStyle);
	});
	return true;
}

//------------------------------------------------------------------------
bool CairoGraphicsDeviceContext::drawRect (CRect rect, PlatformGraphicsDrawStyle drawStyle) const
{
	impl->doInContext ([&] () {
		if (drawStyle != PlatformGraphicsDrawStyle::Filled)
		{
			rect.right -= 1.;
			rect.bottom -= 1.;
		}
		if (impl->state.drawMode.integralMode ())
		{
			rect = pixelAlign (impl->state.tm, rect);
			if (drawStyle != PlatformGraphicsDrawStyle::Filled)
				impl->applyLineWidthCTM ();
			cairo_rectangle (impl->context, rect.left, rect.top, rect.getWidth (),
							 rect.getHeight ());
		}
		else
		{
			cairo_rectangle (impl->context, rect.left + 0.5, rect.top + 0.5, rect.getWidth () - 0.5,
							 rect.getHeight () - 0.5);
		}
		impl->draw (drawStyle);
	});
	return true;
}

//------------------------------------------------------------------------
bool CairoGraphicsDeviceContext::drawArc (CRect rect, double startAngle1, double endAngle2,
										  PlatformGraphicsDrawStyle drawStyle) const
{
	impl->doInContext ([&] () {
		CPoint center = rect.getCenter ();
		cairo_translate (impl->context, center.x, center.y);
		cairo_scale (impl->context, 2.0 / rect.getWidth (), 2.0 / rect.getHeight ());
		cairo_arc (impl->context, 0, 0, 1, startAngle1, endAngle2);
		impl->draw (drawStyle);
	});
	return true;
}

//------------------------------------------------------------------------
bool CairoGraphicsDeviceContext::drawEllipse (CRect rect, PlatformGraphicsDrawStyle drawStyle) const
{
	impl->doInContext ([&] () {
		CPoint center = rect.getCenter ();
		cairo_translate (impl->context, center.x, center.y);
		cairo_scale (impl->context, 2.0 / rect.getWidth (), 2.0 / rect.getHeight ());
		cairo_arc (impl->context, 0, 0, 1, 0, 2 * M_PI);
		impl->draw (drawStyle);
	});
	return true;
}

//------------------------------------------------------------------------
bool CairoGraphicsDeviceContext::drawPoint (CPoint point, CColor color) const { return false; }

//------------------------------------------------------------------------
bool CairoGraphicsDeviceContext::drawBitmap (IPlatformBitmap& bitmap, CRect dest, CPoint offset,
											 double alpha, BitmapInterpolationQuality quality) const
{
	auto cairoBitmap = dynamic_cast<Cairo::Bitmap*> (&bitmap);
	if (!cairoBitmap)
		return false;
	impl->doInContext ([&] () {
		cairo_translate (impl->context, dest.left, dest.top);
		cairo_rectangle (impl->context, 0, 0, dest.getWidth (), dest.getHeight ());
		cairo_clip (impl->context);

		// Setup a pattern for scaling bitmaps and take it as source afterwards.
		auto pattern = cairo_pattern_create_for_surface (cairoBitmap->getSurface ());
		cairo_matrix_t matrix;
		cairo_pattern_get_matrix (pattern, &matrix);
		cairo_matrix_init_scale (&matrix, cairoBitmap->getScaleFactor (),
								 cairoBitmap->getScaleFactor ());
		cairo_matrix_translate (&matrix, offset.x, offset.y);
		cairo_pattern_set_matrix (pattern, &matrix);
		cairo_set_source (impl->context, pattern);

		cairo_rectangle (impl->context, -offset.x, -offset.y, dest.getWidth () + offset.x,
						 dest.getHeight () + offset.y);
		alpha *= impl->state.globalAlpha;
		if (alpha != 1.f)
		{
			cairo_paint_with_alpha (impl->context, alpha);
		}
		else
		{
			cairo_fill (impl->context);
		}
		cairo_pattern_destroy (pattern);
	});
	return true;
}

//------------------------------------------------------------------------
bool CairoGraphicsDeviceContext::clearRect (CRect rect) const
{
	impl->doInContext ([&] () {
		cairo_set_operator (impl->context, CAIRO_OPERATOR_CLEAR);
		cairo_rectangle (impl->context, rect.left, rect.top, rect.getWidth (), rect.getHeight ());
		cairo_fill (impl->context);
	});
	return true;
}

//------------------------------------------------------------------------
bool CairoGraphicsDeviceContext::drawGraphicsPath (IPlatformGraphicsPath& path,
												   PlatformGraphicsPathDrawMode mode,
												   TransformMatrix* transformation) const
{
	auto cairoPath = dynamic_cast<Cairo::GraphicsPath*> (&path);
	if (!cairoPath)
		return false;
	impl->doInContext ([&] () {
		std::unique_ptr<Cairo::GraphicsPath> alignedPath;
		if (impl->state.drawMode.integralMode ())
		{
			alignedPath = cairoPath->copyPixelAlign ([&] (CPoint p) {
				p = pixelAlign (impl->state.tm, p);
				return p;
			});
		}
		auto p = alignedPath ? alignedPath->getCairoPath () : cairoPath->getCairoPath ();
		if (transformation)
		{
			cairo_matrix_t currentMatrix;
			cairo_matrix_t resultMatrix;
			auto matrix = convert (*transformation);
			cairo_get_matrix (impl->context, &currentMatrix);
			cairo_matrix_multiply (&resultMatrix, &matrix, &currentMatrix);
			cairo_set_matrix (impl->context, &resultMatrix);
		}
		cairo_append_path (impl->context, p);
		switch (mode)
		{
			case PlatformGraphicsPathDrawMode::Filled:
			{
				impl->applyFillColor ();
				cairo_fill (impl->context);
				break;
			}
			case PlatformGraphicsPathDrawMode::FilledEvenOdd:
			{
				impl->applyFillColor ();
				cairo_set_fill_rule (impl->context, CAIRO_FILL_RULE_EVEN_ODD);
				cairo_fill (impl->context);
				break;
			}
			case PlatformGraphicsPathDrawMode::Stroked:
			{
				impl->applyLineStyle ();
				impl->applyFrameColor ();
				cairo_stroke (impl->context);
				break;
			}
		}
	});
	return true;
}

//------------------------------------------------------------------------
bool CairoGraphicsDeviceContext::fillLinearGradient (IPlatformGraphicsPath& path,
													 const IPlatformGradient& gradient,
													 CPoint startPoint, CPoint endPoint,
													 bool evenOdd,
													 TransformMatrix* transformation) const
{
	auto cairoPath = dynamic_cast<Cairo::GraphicsPath*> (&path);
	if (!cairoPath)
		return false;
	auto cairoGradient = dynamic_cast<const Cairo::Gradient*> (&gradient);
	if (!cairoGradient)
		return false;
	impl->doInContext ([&] () {
		std::unique_ptr<Cairo::GraphicsPath> alignedPath;
		if (impl->state.drawMode.integralMode ())
		{
			alignedPath = cairoPath->copyPixelAlign ([&] (CPoint p) {
				p = pixelAlign (impl->state.tm, p);
				return p;
			});
		}
		auto p = alignedPath ? alignedPath->getCairoPath () : cairoPath->getCairoPath ();
		cairo_append_path (impl->context, p);
		cairo_set_source (impl->context, cairoGradient->getLinearGradient (startPoint, endPoint));
		if (evenOdd)
			cairo_set_fill_rule (impl->context, CAIRO_FILL_RULE_EVEN_ODD);
		cairo_fill (impl->context);
	});
	return true;
}

//------------------------------------------------------------------------
bool CairoGraphicsDeviceContext::fillRadialGradient (IPlatformGraphicsPath& path,
													 const IPlatformGradient& gradient,
													 CPoint center, CCoord radius,
													 CPoint originOffset, bool evenOdd,
													 TransformMatrix* transformation) const
{
	auto cairoPath = dynamic_cast<Cairo::GraphicsPath*> (&path);
	if (!cairoPath)
		return false;
	// TODO: Implementation
	return false;
}

//------------------------------------------------------------------------
void CairoGraphicsDeviceContext::saveGlobalState () const
{
	cairo_save (impl->context);
	impl->stateStack.push (impl->state);
}

//------------------------------------------------------------------------
void CairoGraphicsDeviceContext::restoreGlobalState () const
{
	vstgui_assert (impl->stateStack.empty () == false,
				   "Unbalanced calls to saveGlobalState and restoreGlobalState");
#if NDEBUG
	if (impl->stateStack.empty ())
		return;
#endif
	cairo_restore (impl->context);
	impl->state = impl->stateStack.top ();
	impl->stateStack.pop ();
}

//------------------------------------------------------------------------
void CairoGraphicsDeviceContext::setLineStyle (const CLineStyle& style) const
{
	impl->state.lineStyle = style;
}

//------------------------------------------------------------------------
void CairoGraphicsDeviceContext::setLineWidth (CCoord width) const
{

	impl->state.lineWidth = width;
}

//------------------------------------------------------------------------
void CairoGraphicsDeviceContext::setDrawMode (CDrawMode mode) const { impl->state.drawMode = mode; }

//------------------------------------------------------------------------
void CairoGraphicsDeviceContext::setClipRect (CRect clip) const { impl->state.clip = clip; }

//------------------------------------------------------------------------
void CairoGraphicsDeviceContext::setFillColor (CColor color) const
{
	impl->state.fillColor = color;
}

//------------------------------------------------------------------------
void CairoGraphicsDeviceContext::setFrameColor (CColor color) const
{
	impl->state.frameColor = color;
}

//------------------------------------------------------------------------
void CairoGraphicsDeviceContext::setGlobalAlpha (double newAlpha) const
{
	impl->state.globalAlpha = newAlpha;
}

//------------------------------------------------------------------------
void CairoGraphicsDeviceContext::setTransformMatrix (const TransformMatrix& tm) const
{
	impl->state.tm = tm;
}

//------------------------------------------------------------------------
const IPlatformGraphicsDeviceContextBitmapExt* CairoGraphicsDeviceContext::asBitmapExt () const
{
	return nullptr;
}

//------------------------------------------------------------------------
void CairoGraphicsDeviceContext::drawPangoLayout (void* layout, CPoint pos, CColor color) const
{
	impl->doInContext ([&] () {
		impl->applyFontColor (color);
		cairo_move_to (impl->context, pos.x, pos.y);
		pango_cairo_show_layout (impl->context, reinterpret_cast<PangoLayout*> (layout));
	});
}

//------------------------------------------------------------------------
} // VSTGUI
