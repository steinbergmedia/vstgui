// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uiscripting.h"
#include "detail/converters.h"
#include "detail/scriptobject.h"
#include "detail/uidescscriptobject.h"
#include "detail/drawcontextobject.h"
#include "detail/drawable.h"
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

static const std::string kAttrScript = "script";

struct ViewScriptObject;

//------------------------------------------------------------------------
class ScriptContext : public IScriptContext
{
public:
	using OnScriptException = std::function<void (std::string_view reason)>;

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
struct TimerScriptObject : CScriptVar
{
	template<typename Proc>
	TimerScriptObject (uint64_t fireTime, CScriptVar* _callback, Proc timerProc)
	: CScriptVar (TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT), callback (owning (_callback))
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
				var->setReturnVar (owning (timerObj));
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
	[] (std::string_view reason) {
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
