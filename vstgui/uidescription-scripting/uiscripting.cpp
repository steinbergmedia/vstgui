// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uiscripting.h"
#include "../uidescription/uiattributes.h"
#include "../uidescription/iviewfactory.h"
#include "../uidescription/uidescriptionaddonregistry.h"
#include "../uidescription/detail/uiviewcreatorattributes.h"
#include "../lib/iviewlistener.h"
#include "../lib/events.h"
#include "../lib/cresourcedescription.h"
#include "../lib/cvstguitimer.h"
#include "../lib/platform/platformfactory.h"
#include "../lib/platform/iplatformresourceinputstream.h"
#include "../lib/controls/ccontrol.h"

#include "tiny-js/TinyJS.h"
#include "tiny-js/TinyJS_Functions.h"
#include "tiny-js/TinyJS_MathFunctions.h"

#include <unordered_map>
#include <array>
#include <iostream>

//------------------------------------------------------------------------
namespace VSTGUI {
using namespace std::literals;
using namespace TJS;

namespace ScriptingInternal {

static const std::string kAttrScript = "script";

//------------------------------------------------------------------------
class ScriptContext
{
public:
	using OnScriptException = std::function<void (std::string reason)>;

	ScriptContext (IUIDescription* uiDesc, const OnScriptException& func);
	~ScriptContext () noexcept;

	void init (const std::string& initScript);
	void onViewCreated (CView* view, const std::string& script);

	void reset ();

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
struct ScriptAddChildScoped
{
	ScriptAddChildScoped (CScriptVar& var, const std::string& name, CScriptVar* obj) : var (var)
	{
		if ((link = var.findChild (name)))
		{
			oldVar = link->var;
			link->var = obj;
		}
		else
		{
			link = var.addChild (name, obj);
		}
	}
	~ScriptAddChildScoped () noexcept
	{
		if (oldVar && link)
		{
			link->var = oldVar;
		}
		else if (link)
		{
			var.removeLink (link);
		}
	}

private:
	CScriptVar& var;
	CScriptVarLink* link {nullptr};
	CScriptVar* oldVar {nullptr};
};

//------------------------------------------------------------------------
template<typename Proc>
CScriptVar* createJSFunction (Proc proc)
{
	auto funcVar = new CScriptVar (TINYJS_BLANK_DATA, SCRIPTVAR_FUNCTION | SCRIPTVAR_NATIVE);
	funcVar->setCallback (proc);
	return funcVar;
}

//------------------------------------------------------------------------
template<typename Proc>
CScriptVar* createJSFunction (Proc proc, const std::initializer_list<const char*>& paramNames)
{
	auto f = createJSFunction (proc);
	for (auto name : paramNames)
		f->addChildNoDup (name);
	return f;
}

//------------------------------------------------------------------------
struct TimerScriptObject : CScriptVar
{
	template<typename Proc>
	TimerScriptObject (uint64_t fireTime, CScriptVar* _callback, Proc timerProc)
	: CScriptVar (TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT), callback (_callback->ref ())
	{
		timer = makeOwned<CVSTGUITimer> (
			[timerProc, this] (auto) {
				if (!timerProc (callback))
					timer->stop ();
			},
			static_cast<uint32_t> (fireTime), false);
		addChild ("invalid"sv, createJSFunction ([this] (auto) { timer = nullptr; }));
		addChild ("start"sv, createJSFunction ([this] (auto) {
					  if (timer)
						  timer->start ();
				  }));
		addChild ("stop"sv, createJSFunction ([this] (auto) {
					  if (timer)
						  timer->stop ();
				  }));
	}
	~TimerScriptObject () noexcept
	{
		timer = nullptr;
		callback->unref ();
	}

private:
	SharedPointer<CVSTGUITimer> timer;
	CScriptVar* callback;
};

//------------------------------------------------------------------------
struct ScriptObject
{
	ScriptObject ()
	{
		scriptVar = new CScriptVar (TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT);
		scriptVar->ref ();
	}
	ScriptObject (ScriptObject&& o) { *this = std::move (o); }
	ScriptObject& operator= (ScriptObject&& o)
	{
		if (scriptVar)
			scriptVar->unref ();
		scriptVar = nullptr;
		std::swap (scriptVar, o.scriptVar);
		return *this;
	}
	virtual ~ScriptObject () noexcept
	{
		if (scriptVar)
		{
			scriptVar->removeAllChildren ();
			scriptVar->unref ();
		}
	}

	CScriptVar* getVar () const { return scriptVar; }
	CScriptVar* take ()
	{
		auto var = scriptVar;
		scriptVar = nullptr;
		return var;
	}

