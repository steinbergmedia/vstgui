// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "drawcontextobject.h"
#include "converters.h"
#include "../../lib/cdrawcontext.h"
#include "../../lib/cgraphicspath.h"
#include "../../lib/cgradient.h"
#include "../../uidescription/uidescription.h"
#include "../../uidescription/uiviewcreator.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ScriptingInternal {

using namespace std::literals;
using namespace TJS;

//------------------------------------------------------------------------
template<typename T>
static T getFromVar (CScriptVar* var, const string& exceptionString)
{
	auto customData = var->getCustomData ();
	if (!customData.has_value ())
		throw CScriptException (exceptionString);
	try
	{
		auto result = std::any_cast<T> (customData);
		return result;
	}
	catch (const std::bad_any_cast& e)
	{
		throw CScriptException (exceptionString);
	}
	return {};
}

//------------------------------------------------------------------------
static SharedPointer<CGraphicsPath> getGraphicsPath (CScriptVar* var, std::string_view varName,
													 std::string_view signature)
{
	auto pathVar = getArgument (var, varName, signature);
	return getFromVar<SharedPointer<CGraphicsPath>> (pathVar, "Variable is not a graphics path");
}

//------------------------------------------------------------------------
static SharedPointer<CGradient> getGradient (CScriptVar* var, std::string_view varName,
											 std::string_view signature)
{
	auto gradientVar = getArgument (var, varName, signature);
	return getFromVar<SharedPointer<CGradient>> (gradientVar, "Variable is not a gradient");
}

//------------------------------------------------------------------------
static std::shared_ptr<CGraphicsTransform> getTransformMatrix (CScriptVar* var,
															   std::string_view varName,
															   std::string_view signature)
{
	auto tmVar = getArgument (var, varName, signature);
	return getFromVar<std::shared_ptr<CGraphicsTransform>> (tmVar,
															"Variable is not a transform matrix");
}

