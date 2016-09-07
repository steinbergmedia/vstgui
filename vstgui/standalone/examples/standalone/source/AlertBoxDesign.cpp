#include "AlertBoxDesign.h"
#include "vstgui/standalone/iuidescwindow.h"
#include "vstgui/standalone/helpers/modelbinding.h"
#include "vstgui/standalone/helpers/value.h"

namespace VSTGUI {
namespace Standalone {

WindowPtr makeAlertBoxDesignWindow ()
{
	auto modelBinding = std::make_shared<UIDesc::ModelBindingCallbacks> ();
	modelBinding->addValue(Value::make("AlertBox.firstButton"));
	modelBinding->addValue(Value::make("AlertBox.secondButton"));
	modelBinding->addValue(Value::make("AlertBox.thirdButton"));
	modelBinding->addValue (Value::makeStringListValue ("AlertBox.headline", {"This is a test headline"}))
	->setActive (false);
	modelBinding->addValue (Value::makeStringListValue ("AlertBox.description", {"This is a test description"}))
	->setActive (false);
	UIDesc::Config config;
	config.uiDescFileName = "alertbox.uidesc";
	config.viewName = "AlertBox";
	config.windowConfig.type = WindowType::Document;
	config.windowConfig.style.close ().movableByWindowBackground ().transparent ();
	config.windowConfig.title = "Alert Box";
	config.modelBinding = modelBinding;
	
	return UIDesc::makeWindow (config);
}

} // Standalone
} // VSTGUI