	void addChild (std::string_view name, ScriptObject&& obj)
	{
		auto var = obj.take ();
		scriptVar->addChild (name, var);
	}
	void addChild (std::string_view name, double d)
	{
		scriptVar->addChild (name, new CScriptVar (d));
	}
	void addChild (std::string_view name, int64_t i)
	{
		scriptVar->addChild (name, new CScriptVar (i));
	}
	void addChild (std::string_view name, int32_t i)
	{
		scriptVar->addChild (name, new CScriptVar (static_cast<int64_t> (i)));
	}
	void addChild (std::string_view name, const std::string& value)
	{
		scriptVar->addChild (name, new CScriptVar (value));
	}

protected:
	CScriptVar* scriptVar {nullptr};
};

//------------------------------------------------------------------------
inline ScriptObject makeScriptRect (const CRect& rect)
{
	ScriptObject obj;
	obj.addChild ("left"sv, rect.left);
	obj.addChild ("top"sv, rect.top);
	obj.addChild ("right"sv, rect.right);
	obj.addChild ("bottom"sv, rect.bottom);
	return obj;
}

//------------------------------------------------------------------------
inline ScriptObject makeScriptPoint (const CPoint& point)
{
	ScriptObject obj;
	obj.addChild ("x"sv, point.x);
	obj.addChild ("y"sv, point.y);
	return obj;
}

//------------------------------------------------------------------------
inline ScriptObject makeScriptEvent (const Event& event)
{
	ScriptObject obj;
	if (auto modifierEvent = asModifierEvent (event))
	{
		ScriptObject mod;
		mod.addChild ("shift"sv, modifierEvent->modifiers.has (ModifierKey::Shift));
		mod.addChild ("alt"sv, modifierEvent->modifiers.has (ModifierKey::Alt));
		mod.addChild ("control"sv, modifierEvent->modifiers.has (ModifierKey::Control));
		mod.addChild ("super"sv, modifierEvent->modifiers.has (ModifierKey::Super));
		obj.addChild ("modifiers"sv, std::move (mod));
	}
	if (auto mouseEvent = asMousePositionEvent (event))
	{
		obj.addChild ("mousePosition"sv, makeScriptPoint (mouseEvent->mousePosition));
	}
	if (auto mouseEvent = asMouseEvent (event))
	{
		ScriptObject buttons;
		buttons.addChild ("left"sv, mouseEvent->buttonState.has (MouseButton::Left));
		buttons.addChild ("right"sv, mouseEvent->buttonState.has (MouseButton::Right));
		buttons.addChild ("middle"sv, mouseEvent->buttonState.has (MouseButton::Middle));
		obj.addChild ("mouseButtons"sv, std::move (buttons));
	}
	if (event.type == EventType::MouseWheel)
	{
		const auto& wheelEvent = castMouseWheelEvent (event);
		ScriptObject wheel;
		wheel.addChild ("deltaX"sv, wheelEvent.deltaX);
		wheel.addChild ("deltaY"sv, wheelEvent.deltaY);
		wheel.addChild (
			"directionInvertedFromDevice"sv,
			wheelEvent.flags & MouseWheelEvent::Flags::DirectionInvertedFromDevice ? true : false);
		wheel.addChild ("preciceDelta"sv,
						wheelEvent.flags & MouseWheelEvent::Flags::PreciseDeltas ? true : false);
		obj.addChild ("mouseWheel"sv, std::move (wheel));
	}
	if (auto keyEvent = asKeyboardEvent (event))
	{
		ScriptObject key;
		key.addChild ("character"sv, static_cast<int> (keyEvent->character));
		key.addChild ("virtual"sv, static_cast<int> (keyEvent->virt));
		key.addChild ("isRepeat"sv, keyEvent->isRepeat);
	}
	obj.addChild ("consume"sv, 0);
	return obj;
}

//------------------------------------------------------------------------
struct UIDescScriptObject : ScriptObject
{
	using StringList = std::list<const std::string*>;
	UIDescScriptObject () = default;
	UIDescScriptObject (IUIDescription* desc, CTinyJS* scriptContext)
	{
		scriptVar->addChild ("colorNames"sv, createJSFunction ([desc] (CScriptVar* var) {
								 StringList names;
								 desc->collectColorNames (names);
								 var->setReturnVar (createArrayFromNames (names));
							 }));
		scriptVar->addChild ("fontNames"sv, createJSFunction ([desc] (CScriptVar* var) {
								 StringList names;
								 desc->collectFontNames (names);
								 var->setReturnVar (createArrayFromNames (names));
							 }));
		scriptVar->addChild ("bitmapNames"sv, createJSFunction ([desc] (CScriptVar* var) {
								 StringList names;
								 desc->collectBitmapNames (names);
								 var->setReturnVar (createArrayFromNames (names));
							 }));
		scriptVar->addChild ("gradientNames"sv, createJSFunction ([desc] (CScriptVar* var) {
								 StringList names;
								 desc->collectGradientNames (names);
								 var->setReturnVar (createArrayFromNames (names));
							 }));
		scriptVar->addChild ("controlTagNames"sv, createJSFunction ([desc] (CScriptVar* var) {
								 StringList names;
								 desc->collectControlTagNames (names);
								 var->setReturnVar (createArrayFromNames (names));
							 }));
		scriptVar->addChild (
			"getTagForName"sv,
			createJSFunction (
				[desc] (CScriptVar* var) {
					auto param = var->getParameter ("name"sv);
					if (!param)
					{
						throw CScriptException ("Expect 'name' argument for getTagForName ");
					}
					std::string name = param->getString ();
					auto tag = desc->getTagForName (name.data ());
					var->setReturnVar (new CScriptVar (static_cast<int64_t> (tag)));
				},
				{"name"}));
		scriptVar->addChild (
			"lookupTagName"sv,
			createJSFunction (
				[desc] (CScriptVar* var) {
					auto param = var->getParameter ("tag"sv);
					if (!param)
					{
						throw CScriptException ("Expect 'tag' argument for lookupTagName ");
					}
					if (!param->isInt ())
					{
						throw CScriptException ("Expect 'tag' argument to be an integer ");
					}
					if (auto tagName =
							desc->lookupControlTagName (static_cast<int32_t> (param->getInt ())))
					{
						var->setReturnVar (new CScriptVar (std::string (tagName)));
					}
				},
				{"tag"}));
	}

