// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "drawcontextobject.h"
#include "converters.h"
#include "../../lib/cdrawcontext.h"
#include "../../lib/cgraphicspath.h"
#include "../../uidescription/uidescription.h"
#include "../../uidescription/uiviewcreator.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ScriptingInternal {

using namespace std::literals;
using namespace TJS;

//------------------------------------------------------------------------
static SharedPointer<CGraphicsPath> getGraphicsPath (CScriptVar* var, std::string_view varName,
													 std::string_view signature)
{
	auto pathVar = getArgument (var, varName, signature);
	auto customData = pathVar->getCustomData ();
	if (!customData.has_value ())
		throw CScriptException ("Variable is not a graphics path");
	try
	{
		auto path = std::any_cast<SharedPointer<CGraphicsPath>> (customData);
		return path;
	}
	catch (const std::bad_any_cast& e)
	{
		throw CScriptException ("Variable is not a graphics path");
	}
	return {};
}

//------------------------------------------------------------------------
static CRect getRect (CScriptVar* var, std::string_view varName, std::string_view signature)
{
	auto rectVar = getArgument (var, varName, signature);
	auto rect = fromScriptRect (*rectVar);
	return rect;
}

//------------------------------------------------------------------------
static CPoint getPoint (CScriptVar* var, std::string_view varName, std::string_view signature)
{
	auto pointVar = getArgument (var, varName, signature);
	auto point = fromScriptPoint (*pointVar);
	return point;
}

//------------------------------------------------------------------------
static double getDouble (CScriptVar* var, std::string_view varName, std::string_view signature)
{
	auto doubleVar = getArgument (var, varName, signature);
	if (!doubleVar->isNumeric ())
	{
		TJS::string s ("'");
		s.append (varName);
		s.append ("' must be a number");
		throw CScriptException (s);
	}
	return doubleVar->getDouble ();
}

//------------------------------------------------------------------------
static int64_t getInt (CScriptVar* var, std::string_view varName, std::string_view signature)
{
	auto intVar = getArgument (var, varName, signature);
	if (!intVar->isNumeric ())
	{
		TJS::string s ("'");
		s.append (varName);
		s.append ("' must be numeric");
		throw CScriptException (s);
	}
	return intVar->getInt ();
}

//------------------------------------------------------------------------
struct GraphicsPathScriptObject : ScriptObject
{
	GraphicsPathScriptObject (const SharedPointer<CGraphicsPath>& p)
	{
		scriptVar = new CScriptVar ("", SCRIPTVAR_OBJECT);
		scriptVar->addRef ();
		scriptVar->setCustomData (p);

		addFunc ("addArc"sv, [p] (auto var) { addArc (p, var); },
				 {"rect"sv, "startAngle"sv, "endAngle"sv, "clockwise"sv});
		addFunc ("addEllipse"sv, [p] (auto var) { addEllipse (p, var); }, {"rect"sv});
		addFunc ("addRect"sv, [p] (auto var) { addRect (p, var); }, {"rect"sv});
		addFunc ("addLine"sv, [p] (auto var) { addLine (p, var); }, {"to"sv});
		addFunc ("addBezierCurve"sv, [p] (auto var) { addBezierCurve (p, var); },
				 {"control1"sv, "control2"sv, "end"sv});
		addFunc ("beginSubpath"sv, [p] (auto var) { beginSubpath (p, var); }, {"start"sv});
		addFunc ("closeSubpath"sv, [p] (auto var) { closeSubpath (p); });
		addFunc ("addRoundRect"sv, [p] (auto var) { addRoundRect (p, var); },
				 {"rect"sv, "radius"sv});
	}

	static void addArc (const SharedPointer<CGraphicsPath>& path, CScriptVar* var)
	{
		static constexpr auto signature = "path.addArc(rect, startAngle, endAngle, clockwise);"sv;
		auto rect = getRect (var, "rect"sv, signature);
		auto startAngle = getDouble (var, "startAngle"sv, signature);
		auto endAngle = getDouble (var, "endAngle"sv, signature);
		auto clockwise = getInt (var, "clockwise"sv, signature);
		path->addArc (rect, startAngle, endAngle, clockwise != 0 ? true : false);
	}

