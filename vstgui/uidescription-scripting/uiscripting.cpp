// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uiscripting.h"
#include "../uidescription/uiattributes.h"
#include "../uidescription/iviewfactory.h"
#include "../uidescription/uiviewfactory.h"
#include "../uidescription/iviewcreator.h"
#include "../uidescription/uiviewcreator.h"
#include "../uidescription/uidescriptionaddonregistry.h"
#include "../uidescription/detail/uiviewcreatorattributes.h"
#include "../lib/iviewlistener.h"
#include "../lib/events.h"
#include "../lib/cresourcedescription.h"
#include "../lib/cvstguitimer.h"
#include "../lib/platform/platformfactory.h"
#include "../lib/platform/iplatformresourceinputstream.h"
#include "../lib/controls/ccontrol.h"
#include "../lib/cdrawcontext.h"

#include "tiny-js/TinyJS.h"
#include "tiny-js/TinyJS_Functions.h"
#include "tiny-js/TinyJS_MathFunctions.h"

#include <unordered_map>
#include <array>
#include <iostream>
#include <sstream>

//------------------------------------------------------------------------
namespace VSTGUI {
using namespace std::literals;
using namespace TJS;

namespace ScriptingInternal {

static const std::string kAttrScript = "script";

struct ViewScriptObject;

//------------------------------------------------------------------------
class ScriptContext : public IScriptContext
{
public:
	using OnScriptException = std::function<void (const std::string& reason)>;

	ScriptContext (IUIDescription* uiDesc, const OnScriptException& func);
	~ScriptContext () noexcept;

	void init (const std::string& initScript);
	void onViewCreated (CView* view, const std::string& script);

