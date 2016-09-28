#pragma once

#include "../include/ialertbox.h"
#include "../include/iapplication.h"
#include "../include/icommand.h"
#include "../include/iwindowlistener.h"
#include "platform/iplatformwindow.h"
#include <functional>
#include <vector>

namespace VSTGUI {
namespace Standalone {
namespace Detail {

//------------------------------------------------------------------------
struct CommandWithKey : Command
{
	char16_t defaultKey;
	uint16_t id;
};

//------------------------------------------------------------------------
struct PlatformCallbacks
{
	using OnCommandUpdateFunc = std::function<void ()>;
	using QuitFunc = std::function<void ()>;
	using AlertFunc = std::function<AlertResult (const AlertBoxConfig&)>;
	using AlertForWindowFunc = std::function<void (const AlertBoxForWindowConfig&)>;

	QuitFunc quit;
	OnCommandUpdateFunc onCommandUpdate;
	AlertFunc showAlert;
	AlertForWindowFunc showAlertForWindow;
};

//------------------------------------------------------------------------
class IPlatformApplication : public IApplication, public IWindowListener, public ICommandHandler
{
public:
	using CommandWithKeyList = std::vector<CommandWithKey>;
	using CommandListPair = std::pair<UTF8String, CommandWithKeyList>;
	using CommandList = std::vector<CommandListPair>;

	virtual void init (IPreference& preferences, UTF8String&& applicationPath,
	                   IApplication::CommandLineArguments&& cmdArgs,
	                   PlatformCallbacks&& callbacks) = 0;

	virtual const CommandList& getCommandList () = 0;
	virtual bool canQuit () = 0;

	virtual bool dontClosePopupOnDeactivation (Platform::IWindow* window) = 0;
};

//------------------------------------------------------------------------
inline IPlatformApplication* getApplicationPlatformAccess ()
{
	return static_cast<IPlatformApplication*> (&IApplication::instance ());
}

//------------------------------------------------------------------------
class PreventPopupClose
{
public:
	PreventPopupClose (IWindow& window);
	~PreventPopupClose () noexcept;

private:
	std::shared_ptr<Platform::IWindow> platformWindow;
};

//------------------------------------------------------------------------
} // Detail
} // Standalone
} // VSTGUI
