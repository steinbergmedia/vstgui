#pragma once

#include <memory>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
namespace Application {
class IDelegate;
//------------------------------------------------------------------------
} // Application

class IWindow;
class IWindowController;
class IWindowListener;
class ICommandHandler;

using WindowControllerPtr = std::shared_ptr<IWindowController>;

struct Command;

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
