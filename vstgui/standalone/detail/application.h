#pragma once

#include "../interface.h"
#include "../icommand.h"
#include "../ialertbox.h"
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
	using CommandListPair = std::pair<std::string, CommandWithKeyList>;
	using CommandList = std::vector<CommandListPair>;

	virtual void init (IPreference& preferences) = 0;

	virtual void setPlatformCallbacks (PlatformCallbacks&& callbacks) = 0;
	virtual const CommandList& getCommandList () = 0;
};

} // Detail
} // Standalone
} // VSTGUI