	static void addEllipse (const SharedPointer<CGraphicsPath>& path, CScriptVar* var)
	{
		static constexpr auto signature = "path.addEllipse(rect);"sv;
		auto rect = getRect (var, "rect"sv, signature);
		path->addEllipse (rect);
	}

	static void addRect (const SharedPointer<CGraphicsPath>& path, CScriptVar* var)
	{
		static constexpr auto signature = "path.addRect(rect);"sv;
		auto rect = getRect (var, "rect"sv, signature);
		path->addRect (rect);
	}

	static void addLine (const SharedPointer<CGraphicsPath>& path, CScriptVar* var)
	{
		static constexpr auto signature = "path.addLine(to);"sv;
		auto point = getPoint (var, "to"sv, signature);
		path->addLine (point);
	}

	static void addBezierCurve (const SharedPointer<CGraphicsPath>& path, CScriptVar* var)
	{
		static constexpr auto signature = "path.addBezierCurve(control1, control2, end);"sv;
		auto control1 = getPoint (var, "control1"sv, signature);
		auto control2 = getPoint (var, "control2"sv, signature);
		auto end = getPoint (var, "end"sv, signature);
		path->addBezierCurve (control1, control2, end);
	}

	static void beginSubpath (const SharedPointer<CGraphicsPath>& path, CScriptVar* var)
	{
		static constexpr auto signature = "path.beginSubpath(start);"sv;
		auto start = getPoint (var, "start"sv, signature);
		path->beginSubpath (start);
	}

	static void closeSubpath (const SharedPointer<CGraphicsPath>& path)
	{
		static constexpr auto signature = "path.closeSubpath();"sv;
		path->closeSubpath ();
	}

	static void addRoundRect (const SharedPointer<CGraphicsPath>& path, CScriptVar* var)
	{
		static constexpr auto signature = "path.addRoundRect(rect, radius);"sv;
		auto rect = getRect (var, "rect"sv, signature);
		auto radius = getDouble (var, "radius"sv, signature);
		path->addRoundRect (rect, radius);
	}
};

//------------------------------------------------------------------------
struct DrawContextObject::Impl
{
	CDrawContext* context {nullptr};
	IUIDescription* uiDesc {nullptr};
	mutable int32_t globalStatesStored {0};

	void setContext (CDrawContext* inContext, IUIDescription* inUIDesc)
	{
		if (context)
		{
			while (globalStatesStored > 0)
			{
				context->restoreGlobalState ();
				--globalStatesStored;
			}
		}
		context = inContext;
		uiDesc = inUIDesc;
		globalStatesStored = 0;
	}

	void checkContextOrThrow () const
	{
		if (!context)
			throw CScriptException ("Native context is missing!");
	}

	CDrawStyle getDrawStyle (CScriptVar* styleVar) const
	{
		CDrawStyle style {};
		auto string = styleVar->getString ();
		if (string == "stroked"sv)
			style = kDrawStroked;
		else if (string == "filled"sv)
			style = kDrawFilled;
		else if (string == "filledAndStroked"sv)
			style = kDrawFilledAndStroked;
		else
			throw CScriptException ("Unknown draw style: " + string);
		return style;
	}

	CDrawContext::PathDrawMode getPathDrawMode (CScriptVar* var) const
	{
		CDrawContext::PathDrawMode mode {};
		auto string = var->getString ();
		if (string == "stroked"sv)
			mode = CDrawContext::PathDrawMode::kPathStroked;
		else if (string == "filled"sv)
			mode = CDrawContext::PathDrawMode::kPathFilled;
		else if (string == "filledEvenOdd"sv)
			mode = CDrawContext::PathDrawMode::kPathFilledEvenOdd;
		else
			throw CScriptException ("Unknown path draw mode: " + string);
		return mode;
	}

