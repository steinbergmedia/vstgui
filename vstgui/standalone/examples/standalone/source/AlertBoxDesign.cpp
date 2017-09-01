// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "AlertBoxDesign.h"
#include "vstgui/standalone/include/iuidescwindow.h"
#include "vstgui/standalone/include/helpers/uidesc/modelbinding.h"
#include "vstgui/standalone/include/helpers/value.h"

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
