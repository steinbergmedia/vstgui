// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "viewscriptobject.h"
#include "converters.h"
#include "drawable.h"
#include "../uiscripting.h"
#include "../../uidescription/iviewfactory.h"
#include "../../uidescription/uiattributes.h"
#include "../../lib/cview.h"
#include "../../lib/controls/ccontrol.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ScriptingInternal {

using namespace std::literals;
using namespace TJS;

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
ViewScriptObject::ViewScriptObject (CView* view, IViewScriptObjectContext* context)
: view (view), context (context)
{
	scriptVar->setLifeTimeObserver (this);
	auto viewType = IViewFactory::getViewName (view);
	scriptVar->addChild ("type"sv, new CScriptVar (std::string (viewType ? viewType : "unknown")));
	addFunc ("setAttribute"sv,
			 [uiDesc = context->getUIDescription (), view] (CScriptVar* var) {
				 auto key = var->getParameter ("key"sv);
				 auto value = var->getParameter ("value"sv);
				 UIAttributes attr;
				 attr.setAttribute (key->getString ().data (), value->getString ().data ());
				 auto result = uiDesc->getViewFactory ()->applyAttributeValues (view, attr, uiDesc);
				 var->getReturnVar ()->setInt (result);
			 },
			 {"key", "value"});
	addFunc ("getAttribute"sv,
			 [uiDesc = context->getUIDescription (), view] (CScriptVar* var) {
				 auto key = var->getParameter ("key"sv);
				 std::string result;
				 if (uiDesc->getViewFactory ()->getAttributeValue (view, key->getString ().data (),
																   result, uiDesc))
				 {
					 var->getReturnVar ()->setString (result);
				 }
				 else
				 {
					 var->getReturnVar ()->setUndefined ();
				 }
			 },
			 {"key"});
	addFunc ("isTypeOf"sv,
			 [uiDesc = context->getUIDescription (), view] (CScriptVar* var) {
				 auto typeName = var->getParameter ("typeName"sv);
				 auto result =
					 uiDesc->getViewFactory ()->viewIsTypeOf (view, typeName->getString ().data ());
				 var->getReturnVar ()->setInt (result);
			 },
			 {"typeName"});
	addFunc ("invalid"sv, [view] (CScriptVar* var) { view->invalid (); });
	addFunc ("invalidRect"sv,
			 [view] (CScriptVar* var) {
				 auto rectVar = var->getParameter ("rect"sv);
				 if (!rectVar)
					 throw CScriptException ("Missing 'rect' argument in view.invalidRect(rect) ");
				 auto rect = fromScriptRect (*rectVar);
				 view->invalidRect (rect);
			 },
			 {"rect"});
	addFunc ("getBounds"sv, [view] (CScriptVar* var) {
		auto bounds = view->getViewSize ();
		bounds.originize ();
		var->setReturnVar (makeScriptRect (bounds).take ());
	});
	addFunc ("getParent"sv, [view, context] (CScriptVar* var) {
		auto parentView = view->getParentView ();
		if (!parentView)
		{
			var->getReturnVar ()->setUndefined ();
			return;
		}
		auto obj = context->addView (parentView);
		vstgui_assert (obj);
		var->setReturnVar (obj->getVar ());
		obj->getVar ()->release ();
	});
	addFunc ("getControllerProperty"sv,
			 [view] (CScriptVar* var) {
				 auto viewController = getViewController (view, true);
				 auto controller = dynamic_cast<IScriptControllerExtension*> (viewController);
				 auto name = var->getParameter ("name"sv);
				 if (!controller || !name)
				 {
					 var->getReturnVar ()->setUndefined ();
					 return;
				 }
				 IScriptControllerExtension::PropertyValue value;
				 if (!controller->getProperty (view, name->getString (), value))
				 {
					 var->getReturnVar ()->setUndefined ();
					 return;
				 }
				 std::visit (
					 [&] (auto&& value) {
						 using T = std::decay_t<decltype (value)>;
						 if constexpr (std::is_same_v<T, int64_t>)
							 var->getReturnVar ()->setInt (value);
						 else if constexpr (std::is_same_v<T, double>)
							 var->getReturnVar ()->setDouble (value);
						 else if constexpr (std::is_same_v<T, std::string>)
							 var->getReturnVar ()->setString (value);
						 else if constexpr (std::is_same_v<T, nullptr_t>)
							 var->getReturnVar ()->setUndefined ();
					 },
					 value);
			 },
			 {"name"});
	addFunc ("setControllerProperty"sv,
			 [view] (CScriptVar* var) {
				 auto viewController = getViewController (view, true);
				 auto controller = dynamic_cast<IScriptControllerExtension*> (viewController);
				 auto name = var->getParameter ("name"sv);
				 auto value = var->getParameter ("value"sv);
				 if (!controller || !name || !value || !(value->isNumeric () || value->isString ()))
				 {
					 var->getReturnVar ()->setUndefined ();
					 return;
				 }
				 IScriptControllerExtension::PropertyValue propValue;
				 if (value->isInt ())
					 propValue = value->getInt ();
				 else if (value->isDouble ())
					 propValue = value->getDouble ();
				 else if (value->isString ())
					 propValue = value->getString ().data ();
				 auto result = controller->setProperty (view, name->getString (), propValue);
				 var->getReturnVar ()->setInt (result);
			 },
			 {"name", "value"});
	if (auto control = dynamic_cast<CControl*> (view))
	{
		addFunc ("setValue"sv,
				 [control] (CScriptVar* var) {
					 auto value = var->getParameter ("value"sv);
					 if (value->isNumeric ())
					 {
						 auto oldValue = control->getValue ();
						 control->setValue (static_cast<float> (value->getDouble ()));
						 if (oldValue != control->getValue ())
							 control->valueChanged ();
					 }
				 },
				 {"value"});
		addFunc ("getValue"sv, [control] (CScriptVar* var) {
			var->getReturnVar ()->setDouble (control->getValue ());
		});
		addFunc ("setValueNormalized"sv,
				 [control] (CScriptVar* var) {
					 auto value = var->getParameter ("value"sv);
					 if (value->isNumeric ())
					 {
						 auto oldValue = control->getValue ();
						 control->setValueNormalized (static_cast<float> (value->getDouble ()));
						 if (oldValue != control->getValue ())
							 control->valueChanged ();
					 }
				 },
				 {"value"});
		addFunc ("getValueNormalized"sv, [control] (CScriptVar* var) {
			var->getReturnVar ()->setDouble (control->getValueNormalized ());
		});
		addFunc ("beginEdit"sv, [control] (CScriptVar* var) { control->beginEdit (); });
		addFunc ("endEdit"sv, [control] (CScriptVar* var) { control->endEdit (); });
		addFunc ("getMinValue"sv, [control] (CScriptVar* var) {
			var->getReturnVar ()->setDouble (control->getMin ());
		});
		addFunc ("getMaxValue"sv, [control] (CScriptVar* var) {
			var->getReturnVar ()->setDouble (control->getMax ());
		});
		addFunc ("getTag"sv, [control] (CScriptVar* var) {
			var->getReturnVar ()->setInt (control->getTag ());
		});
	}
	if (auto drawable = dynamic_cast<JavaScriptDrawable*> (view))
		drawable->setup (this);
}

//------------------------------------------------------------------------
ViewScriptObject::~ViewScriptObject () noexcept
{
	if (scriptVar)
		scriptVar->setLifeTimeObserver (nullptr);
}

//------------------------------------------------------------------------
void ViewScriptObject::onDestroy (CScriptVar* v)
{
	v->setLifeTimeObserver (nullptr);
	scriptVar = nullptr;
	if (context)
		context->removeView (view);
}

//------------------------------------------------------------------------
} // ScriptingInternal
} // VSTGUI
