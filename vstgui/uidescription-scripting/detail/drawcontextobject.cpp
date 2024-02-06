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
//------------------------------------------------------------------------
DrawContextObject::DrawContextObject ()
{
	// TODO: decide if this object should expose the same interface as "CanvasRenderingContext2D"
	scriptVar->setLifeTimeObserver (this);
#if 0
#else
	addFunc ("drawLine"sv,
			 [this] (CScriptVar* var) {
				 if (!context)
					 throw CScriptException ("Native context is missing!");
				 auto fromPoint = var->getParameter ("from"sv);
				 if (!fromPoint)
					 throw CScriptException (
						 "Missing `from` argument in drawContext.drawLine(from, to);");
				 auto toPoint = var->getParameter ("to"sv);
				 if (!toPoint)
					 throw CScriptException (
						 "Missing `to` argument in drawContext.drawLine(from, to);");
				 auto from = fromScriptPoint (*fromPoint);
				 auto to = fromScriptPoint (*toPoint);
				 context->drawLine (from, to);
			 },
			 {"from", "to"});
	addFunc ("drawRect"sv,
			 [this] (CScriptVar* var) {
				 if (!context)
					 throw CScriptException ("Native context is missing!");
				 auto rectVar = var->getParameter ("rect"sv);
				 if (!rectVar)
					 throw CScriptException (
						 "Missing `rect` argument in drawContext.drawRect(rect, style);");
				 auto styleVar = var->getParameter ("style"sv);
				 if (!styleVar)
					 throw CScriptException (
						 "Missing `style` argument in drawContext.drawRect(rect, style);");
				 auto rect = fromScriptRect (*rectVar);
				 CDrawStyle style {};
				 if (styleVar->getString () == "stroked")
					 style = kDrawStroked;
				 else if (styleVar->getString () == "filled")
					 style = kDrawFilled;
				 else if (styleVar->getString () == "filledAndStroked")
					 style = kDrawFilledAndStroked;
				 context->drawRect (rect, style);
			 },
			 {"rect", "style"});
	addFunc ("drawEllipse"sv,
			 [this] (CScriptVar* var) {
				 if (!context)
					 throw CScriptException ("Native context is missing!");
				 auto rectVar = var->getParameter ("rect"sv);
				 if (!rectVar)
					 throw CScriptException (
						 "Missing `rect` argument in drawContext.drawEllipse(rect, style);");
				 auto styleVar = var->getParameter ("style"sv);
				 if (!styleVar)
					 throw CScriptException (
						 "Missing `style` argument in drawContext.drawEllipse(rect, style);");
				 auto rect = fromScriptRect (*rectVar);
				 CDrawStyle style {};
				 if (styleVar->getString () == "stroked")
					 style = kDrawStroked;
				 else if (styleVar->getString () == "filled")
					 style = kDrawFilled;
				 else if (styleVar->getString () == "filledAndStroked")
					 style = kDrawFilledAndStroked;
				 context->drawEllipse (rect, style);
			 },
			 {"rect", "style"});
	addFunc ("clearRect"sv,
			 [this] (CScriptVar* var) {
				 if (!context)
					 throw CScriptException ("Native context is missing!");
				 auto rectVar = var->getParameter ("rect"sv);
				 if (!rectVar)
					 throw CScriptException (
						 "Missing `rect` argument in drawContext.clearRect(rect);");
				 auto rect = fromScriptRect (*rectVar);
				 context->clearRect (rect);
			 },
			 {"rect", "style"});
	addFunc ("drawPolygon"sv,
			 [this] (CScriptVar* var) {
				 if (!context)
					 throw CScriptException ("Native context is missing!");
				 auto pointsVar = var->getParameter ("points"sv);
				 if (!pointsVar)
					 throw CScriptException (
						 "Missing `points` argument in drawContext.drawPolygon(points, style);");
				 if (!pointsVar->isArray ())
					 throw CScriptException ("`points` argument must be an array of points in "
											 "drawContext.drawPolygon(points, style);");
				 auto styleVar = var->getParameter ("style"sv);
				 if (!styleVar)
					 throw CScriptException (
						 "Missing `style` argument in drawContext.drawPolygon(points, style);");
				 PointList points;
				 auto numPoints = pointsVar->getArrayLength ();
				 for (auto index = 0; index < numPoints; ++index)
				 {
					 auto pointVar = pointsVar->getArrayIndex (index);
					 vstgui_assert (pointVar != nullptr);
					 points.emplace_back (fromScriptPoint (*pointVar));
				 }
				 CDrawStyle style {};
				 if (styleVar->getString () == "stroked")
					 style = kDrawStroked;
				 else if (styleVar->getString () == "filled")
					 style = kDrawFilled;
				 else if (styleVar->getString () == "filledAndStroked")
					 style = kDrawFilledAndStroked;
				 context->drawPolygon (points, style);
			 },
			 {"points", "style"});
	addFunc ("setClipRect"sv,
			 [this] (CScriptVar* var) {
				 if (!context)
					 throw CScriptException ("Native context is missing!");
				 auto rectVar = var->getParameter ("rect"sv);
				 if (!rectVar)
					 throw CScriptException (
						 "Missing `rect` argument in drawContext.setClipRect(rect);");
				 auto rect = fromScriptRect (*rectVar);
				 context->setClipRect (rect);
			 },
			 {"rect"});
#if 1 // TODO: make a bitmap js object instead
	addFunc ("drawBitmap"sv,
			 [this] (CScriptVar* var) {
				 if (!context)
					 throw CScriptException ("Native context is missing!");
				 auto bitmapNameVar = var->getParameter ("bitmap"sv);
				 if (!bitmapNameVar)
					 throw CScriptException (
						 "Missing `bitmap` argument in drawContext.drawBitmap(bitmap, destRect, "
						 "offsetPoint, alpha);");
				 auto destRectVar = var->getParameter ("destRect"sv);
				 if (!destRectVar)
					 throw CScriptException (
						 "Missing `destRect` argument in drawContext.drawBitmap(bitmap, destRect, "
						 "offsetPoint, alpha);");
				 auto offsetPointVar = var->findChild ("offsetPoint"sv);
				 auto alphaVar = var->findChild ("alpha"sv);
				 auto bitmap = uiDesc->getBitmap (bitmapNameVar->getString ().data ());
				 if (!bitmap)
					 throw CScriptException ("bitmap not found in uiDescription");
				 auto destRect = fromScriptRect (*destRectVar);
				 auto offset =
					 offsetPointVar ? fromScriptPoint (*offsetPointVar->getVar ()) : CPoint (0, 0);
				 auto alpha =
					 static_cast<float> (alphaVar ? alphaVar->getVar ()->getDouble () : 1.);
				 context->drawBitmap (bitmap, destRect, offset, alpha);
			 },
			 {"bitmap", "destRect"});
#endif
	addFunc (
		"drawString"sv,
		[this] (CScriptVar* var) {
			if (!context)
				throw CScriptException ("Native context is missing!");
			auto stringVar = var->getParameter ("string"sv);
			if (!stringVar)
				throw CScriptException (
					"Missing `string` argument in drawContext.drawString(string, rect, align);");
			auto rectVar = var->getParameter ("rect"sv);
			if (!rectVar)
				throw CScriptException (
					"Missing `rect` argument in drawContext.drawString(string, rect, align);");
			auto alignVar = var->getParameter ("align"sv);
			if (!alignVar)
				throw CScriptException (
					"Missing `align` argument in drawContext.drawString(string, rect, align);");
			auto string = stringVar->getString ().data ();
			auto rect = fromScriptRect (*rectVar);
			CHoriTxtAlign align = kCenterText;
			if (alignVar->getString () == "left")
				align = kLeftText;
			else if (alignVar->getString () == "center")
				align = kCenterText;
			else if (alignVar->getString () == "right")
				align = kRightText;
			else
				throw CScriptException (
					"wrong `align` argument. expected 'left', 'center' or 'right'");
			context->drawString (string, rect, align, true);
		},
		{"string", "rect", "align"});
	addFunc ("setFont"sv,
			 [this] (CScriptVar* var) {
				 if (!context)
					 throw CScriptException ("Native context is missing!");
				 auto fontVar = var->getParameter ("font"sv);
				 if (!fontVar)
					 throw CScriptException (
						 "Missing `font` argument in drawContext.setFont(font);");
				 if (auto font = uiDesc->getFont (fontVar->getString ().data ()))
					 context->setFont (font);
			 },
			 {"font"});
	addFunc ("setFontColor"sv,
			 [this] (CScriptVar* var) {
				 if (!context)
					 throw CScriptException ("Native context is missing!");
				 auto colorVar = var->getParameter ("color"sv);
				 if (!colorVar)
					 throw CScriptException (
						 "Missing `color` argument in drawContext.setFontColor(color);");
				 CColor color {};
				 UIViewCreator::stringToColor (colorVar->getString (), color, uiDesc);
				 context->setFontColor (color);
			 },
			 {"color"});
	addFunc ("setFillColor"sv,
			 [this] (CScriptVar* var) {
				 if (!context)
					 throw CScriptException ("Native context is missing!");
				 auto colorVar = var->getParameter ("color"sv);
				 if (!colorVar)
					 throw CScriptException (
						 "Missing `color` argument in drawContext.setFillColor(color);");
				 CColor color {};
				 UIViewCreator::stringToColor (colorVar->getString (), color, uiDesc);
				 context->setFillColor (color);
			 },
			 {"color"});
	addFunc ("setFrameColor"sv,
			 [this] (CScriptVar* var) {
				 if (!context)
					 throw CScriptException ("Native context is missing!");
				 auto colorVar = var->getParameter ("color"sv);
				 if (!colorVar)
					 throw CScriptException (
						 "Missing `color` argument in drawContext.setFrameColor(color);");
				 CColor color {};
				 if (!UIViewCreator::stringToColor (colorVar->getString (), color, uiDesc))
					 throw CScriptException (
						 "Unknown `color` argument in drawContext.setFrameColor(color);");
				 context->setFrameColor (color);
			 },
			 {"color"});
	addFunc ("setLineWidth"sv,
			 [this] (CScriptVar* var) {
				 if (!context)
					 throw CScriptException ("Native context is missing!");
				 auto widthVar = var->getParameter ("width"sv);
				 if (!widthVar)
					 throw CScriptException (
						 "Missing `width` argument in drawContext.setLineWidth(width);");
				 context->setLineWidth (widthVar->getDouble ());
			 },
			 {"width"});
#endif
}

//------------------------------------------------------------------------
void DrawContextObject::setDrawContext (CDrawContext* inContext, IUIDescription* inUIDesc)
{
	context = inContext;
	uiDesc = inUIDesc;
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
