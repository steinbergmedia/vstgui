#pragma once

#include "fwd.h"
#include "iwindow.h"
#include "icommand.h"
#include "ivalue.h"

#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace UIDescription {

//------------------------------------------------------------------------
class IModelHandler : public ICommandHandler
{
public:
	using CommandList = std::vector<Command>;
	using ValueList = std::vector<ValuePtr>;
	
	virtual const CommandList& getCommands () const = 0;
	virtual const ValueList& getValues () const = 0;
};

//------------------------------------------------------------------------
using ModelHandlerPtr = std::shared_ptr<IModelHandler>;

//------------------------------------------------------------------------
struct Config {
	UTF8String fileName;
	UTF8String viewName;
	ModelHandlerPtr modelHandler;
	WindowConfiguration windowConfig;
};

//------------------------------------------------------------------------
WindowPtr makeWindow (const Config& config);

//------------------------------------------------------------------------
} // UIDescription
} // Standalone
} // VSTGUI
