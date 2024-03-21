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
		auto rectVar = getArgument (var, "rect"sv, signature);
		auto styleVar = getArgument (var, "style"sv, signature);
		auto rect = fromScriptRect (*rectVar);
		auto style = getDrawStyle (styleVar);
		context->drawRect (rect, style);
	}
	void drawEllipse (CScriptVar* var) const
	{
		static constexpr auto signature = "drawContext.drawEllipse(rect, style);"sv;
		checkContextOrThrow ();
		auto rectVar = getArgument (var, "rect"sv, signature);
		auto styleVar = getArgument (var, "style"sv, signature);
		auto rect = fromScriptRect (*rectVar);
		auto style = getDrawStyle (styleVar);
		context->drawEllipse (rect, style);
	}
	void drawArc (CScriptVar* var) const
	{
		static constexpr auto signature =
			"drawContext.drawArc(rect, startAngle, endAngle, style);"sv;
		checkContextOrThrow ();
		auto rectVar = getArgument (var, "rect"sv, signature);
		auto startAngleVar = getArgument (var, "startAngle"sv, signature);
		auto endAngleVar = getArgument (var, "endAngle"sv, signature);
		auto styleVar = getArgument (var, "style"sv, signature);
		auto rect = fromScriptRect (*rectVar);
		if (!startAngleVar->isNumeric ())
			throw CScriptException ("`startAngle` must be a number.");
		if (!endAngleVar->isNumeric ())
			throw CScriptException ("`endAngle` must be a number.");
		auto startAngle = static_cast<float> (startAngleVar->getDouble ());
		auto endAngle = static_cast<float> (endAngleVar->getDouble ());
		auto style = getDrawStyle (styleVar);
		context->drawArc (rect, startAngle, endAngle, style);
	}
	void clearRect (CScriptVar* var) const
	{
		static constexpr auto signature = "drawContext.clearRect(rect);"sv;
		checkContextOrThrow ();
		auto rectVar = getArgument (var, "rect"sv, signature);
		auto rect = fromScriptRect (*rectVar);
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
		auto rectVar = getArgument (var, "rect"sv, signature);
		auto rect = fromScriptRect (*rectVar);
		context->setClipRect (rect);
	}
	void drawBitmap (CScriptVar* var) const
	{
		static constexpr auto signature =
			"drawContext.drawBitmap(name, destRect, offsetPoint?, alpha?);"sv;
		checkContextOrThrow ();
		auto nameVar = getArgument (var, "name"sv, signature);
		auto destRectVar = getArgument (var, "destRect"sv, signature);
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
		static constexpr auto signature = "drawContext.drawString(string, rect, align?);"sv;
		checkContextOrThrow ();
		auto stringVar = getArgument (var, "string"sv, signature);
		auto rectVar = getArgument (var, "rect"sv, signature);
		auto alignVar = var->findChild ("align"sv);
		auto string = stringVar->getString ().data ();
		auto rect = fromScriptRect (*rectVar);
		CHoriTxtAlign align = kCenterText;
		if (!alignVar || alignVar->getVar ()->isUndefined ())
			align = kCenterText;
		else if (alignVar->getVar ()->getString () == "left"sv)
			align = kLeftText;
		else if (alignVar->getVar ()->getString () == "center"sv)
			align = kCenterText;
		else if (alignVar->getVar ()->getString () == "right"sv)
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
		auto lineJoinVarLink = var->findChild ("lineJoin"sv);
		auto dashLengthsVarLink = var->findChild ("dashLengths"sv);
		auto dashPhaseVarLink = var->findChild ("dashPhase"sv);
		auto styleOrLineCap = styleOrLineCapVar->getString ();
		std::unique_ptr<CLineStyle> lineStyle;
		if (styleOrLineCap == "solid"sv)
		{
			lineStyle = std::make_unique<CLineStyle> (kLineSolid);
		}
		else if (styleOrLineCap == "onOff"sv)
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
			if (lineJoinVarLink)
			{
				auto lineJoin = lineJoinVarLink->getVar ()->getString ();
				if (lineJoin == "miter"sv)
					lineStyle->setLineJoin (CLineStyle::LineJoin::kLineJoinMiter);
				else if (lineJoin == "round"sv)
					lineStyle->setLineJoin (CLineStyle::LineJoin::kLineJoinRound);
				else if (lineJoin == "bevel"sv)
					lineStyle->setLineJoin (CLineStyle::LineJoin::kLineJoinBevel);
			}
			if (dashLengthsVarLink)
			{
				auto dashLengthsVar = dashLengthsVarLink->getVar ();
				if (!dashLengthsVarLink->getVar ()->isArray ())
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
			if (dashPhaseVarLink)
			{
				lineStyle->setDashPhase (dashPhaseVarLink->getVar ()->getDouble ());
			}
		}
		if (lineStyle)
			context->setLineStyle (*lineStyle.get ());
	}
	void setGlobalAlpha (CScriptVar* var) const
	{
		static constexpr auto signature = "drawContext.setGlobalAlpha(alpha);"sv;
		checkContextOrThrow ();
		auto alphaVar = getArgument (var, "alpha"sv, signature);
		if (!alphaVar->isNumeric ())
			throw CScriptException ("alpha must be a number.");
		context->setGlobalAlpha (alphaVar->getDouble ());
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
	addFunc ("drawArc"sv, [this] (auto var) { impl->drawArc (var); },
			 {"rect"sv, "startAngle"sv, "endAngle"sv, "style"sv});
	addFunc ("setClipRect"sv, [this] (auto var) { impl->setClipRect (var); }, {"rect"sv});
#if 1 // TODO: make a bitmap js object instead
	addFunc ("drawBitmap"sv, [this] (auto var) { impl->drawBitmap (var); },
			 {"name"sv, "destRect"sv, "offsetPoint"sv, "alpha"sv});
#endif
	addFunc ("drawString"sv, [this] (auto var) { impl->drawString (var); },
			 {"string"sv, "rect"sv, "align"sv});
	addFunc ("setFont"sv, [this] (auto var) { impl->setFont (var); }, {"font"sv});
	addFunc ("setFontColor"sv, [this] (auto var) { impl->setFontColor (var); }, {"color"sv});
	addFunc ("setFillColor"sv, [this] (auto var) { impl->setFillColor (var); }, {"color"sv});
	addFunc ("setFrameColor"sv, [this] (auto var) { impl->setFrameColor (var); }, {"color"sv});
	addFunc ("setLineWidth"sv, [this] (auto var) { impl->setLineWidth (var); }, {"width"sv});
	addFunc ("setLineStyle"sv, [this] (auto var) { impl->setLineStyle (var); },
			 {"styleOrLineCap"sv, "lineJoin"sv, "dashLengths"sv, "dashPhase"sv});
	addFunc ("setGlobalAlpha", [this] (auto var) { impl->setGlobalAlpha (var); }, {"alpha"sv});
	addFunc ("saveGlobalState"sv, [this] (auto var) { impl->saveGlobalState (); });
	addFunc ("restoreGlobalState"sv, [this] (auto var) { impl->restoreGlobalState (); });
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
