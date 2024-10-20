// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uiscripting.h"
#include "detail/converters.h"
#include "detail/scriptobject.h"
#include "detail/uidescscriptobject.h"
#include "detail/drawcontextobject.h"
#include "detail/drawable.h"
#include "detail/iscriptcontextinternal.h"
#include "detail/scriptingviewfactory.h"
#include "../uidescription/uiattributes.h"
#include "../uidescription/uiviewfactory.h"
#include "../uidescription/uidescriptionaddonregistry.h"
#include "../lib/iviewlistener.h"
#include "../lib/cresourcedescription.h"
#include "../lib/cvstguitimer.h"
#include "../lib/platform/platformfactory.h"
#include "../lib/platform/iplatformresourceinputstream.h"

#include "tiny-js/TinyJS.h"
#include "tiny-js/TinyJS_Functions.h"
#include "tiny-js/TinyJS_MathFunctions.h"

#include <iostream>
#include <sstream>

//------------------------------------------------------------------------
namespace VSTGUI {
using namespace std::literals;
using namespace TJS;

namespace ScriptingInternal {

struct ViewScriptObject;

//------------------------------------------------------------------------
class ScriptContext : public IScriptContextInternal
{
public:
	using OnScriptExceptionFunc = UIScripting::OnScriptExceptionFunc;
	using ReadScriptContentsFunc = UIScripting::ReadScriptContentsFunc;

	ScriptContext (IUIDescription* uiDesc, OnScriptExceptionFunc&& onExceptionFunc,
				   ReadScriptContentsFunc&& readContentsFunc);
	~ScriptContext () noexcept;

	void init (const std::string& initScript);
	void onViewCreated (CView* view, const std::string& script) override;

	void reset ();

private:
	std::string eval (std::string_view script) const override;

	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
struct TimerScriptObject : ScriptObject
{
	template<typename Proc>
	TimerScriptObject (uint64_t fireTime, CScriptVar* _callback, Proc timerProc)
	{
		auto cb = owning (_callback);
		auto t = makeOwned<CVSTGUITimer> (
			[timerProc, cb] (auto timer) {
				if (!timerProc (cb))
					timer->stop ();
			},
			static_cast<uint32_t> (fireTime), false);
		scriptVar->setCustomData (t);
		addFunc ("invalid"sv, [t] (auto v) mutable {
			t = nullptr;
			v->setCustomData (nullptr);
		});
		addFunc ("start"sv, [t] (auto) { t->start (); });
		addFunc ("stop"sv, [t] (auto) { t->stop (); });
		setOnDestroy ([cb] (auto) { cb->release (); });
	}
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
	OnScriptExceptionFunc onScriptException;
	ReadScriptContentsFunc readScriptContents;
	using ViewScriptMap = ScriptingInternal::ViewScriptMap;
	ViewScriptMap viewScriptMap;

	UIDescScriptObject uiDescObject;

	Impl (IUIDescription* uiDesc, OnScriptExceptionFunc&& onExceptionFunc,
		  ReadScriptContentsFunc&& readContentsFunc)
	: uiDesc (uiDesc)
	, onScriptException (std::move (onExceptionFunc))
	, readScriptContents (std::move (readContentsFunc))
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