	static CScriptVar* createArrayFromNames (const StringList& names)
	{
		auto array = new CScriptVar ();
		array->setArray ();
		int index = 0;
		for (auto colorName : names)
		{
			array->addChild (std::to_string (index), new CScriptVar (*colorName));
			++index;
		}
		return array;
	}
};

struct IViewScriptObjectContext;

//------------------------------------------------------------------------
struct ViewScriptObject : ScriptObject,
						  IScriptVarLifeTimeObserver
{
	ViewScriptObject (CView* view, IViewScriptObjectContext* context);
	~ViewScriptObject () noexcept;
	void onDestroy (CScriptVar* v) override;

private:
	CView* view {nullptr};
	IViewScriptObjectContext* context {nullptr};
};

using ViewScriptMap = std::unordered_map<CView*, std::unique_ptr<ViewScriptObject>>;

//------------------------------------------------------------------------
struct IViewScriptObjectContext
{
	virtual ~IViewScriptObjectContext () = default;

	virtual IUIDescription* getUIDescription () const = 0;
	virtual ViewScriptObject* addView (CView* view) = 0;
	virtual ViewScriptMap::iterator removeView (CView* view) = 0;
};

//------------------------------------------------------------------------
struct ScriptContext::Impl : ViewListenerAdapter,
							 ViewEventListenerAdapter,
							 ViewContainerListenerAdapter,
							 IControlListener,
							 ScriptingInternal::IViewScriptObjectContext
{
	using ViewScriptObject = ScriptingInternal::ViewScriptObject;
	using ScriptAddChildScoped = ScriptingInternal::ScriptAddChildScoped;
	using ScriptObject = ScriptingInternal::ScriptObject;
	using TimerScriptObject = ScriptingInternal::TimerScriptObject;
	using UIDescScriptObject = ScriptingInternal::UIDescScriptObject;

	IUIDescription* uiDesc {nullptr};

	std::unique_ptr<CTinyJS> jsContext;
	OnScriptException onScriptException;
	using ViewScriptMap = ScriptingInternal::ViewScriptMap;
	ViewScriptMap viewScriptMap;

	UIDescScriptObject uiDescObject;

	Impl (IUIDescription* uiDesc, const OnScriptException& func)
	: uiDesc (uiDesc), onScriptException (func)
	{
		init ();
	}

	~Impl () noexcept { reset (true); }

	void init ()
	{
		jsContext = std::make_unique<CTinyJS> ();
		uiDescObject = UIDescScriptObject (uiDesc, jsContext.get ());

		registerFunctions (jsContext.get ());
		registerMathFunctions (jsContext.get ());

		jsContext->root->addChild ("uiDesc"sv, uiDescObject.getVar ());

		jsContext->addNative (
			"function createTimer(context, fireTime, callback)"sv, [this] (CScriptVar* var) {
				auto context = var->getParameter ("context"sv);
				auto fireTime = var->getParameter ("fireTime"sv);
				auto callback = var->getParameter ("callback"sv);
				if (!fireTime->isInt ())
				{
					throw new CScriptException (
						"Expect integer as first parameter on timer creation");
				}
				if (!callback->isFunction ())
				{
					throw new CScriptException (
						"Expect function as second parameter on timer creation");
				}
				auto timerObj = new TimerScriptObject (
					fireTime->getInt (), callback->deepCopy (), [this, context] (auto callback) {
						using namespace ScriptingInternal;
						ScriptAddChildScoped scs (*jsContext->root, "timerContext", context);
						return evalScript (callback, "timerCallback (timerContext); ",
										   "timerCallback");
					});
				timerObj->ref ();
				var->setReturnVar (timerObj);
				timerObj->unref ();
			});
		jsContext->addNative (
			"function iterateSubViews(view, context, callback)"sv, [this] (CScriptVar* var) {
				auto view = var->getParameter ("view"sv);
				auto context = var->getParameter ("context"sv);
				auto callback = var->getParameter ("callback"sv);
				if (!view->isObject ())
				{
					throw new CScriptException (
						"Expect object as first parameter on iterateSubViews");
				}
				if (!callback->isFunction ())
				{
					throw new CScriptException (
						"Expect function as second parameter on iterateSubViews");
				}
				auto it =
					std::find_if (viewScriptMap.begin (), viewScriptMap.end (),
								  [&] (const auto& el) { return el.second->getVar () == view; });
				if (it == viewScriptMap.end ())
					throw new CScriptException ("View not found in iterateSubViews");
				auto container = it->first->asViewContainer ();
				if (!container)
					return; // no sub views
				container->forEachChild ([&] (auto child) {
					using namespace ScriptingInternal;
					auto childScriptObject = viewScriptMap.find (child);
					if (childScriptObject != viewScriptMap.end ())
					{
						ScriptAddChildScoped scs (*jsContext->root, "child",
												  childScriptObject->second->getVar ());
						ScriptAddChildScoped scs2 (*jsContext->root, "context", context);
						evalScript (callback, "callback (child, context); ", "callback");
					}
					else
					{
						auto scriptObj =
							addView (child, std::make_unique<ViewScriptObject> (child, this));
						ScriptAddChildScoped scs (*jsContext->root, "child", scriptObj->getVar ());
						ScriptAddChildScoped scs2 (*jsContext->root, "context", context);
						evalScript (callback, "callback (child, context); ", "callback");
					}
				});
			});
		jsContext->addNative ("function log(obj)",
							  [this] (CScriptVar* var) { log (var->getParameter ("obj"sv)); });
	}

	void reset (bool terminate = false)
	{
		for (auto it = viewScriptMap.begin (); it != viewScriptMap.end ();)
		{
			viewRemoved (it->first);
			uninstallListeners (it->first);
			it = eraseViewFromMap (it);
		}
		if (!terminate)
			init ();
	}

	void initWithScript (const std::string* script)
	{
		if (script)
		{
			auto scriptContent = getScriptFileContent (*script);
			if (!scriptContent.empty ())
				script = &scriptContent;
			evalScript (*script);
		}
	}

	IUIDescription* getUIDescription () const override { return uiDesc; }

	void installListeners (CView* view)
	{
		view->registerViewListener (this);
		view->registerViewEventListener (this);
		if (auto viewContainer = view->asViewContainer ())
			viewContainer->registerViewContainerListener (this);
		else if (auto control = dynamic_cast<CControl*> (view))
			control->registerControlListener (this);
	}

	void uninstallListeners (CView* view)
	{
		view->unregisterViewListener (this);
		view->unregisterViewEventListener (this);
		if (auto viewContainer = view->asViewContainer ())
			viewContainer->unregisterViewContainerListener (this);
		if (auto control = dynamic_cast<CControl*> (view))
			control->unregisterControlListener (this);
	}

	bool evalScript (std::string_view script)
	{
		try
		{
			auto result = jsContext->evaluateComplex (script);
			if (result.var && !result.var->isUndefined ())
			{
#if DEBUG
				DebugPrint ("%s\n", result.var->getString ().data ());
#endif
			}
		}
		catch (const CScriptException* exc)
		{
#if DEBUG
			DebugPrint ("Scripting Exception: %s\n", exc->text.data ());
#endif
			if (onScriptException)
				onScriptException (exc->text);
			return false;
		}
		return true;
	}

	bool evalScript (CScriptVar* object, std::string_view script,
					 const std::string& objectName = "view")
	{
		if (!object || script.empty ())
			return false;
		vstgui_assert (object->getRefs () > 0);
		object->ref ();
		ScriptAddChildScoped scs (*jsContext->root, objectName, object);
		auto result = evalScript (script);
		object->unref ();
		return result;
	}

	template<typename Proc>
	void callWhenScriptHasFunction (CView* view, std::string_view funcName, Proc proc)
	{
		auto it = viewScriptMap.find (view);
		if (it == viewScriptMap.end ())
			return;
		if (it->second->getVar ()->findChild (funcName))
			proc (this, it->second);
	}

	void viewAttached (CView* view) override
	{
		callWhenScriptHasFunction (view, "onAttached"sv, [] (auto This, auto& obj) {
			static constexpr auto script = R"(view.onAttached(view);)"sv;
			This->evalScript (obj->getVar (), script);
		});
	}

	void viewRemoved (CView* view) override
	{
		callWhenScriptHasFunction (view, "onRemoved"sv, [] (auto This, auto& obj) {
			static constexpr auto script = R"(view.onRemoved(view);)"sv;
			This->evalScript (obj->getVar (), script);
		});
	}

	void viewSizeChanged (CView* view, const CRect& oldSize) override
	{
		callWhenScriptHasFunction (view, "onSizeChanged"sv, [&] (auto This, auto& obj) {
			auto newSize = view->getViewSize ();
			ScriptObject newSizeObject = ScriptingInternal::makeScriptRect (newSize);
			ScriptAddChildScoped scs (*jsContext->root, "newSize", newSizeObject.getVar ());
			static constexpr auto script = R"(view.onSizeChanged(view, newSize);)"sv;
			This->evalScript (obj->getVar (), script);
		});
	}

	void viewLostFocus (CView* view) override
	{
		callWhenScriptHasFunction (view, "onLostFocus"sv, [&] (auto This, auto& obj) {
			static constexpr auto script = R"(view.onLostFocus(view);)"sv;
			This->evalScript (obj->getVar (), script);
		});
	}

	void viewTookFocus (CView* view) override
	{
		callWhenScriptHasFunction (view, "onTookFocus"sv, [&] (auto This, auto& obj) {
			static constexpr auto script = R"(view.onTookFocus(view);)"sv;
			This->evalScript (obj->getVar (), script);
		});
	}

	void viewOnMouseEnabled (CView* view, bool state) override
	{
		callWhenScriptHasFunction (view, "onMouseEnabled"sv, [&] (auto This, auto& obj) {
			static constexpr auto scriptEnabled = R"(view.onMouseEnabled(view, true);)"sv;
			static constexpr auto scriptDisabled = R"(view.onMouseEnabled(view, false);)"sv;
			This->evalScript (obj->getVar (), state ? scriptEnabled : scriptDisabled);
		});
	}

	void callViewAddedOrRemoved (CViewContainer* container, CView* view, std::string_view function,
								 std::string_view script)
	{
		callWhenScriptHasFunction (container, function, [&] (auto This, auto& obj) {
			auto childScriptObject = viewScriptMap.find (view);
			if (childScriptObject != viewScriptMap.end ())
			{
				ScriptAddChildScoped scs (*jsContext->root, "child",
										  childScriptObject->second->getVar ());
				This->evalScript (obj->getVar (), script);
			}
			else
			{
				auto scriptObj = addView (view, std::make_unique<ViewScriptObject> (view, this));
				ScriptAddChildScoped scs (*jsContext->root, "child", scriptObj->getVar ());
				This->evalScript (obj->getVar (), script);
			}
		});
	}

	void viewContainerViewAdded (CViewContainer* container, CView* view) override
	{
		callViewAddedOrRemoved (container, view, "onViewAdded"sv,
								R"(view.onViewAdded(view, child);)"sv);
	}

	void viewContainerViewRemoved (CViewContainer* container, CView* view) override
	{
		callViewAddedOrRemoved (container, view, "onViewRemoved"sv,
								R"(view.onViewRemoved(view, child);)"sv);
	}

	void checkEventConsumed (ScriptObject& obj, Event& event)
	{
		if (auto consume = obj.getVar ()->findChild ("consumed"))
		{
			if (consume->var->isInt () && consume->var->getInt ())
				event.consumed = true;
		}
	}

	void callEventFunction (CScriptVar* var, Event& event, std::string_view script)
	{
		auto scriptEvent = ScriptingInternal::makeScriptEvent (event);
		ScriptAddChildScoped scs (*jsContext->root, "event", scriptEvent.getVar ());
		evalScript (var, script);
		checkEventConsumed (scriptEvent, event);
	}

	void viewOnEvent (CView* view, Event& event) override
	{
		switch (event.type)
		{
			case EventType::MouseEnter:
			{
				callWhenScriptHasFunction (view, "onMouseEnter"sv, [&] (auto This, auto& obj) {
					static constexpr auto script = R"(view.onMouseEnter(view, event);)"sv;
					callEventFunction (obj->getVar (), event, script);
				});
				break;
			}
			case EventType::MouseExit:
			{
				callWhenScriptHasFunction (view, "onMouseExit"sv, [&] (auto This, auto& obj) {
					static constexpr auto script = R"(view.onMouseExit(view, event);)"sv;
					callEventFunction (obj->getVar (), event, script);
				});
				break;
			}
			case EventType::MouseDown:
			{
				callWhenScriptHasFunction (view, "onMouseDown"sv, [&] (auto This, auto& obj) {
					static constexpr auto script = R"(view.onMouseDown(view, event);)"sv;
					callEventFunction (obj->getVar (), event, script);
				});
				break;
			}
			case EventType::MouseUp:
			{
				callWhenScriptHasFunction (view, "onMouseUp"sv, [&] (auto This, auto& obj) {
					static constexpr auto script = R"(view.onMouseUp(view, event);)"sv;
					callEventFunction (obj->getVar (), event, script);
				});
				break;
			}
			case EventType::MouseMove:
			{
				callWhenScriptHasFunction (view, "onMouseMove"sv, [&] (auto This, auto& obj) {
					static constexpr auto script = R"(view.onMouseMove(view, event);)"sv;
					callEventFunction (obj->getVar (), event, script);
				});
				break;
			}
			case EventType::MouseWheel:
			{
				callWhenScriptHasFunction (view, "onMouseWheel"sv, [&] (auto This, auto& obj) {
					static constexpr auto script = R"(view.onMouseWheel(view, event);)"sv;
					callEventFunction (obj->getVar (), event, script);
				});
				break;
			}
			case EventType::KeyDown:
			{
				callWhenScriptHasFunction (view, "onKeyDown"sv, [&] (auto This, auto& obj) {
					static constexpr auto script = R"(view.onKeyDown(view, event);)"sv;
					callEventFunction (obj->getVar (), event, script);
				});
				break;
			}
			case EventType::KeyUp:
			{
				callWhenScriptHasFunction (view, "onKeyUp"sv, [&] (auto This, auto& obj) {
					static constexpr auto script = R"(view.onKeyUp(view, event);)"sv;
					callEventFunction (obj->getVar (), event, script);
				});
				break;
			}
			default:
				break;
		}
	}

	void valueChanged (CControl* control) override
	{
		callWhenScriptHasFunction (control, "onValueChanged"sv, [&] (auto This, auto& obj) {
			ScriptObject controlValue;
			controlValue.getVar ()->setDouble (control->getValue ());
			ScriptAddChildScoped scs (*jsContext->root, "value", controlValue.getVar ());
			static constexpr auto script = R"(view.onValueChanged(view, value);)"sv;
			This->evalScript (obj->getVar (), script);
		});
	}

	void controlBeginEdit (CControl* control) override
	{
		callWhenScriptHasFunction (control, "onBeginEdit"sv, [] (auto This, auto& obj) {
			static constexpr auto script = R"(view.onBeginEdit(view);)"sv;
			This->evalScript (obj->getVar (), script);
		});
	}
	void controlEndEdit (CControl* control) override
	{
		callWhenScriptHasFunction (control, "onEndEdit"sv, [] (auto This, auto& obj) {
			static constexpr auto script = R"(view.onEndEdit(view);)"sv;
			This->evalScript (obj->getVar (), script);
		});
	}

	void viewWillDelete (CView* view) override { removeView (view); }

	ViewScriptObject* addView (CView* view, std::unique_ptr<ViewScriptObject>&& scriptObject)
	{
		installListeners (view);
		viewScriptMap[view] = std::move (scriptObject);
		return viewScriptMap[view].get ();
	}

	ViewScriptMap::iterator eraseViewFromMap (ViewScriptMap::iterator el)
	{
		return viewScriptMap.erase (el);
	}

	ViewScriptMap::iterator removeView (CView* view) override
	{
		uninstallListeners (view);
		auto it = viewScriptMap.find (view);
		if (it != viewScriptMap.end ())
			return eraseViewFromMap (it);
		return viewScriptMap.end ();
	}

	ViewScriptObject* addView (CView* view) override
	{
		auto it = viewScriptMap.find (view);
		if (it != viewScriptMap.end ())
			return it->second.get ();
		return addView (view, std::make_unique<ViewScriptObject> (view, this));
	}

	void addView (CView* view, const std::string* script)
	{
		addView (view);
		if (script)
		{
			auto scriptContent = getScriptFileContent (*script);
			if (!scriptContent.empty ())
				script = &scriptContent;
			evalScript (viewScriptMap[view]->getVar (), *script);
		}
	}

	std::string getScriptFileContent (const std::string& script) const
	{
		constexpr std::string_view scriptFileCommand ("//scriptfile ");
		std::string_view scriptView (script);
		if (scriptView.length () > scriptFileCommand.length ())
		{
			if (scriptView.substr (0, scriptFileCommand.length ()) == scriptFileCommand)
			{
				auto scriptFileStr = scriptView.substr (scriptFileCommand.length ());
				if (scriptFileStr.length () < 255)
				{
					std::string fileStr (scriptFileStr);
					CResourceDescription desc (fileStr.data ());
					if (auto stream = getPlatformFactory ().createResourceInputStream (desc))
					{
						std::string fileContent;
						while (true)
						{
							char buffer[256];
							auto numRead = stream->readRaw (
								buffer, static_cast<uint32_t> (std::size (buffer)));
							if (numRead == kStreamIOError)
								return {};
							fileContent.append (buffer, numRead);
							if (numRead < std::size (buffer))
								break;
						}
						return fileContent;
					}
				}
			}
		}
		return {};
	}

	void log (CScriptVar* var) const
	{
#if DEBUG
		static constexpr size_t bufferSize = 512;

		struct DebugOutputStream : private std::streambuf,
								   public std::ostream
		{

			DebugOutputStream () : std::ostream (this) { data.reserve (bufferSize); }
			~DebugOutputStream () noexcept
			{
				if (!data.empty ())
					flush ();
			}

		private:
			int overflow (int c) override
			{
				if (data.length () >= bufferSize)
					flush ();
				data += static_cast<char> (c);
				return 0;
			}
			void flush ()
			{
				DebugPrint ("%s", data.data ());
				data.clear ();
				data.reserve (bufferSize);
			}
			std::string data;
		};
		DebugOutputStream logStream;
		var->getJSON (logStream);
		logStream << "\n";
#endif
	}
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
ScriptContext::ScriptContext (IUIDescription* uiDesc, const OnScriptException& func)
{
	using namespace ScriptingInternal;

	impl = std::make_unique<Impl> (uiDesc, func);
}

//------------------------------------------------------------------------
ScriptContext::~ScriptContext () noexcept {}

//------------------------------------------------------------------------
void ScriptContext::init (const std::string& initScript) { impl->initWithScript (&initScript); }

//------------------------------------------------------------------------
void ScriptContext::onViewCreated (CView* view, const std::string& script)
{
	impl->addView (view, &script);
}

//------------------------------------------------------------------------
void ScriptContext::reset () { impl->reset (); }

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
ViewScriptObject::ViewScriptObject (CView* view, IViewScriptObjectContext* context)
: view (view), context (context)
{
	scriptVar->setLifeTimeObserver (this);
	auto viewType = IViewFactory::getViewName (view);
	scriptVar->addChild ("type"sv, new CScriptVar (std::string (viewType ? viewType : "unknown")));
	scriptVar->addChild ("setAttribute"sv,
						 createJSFunction (
							 [uiDesc = context->getUIDescription (), view] (CScriptVar* var) {
								 auto key = var->getParameter ("key"sv);
								 auto value = var->getParameter ("value"sv);
								 UIAttributes attr;
								 attr.setAttribute (key->getString (), value->getString ());
								 auto result = uiDesc->getViewFactory ()->applyAttributeValues (
									 view, attr, uiDesc);
								 var->getReturnVar ()->setInt (result);
							 },
							 {"key", "value"}));
	scriptVar->addChild ("getAttribute"sv,
						 createJSFunction (
							 [uiDesc = context->getUIDescription (), view] (CScriptVar* var) {
								 auto key = var->getParameter ("key"sv);
								 std::string result;
								 if (uiDesc->getViewFactory ()->getAttributeValue (
										 view, key->getString (), result, uiDesc))
								 {
									 var->getReturnVar ()->setString (result);
								 }
								 else
								 {
									 var->getReturnVar ()->setUndefined ();
								 }
							 },
							 {"key"}));
	scriptVar->addChild ("isTypeOf"sv,
						 createJSFunction (
							 [uiDesc = context->getUIDescription (), view] (CScriptVar* var) {
								 auto typeName = var->getParameter ("typeName"sv);
								 auto result = uiDesc->getViewFactory ()->viewIsTypeOf (
									 view, typeName->getString ());
								 var->getReturnVar ()->setInt (result);
							 },
							 {"typeName"}));
	scriptVar->addChild ("getParent"sv, createJSFunction ([view, context] (CScriptVar* var) {
							 auto parentView = view->getParentView ();
							 if (!parentView)
							 {
								 var->getReturnVar ()->setUndefined ();
								 return;
							 }
							 auto obj = context->addView (parentView);
							 vstgui_assert (obj);
							 var->setReturnVar (obj->getVar ());
							 obj->getVar ()->unref ();
						 }));
	if (auto control = dynamic_cast<CControl*> (view))
	{
		scriptVar->addChild (
			"setValue"sv, createJSFunction (
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
							  {"value"}));
		scriptVar->addChild ("getValue"sv, createJSFunction ([control] (CScriptVar* var) {
								 var->getReturnVar ()->setDouble (control->getValue ());
							 }));
		scriptVar->addChild ("setValueNormalized"sv,
							 createJSFunction (
								 [control] (CScriptVar* var) {
									 auto value = var->getParameter ("value"sv);
									 if (value->isNumeric ())
									 {
										 auto oldValue = control->getValue ();
										 control->setValueNormalized (
											 static_cast<float> (value->getDouble ()));
										 if (oldValue != control->getValue ())
											 control->valueChanged ();
									 }
								 },
								 {"value"}));
		scriptVar->addChild ("getValueNormalized"sv, createJSFunction ([control] (CScriptVar* var) {
								 var->getReturnVar ()->setDouble (control->getValueNormalized ());
							 }));
		scriptVar->addChild ("beginEdit"sv, createJSFunction ([control] (CScriptVar* var) {
								 control->beginEdit ();
							 }));
		scriptVar->addChild (
			"endEdit"sv, createJSFunction ([control] (CScriptVar* var) { control->endEdit (); }));
		scriptVar->addChild ("getMinValue"sv, createJSFunction ([control] (CScriptVar* var) {
								 var->getReturnVar ()->setDouble (control->getMin ());
							 }));
		scriptVar->addChild ("getMaxValue"sv, createJSFunction ([control] (CScriptVar* var) {
								 var->getReturnVar ()->setDouble (control->getMax ());
							 }));
		scriptVar->addChild ("getTag"sv, createJSFunction ([control] (CScriptVar* var) {
								 var->getReturnVar ()->setInt (control->getTag ());
							 }));
	}
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
//------------------------------------------------------------------------
//------------------------------------------------------------------------
struct JavaScriptViewFactory : ViewFactoryDelegate
{
	static constexpr CViewAttributeID scriptAttrID = 'scri';

	using Super = ViewFactoryDelegate;

	JavaScriptViewFactory (ScriptingInternal::ScriptContext* scripting, IViewFactory* origFactory)
	: Super (origFactory), scriptContext (scripting)
	{
	}

	CView* createView (const UIAttributes& attributes,
					   const IUIDescription* description) const override
	{
		if (auto view = Super::createView (attributes, description))
		{
			if (auto value = attributes.getAttributeValue (ScriptingInternal::kAttrScript))
			{
				view->setAttribute (scriptAttrID, static_cast<uint32_t> (value->size () + 1),
									value->data ());
				if (!disabled)
					scriptContext->onViewCreated (view, *value);
			}
			return view;
		}
		return {};
	}

	bool getAttributeNamesForView (CView* view, StringList& attributeNames) const override
	{
		if (Super::getAttributeNamesForView (view, attributeNames))
		{
			attributeNames.emplace_back (ScriptingInternal::kAttrScript);
			return true;
		}
		return false;
	}

	IViewCreator::AttrType getAttributeType (CView* view,
											 const std::string& attributeName) const override
	{
		if (attributeName == ScriptingInternal::kAttrScript)
			return IViewCreator::kScriptType;
		return Super::getAttributeType (view, attributeName);
	}

	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue,
							const IUIDescription* desc) const override
	{
		if (attributeName == ScriptingInternal::kAttrScript)
		{
			uint32_t attrSize = 0;
			if (view->getAttributeSize (scriptAttrID, attrSize) && attrSize > 0)
			{
				stringValue.resize (attrSize - 1);
				if (!view->getAttribute (scriptAttrID, attrSize, stringValue.data (), attrSize))
					stringValue = "";
				return true;
			}
			return false;
		}
		return Super::getAttributeValue (view, attributeName, stringValue, desc);
	}
	bool applyAttributeValues (CView* view, const UIAttributes& attributes,
							   const IUIDescription* desc) const override
	{
		if (auto value = attributes.getAttributeValue (ScriptingInternal::kAttrScript))
		{
			if (value->empty ())
				view->removeAttribute (scriptAttrID);
			else
				view->setAttribute (scriptAttrID, static_cast<uint32_t> (value->size () + 1),
									value->data ());
			if (!disabled)
				scriptContext->onViewCreated (view, *value);
			return true;
		}
		return Super::applyAttributeValues (view, attributes, desc);
	}

	void setScriptingDisabled (bool state) { disabled = state; }

private:
	ScriptingInternal::ScriptContext* scriptContext;
	bool disabled {false};
};

//------------------------------------------------------------------------
} // ScriptingInternal

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
struct UIScripting::Impl
{
	std::unordered_map<const IUIDescription*,
					   std::pair<std::unique_ptr<ScriptingInternal::JavaScriptViewFactory>,
								 std::unique_ptr<ScriptingInternal::ScriptContext>>>
		map;
};

//------------------------------------------------------------------------
UIScripting::UIScripting () { impl = std::make_unique<Impl> (); }

//------------------------------------------------------------------------
UIScripting::~UIScripting () noexcept = default;

//------------------------------------------------------------------------
void UIScripting::afterParsing (IUIDescription* desc) {}

//------------------------------------------------------------------------
void UIScripting::beforeSaving (IUIDescription* desc) {}

//------------------------------------------------------------------------
void UIScripting::onDestroy (IUIDescription* desc)
{
	auto it = impl->map.find (desc);
	if (it != impl->map.end ())
		impl->map.erase (it);
}

//------------------------------------------------------------------------
auto UIScripting::onCreateTemplateView (const IUIDescription* desc, const CreateTemplateViewFunc& f)
	-> CreateTemplateViewFunc
{
	return [=] (auto name, auto controller) {
		return f (name, controller);
	};
}

//------------------------------------------------------------------------
IViewFactory* UIScripting::getViewFactory (IUIDescription* desc, IViewFactory* originalFactory)
{
	auto it = impl->map.find (desc);
	if (it != impl->map.end ())
		return it->second.first.get ();
	auto scripting = std::make_unique<ScriptingInternal::ScriptContext> (
		desc, [] (std::string reason) { std::cerr << reason << '\n'; });
	auto result = impl->map.emplace (
		desc, std::make_pair (std::make_unique<ScriptingInternal::JavaScriptViewFactory> (
								  scripting.get (), originalFactory),
							  std::move (scripting)));
	return result.first->second.first.get ();
}

//------------------------------------------------------------------------
void UIScripting::onEditingStart (IUIDescription* desc)
{
	auto it = impl->map.find (desc);
	if (it != impl->map.end ())
	{
		it->second.second->reset ();
		it->second.first->setScriptingDisabled (true);
	}
}

//------------------------------------------------------------------------
void UIScripting::onEditingEnd (IUIDescription* desc)
{
	auto it = impl->map.find (desc);
	if (it != impl->map.end ())
		it->second.first->setScriptingDisabled (false);
}

//------------------------------------------------------------------------
void UIScripting::init () { UIDescriptionAddOnRegistry::add (std::make_unique<UIScripting> ()); }

//------------------------------------------------------------------------
} // VSTGUI
