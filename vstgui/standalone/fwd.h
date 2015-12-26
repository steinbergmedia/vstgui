#pragma once

#include <memory>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

class IWindow;
class IWindowController;
class IWindowListener;
class IPreference;
class ICommandHandler;
class IValue;
class IStepValue;
class IValueListener;
class IValueStringConverter;

using WindowPtr = std::shared_ptr<IWindow>;
using WindowControllerPtr = std::shared_ptr<IWindowController>;
using ValuePtr = std::shared_ptr<IValue>;

struct Command;
struct AlertBoxConfig;
struct AlertBoxForWindowConfig;

enum class AlertResult;

//------------------------------------------------------------------------
namespace UIDescription {

class IModelBinding;
using ModelBindingPtr = std::shared_ptr<IModelBinding>;

struct Config;

//------------------------------------------------------------------------
} // UIDescription

//------------------------------------------------------------------------
namespace Application {

class IDelegate;
using DelegatePtr = std::shared_ptr<IDelegate>;

//------------------------------------------------------------------------
} // Application

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
