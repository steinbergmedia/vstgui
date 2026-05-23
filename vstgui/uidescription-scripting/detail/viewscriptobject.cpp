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
ViewScriptObject::ViewScriptObject (const WeakPointer<CView>& view,
									IViewScriptObjectContext& context)
: view (view), context (context)
{
	auto viewPtr = view.lock ();
	vstgui_assert (viewPtr != nullptr);
	scriptVar->setLifeTimeObserver (this);
	auto viewType = IViewFactory::getViewName (*viewPtr.get ());
	scriptVar->addChild ("type"sv, new CScriptVar (std::string (viewType ? viewType : "unknown")));
	addFunc ("setAttribute"sv,
			 [uiDesc = context.getUIDescription (), view] (CScriptVar& var) {
				 auto viewPtr = view.lock ();
				 if (viewPtr == nullptr)
					 throw CScriptException ("View already destroyed");
				 auto key = var.getParameter ("key"sv);
				 auto value = var.getParameter ("value"sv);
				 UIAttributes attr;
				 attr.setAttribute (key->getString ().data (), value->getString ().data ());
				 auto result = uiDesc->getViewFactory ().applyAttributeValues (
					 *viewPtr.get (), attr, *uiDesc.get ());
				 var.getReturnVar ()->setInt (result);
			 },
			 {"key", "value"});
	addFunc ("getAttribute"sv,
			 [uiDesc = context.getUIDescription (), view] (CScriptVar& var) {
				 auto viewPtr = view.lock ();
				 if (viewPtr == nullptr)
					 throw CScriptException ("View already destroyed");
				 auto key = var.getParameter ("key"sv);
				 std::string result;
				 if (uiDesc->getViewFactory ().getAttributeValue (
						 *viewPtr.get (), key->getString ().data (), result, *uiDesc.get ()))
				 {
					 var.getReturnVar ()->setString (result);
				 }
				 else
				 {
					 var.getReturnVar ()->setUndefined ();
				 }
			 },
			 {"key"});
	addFunc ("isTypeOf"sv,
			 [uiDesc = context.getUIDescription (), view] (CScriptVar& var) {
				 auto viewPtr = view.lock ();
				 if (viewPtr == nullptr)
					 throw CScriptException ("View already destroyed");
				 auto typeName = var.getParameter ("typeName"sv);
				 auto result = uiDesc->getViewFactory ().viewIsTypeOf (
					 *viewPtr.get (), typeName->getString ().data ());
				 var.getReturnVar ()->setInt (result);
			 },
			 {"typeName"});
	addFunc ("invalid"sv, [view] (CScriptVar& var) {
		auto viewPtr = view.lock ();
		if (viewPtr == nullptr)
			throw CScriptException ("View already destroyed");
		viewPtr->invalid ();
	});
	addFunc ("invalidRect"sv,
			 [view] (CScriptVar& var) {
				 auto viewPtr = view.lock ();
				 if (viewPtr == nullptr)
					 throw CScriptException ("View already destroyed");
				 auto rectVar = var.getParameter ("rect"sv);
				 if (!rectVar)
					 throw CScriptException ("Missing 'rect' argument in view.invalidRect(rect) ");
				 auto rect = fromScriptRect (*rectVar);
				 viewPtr->invalidRect (rect);
			 },
			 {"rect"});
	addFunc ("getBounds"sv, [view] (CScriptVar& var) {
		auto viewPtr = view.lock ();
		if (viewPtr == nullptr)
			throw CScriptException ("View already destroyed");
		auto bounds = viewPtr->getViewSize ();
		bounds.originize ();
		var.setReturnVar (makeScriptRect (bounds));
	});
	addFunc ("getParent"sv, [view, &context] (CScriptVar& var) {
		auto viewPtr = view.lock ();
		if (viewPtr == nullptr)
			throw CScriptException ("View already destroyed");
		auto parentView = viewPtr->getParentView ();
		if (!parentView)
		{
			var.getReturnVar ()->setUndefined ();
			return;
		}
		auto obj = context.addView (*parentView);
		vstgui_assert (obj);
		var.setReturnVar (obj->getVar ());
		obj->getVar ()->release ();
	});
	addFunc ("getControllerProperty"sv,
			 [view] (CScriptVar& var) {
				 auto viewPtr = view.lock ();
				 if (viewPtr == nullptr)
					 throw CScriptException ("View already destroyed");
				 auto viewController = getViewController (*viewPtr.get (), true);
				 auto controller = viewController.cast<IScriptControllerExtension> ();
				 auto name = var.getParameter ("name"sv);
				 if (!controller || !name)
				 {
					 var.getReturnVar ()->setUndefined ();
					 return;
				 }
				 IScriptControllerExtension::PropertyValue value;
				 if (!controller->getProperty (*viewPtr.get (), name->getString (), value))
				 {
					 var.getReturnVar ()->setUndefined ();
					 return;
				 }
				 std::visit (
					 [&] (auto&& value) {
						 using T = std::decay_t<decltype (value)>;
						 if constexpr (std::is_same_v<T, int64_t>)
							 var.getReturnVar ()->setInt (value);
						 else if constexpr (std::is_same_v<T, double>)
							 var.getReturnVar ()->setDouble (value);
						 else if constexpr (std::is_same_v<T, std::string>)
							 var.getReturnVar ()->setString (value);
						 else if constexpr (std::is_same_v<T, std::nullptr_t>)
							 var.getReturnVar ()->setUndefined ();
					 },
					 value);
			 },
			 {"name"});
	addFunc ("setControllerProperty"sv,
			 [view] (CScriptVar& var) {
				 auto viewPtr = view.lock ();
				 if (viewPtr == nullptr)
					 throw CScriptException ("View already destroyed");
				 auto viewController = getViewController (*viewPtr.get (), true);
				 auto controller = viewController.cast<IScriptControllerExtension> ();
				 auto name = var.getParameter ("name"sv);
				 auto value = var.getParameter ("value"sv);
				 if (!controller || !name || !value || !(value->isNumeric () || value->isString ()))
				 {
					 var.getReturnVar ()->setUndefined ();
					 return;
				 }
				 IScriptControllerExtension::PropertyValue propValue;
				 if (value->isInt ())
					 propValue = value->getInt ();
				 else if (value->isDouble ())
					 propValue = value->getDouble ();
				 else if (value->isString ())
					 propValue = value->getString ().data ();
				 auto result =
					 controller->setProperty (*viewPtr.get (), name->getString (), propValue);
				 var.getReturnVar ()->setInt (result);
			 },
			 {"name", "value"});
	if (auto controlPtr = viewPtr.cast<CControl> ())
	{
		WeakPointer<CControl> control = controlPtr;
		addFunc ("setValue"sv,
				 [control] (CScriptVar& var) {
					 auto controlPtr = control.lock ();
					 if (controlPtr == nullptr)
						 throw CScriptException ("View already destroyed");

					 auto value = var.getParameter ("value"sv);
					 if (value->isNumeric ())
					 {
						 auto oldValue = controlPtr->getValue ();
						 controlPtr->setValue (static_cast<float> (value->getDouble ()));
						 if (oldValue != controlPtr->getValue ())
							 controlPtr->valueChanged ();
					 }
				 },
				 {"value"});
		addFunc ("getValue"sv, [control] (CScriptVar& var) {
			auto controlPtr = control.lock ();
			if (controlPtr == nullptr)
				throw CScriptException ("View already destroyed");
			var.getReturnVar ()->setDouble (controlPtr->getValue ());
		});
		addFunc ("setValueNormalized"sv,
				 [control] (CScriptVar& var) {
					 auto controlPtr = control.lock ();
					 if (controlPtr == nullptr)
						 throw CScriptException ("View already destroyed");
					 auto value = var.getParameter ("value"sv);
					 if (value->isNumeric ())
					 {
						 auto oldValue = controlPtr->getValue ();
						 controlPtr->setValueNormalized (static_cast<float> (value->getDouble ()));
						 if (oldValue != controlPtr->getValue ())
							 controlPtr->valueChanged ();
					 }
				 },
				 {"value"});
		addFunc ("getValueNormalized"sv, [control] (CScriptVar& var) {
			auto controlPtr = control.lock ();
			if (controlPtr == nullptr)
				throw CScriptException ("View already destroyed");
			var.getReturnVar ()->setDouble (controlPtr->getValueNormalized ());
		});
		addFunc ("beginEdit"sv, [control] (CScriptVar& var) {
			auto controlPtr = control.lock ();
			if (controlPtr == nullptr)
				throw CScriptException ("View already destroyed");
			controlPtr->beginEdit ();
		});
		addFunc ("endEdit"sv, [control] (CScriptVar& var) {
			auto controlPtr = control.lock ();
			if (controlPtr == nullptr)
				throw CScriptException ("View already destroyed");
			controlPtr->endEdit ();
		});
		addFunc ("getMinValue"sv, [control] (CScriptVar& var) {
			auto controlPtr = control.lock ();
			if (controlPtr == nullptr)
				throw CScriptException ("View already destroyed");
			var.getReturnVar ()->setDouble (controlPtr->getMin ());
		});
		addFunc ("getMaxValue"sv, [control] (CScriptVar& var) {
			auto controlPtr = control.lock ();
			if (controlPtr == nullptr)
				throw CScriptException ("View already destroyed");
			var.getReturnVar ()->setDouble (controlPtr->getMax ());
		});
		addFunc ("getTag"sv, [control] (CScriptVar& var) {
			auto controlPtr = control.lock ();
			if (controlPtr == nullptr)
				throw CScriptException ("View already destroyed");
			var.getReturnVar ()->setInt (controlPtr->getTag ());
		});
	}
	if (auto drawable = dynamic_cast<JavaScriptDrawable*> (viewPtr.get ()))
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
	if (auto viewPtr = view.lock ())
		context.removeView (*viewPtr.get ());
}

//------------------------------------------------------------------------
} // ScriptingInternal
} // VSTGUI
