// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "drawcontextobject.h"
#include "converters.h"
#include "../../lib/cdrawcontext.h"
#include "../../uidescription/uidescription.h"
#include "../../uidescription/uiviewcreator.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ScriptingInternal {

using namespace std::literals;
using namespace TJS;

//------------------------------------------------------------------------
//------------------------------------------------------------------------
struct DrawContextObject::Impl
{
	CDrawContext* context {nullptr};
	IUIDescription* uiDesc {nullptr};

	CScriptVar* getArgument (CScriptVar* var, std::string_view paramName,
							 std::string_view funcSignature) const
	{
		auto child = var->findChild (paramName);
		auto result = child ? child->getVar () : nullptr;
		if (!result || result->isUndefined ())
		{
			string s ("Missing `");
			s.append (paramName);
			s.append ("` argument in ");
			s.append (funcSignature);
			throw CScriptException (std::move (s));
		}
		return result;
	}

	void checkContextOrThrow () const
	{
		if (!context)
			throw CScriptException ("Native context is missing!");
	}

	CDrawStyle getDrawStyle (CScriptVar*& styleVar) const
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

	void drawLine (CScriptVar* var) const
	{
		checkContextOrThrow ();
		auto fromPoint = getArgument (var, "from"sv, "drawContext.drawLine(from, to);"sv);
		auto toPoint = getArgument (var, "to"sv, "drawContext.drawLine(from, to);"sv);
		auto from = fromScriptPoint (*fromPoint);
		auto to = fromScriptPoint (*toPoint);
		context->drawLine (from, to);
	}
	void drawRect (CScriptVar* var) const
	{
		checkContextOrThrow ();
		auto rectVar = getArgument (var, "rect"sv, "drawContext.drawRect(rect, style);"sv);
		auto styleVar = getArgument (var, "style"sv, "drawContext.drawRect(rect, style);"sv);
		auto rect = fromScriptRect (*rectVar);
		auto style = getDrawStyle (styleVar);
		context->drawRect (rect, style);
	}
	void drawEllipse (CScriptVar* var) const
	{
		checkContextOrThrow ();
		auto rectVar = getArgument (var, "rect"sv, "drawContext.drawEllipse(rect, style);"sv);
		auto styleVar = getArgument (var, "style"sv, "drawContext.drawEllipse(rect, style);"sv);
		auto rect = fromScriptRect (*rectVar);
		auto style = getDrawStyle (styleVar);
		context->drawEllipse (rect, style);
	}
	void clearRect (CScriptVar* var) const
	{
		checkContextOrThrow ();
		auto rectVar = getArgument (var, "rect"sv, "drawContext.clearRect(rect);");
		auto rect = fromScriptRect (*rectVar);
		context->clearRect (rect);
	}
	void drawPolygon (CScriptVar* var) const
	{
		checkContextOrThrow ();
		auto pointsVar = getArgument (var, "points"sv, "drawContext.drawPolygon(points, style);"sv);
		if (!pointsVar->isArray ())
			throw CScriptException ("`points` argument must be an array of points in "
									"drawContext.drawPolygon(points, style);");
		auto styleVar = getArgument (var, "style"sv, "drawContext.drawPolygon(points, style);"sv);
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
		checkContextOrThrow ();
		auto rectVar = getArgument (var, "rect"sv, "drawContext.setClipRect(rect);"sv);
		auto rect = fromScriptRect (*rectVar);
		context->setClipRect (rect);
	}
	void drawBitmap (CScriptVar* var) const
	{
		checkContextOrThrow ();
		auto nameVar = getArgument (
			var, "name"sv, "drawContext.drawBitmap(name, destRect, offsetPoint?, alpha?);"sv);
		auto destRectVar = getArgument (
			var, "destRect"sv, "drawContext.drawBitmap(name, destRect, offsetPoint?, alpha?);"sv);
		auto offsetPointVar = var->findChild ("offsetPoint"sv);
		auto alphaVar = var->findChild ("alpha"sv);
		auto bitmap = uiDesc->getBitmap (nameVar->getString ().data ());
		if (!bitmap)
			throw CScriptException ("bitmap not found in uiDescription");
		auto destRect = fromScriptRect (*destRectVar);
		auto offset = offsetPointVar ? fromScriptPoint (*offsetPointVar->getVar ()) : CPoint (0, 0);
		auto alpha = static_cast<float> (alphaVar ? alphaVar->getVar ()->getDouble () : 1.);
		context->drawBitmap (bitmap, destRect, offset, alpha);
	}
	void drawString (CScriptVar* var) const
	{
		checkContextOrThrow ();
		auto stringVar =
			getArgument (var, "string"sv, "drawContext.drawString(string, rect, align?);"sv);
		auto rectVar =
			getArgument (var, "rect"sv, "drawContext.drawString(string, rect, align?);"sv);
		auto alignVar = var->getParameter ("align"sv);
		auto string = stringVar->getString ().data ();
		auto rect = fromScriptRect (*rectVar);
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
			throw CScriptException ("wrong `align` argument. expected 'left', 'center' or 'right'");
		context->drawString (string, rect, align, true);
	}
	void setFont (CScriptVar* var) const
	{
		checkContextOrThrow ();
		auto fontVar = getArgument (var, "font"sv, "drawContext.setFont(font);"sv);
		if (auto font = uiDesc->getFont (fontVar->getString ().data ()))
			context->setFont (font);
	}
	void setFontColor (CScriptVar* var) const
	{
		checkContextOrThrow ();
		auto colorVar = getArgument (var, "color"sv, "drawContext.setFontColor(color);"sv);
		CColor color {};
		UIViewCreator::stringToColor (colorVar->getString (), color, uiDesc);
		context->setFontColor (color);
	}
	void setFillColor (CScriptVar* var) const
	{
		checkContextOrThrow ();
		auto colorVar = getArgument (var, "color"sv, "drawContext.setFillColor(color);"sv);
		CColor color {};
		UIViewCreator::stringToColor (colorVar->getString (), color, uiDesc);
		context->setFillColor (color);
	}
	void setFrameColor (CScriptVar* var) const
	{
		checkContextOrThrow ();
		auto colorVar = getArgument (var, "color"sv, "drawContext.setFrameColor(color);"sv);
		CColor color {};
		if (!UIViewCreator::stringToColor (colorVar->getString (), color, uiDesc))
			throw CScriptException (
				"Unknown `color` argument in drawContext.setFrameColor(color);");
		context->setFrameColor (color);
	}
	void setLineWidth (CScriptVar* var) const
	{
		checkContextOrThrow ();
		auto widthVar = getArgument (var, "width"sv, "drawContext.setLineWidth(width);"sv);
		context->setLineWidth (widthVar->getDouble ());
	}
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
DrawContextObject::DrawContextObject ()
{
	// TODO: decide if this object should expose the same interface as "CanvasRenderingContext2D"
	impl = std::make_unique<Impl> ();
	scriptVar->setLifeTimeObserver (this);
#if 0
#else
	addFunc ("drawLine"sv, [this] (auto var) { impl->drawLine (var); }, {"from"sv, "to"sv});
	addFunc ("drawRect"sv, [this] (auto var) { impl->drawRect (var); }, {"rect"sv, "style"sv});
	addFunc ("drawEllipse"sv, [this] (auto var) { impl->drawEllipse (var); },
			 {"rect"sv, "style"sv});
	addFunc ("clearRect"sv, [this] (auto var) { impl->clearRect (var); }, {"rect"sv, "style"sv});
	addFunc ("drawPolygon"sv, [this] (auto var) { impl->drawPolygon (var); },
			 {"points"sv, "style"sv});
	addFunc ("setClipRect"sv, [this] (auto var) { impl->setClipRect (var); }, {"rect"sv});
#if 1 // TODO: make a bitmap js object instead
	addFunc ("drawBitmap"sv, [this] (auto var) { impl->drawBitmap (var); },
			 {"name"sv, "destRect"sv});
#endif
	addFunc ("drawString"sv, [this] (auto var) { impl->drawString (var); },
			 {"string"sv, "rect"sv, "align"sv});
	addFunc ("setFont"sv, [this] (auto var) { impl->setFont (var); }, {"font"sv});
	addFunc ("setFontColor"sv, [this] (auto var) { impl->setFontColor (var); }, {"color"sv});
	addFunc ("setFillColor"sv, [this] (auto var) { impl->setFillColor (var); }, {"color"sv});
	addFunc ("setFrameColor"sv, [this] (auto var) { impl->setFrameColor (var); }, {"color"sv});
	addFunc ("setLineWidth"sv, [this] (auto var) { impl->setLineWidth (var); }, {"width"sv});
#endif
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
	impl->context = inContext;
	impl->uiDesc = inUIDesc;
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
