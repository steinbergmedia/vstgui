// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../iuidescwindow.h"
#include <string>
#include <unordered_map>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace UIDesc {

//------------------------------------------------------------------------
/**	ICustomization adapter
 *	@ingroup standalone
 */
class CustomizationAdapter : public ICustomization
{
public:
	IController* createController (const UTF8StringView& name, IController* parent,
	                               const IUIDescription* uiDesc) override
	{
		return nullptr;
	}

	void onUIDescriptionParsed (const IUIDescription* uiDesc) override {}
};

//------------------------------------------------------------------------
/** Customization helper for an UIDesc window
 *
 *	Use this class to create controllers for your views
 *
 *	Example:
 *	@code{.cpp}
 *	using namespace VSTGUI::Standalone;
 *	auto customization = UIDesc::Customization::make ();
 *
 *	customization->addCreateViewControllerFunc (
 *		"MyFirstViewController", [] (const auto& name, auto parent, const auto uiDesc) {
 *			return new MyFirstViewController (parent);
 *		});
 *	customization->addCreateViewControllerFunc (
 *		"MySecondViewController", [] (const auto& name, auto parent, const auto uiDesc) {
 *			return new MySecondViewController (parent);
 *		});
 *
 *	UIDesc::Config config;
 *	config.uiDescFileName = "Window.uidesc";
 *	config.viewName = "Window";
 *	config.customization = customization;
 *	config.windowConfig.title = "MyWindow";
 *	config.windowConfig.style.border ().close ().size ().centered ();
 *	if (auto window = UIDesc::makeWindow (config))
 *		window->show ();
 *
 *	@endcode
 *
 *	The view controller MyFirstViewController will be created when the sub-controller attribute of a
 *	view is equal to "MyFirstController" and the same for "MySecondViewController".
 *
 *	@ingroup standalone
 */
class Customization : public CustomizationAdapter
{
public:
	static std::shared_ptr<Customization> make () { return std::make_shared<Customization> (); }

	using CreateViewControllerFunc = std::function<IController*(
	    const UTF8StringView& name, IController* parent, const IUIDescription* uiDesc)>;

	void addCreateViewControllerFunc (const UTF8String& name, CreateViewControllerFunc func)
	{
		createViewControllerMap.emplace (name.getString (), func);
	}

	IController* createController (const UTF8StringView& name, IController* parent,
	                               const IUIDescription* uiDesc) override
	{
		auto it = createViewControllerMap.find (std::string (name));
		if (it != createViewControllerMap.end ())
		{
			return it->second (name, parent, uiDesc);
		}
		return nullptr;
	}

private:
	using CreateViewControllerMap = std::unordered_map<std::string, CreateViewControllerFunc>;

	CreateViewControllerMap createViewControllerMap;
};

//------------------------------------------------------------------------
} // UIDesc
} // Standalone
} // VSTGUI
