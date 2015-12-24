#pragma once

#include "../interface.h"
#include "../icommand.h"
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
class IApplicationPlatformAccess : public Interface
{
public:
	using OnCommandUpdateFunc = std::function<void ()>;
	using QuitFunc = std::function<void ()>;
	using CommandWithKeyList = std::vector<CommandWithKey>;
	using CommandListPair = std::pair<std::string, CommandWithKeyList>;
	using CommandList = std::vector<CommandListPair>;

	virtual void init () = 0;

	virtual void setOnCommandUpdate (const OnCommandUpdateFunc& func) = 0;
	virtual void setQuitFunction (const QuitFunc& func) = 0;
	virtual const CommandList& getCommandList () = 0;
};

} // Detail
} // Standalone
} // VSTGUI
