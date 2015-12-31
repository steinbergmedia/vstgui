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

	auto valueList = std::make_shared<UIDesc::ValueList> ();
	auto closeValue = IValue::make ("Close", 0);
	valueList->addValue (closeValue);
	closeValue->registerListener (gInstance.get ());

	UIDesc::Config c;
	c.uiDescFileName = "about.uidesc";
	c.viewName = "view";
	c.modelBinding = valueList;
	c.windowConfig.type = WindowType::Popup;
	c.windowConfig.style.close ().transparent ();
	c.windowConfig.autoSaveFrameName = "AboutDialogFrame";
	gInstance->window = UIDesc::makeWindow (c);
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
void About::onEndEdit (const IValue& value)
{
	if (value.getValue () > 0.)
		window->close ();
}

//------------------------------------------------------------------------
} // MyApp