	void createRoundGraphicsPath (CScriptVar* var) const
	{
		static constexpr auto signature = "drawContext.createRoundGraphicsPath(rect, radius);"sv;
		checkContextOrThrow ();
		auto rect = getRect (var, "rect"sv, signature);
		auto radius = getDouble (var, "radius"sv, signature);
		if (auto path = owned (context->createRoundRectGraphicsPath (rect, radius)))
		{
			GraphicsPathScriptObject obj (path);
			var->setReturnVar (obj.getVar ());
		}
	}

	void createGraphicsPath (CScriptVar* var) const
	{
		static constexpr auto signature = "drawContext.createGraphicsPath();"sv;
		checkContextOrThrow ();
		if (auto path = owned (context->createGraphicsPath ()))
		{
			GraphicsPathScriptObject obj (path);
			var->setReturnVar (obj.getVar ());
		}
	}

	void drawGraphicsPath (CScriptVar* var) const
	{
		static constexpr auto signature =
			"drawContext.drawGraphicsPath(path, mode?, transform?);"sv;
		checkContextOrThrow ();
		auto path = getGraphicsPath (var, "path"sv, signature);
		auto modeVar = getOptionalArgument (var, "mode?");
		auto mode = modeVar ? getPathDrawMode (modeVar) : CDrawContext::PathDrawMode::kPathFilled;
		context->drawGraphicsPath (path, mode);
	}