//------------------------------------------------------------------------
static std::shared_ptr<CGraphicsTransform> getOptionalTransformMatrix (CScriptVar* var,
																	   std::string_view varName,
																	   std::string_view signature)
{
	if (auto tmVar = getOptionalArgument (var, varName))
		return getFromVar<std::shared_ptr<CGraphicsTransform>> (
			tmVar, "Variable is not a transform matrix");
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
static CPoint getOptionalPoint (CScriptVar* var, std::string_view varName,
								std::string_view signature, CPoint defaultPoint = {})
{
	if (auto pointVar = getOptionalArgument (var, varName))
		return fromScriptPoint (*pointVar);
	return defaultPoint;
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
static int64_t getOptionalInt (CScriptVar* var, std::string_view varName,
							   std::string_view signature, int64_t defaultInt = {})
{
	if (auto intVar = getOptionalArgument (var, varName))
	{
		if (!intVar->isNumeric ())
		{
			TJS::string s ("'");
			s.append (varName);
			s.append ("' must be numeric");
			throw CScriptException (s);
		}
		return intVar->getInt ();
	}
	return defaultInt;
}

//------------------------------------------------------------------------
static CColor getColor (CScriptVar* var, const IUIDescription* uiDesc, std::string_view varName,
						std::string_view signature)
{
	auto colorVar = getArgument (var, varName, signature);
	auto colorStr = colorVar->getString ();
	CColor color {};
	if (!UIViewCreator::stringToColor (colorStr, color, uiDesc))
	{
		string str ("'");
		str += colorStr;
		str += "' is not a color in a call to ";
		str += signature;
		throw CScriptException (str);
	}
	return color;
}

//------------------------------------------------------------------------
struct TransformMatrixScriptObject : ScriptObject
{
	using TransformMatrixPtr = std::shared_ptr<CGraphicsTransform>;

	TransformMatrixScriptObject (TransformMatrixPtr tm = std::make_shared<CGraphicsTransform> ())
	{
		scriptVar = new CScriptVar ("", SCRIPTVAR_OBJECT);
		scriptVar->addRef ();
		scriptVar->setCustomData (tm);
		addFunc ("concat"sv, [tm] (auto var) { concat (tm, var); }, {"transformMatrix"sv});
		addFunc ("inverse"sv, [tm] (auto var) { inverse (tm, var); });
		addFunc ("rotate"sv, [tm] (auto var) { rotate (tm, var); }, {"angle"sv, "center?"sv});
		addFunc ("scale"sv, [tm] (auto var) { scale (tm, var); }, {"x"sv, "y"sv});
		addFunc ("skewX"sv, [tm] (auto var) { skewX (tm, var); }, {"skewX"sv});
		addFunc ("skewY"sv, [tm] (auto var) { skewY (tm, var); }, {"skewY"sv});
		addFunc ("translate"sv, [tm] (auto var) { translate (tm, var); }, {"x"sv, "y"sv});
		addFunc ("transform"sv, [tm] (auto var) { transform (tm, var); }, {"pointOrRect"sv});
	}

	static void inverse (const TransformMatrixPtr& tm, CScriptVar* var)
	{
		static constexpr auto signature = "matrix.inverse();"sv;
		auto iTm = tm->inverse ();
		TransformMatrixScriptObject obj (std::make_shared<CGraphicsTransform> (iTm));
		var->setReturnVar (obj.take ());
	}

	static void concat (const TransformMatrixPtr& tm, CScriptVar* var)
	{
		static constexpr auto signature = "matrix.concat(transformMatrix);"sv;
		auto other = getTransformMatrix (var, "transformMatrix"sv, signature);
		auto result = *(tm.get ()) * *(other.get ());
		*(tm.get ()) = result;
	}

	static void translate (const TransformMatrixPtr& tm, CScriptVar* var)
	{
		static constexpr auto signature = "matrix.translate(x, y);"sv;
		auto x = getDouble (var, "x"sv, signature);
		auto y = getDouble (var, "y"sv, signature);
		tm->translate (x, y);
	}

	static void scale (const TransformMatrixPtr& tm, CScriptVar* var)
	{
		static constexpr auto signature = "matrix.scale(x, y);"sv;
		auto x = getDouble (var, "x"sv, signature);
		auto y = getDouble (var, "y"sv, signature);
		tm->scale (x, y);
	}

	static void rotate (const TransformMatrixPtr& tm, CScriptVar* var)
	{
		static constexpr auto signature = "matrix.rotate(angle, center?);"sv;
		auto angle = getDouble (var, "angle"sv, signature);
		if (auto centerVar = getOptionalArgument (var, "center?"sv))
		{
			auto center = fromScriptPoint (*centerVar);
			tm->rotate (angle, center);
		}
		else
		{
			tm->rotate (angle);
		}
	}

	static void skewX (const TransformMatrixPtr& tm, CScriptVar* var)
	{
		static constexpr auto signature = "matrix.skewX(angle);"sv;
		auto angle = getDouble (var, "angle"sv, signature);
		tm->skewX (angle);
	}

	static void skewY (const TransformMatrixPtr& tm, CScriptVar* var)
	{
		static constexpr auto signature = "matrix.skewY(angle);"sv;
		auto angle = getDouble (var, "angle"sv, signature);
		tm->skewY (angle);
	}

	static void transform (const TransformMatrixPtr& tm, CScriptVar* var)
	{
		static constexpr auto signature = "matrix.transform(pointOrRect);"sv;
		auto pointOrRect = getArgument (var, "pointOrRect"sv, signature);
		auto xVar = getOptionalArgument (pointOrRect, "x"sv);
		auto yVar = getOptionalArgument (pointOrRect, "y"sv);
		if (xVar && yVar)
		{
			auto x = xVar->getDouble ();
			auto y = yVar->getDouble ();
			tm->transform (x, y);
			xVar->setDouble (x);
			yVar->setDouble (y);
			return;
		}
		auto leftVar = getOptionalArgument (pointOrRect, "left"sv);
		auto topVar = getOptionalArgument (pointOrRect, "top"sv);
		auto rightVar = getOptionalArgument (pointOrRect, "right"sv);
		auto bottomVar = getOptionalArgument (pointOrRect, "bottom"sv);
		if (leftVar && topVar && rightVar && bottomVar)
		{
			CRect r (leftVar->getDouble (), topVar->getDouble (), rightVar->getDouble (),
					 bottomVar->getDouble ());
			tm->transform (r);
			leftVar->setDouble (r.left);
			topVar->setDouble (r.top);
			rightVar->setDouble (r.right);
			bottomVar->setDouble (r.bottom);
			return;
		}

		string s ("Argument is not a point or a rect in ");
		s.append (signature);
		throw CScriptException (std::move (s));
	}
};

//------------------------------------------------------------------------
TJS::CScriptVar* makeTransformMatrixObject ()
{
	TransformMatrixScriptObject obj;
	return obj.take ();
}

//------------------------------------------------------------------------
struct GradientScriptObject : ScriptObject
{
	GradientScriptObject (const SharedPointer<CGradient>& g, const IUIDescription* uiDesc)
	{
		scriptVar = new CScriptVar ("", SCRIPTVAR_OBJECT);
		scriptVar->addRef ();
		scriptVar->setCustomData (g);

		addFunc ("addColorStop"sv, [g, uiDesc] (auto var) { addColorStop (g, uiDesc, var); },
				 {"position"sv, "color"sv});
	}

	static void addColorStop (const SharedPointer<CGradient>& g, const IUIDescription* uiDesc,
							  CScriptVar* var)
	{
		static constexpr auto signature = "gradient.addColorStop(position, color);"sv;
		auto position = getDouble (var, "position"sv, signature);
		auto color = getColor (var, uiDesc, "color"sv, signature);
		g->addColorStop (position, color);
	}
};

//------------------------------------------------------------------------
struct GraphicsPathScriptObject : ScriptObject
{
	GraphicsPathScriptObject (const SharedPointer<CGraphicsPath>& p)
	{
		scriptVar = new CScriptVar ("", SCRIPTVAR_OBJECT);
		scriptVar->addRef ();
		scriptVar->setCustomData (p);

		addFunc ("addEllipse"sv, [p] (auto var) { addEllipse (p, var); }, {"rect"sv});
		addFunc ("addArc"sv, [p] (auto var) { addArc (p, var); },
				 {"rect"sv, "startAngle"sv, "endAngle"sv, "clockwise"sv});
		addFunc ("addBezierCurve"sv, [p] (auto var) { addBezierCurve (p, var); },
				 {"control1"sv, "control2"sv, "end"sv});
		addFunc ("addLine"sv, [p] (auto var) { addLine (p, var); }, {"to"sv});
		addFunc ("addPath"sv, [p] (auto var) { addPath (p, var); },
				 {"path"sv, "transformMatrix?"sv});
		addFunc ("addRect"sv, [p] (auto var) { addRect (p, var); }, {"rect"sv});
		addFunc ("addRoundRect"sv, [p] (auto var) { addRoundRect (p, var); },
				 {"rect"sv, "radius"sv});
		addFunc ("closeSubpath"sv, [p] (auto var) { closeSubpath (p); });
		addFunc ("beginSubpath"sv, [p] (auto var) { beginSubpath (p, var); }, {"start"sv});
	}

	static void addPath (const SharedPointer<CGraphicsPath>& path, CScriptVar* var)
	{
		static constexpr auto signature = "path.addPath(path, transformMatrix?);"sv;
		auto otherPath = getGraphicsPath (var, "path"sv, signature);
		auto tm = getOptionalTransformMatrix (var, "transformMatrix?"sv, signature);
		path->addPath (*otherPath.get (), tm ? tm.get () : nullptr);
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

	void createGradient (CScriptVar* var) const
	{
		static constexpr auto signature =
			"drawContext.createGradient(startColorPosition, startColor, endColorPosition, endColor);"sv;
		auto startColorPosition = getDouble (var, "startColorPosition", signature);
		auto endColorPosition = getDouble (var, "endColorPosition", signature);
		auto startColor = getColor (var, uiDesc, "startColor"sv, signature);
		auto endColor = getColor (var, uiDesc, "endColor"sv, signature);
		if (auto gradient = owned (
				CGradient::create (startColorPosition, endColorPosition, startColor, endColor)))
		{
			GradientScriptObject obj (gradient, uiDesc);
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
		auto tm = getOptionalTransformMatrix (var, "transform?"sv, signature);
		context->drawGraphicsPath (path, mode, tm ? tm.get () : nullptr);
	}

	void fillLinearGradient (CScriptVar* var) const
	{
		static constexpr auto signature =
			"drawContext.fillLinearGradient(path, gradient, startPoint, endPoint, evenOdd?, transform?);"sv;
		checkContextOrThrow ();
		auto path = getGraphicsPath (var, "path"sv, signature);
		auto gradient = getGradient (var, "gradient"sv, signature);
		auto startPoint = getPoint (var, "startPoint"sv, signature);
		auto endPoint = getPoint (var, "endPoint"sv, signature);
		auto tm = getOptionalTransformMatrix (var, "transform?"sv, signature);
		auto evenOdd = getOptionalInt (var, "evenOdd?", signature);
		context->fillLinearGradient (path, *gradient, startPoint, endPoint, evenOdd > 0, tm.get ());
	}

	void fillRadialGradient (CScriptVar* var) const
	{
		static constexpr auto signature =
			"drawContext.fillRadialGradient(path, gradient, centerPoint, radius, originOffsetPoint?, evenOdd?, transform?);"sv;
		checkContextOrThrow ();
		auto path = getGraphicsPath (var, "path"sv, signature);
		auto gradient = getGradient (var, "gradient"sv, signature);
		auto centerPoint = getPoint (var, "centerPoint"sv, signature);
		auto radius = getDouble (var, "radius"sv, signature);
		auto originOffsetPoint = getOptionalPoint (var, "originOffsetPoint?"sv, signature);
		auto evenOdd = getOptionalInt (var, "evenOdd?", signature);
		auto tm = getOptionalTransformMatrix (var, "transform?"sv, signature);
		context->fillRadialGradient (path, *gradient, centerPoint, radius, originOffsetPoint,
									 evenOdd, tm.get ());
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
		auto color = getColor (var, uiDesc, "color"sv, signature);
		context->setFillColor (color);
	}
	void setFrameColor (CScriptVar* var) const
	{
		static constexpr auto signature = "drawContext.setFrameColor(color);"sv;
		checkContextOrThrow ();
		auto color = getColor (var, uiDesc, "color"sv, signature);
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
	addFunc ("createGradient"sv, [this] (auto var) { impl->createGradient (var); },
			 {"startColorPosition"sv, "startColor"sv, "endColorPosition"sv, "endColor"sv});
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
	addFunc ("fillLinearGradient"sv, [this] (auto var) { impl->fillLinearGradient (var); },
			 {"path"sv, "gradient"sv, "startPoint"sv, "endPoint"sv, "evenOdd?"sv, "transform?"sv});
	addFunc ("fillRadialGradient"sv, [this] (auto var) { impl->fillRadialGradient (var); },
			 {"path"sv, "gradient"sv, "centerPoint"sv, "radius"sv, "originOffsetPoint?"sv,
			  "evenOdd?"sv, "transform?"sv});
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