		jsContext->getRoot ()->addChild ("uiDesc"sv, uiDescObject);

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
				auto timerObj = TimerScriptObject (
					fireTime->getInt (), callback->deepCopy (), [this, context] (auto callback) {
						using namespace ScriptingInternal;
						ScriptAddChildScoped scs (*jsContext->getRoot (), "timerContext"sv,
												  context);
						return evalScript (callback, "timerCallback (timerContext);"sv,
										   "timerCallback");
					});
				var->setReturnVar (timerObj);
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
						ScriptAddChildScoped scs (*jsContext->getRoot (), "child"sv,
												  childScriptObject->second->getVar ());
						ScriptAddChildScoped scs2 (*jsContext->getRoot (), "context"sv, context);
						evalScript (callback, "callback (child, context);"sv, "callback");
					}
					else
					{
						auto scriptObj =
							addView (child, std::make_unique<ViewScriptObject> (child, this));
						ScriptAddChildScoped scs (*jsContext->getRoot (), "child"sv,
												  scriptObj->getVar ());
						ScriptAddChildScoped scs2 (*jsContext->getRoot (), "context"sv, context);
						evalScript (callback, "callback (child, context);"sv, "callback");
					}
				});
			});
		jsContext->addNative ("function log(obj)",
							  [this] (CScriptVar* var) { log (var->getParameter ("obj"sv)); });

		jsContext->addNative ("function makeTransformMatrix()", [] (CScriptVar* var) {
			auto tm = makeTransformMatrixObject ();
			var->setReturnVar (tm);
			tm->release ();
		});
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

	ScriptObject evalScript (std::string_view script) noexcept override
	{
		try
		{
			auto result = jsContext->evaluateComplex (script);
#if 0 // DEBUG
			if (result.getVar () && !result.getVar ()->isUndefined ())
			{
				DebugPrint ("%s\n", result.getVar ()->getString ().data ());
			}
#endif
			return result.getVar ();
		}
		catch (const CScriptException& exc)
		{
#if DEBUG
			DebugPrint ("Scripting Exception: %s\n", exc.text.data ());
#endif
			if (onScriptException)
				onScriptException (exc.text);
			return {};
		}
		return {};
	}

	ScriptObject evalScript (CScriptVar* object, std::string_view script,
							 const std::string& objectName = "view") noexcept
	{
		if (!object || script.empty ())
			return {};
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
			ScriptAddChildScoped scs (*jsContext->getRoot (), "newSize"sv, newSizeObject);
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
				ScriptAddChildScoped scs (*jsContext->getRoot (), "child"sv,
										  childScriptObject->second->getVar ());
				This->evalScript (obj->getVar (), script);
			}
			else
			{
				auto scriptObj = addView (view, std::make_unique<ViewScriptObject> (view, this));
				ScriptAddChildScoped scs (*jsContext->getRoot (), "child"sv, scriptObj->getVar ());
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
		if (auto consume = obj->findChild ("consumed"))
		{
			if (consume->getVar ()->isInt () && consume->getVar ()->getInt ())
				event.consumed = true;
		}
	}

	void callEventFunction (CScriptVar* var, Event& event, std::string_view script) noexcept
	{
		auto scriptEvent = ScriptingInternal::makeScriptEvent (event);
		ScriptAddChildScoped scs (*jsContext->getRoot (), "event"sv, scriptEvent);
		evalScript (var, script);
		checkEventConsumed (scriptEvent, event);
	}

	void viewOnEvent (CView* view, Event& event) override
	{
		auto applyEventMouseLocalPosition = [&] (auto proc) {
			auto& mouseEvent = castMousePositionEvent (event);
			auto oldPos = mouseEvent.mousePosition;
			mouseEvent.mousePosition -= view->getViewSize ().getTopLeft ();
			proc ();
			mouseEvent.mousePosition = oldPos;
		};
		switch (event.type)
		{
			case EventType::MouseEnter:
			{
				applyEventMouseLocalPosition ([&] () {
					callWhenScriptHasFunction (view, "onMouseEnter"sv, [&] (auto This, auto& obj) {
						static constexpr auto script = R"(view.onMouseEnter(view, event);)"sv;
						callEventFunction (obj->getVar (), event, script);
					});
				});
				break;
			}
			case EventType::MouseExit:
			{
				applyEventMouseLocalPosition ([&] () {
					callWhenScriptHasFunction (view, "onMouseExit"sv, [&] (auto This, auto& obj) {
						static constexpr auto script = R"(view.onMouseExit(view, event);)"sv;
						callEventFunction (obj->getVar (), event, script);
					});
				});
				break;
			}
			case EventType::MouseDown:
			{
				applyEventMouseLocalPosition ([&] () {
					callWhenScriptHasFunction (view, "onMouseDown"sv, [&] (auto This, auto& obj) {
						static constexpr auto script = R"(view.onMouseDown(view, event);)"sv;
						callEventFunction (obj->getVar (), event, script);
					});
				});
				break;
			}
			case EventType::MouseUp:
			{
				applyEventMouseLocalPosition ([&] () {
					callWhenScriptHasFunction (view, "onMouseUp"sv, [&] (auto This, auto& obj) {
						static constexpr auto script = R"(view.onMouseUp(view, event);)"sv;
						callEventFunction (obj->getVar (), event, script);
					});
				});
				break;
			}
			case EventType::MouseMove:
			{
				applyEventMouseLocalPosition ([&] () {
					callWhenScriptHasFunction (view, "onMouseMove"sv, [&] (auto This, auto& obj) {
						static constexpr auto script = R"(view.onMouseMove(view, event);)"sv;
						callEventFunction (obj->getVar (), event, script);
					});
				});
				break;
			}
			case EventType::MouseWheel:
			{
				applyEventMouseLocalPosition ([&] () {
					callWhenScriptHasFunction (view, "onMouseWheel"sv, [&] (auto This, auto& obj) {
						static constexpr auto script = R"(view.onMouseWheel(view, event);)"sv;
						callEventFunction (obj->getVar (), event, script);
					});
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
			controlValue->setDouble (control->getValue ());
			ScriptAddChildScoped scs (*jsContext->getRoot (), "value"sv, controlValue);
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
					return readScriptContents (scriptFileStr);
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
ScriptContext::ScriptContext (IUIDescription* uiDesc, OnScriptExceptionFunc&& onExceptionFunc,
							  ReadScriptContentsFunc&& readContentsFunc)
{
	impl =
		std::make_unique<Impl> (uiDesc, std::move (onExceptionFunc), std::move (readContentsFunc));
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
} // ScriptingInternal

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
struct UIScripting::Impl
{
	using JSViewFactoryPtr = std::unique_ptr<ScriptingInternal::JavaScriptViewFactory>;
	using ScriptContextPtr = std::unique_ptr<ScriptingInternal::ScriptContext>;

	std::unordered_map<const IUIDescription*, std::pair<JSViewFactoryPtr, ScriptContextPtr>> map;

	static OnScriptExceptionFunc onScriptExceptionFunc;
	static ReadScriptContentsFunc readScriptContentsFunc;

	static std::string readScriptContentsFromResource (std::string_view filename);
};

//------------------------------------------------------------------------
UIScripting::OnScriptExceptionFunc UIScripting::Impl::onScriptExceptionFunc;
UIScripting::ReadScriptContentsFunc UIScripting::Impl::readScriptContentsFunc;

//------------------------------------------------------------------------
std::string UIScripting::Impl::readScriptContentsFromResource (std::string_view filename)
{
	std::string fileStr (filename);
	CResourceDescription desc (fileStr.data ());
	if (auto stream = getPlatformFactory ().createResourceInputStream (desc))
	{
		std::string fileContent;
		while (true)
		{
			char buffer[256];
			auto numRead = stream->readRaw (buffer, static_cast<uint32_t> (std::size (buffer)));
			if (numRead == kStreamIOError)
				return {};
			fileContent.append (buffer, numRead);
			if (numRead < std::size (buffer))
				break;
		}
		return fileContent;
	}
	return {};
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
	using namespace ScriptingInternal;

	auto it = impl->map.find (desc);
	if (it != impl->map.end ())
		return it->second.first.get ();
	auto onScriptException = Impl::onScriptExceptionFunc;
	if (!onScriptException)
		onScriptException = [] (std::string_view reason) {
			std::cerr << reason << '\n';
		};
	auto readScriptContentsFunc = [] (auto filename) {
		std::string result;
		if (Impl::readScriptContentsFunc)
			result = Impl::readScriptContentsFunc (filename);
		if (result.empty ())
		{
			result = Impl::readScriptContentsFromResource (filename);
		}
		return result;
	};
	auto scripting = std::make_unique<ScriptContext> (desc, std::move (onScriptException),
													  std::move (readScriptContentsFunc));
	auto viewFactory = std::make_unique<JavaScriptViewFactory> (scripting.get (), originalFactory);
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
void UIScripting::init (const OnScriptExceptionFunc& onExceptionFunc,
						const ReadScriptContentsFunc& readScriptContentsFunc)
{
	if (onExceptionFunc)
		Impl::onScriptExceptionFunc = onExceptionFunc;
	if (readScriptContentsFunc)
		Impl::readScriptContentsFunc = readScriptContentsFunc;
	UIDescriptionAddOnRegistry::add (std::make_unique<UIScripting> ());
	static ScriptingInternal::JavaScriptDrawableViewCreator jsViewCreator;
	UIViewFactory::registerViewCreator (jsViewCreator);
	static ScriptingInternal::JavaScriptDrawableControlCreator jsControlCreator;
	UIViewFactory::registerViewCreator (jsControlCreator);
}

//------------------------------------------------------------------------
} // VSTGUI
