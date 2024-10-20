// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "drawable.h"
#include "converters.h"
#include "../../lib/cframe.h"
#include "../../lib/cdrawcontext.h"
#include "../../lib/cgraphicspath.h"
#include "../../lib/cgraphicstransform.h"
#include "../../uidescription/detail/uiviewcreatorattributes.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ScriptingInternal {
using namespace std::literals;
using namespace TJS;

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
void JavaScriptDrawable::onDraw (CDrawContext* context, const CRect& rect, const CRect& viewSize)
{
	if (!scriptObject)
	{
		auto dashLength = std::round ((viewSize.getWidth () * 2 + viewSize.getHeight () * 2) / 40.);
		CLineStyle ls (CLineStyle::kLineCapButt, CLineStyle::kLineJoinMiter, 0,
					   {dashLength, dashLength});

		auto lineWidth = 1.;
		auto size = viewSize;
		size.inset (lineWidth / 2., lineWidth / 2.);
		context->setLineStyle (ls);
		context->setLineWidth (lineWidth);
		context->setFrameColor (kBlackCColor);
		context->drawRect (size, kDrawStroked);

		ls.setDashPhase (dashLength * lineWidth);
		context->setLineStyle (ls);
		context->setFrameColor (kWhiteCColor);
		context->drawRect (size, kDrawStroked);
		return;
	}
	auto scriptContext = scriptObject->getContext ();
	if (!scriptContext)
		return;
	context->saveGlobalState ();

	drawContext.setDrawContext (context, scriptContext->getUIDescription ());

	CDrawContext::Transform tm (*context, CGraphicsTransform ().translate (viewSize.getTopLeft ()));

	auto rectVar = makeScriptRect (rect);
	auto scriptRoot = scriptContext->getRoot ();
	ScriptAddChildScoped scs (*scriptRoot, "view"sv, *scriptObject);
	ScriptAddChildScoped scs2 (*scriptRoot, "context"sv, drawContext);
	ScriptAddChildScoped scs3 (*scriptRoot, "rect"sv, rectVar);
	scriptContext->evalScript ("view.draw(context, rect);"sv);

	drawContext.setDrawContext (nullptr, nullptr);

	context->restoreGlobalState ();
}

//------------------------------------------------------------------------
void JavaScriptDrawable::setup (ViewScriptObject* inObject) { scriptObject = inObject; }

//------------------------------------------------------------------------
bool JavaScriptDrawable::onDrawFocusOnTop ()
{
	auto scriptContext = scriptObject->getContext ();
	if (!scriptContext)
		return false;

	if (scriptObject->getVar ()->findChild ("drawFocusOnTop"sv) == nullptr)
		return false;

	auto scriptRoot = scriptContext->getRoot ();
	ScriptAddChildScoped scs (*scriptRoot, "view"sv, *scriptObject);
	auto boolResult = scriptContext->evalScript ("view.drawFocusOnTop();"sv);
	return boolResult->isNumeric () ? boolResult->getInt () : false;
}

//------------------------------------------------------------------------
bool JavaScriptDrawable::onGetFocusPath (CGraphicsPath& outPath, CCoord focusWidth,
										 const CRect& viewSize)
{
	if (auto scriptContext = scriptObject->getContext ())
	{
		if (scriptObject->getVar ()->findChild ("getFocusPath"sv) == nullptr)
		{
			auto r = viewSize;
			outPath.addRect (r);
			r.extend (focusWidth, focusWidth);
			outPath.addRect (r);
			return true;
		}

		auto scriptRoot = scriptContext->getRoot ();
		auto path = makeOwned<CGraphicsPath> (outPath);
		ScriptObject focusWidthVar;
		focusWidthVar->setDouble (focusWidth);
		ScriptAddChildScoped scs (*scriptRoot, "view"sv, *scriptObject);
		ScriptAddChildScoped scs2 (*scriptRoot, "path"sv, makeGraphicsPathScriptObject (path));
		ScriptAddChildScoped scs3 (*scriptRoot, "focusWidth"sv, focusWidthVar);
		auto boolResult = scriptContext->evalScript ("view.getFocusPath(path, focusWidth);"sv);
		if (boolResult->isNumeric ())
		{
			if (boolResult->getInt () == 1)
			{
				CGraphicsTransform tm;
				tm.translate (viewSize.left, viewSize.top);
				outPath.addPath (*path, &tm);
				return true;
			}
			return false;
		}
	}
	return false;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
void JavaScriptDrawableView::drawRect (CDrawContext* context, const CRect& rect)
{
	onDraw (context, rect, getViewSize ());
}

//------------------------------------------------------------------------
bool JavaScriptDrawableView::drawFocusOnTop ()
{
	if (wantsFocus ())
		return onDrawFocusOnTop ();
	return false;
}

//------------------------------------------------------------------------
bool JavaScriptDrawableView::getFocusPath (CGraphicsPath& outPath)
{
	if (wantsFocus ())
		return onGetFocusPath (outPath, getFrame ()->getFocusWidth (), getViewSize ());
	return false;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
void JavaScriptDrawableControl::draw (CDrawContext* context) { drawRect (context, getViewSize ()); }

//------------------------------------------------------------------------
void JavaScriptDrawableControl::drawRect (CDrawContext* context, const CRect& rect)
{
	onDraw (context, rect, getViewSize ());
}

//------------------------------------------------------------------------
bool JavaScriptDrawableControl::drawFocusOnTop ()
{
	if (wantsFocus ())
		return onDrawFocusOnTop ();
	return false;
}

//------------------------------------------------------------------------
bool JavaScriptDrawableControl::getFocusPath (CGraphicsPath& outPath)
{
	if (wantsFocus ())
		return onGetFocusPath (outPath, getFrame ()->getFocusWidth (), getViewSize ());
	return false;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
IdStringPtr JavaScriptDrawableViewCreator::getViewName () const { return "JavaScriptDrawableView"; }

//------------------------------------------------------------------------
IdStringPtr JavaScriptDrawableViewCreator::getBaseViewName () const
{
	return UIViewCreator::kCView;
}

//------------------------------------------------------------------------
CView* JavaScriptDrawableViewCreator::create (const UIAttributes& attributes,
											  const IUIDescription* description) const
{
	return new JavaScriptDrawableView (CRect ());
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
IdStringPtr JavaScriptDrawableControlCreator::getViewName () const
{
	return "JavaScriptDrawableControl";
}

//------------------------------------------------------------------------
IdStringPtr JavaScriptDrawableControlCreator::getBaseViewName () const
{
	return UIViewCreator::kCControl;
}

//------------------------------------------------------------------------
CView* JavaScriptDrawableControlCreator::create (const UIAttributes& attributes,
												 const IUIDescription* description) const
{
	return new JavaScriptDrawableControl (CRect ());
}

//------------------------------------------------------------------------
} // ScriptingInternal
} // VSTGUI
