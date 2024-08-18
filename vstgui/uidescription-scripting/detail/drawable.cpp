// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "drawable.h"
#include "converters.h"
#include "../../lib/cdrawcontext.h"
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
	ScriptAddChildScoped scs (*scriptRoot, "view", *scriptObject);
	ScriptAddChildScoped scs2 (*scriptRoot, "context", drawContext);
	ScriptAddChildScoped scs3 (*scriptRoot, "rect", rectVar);
	scriptContext->evalScript ("view.draw(context, rect);"sv);

	drawContext.setDrawContext (nullptr, nullptr);

	context->restoreGlobalState ();
}

//------------------------------------------------------------------------
void JavaScriptDrawable::setup (ViewScriptObject* inObject) { scriptObject = inObject; }

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
void JavaScriptDrawableView::drawRect (CDrawContext* context, const CRect& rect)
{
	onDraw (context, rect, getViewSize ());
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
