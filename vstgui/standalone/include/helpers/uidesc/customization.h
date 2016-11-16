#pragma once

#include "../../iuidescwindow.h"
#include <string>
#include <unordered_map>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace UIDesc {

//------------------------------------------------------------------------
class Customization : public ICustomization
{
public:
	static std::shared_ptr<Customization> make () { return std::make_shared<Customization> (); }

	using CreateViewControllerFunc = std::function<IController*(
	    const UTF8StringView& name, IController* parent, const IUIDescription* uiDesc)>;

	void addCreateViewController (const UTF8String& name, CreateViewControllerFunc func)
	{
		createViewControllerMap.emplace (name.getString (), func);
	}

	IController* createController (const UTF8StringView& name, IController* parent,
	                               const IUIDescription* uiDesc)
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
