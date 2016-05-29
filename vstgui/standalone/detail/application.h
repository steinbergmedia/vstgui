#pragma once

#include "../ialertbox.h"
#include "../iapplication.h"
#include "../icommand.h"
#include "platform/iplatformwindow.h"
#include "../interface.h"
#include <functional>
#include <vector>

namespace VSTGUI {
namespace Standalone {
namespace Detail {

//------------------------------------------------------------------------
struct CommandWithKey : Command
{
	char16_t defaultKey;
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
class IApplicationPlatformAccess : public Interface
{
public:
	using CommandWithKeyList = std::vector<CommandWithKey>;
	using CommandListPair = std::pair<UTF8String, CommandWithKeyList>;
	using CommandList = std::vector<CommandListPair>;

	virtual void init (IPreference& preferences, IApplication::CommandLineArguments&& cmdArgs,
	                   PlatformCallbacks&& callbacks) = 0;

	virtual const CommandList& getCommandList () = 0;
	virtual bool canQuit () = 0;

	virtual bool dontClosePopupOnDeactivation (Platform::IWindow* window) = 0;
};

//------------------------------------------------------------------------
inline IApplicationPlatformAccess* getApplicationPlatformAccess ()
{
	return IApplication::instance ().dynamicCast<IApplicationPlatformAccess> ();
}

//------------------------------------------------------------------------
class PreventPopupClose
{
public:
	PreventPopupClose (IWindow* window);
	~PreventPopupClose () noexcept;

private:
	Platform::IWindow* platformWindow {nullptr};
};

//------------------------------------------------------------------------
} // Detail
} // Standalone
} // VSTGUI