	void reset ();

private:
	std::string eval (std::string_view script) const override;

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
			oldVar = link->getVar ();
			oldVar->addRef ();
			link->setVar (obj);
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
			link->setVar (oldVar);
			oldVar->release ();
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
inline CScriptVar* createJSFunction (JSCallback&& proc)
{
	auto funcVar = new CScriptVar (TINYJS_BLANK_DATA, SCRIPTVAR_FUNCTION | SCRIPTVAR_NATIVE);
	funcVar->setCallback (std::move (proc));
	return funcVar;
}

//------------------------------------------------------------------------
inline CScriptVar* createJSFunction (JSCallback&& proc,
									 const std::initializer_list<const char*>& argNames)
{
	auto f = createJSFunction (std::move (proc));
	for (auto name : argNames)
		f->addChildNoDup (name);
	return f;
}

//------------------------------------------------------------------------
struct TimerScriptObject : CScriptVar
{
	template<typename Proc>
	TimerScriptObject (uint64_t fireTime, CScriptVar* _callback, Proc timerProc)
	: CScriptVar (TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT), callback (_callback->addRef ())
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
		callback->release ();
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
		scriptVar = new CScriptVar ();
		scriptVar->addRef ();
	}
	ScriptObject (ScriptObject&& o) { *this = std::move (o); }
	ScriptObject& operator= (ScriptObject&& o)
	{
		if (scriptVar)
			scriptVar->release ();
		scriptVar = nullptr;
		std::swap (scriptVar, o.scriptVar);
		return *this;
	}
	virtual ~ScriptObject () noexcept
	{
		if (scriptVar)
		{
			scriptVar->removeAllChildren ();
			scriptVar->release ();
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
	void addFunc (std::string_view name, std::function<void (CScriptVar*)>&& func)
	{
		scriptVar->addChild (name, createJSFunction (std::move (func)));
	}
	void addFunc (std::string_view name, std::function<void (CScriptVar*)>&& func,
				  const std::initializer_list<const char*>& argNames)
	{
		scriptVar->addChild (name, createJSFunction (std::move (func), argNames));
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
inline CPoint fromScriptPoint (CScriptVar& var)
{
	CPoint result {};
	if (auto xVar = var.findChild ("x"sv))
		result.x = xVar->getVar ()->getDouble ();
	else
		throw CScriptException ("Not a point object, missing 'x' member");
	if (auto yVar = var.findChild ("y"sv))
		result.y = yVar->getVar ()->getDouble ();
	else
		throw CScriptException ("Not a point object, missing 'y' member");
	return result;
}

//------------------------------------------------------------------------
inline CRect fromScriptRect (CScriptVar& var)
{
	CRect result {};
	auto leftVar = var.findChild ("left");
	auto topVar = var.findChild ("top");
	auto rightVar = var.findChild ("right");
	auto bottomVar = var.findChild ("bottom");
	if (!leftVar || !topVar || !rightVar || !bottomVar)
		throw CScriptException ("Expecting a rect object here");
	result.left = leftVar->getVar ()->getDouble ();
	result.top = topVar->getVar ()->getDouble ();
	result.right = rightVar->getVar ()->getDouble ();
	result.bottom = bottomVar->getVar ()->getDouble ();
	return result;
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
		addFunc ("colorNames"sv, [desc] (CScriptVar* var) {
			StringList names;
			desc->collectColorNames (names);
			var->setReturnVar (createArrayFromNames (names));
		});
		addFunc ("fontNames"sv, [desc] (CScriptVar* var) {
			StringList names;
			desc->collectFontNames (names);
			var->setReturnVar (createArrayFromNames (names));
		});
		addFunc ("bitmapNames"sv, [desc] (CScriptVar* var) {
			StringList names;
			desc->collectBitmapNames (names);
			var->setReturnVar (createArrayFromNames (names));
		});
		addFunc ("gradientNames"sv, [desc] (CScriptVar* var) {
			StringList names;
			desc->collectGradientNames (names);
			var->setReturnVar (createArrayFromNames (names));
		});
		addFunc ("controlTagNames"sv, [desc] (CScriptVar* var) {
			StringList names;
			desc->collectControlTagNames (names);
			var->setReturnVar (createArrayFromNames (names));
		});
		addFunc ("getTagForName"sv,
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
				 {"name"});
		addFunc ("lookupTagName"sv,
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
				 {"tag"});
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

//------------------------------------------------------------------------
struct DrawContextObject : ScriptObject,
						   IScriptVarLifeTimeObserver
{
	DrawContextObject ();

	void setDrawContext (CDrawContext* context, IUIDescription* uiDesc);

	void onDestroy (CScriptVar* v) override;

private:
	CDrawContext* context {nullptr};
	IUIDescription* uiDesc {nullptr};
};

//------------------------------------------------------------------------
struct JavaScriptDrawable
{
	void onDraw (CDrawContext* context, const CRect& rect, const CRect& viewSize);

	void setup (ViewScriptObject* object);

private:
	ViewScriptObject* scriptObject {nullptr};
	DrawContextObject drawContext;
};

//------------------------------------------------------------------------
struct JavaScriptDrawableView : CView,
								JavaScriptDrawable
{
	using CView::CView;

	void drawRect (CDrawContext* context, const CRect& rect) override;
};

//------------------------------------------------------------------------
struct JavaScriptDrawableControl : CControl,
								   JavaScriptDrawable
{
	using CControl::CControl;

	void draw (CDrawContext* pContext) override;
	void drawRect (CDrawContext* context, const CRect& rect) override;

	CLASS_METHODS_NOCOPY (JavaScriptDrawableControl, CControl);
};

//------------------------------------------------------------------------
struct JavaScriptDrawableViewCreator : ViewCreatorAdapter
{
	IdStringPtr getViewName () const override { return "JavaScriptDrawableView"; }
	IdStringPtr getBaseViewName () const override { return UIViewCreator::kCView; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override
	{
		return new JavaScriptDrawableView (CRect ());
	}
};

//------------------------------------------------------------------------
struct JavaScriptDrawableControlCreator : ViewCreatorAdapter
{
	IdStringPtr getViewName () const override { return "JavaScriptDrawableControl"; }
	IdStringPtr getBaseViewName () const override { return UIViewCreator::kCControl; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override
	{
		return new JavaScriptDrawableControl (CRect ());
	}
};

struct IViewScriptObjectContext;

//------------------------------------------------------------------------
struct ViewScriptObject : ScriptObject,
						  IScriptVarLifeTimeObserver
{
	ViewScriptObject (CView* view, IViewScriptObjectContext* context);
	~ViewScriptObject () noexcept;

	IViewScriptObjectContext* getContext () const { return context; }

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
	virtual bool evalScript (std::string_view script) noexcept = 0;
	virtual CScriptVar* getRoot () const = 0;
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

		jsContext->getRoot ()->addChild ("uiDesc"sv, uiDescObject.getVar ());

		jsContext->addNative (
			"function createTimer(context, fireTime, callback)"sv, [this] (CScriptVar* var) {
				auto context = var->getParameter ("context"sv);
				auto fireTime = var->getParameter ("fireTime"sv);
				auto callback = var->getParameter ("callback"sv);
				if (!fireTime->isInt ())
				{
					throw CScriptException ("Expect integer as first parameter on timer creation");
				}
				if (!callback->isFunction ())
				{
					throw CScriptException (
						"Expect function as second parameter on timer creation");
				}
				auto timerObj = new TimerScriptObject (
					fireTime->getInt (), callback->deepCopy (), [this, context] (auto callback) {
						using namespace ScriptingInternal;
						ScriptAddChildScoped scs (*jsContext->getRoot (), "timerContext", context);
						return evalScript (callback, "timerCallback (timerContext); ",
										   "timerCallback");
					});
				timerObj->addRef ();
				var->setReturnVar (timerObj);
				timerObj->release ();
			});
		jsContext->addNative (
			"function iterateSubViews(view, context, callback)"sv, [this] (CScriptVar* var) {
				auto view = var->getParameter ("view"sv);
				auto context = var->getParameter ("context"sv);
				auto callback = var->getParameter ("callback"sv);
				if (!view->isObject ())
				{
					throw CScriptException ("Expect object as first parameter on iterateSubViews");
				}
				if (!callback->isFunction ())
				{
					throw CScriptException (
						"Expect function as second parameter on iterateSubViews");
				}
				auto it =
					std::find_if (viewScriptMap.begin (), viewScriptMap.end (),
								  [&] (const auto& el) { return el.second->getVar () == view; });
				if (it == viewScriptMap.end ())
					throw CScriptException ("View not found in iterateSubViews");
				auto container = it->first->asViewContainer ();
				if (!container)
					return; // no sub views
				container->forEachChild ([&] (auto child) {
					using namespace ScriptingInternal;
					auto childScriptObject = viewScriptMap.find (child);
					if (childScriptObject != viewScriptMap.end ())
					{
						ScriptAddChildScoped scs (*jsContext->getRoot (), "child",
												  childScriptObject->second->getVar ());
						ScriptAddChildScoped scs2 (*jsContext->getRoot (), "context", context);
						evalScript (callback, "callback (child, context); ", "callback");
					}
					else
					{
						auto scriptObj =
							addView (child, std::make_unique<ViewScriptObject> (child, this));
						ScriptAddChildScoped scs (*jsContext->getRoot (), "child",
												  scriptObj->getVar ());
						ScriptAddChildScoped scs2 (*jsContext->getRoot (), "context", context);
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
	CScriptVar* getRoot () const override { return jsContext->getRoot (); }
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

	bool evalScript (std::string_view script) noexcept override
	{
		try
		{
			auto result = jsContext->evaluateComplex (script);
			if (result.getVar () && !result.getVar ()->isUndefined ())
			{
#if DEBUG
				DebugPrint ("%s\n", result.getVar ()->getString ().data ());
#endif
			}
		}
		catch (const CScriptException& exc)
		{
#if DEBUG
			DebugPrint ("Scripting Exception: %s\n", exc.text.data ());
#endif
			if (onScriptException)
				onScriptException (exc.text);
			return false;
		}
		return true;
	}

	bool evalScript (CScriptVar* object, std::string_view script,
					 const std::string& objectName = "view") noexcept
	{
		if (!object || script.empty ())
			return false;
		vstgui_assert (object->getRefs () > 0);
		object->addRef ();
		ScriptAddChildScoped scs (*jsContext->getRoot (), objectName, object);
		auto result = evalScript (script);
		object->release ();
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
			ScriptAddChildScoped scs (*jsContext->getRoot (), "newSize", newSizeObject.getVar ());
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
				ScriptAddChildScoped scs (*jsContext->getRoot (), "child",
										  childScriptObject->second->getVar ());
				This->evalScript (obj->getVar (), script);
			}
			else
			{
				auto scriptObj = addView (view, std::make_unique<ViewScriptObject> (view, this));
				ScriptAddChildScoped scs (*jsContext->getRoot (), "child", scriptObj->getVar ());
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
			if (consume->getVar ()->isInt () && consume->getVar ()->getInt ())
				event.consumed = true;
		}
	}

	void callEventFunction (CScriptVar* var, Event& event, std::string_view script) noexcept
	{
		auto scriptEvent = ScriptingInternal::makeScriptEvent (event);
		ScriptAddChildScoped scs (*jsContext->getRoot (), "event", scriptEvent.getVar ());
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
			ScriptAddChildScoped scs (*jsContext->getRoot (), "value", controlValue.getVar ());
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

	void addView (CView* view, const std::string* script) noexcept
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
std::string ScriptContext::eval (std::string_view script) const
{
	if (!impl->jsContext)
		return {};
	try
	{
		auto result = impl->jsContext->evaluateComplex (script);
		std::stringstream stream;
		result.getVar ()->getJSON (stream);
		return stream.str ();
	}
	catch (const CScriptException& exc)
	{
		if (impl->onScriptException)
			impl->onScriptException (exc.text);
	}
	return {};
}

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
				 attr.setAttribute (key->getString (), value->getString ());
				 auto result = uiDesc->getViewFactory ()->applyAttributeValues (view, attr, uiDesc);
				 var->getReturnVar ()->setInt (result);
			 },
			 {"key", "value"});
	addFunc ("getAttribute"sv,
			 [uiDesc = context->getUIDescription (), view] (CScriptVar* var) {
				 auto key = var->getParameter ("key"sv);
				 std::string result;
				 if (uiDesc->getViewFactory ()->getAttributeValue (view, key->getString (), result,
																   uiDesc))
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
					 uiDesc->getViewFactory ()->viewIsTypeOf (view, typeName->getString ());
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
					 propValue = value->getString ();
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
//------------------------------------------------------------------------
//------------------------------------------------------------------------
DrawContextObject::DrawContextObject ()
{
	// TODO: decide if this object should expose the same interface as "CanvasRenderingContext2D"
	scriptVar->setLifeTimeObserver (this);
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
				 auto alpha = alphaVar ? alphaVar->getVar ()->getDouble () : 1.f;
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
				 UIViewCreator::stringToColor (&colorVar->getString (), color, uiDesc);
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
				 UIViewCreator::stringToColor (&colorVar->getString (), color, uiDesc);
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
				 if (!UIViewCreator::stringToColor (&colorVar->getString (), color, uiDesc))
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

	auto rectObj = makeScriptRect (rect);
	auto scriptRoot = scriptContext->getRoot ();
	ScriptAddChildScoped scs (*scriptRoot, "view", scriptObject->getVar ());
	ScriptAddChildScoped scs2 (*scriptRoot, "context", drawContext.getVar ());
	ScriptAddChildScoped scs3 (*scriptRoot, "rect", rectObj.getVar ());
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
struct JavaScriptViewFactory : ViewFactoryDelegate,
							   ViewListenerAdapter
{
	static constexpr CViewAttributeID scriptAttrID = 'scri';

	using Super = ViewFactoryDelegate;
	using ViewControllerLink = std::pair<CView*, IScriptControllerExtension*>;
	using ViewControllerLinkVector = std::vector<ViewControllerLink>;

	JavaScriptViewFactory (ScriptingInternal::ScriptContext* scripting, IViewFactory* origFactory)
	: Super (origFactory), scriptContext (scripting)
	{
	}

	~JavaScriptViewFactory () noexcept
	{
		std::for_each (viewControllerLinks.begin (), viewControllerLinks.end (),
					   [this] (const auto& el) {
						   el.first->unregisterViewListener (this);
						   el.second->scriptContextDestroyed (scriptContext);
					   });
	}

	CView* createView (const UIAttributes& attributes,
					   const IUIDescription* description) const override
	{
		if (auto view = Super::createView (attributes, description))
		{
			if (auto value = attributes.getAttributeValue (ScriptingInternal::kAttrScript))
			{
				std::optional<std::string> verifiedScript;
				if (auto scriptViewController =
						dynamic_cast<IScriptControllerExtension*> (description->getController ()))
				{
					verifiedScript =
						scriptViewController->verifyScript (view, *value, scriptContext);
					view->registerViewListener (const_cast<JavaScriptViewFactory*> (this));
					viewControllerLinks.emplace_back (view, scriptViewController);
				}
				const auto& script = verifiedScript ? *verifiedScript : *value;
				auto scriptSize = static_cast<uint32_t> (script.size () + 1);
				view->setAttribute (scriptAttrID, scriptSize, script.data ());
				if (!disabled)
				{
					scriptContext->onViewCreated (view, script);
				}
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
	void viewWillDelete (CView* view) override
	{
		auto it = std::find_if (viewControllerLinks.begin (), viewControllerLinks.end (),
								[view] (const auto& el) { return el.first == view; });
		if (it != viewControllerLinks.end ())
		{
			it->second->scriptContextDestroyed (scriptContext);
			viewControllerLinks.erase (it);
		}
		view->unregisterViewListener (this);
	}

	ScriptingInternal::ScriptContext* scriptContext;
	mutable ViewControllerLinkVector viewControllerLinks;
	bool disabled {false};
};

//------------------------------------------------------------------------
} // ScriptingInternal

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
struct UIScripting::Impl
{
	using JSViewFactoryPtr = std::unique_ptr<ScriptingInternal::JavaScriptViewFactory>;
	using ScriptContextPtr = std::unique_ptr<ScriptingInternal::ScriptContext>;

	std::unordered_map<const IUIDescription*, std::pair<JSViewFactoryPtr, ScriptContextPtr>> map;

	static OnScriptException onScriptExceptionFunc;
};
UIScripting::OnScriptException UIScripting::Impl::onScriptExceptionFunc =
	[] (const std::string& reason) {
		std::cerr << reason << '\n';
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
	auto scripting =
		std::make_unique<ScriptingInternal::ScriptContext> (desc, Impl::onScriptExceptionFunc);
	auto viewFactory = std::make_unique<ScriptingInternal::JavaScriptViewFactory> (scripting.get (),
																				   originalFactory);
	auto result =
		impl->map.emplace (desc, std::make_pair (std::move (viewFactory), std::move (scripting)));
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
void UIScripting::init (const OnScriptException& func)
{
	if (func)
		Impl::onScriptExceptionFunc = func;
	UIDescriptionAddOnRegistry::add (std::make_unique<UIScripting> ());
	static ScriptingInternal::JavaScriptDrawableViewCreator jsViewCreator;
	UIViewFactory::registerViewCreator (jsViewCreator);
	static ScriptingInternal::JavaScriptDrawableControlCreator jsControlCreator;
	UIViewFactory::registerViewCreator (jsControlCreator);
}

//------------------------------------------------------------------------
} // VSTGUI
