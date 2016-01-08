#include "testabout.h"
#include "vstgui/standalone/iapplication.h"
#include "vstgui/standalone/iuidescwindow.h"

//------------------------------------------------------------------------
namespace MyApp {

using namespace VSTGUI::Standalone;

//------------------------------------------------------------------------
std::unique_ptr<About> About::gInstance;

//------------------------------------------------------------------------
void About::show ()
{
	if (gInstance)
	{
		gInstance->window->activate ();
		return;
	}
	gInstance = std::unique_ptr<About> (new About ());
	auto modelBinding = std::make_shared<ModelBindingCallbacks> ();
	auto close = [] (const IValue& value) {
		if (gInstance && value.getValue () > 0.)
			gInstance->window->close ();
	};
	modelBinding->addValue (IValue::make ("Close"), ValueCalls::onEndEdit (close));

	UIDesc::Config config;
	config.uiDescFileName = "about.uidesc";
	config.viewName = "view";
	config.modelBinding = modelBinding;
	config.windowConfig.type = WindowType::Document;
	config.windowConfig.title = "About";
	config.windowConfig.style.close ().transparent ().movableByWindowBackground ();
	config.windowConfig.autoSaveFrameName = "AboutDialogFrame";
	gInstance->window = UIDesc::makeWindow (config);
	if (gInstance->window)
	{
		gInstance->window->registerWindowListener (gInstance.get ());
		gInstance->window->show ();
	}
}

//------------------------------------------------------------------------
void About::onClosed (const IWindow& window)
{
	gInstance = nullptr;
}

//------------------------------------------------------------------------
ModelBindingCallbacks::~ModelBindingCallbacks ()
{
	for (auto& v : valueList)
		v->unregisterListener (this);
	values.clear ();
	valueList.clear ();
}

//------------------------------------------------------------------------
void ModelBindingCallbacks::addValue (ValuePtr value, const ValueCalls& callbacks)
{
	values.emplace (value.get (), callbacks);
	valueList.emplace_back (value);
	value->registerListener (this);
}

//------------------------------------------------------------------------
void ModelBindingCallbacks::addValue (ValuePtr value, ValueCalls&& callbacks)
{
	values.emplace (value.get (), std::move (callbacks));
	valueList.emplace_back (value);
	value->registerListener (this);
}

//------------------------------------------------------------------------
ValuePtr ModelBindingCallbacks::getValue (const UTF8String& valueID) const
{
	for (auto& v : valueList)
	{
		if (v->getID () == valueID)
			return v;
	}
	return nullptr;
}

//------------------------------------------------------------------------
void ModelBindingCallbacks::onBeginEdit (const IValue& value)
{
	auto it = values.find (&value);
	if (it != values.end () && it->second.onBeginEditCall)
		it->second.onBeginEditCall (value);
}

//------------------------------------------------------------------------
void ModelBindingCallbacks::onPerformEdit (const IValue& value, IValue::Type newValue)
{
	auto it = values.find (&value);
	if (it != values.end () && it->second.onPerformEditCall)
		it->second.onPerformEditCall (value);
}

//------------------------------------------------------------------------
void ModelBindingCallbacks::onEndEdit (const IValue& value)
{
	auto it = values.find (&value);
	if (it != values.end () && it->second.onEndEditCall)
		it->second.onEndEditCall (value);
}

//------------------------------------------------------------------------
void ModelBindingCallbacks::onStateChange (const IValue& value)
{
	auto it = values.find (&value);
	if (it != values.end () && it->second.onStateChangeCall)
		it->second.onStateChangeCall (value);
	
}

//------------------------------------------------------------------------
} // MyApp
