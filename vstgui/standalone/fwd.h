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
class IValue;
class IStepValue;
class IValueListener;
class IValueStringConverter;

using WindowControllerPtr = std::shared_ptr<IWindowController>;
using ValuePtr = std::shared_ptr<IValue>;

struct Command;
struct AlertBoxConfig;

enum class AlertResult;

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