	void drawLine (CScriptVar* var) const
	{
		static constexpr auto signature = "drawContext.drawLine(from, to);"sv;
		checkContextOrThrow ();
		auto fromPoint = getArgument (var, "from"sv, signature);
		auto toPoint = getArgument (var, "to"sv, signature);
		auto from = fromScriptPoint (*fromPoint);
		auto to = fromScriptPoint (*toPoint);
		context->drawLine (from, to);
	}
	void drawRect (CScriptVar* var) const
	{
		static constexpr auto signature = "drawContext.drawRect(rect, style);"sv;
		checkContextOrThrow ();
		auto rect = getRect (var, "rect"sv, signature);
		auto styleVar = getArgument (var, "style"sv, signature);
		auto style = getDrawStyle (styleVar);
		context->drawRect (rect, style);
	}
	void drawEllipse (CScriptVar* var) const
	{
		static constexpr auto signature = "drawContext.drawEllipse(rect, style);"sv;
		checkContextOrThrow ();
		auto rect = getRect (var, "rect"sv, signature);
		auto styleVar = getArgument (var, "style"sv, signature);
		auto style = getDrawStyle (styleVar);
		context->drawEllipse (rect, style);
	}
	void drawArc (CScriptVar* var) const
	{
		static constexpr auto signature =
			"drawContext.drawArc(rect, startAngle, endAngle, style);"sv;
		checkContextOrThrow ();
		auto rect = getRect (var, "rect"sv, signature);
		auto startAngle = getDouble (var, "startAngle"sv, signature);
		auto endAngle = getDouble (var, "endAngle"sv, signature);
		auto styleVar = getArgument (var, "style"sv, signature);
		auto style = getDrawStyle (styleVar);
		context->drawArc (rect, startAngle, endAngle, style);
	}
	void clearRect (CScriptVar* var) const
	{
		static constexpr auto signature = "drawContext.clearRect(rect);"sv;
		checkContextOrThrow ();
		auto rect = getRect (var, "rect"sv, signature);
		context->clearRect (rect);
	}
	void drawPolygon (CScriptVar* var) const
	{
		static constexpr auto signature = "drawContext.drawPolygon(points, style);"sv;
		checkContextOrThrow ();
		auto pointsVar = getArgument (var, "points"sv, signature);
		auto styleVar = getArgument (var, "style"sv, signature);
		if (!pointsVar->isArray ())
			throw CScriptException ("`points` argument must be an array of points in "
									"drawContext.drawPolygon(points, style);");
		PointList points;
		auto numPoints = pointsVar->getArrayLength ();
		for (auto index = 0; index < numPoints; ++index)
		{
			auto pointVar = pointsVar->getArrayIndex (index);
			vstgui_assert (pointVar != nullptr);
			points.emplace_back (fromScriptPoint (*pointVar));
		}
		auto style = getDrawStyle (styleVar);
		context->drawPolygon (points, style);
	}
	void setClipRect (CScriptVar* var) const
	{
		static constexpr auto signature = "drawContext.setClipRect(rect);"sv;
		checkContextOrThrow ();
		auto rect = getRect (var, "rect"sv, signature);
		context->setClipRect (rect);
	}
	void drawBitmap (CScriptVar* var) const
	{
		static constexpr auto signature =
			"drawContext.drawBitmap(name, destRect, offsetPoint?, alpha?);"sv;
		checkContextOrThrow ();
		auto nameVar = getArgument (var, "name"sv, signature);
		auto destRect = getRect (var, "destRect"sv, signature);
		auto offsetPointVar = getOptionalArgument (var, "offsetPoint?"sv);
		auto alphaVar = getOptionalArgument (var, "alpha?"sv);
		auto bitmap = uiDesc->getBitmap (nameVar->getString ().data ());
		if (!bitmap)
			throw CScriptException ("bitmap not found in uiDescription");
		auto offset = offsetPointVar ? fromScriptPoint (*offsetPointVar) : CPoint (0, 0);
		auto alpha = static_cast<float> (alphaVar ? alphaVar->getDouble () : 1.);
		context->drawBitmap (bitmap, destRect, offset, alpha);
	}
	void drawString (CScriptVar* var) const
	{
		static constexpr auto signature = "drawContext.drawString(string, rect, align?);"sv;
		checkContextOrThrow ();
		auto stringVar = getArgument (var, "string"sv, signature);
		auto rect = getRect (var, "rect"sv, signature);
		auto alignVar = getOptionalArgument (var, "align?"sv);
		auto string = stringVar->getString ().data ();
		CHoriTxtAlign align = kCenterText;
		if (!alignVar || alignVar->isUndefined ())
			align = kCenterText;
		else if (alignVar->getString () == "left"sv)
			align = kLeftText;
		else if (alignVar->getString () == "center"sv)
			align = kCenterText;
		else if (alignVar->getString () == "right"sv)
			align = kRightText;
		else
			throw CScriptException (
				"wrong `align` argument. Expecting 'left', 'center' or 'right'");
		context->drawString (string, rect, align, true);
	}
	void setFont (CScriptVar* var) const
	{
		static constexpr auto signature = "drawContext.setFont(font);"sv;
		checkContextOrThrow ();
		auto fontVar = getArgument (var, "font"sv, signature);
		if (auto font = uiDesc->getFont (fontVar->getString ().data ()))
			context->setFont (font);
	}
	void setFontColor (CScriptVar* var) const
	{
		static constexpr auto signature = "drawContext.setFontColor(color);"sv;
		checkContextOrThrow ();
		auto colorVar = getArgument (var, "color"sv, signature);
		CColor color {};
		UIViewCreator::stringToColor (colorVar->getString (), color, uiDesc);
		context->setFontColor (color);
	}
	void setFillColor (CScriptVar* var) const
	{
		static constexpr auto signature = "drawContext.setFillColor(color);"sv;
		checkContextOrThrow ();
		auto colorVar = getArgument (var, "color"sv, signature);
		CColor color {};
		UIViewCreator::stringToColor (colorVar->getString (), color, uiDesc);
		context->setFillColor (color);
	}
	void setFrameColor (CScriptVar* var) const
	{
		static constexpr auto signature = "drawContext.setFrameColor(color);"sv;
		checkContextOrThrow ();
		auto colorVar = getArgument (var, "color"sv, signature);
		CColor color {};
		if (!UIViewCreator::stringToColor (colorVar->getString (), color, uiDesc))
			throw CScriptException (
				"Unknown `color` argument in drawContext.setFrameColor(color);");
		context->setFrameColor (color);
	}
	void setLineWidth (CScriptVar* var) const
	{
		static constexpr auto signature = "drawContext.setLineWidth(width);"sv;
		checkContextOrThrow ();
		auto widthVar = getArgument (var, "width"sv, signature);
		context->setLineWidth (widthVar->getDouble ());
	}
	void setLineStyle (CScriptVar* var) const
	{
		static constexpr auto signature =
			"drawContext.setLineStyle(styleOrLineCap, lineJoin?, dashLengths?, dashPhase);"sv;
		checkContextOrThrow ();
		auto styleOrLineCapVar = getArgument (var, "styleOrLineCap"sv, signature);
		auto lineJoinVar = getOptionalArgument (var, "lineJoin?"sv);
		auto dashLengthsVar = getOptionalArgument (var, "dashLengths?"sv);
		auto dashPhaseVar = getOptionalArgument (var, "dashPhase?"sv);
		auto styleOrLineCap = styleOrLineCapVar->getString ();
		std::unique_ptr<CLineStyle> lineStyle;
		if (styleOrLineCap == "solid"sv)
		{
			lineStyle = std::make_unique<CLineStyle> (kLineSolid);
		}
		else if (styleOrLineCap == "dotted"sv)
		{
			lineStyle = std::make_unique<CLineStyle> (kLineOnOffDash);
		}
		else
		{
			if (styleOrLineCap == "butt"sv)
				lineStyle = std::make_unique<CLineStyle> (CLineStyle::LineCap::kLineCapButt);
			else if (styleOrLineCap == "round"sv)
				lineStyle = std::make_unique<CLineStyle> (CLineStyle::LineCap::kLineCapRound);
			else if (styleOrLineCap == "square"sv)
				lineStyle = std::make_unique<CLineStyle> (CLineStyle::LineCap::kLineCapSquare);
			else
				throw CScriptException ("unknown `line cap` argument");
			if (lineJoinVar)
			{
				auto lineJoin = lineJoinVar->getString ();
				if (lineJoin == "miter"sv)
					lineStyle->setLineJoin (CLineStyle::LineJoin::kLineJoinMiter);
				else if (lineJoin == "round"sv)
					lineStyle->setLineJoin (CLineStyle::LineJoin::kLineJoinRound);
				else if (lineJoin == "bevel"sv)
					lineStyle->setLineJoin (CLineStyle::LineJoin::kLineJoinBevel);
			}
			if (dashLengthsVar)
			{
				if (!dashLengthsVar->isArray ())
					throw CScriptException ("`dashLengths` must be an array of numbers");
				CLineStyle::CoordVector lengths;
				auto numValues = dashLengthsVar->getArrayLength ();
				for (auto index = 0; index < numValues; ++index)
				{
					if (auto lengthVar = dashLengthsVar->getArrayIndex (index))
						lengths.push_back (lengthVar->getDouble ());
				}
				lineStyle->getDashLengths () = lengths;
			}
			if (dashPhaseVar)
			{
				lineStyle->setDashPhase (dashPhaseVar->getDouble ());
			}
		}
		if (lineStyle)
			context->setLineStyle (*lineStyle.get ());
	}
	void setGlobalAlpha (CScriptVar* var) const
	{
		static constexpr auto signature = "drawContext.setGlobalAlpha(alpha);"sv;
		checkContextOrThrow ();
		auto alpha = getDouble (var, "alpha"sv, signature);
		context->setGlobalAlpha (alpha);
	}
	void saveGlobalState () const
	{
		checkContextOrThrow ();
		context->saveGlobalState ();
		++globalStatesStored;
	}
	void restoreGlobalState () const
	{
		checkContextOrThrow ();
		context->restoreGlobalState ();
		--globalStatesStored;
	}
	void getStringWidth (CScriptVar* var) const
	{
		static constexpr auto signature = "drawContext.getStringWidth(string);"sv;
		checkContextOrThrow ();
		auto stringVar = getArgument (var, "string"sv, signature);
		auto width = context->getStringWidth (stringVar->getString ().data ());
		var->getReturnVar ()->setDouble (width);
	}
	void setDrawMode (CScriptVar* var) const
	{
		static constexpr auto signature = "drawContext.setDrawMode(mode);"sv;
		checkContextOrThrow ();
		auto modeVar = getArgument (var, "mode"sv, signature);
		if (modeVar->getString () == "aliasing"sv)
			context->setDrawMode (kAliasing);
		else if (modeVar->getString () == "anti-aliasing"sv)
			context->setDrawMode (kAntiAliasing);
		else
			throw CScriptException ("`mode` must be `aliasing` or `anti-aliasing`");
	}
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
DrawContextObject::DrawContextObject ()
{
	impl = std::make_unique<Impl> ();
	scriptVar->setLifeTimeObserver (this);
	addFunc ("clearRect"sv, [this] (auto var) { impl->clearRect (var); }, {"rect"sv, "style"sv});
	addFunc ("createRoundGraphicsPath"sv,
			 [this] (auto var) { impl->createRoundGraphicsPath (var); }, {"rect"sv, "radius"sv});
	addFunc ("createGraphicsPath"sv, [this] (auto var) { impl->createGraphicsPath (var); });
	addFunc ("getStringWidth"sv, [this] (auto var) { impl->getStringWidth (var); }, {"string"sv});
	addFunc ("drawArc"sv, [this] (auto var) { impl->drawArc (var); },
			 {"rect"sv, "startAngle"sv, "endAngle"sv, "style"sv});
	addFunc ("drawBitmap"sv, [this] (auto var) { impl->drawBitmap (var); },
			 {"name"sv, "destRect"sv, "offsetPoint?"sv, "alpha?"sv});
	addFunc ("drawEllipse"sv, [this] (auto var) { impl->drawEllipse (var); },
			 {"rect"sv, "style"sv});
	addFunc ("drawGraphicsPath"sv, [this] (auto var) { impl->drawGraphicsPath (var); },
			 {"path"sv, "mode?"sv, "transform?"sv});
	addFunc ("drawLine"sv, [this] (auto var) { impl->drawLine (var); }, {"from"sv, "to"sv});
	addFunc ("drawPolygon"sv, [this] (auto var) { impl->drawPolygon (var); },
			 {"points"sv, "style"sv});
	addFunc ("drawRect"sv, [this] (auto var) { impl->drawRect (var); }, {"rect"sv, "style"sv});
	addFunc ("drawString"sv, [this] (auto var) { impl->drawString (var); },
			 {"string"sv, "rect"sv, "align?"sv});
	addFunc ("restoreGlobalState"sv, [this] (auto var) { impl->restoreGlobalState (); });
	addFunc ("saveGlobalState"sv, [this] (auto var) { impl->saveGlobalState (); });
	addFunc ("setClipRect"sv, [this] (auto var) { impl->setClipRect (var); }, {"rect"sv});
	addFunc ("setFont"sv, [this] (auto var) { impl->setFont (var); }, {"font"sv});
	addFunc ("setFontColor"sv, [this] (auto var) { impl->setFontColor (var); }, {"color"sv});
	addFunc ("setFillColor"sv, [this] (auto var) { impl->setFillColor (var); }, {"color"sv});
	addFunc ("setFrameColor"sv, [this] (auto var) { impl->setFrameColor (var); }, {"color"sv});
	addFunc ("setGlobalAlpha", [this] (auto var) { impl->setGlobalAlpha (var); }, {"alpha"sv});
	addFunc ("setLineWidth"sv, [this] (auto var) { impl->setLineWidth (var); }, {"width"sv});
	addFunc ("setLineStyle"sv, [this] (auto var) { impl->setLineStyle (var); },
			 {"styleOrLineCap"sv, "lineJoin?"sv, "dashLengths?"sv, "dashPhase?"sv});
	addFunc ("setDrawMode"sv, [this] (auto var) { impl->setDrawMode (var); }, {"mode"sv});
}

//------------------------------------------------------------------------
DrawContextObject::~DrawContextObject () noexcept
{
	if (scriptVar)
		scriptVar->setLifeTimeObserver (nullptr);
}

//------------------------------------------------------------------------
void DrawContextObject::setDrawContext (CDrawContext* inContext, IUIDescription* inUIDesc)
{
	impl->setContext (inContext, inUIDesc);
}

//------------------------------------------------------------------------
void DrawContextObject::onDestroy (CScriptVar* v)
{
	v->setLifeTimeObserver (nullptr);
	scriptVar = nullptr;
}

//------------------------------------------------------------------------
} // ScriptingInternal
} // VSTGUI
