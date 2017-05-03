// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
class IValueConverter;
class ISharedUIResources;
class ICommonDirectories;

using WindowPtr = std::shared_ptr<IWindow>;
using WindowControllerPtr = std::shared_ptr<IWindowController>;
using ValuePtr = std::shared_ptr<IValue>;
using ValueConverterPtr = std::shared_ptr<IValueConverter>;

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
using DelegatePtr = std::unique_ptr<IDelegate>;

//------------------------------------------------------------------------
} // Application

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
