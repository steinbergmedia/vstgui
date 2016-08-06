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
class IMenuBuilder;
class IValue;
class IStepValue;
class IValueListener;
class IValueStringConverter;
class ISharedUIResources;

using WindowPtr = std::shared_ptr<IWindow>;
using WindowControllerPtr = std::shared_ptr<IWindowController>;
using ValuePtr = std::shared_ptr<IValue>;
using ValueStringConverterPtr = std::shared_ptr<IValueStringConverter>;

struct Command;
struct AlertBoxConfig;
struct AlertBoxForWindowConfig;

enum class AlertResult;

//------------------------------------------------------------------------
namespace UIDesc {

class IModelBinding;
class ICustomization;
using ModelBindingPtr = std::shared_ptr<IModelBinding>;
using CustomizationPtr = std::shared_ptr<ICustomization>;

struct Config;

//------------------------------------------------------------------------
} // UIDesc

//------------------------------------------------------------------------
namespace Application {

class IDelegate;
using DelegatePtr = std::shared_ptr<IDelegate>;

//------------------------------------------------------------------------
} // Application

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
