#pragma once

#include "fwd.h"
#include "interface.h"
#include <functional>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
enum class AlertResult
{
	defaultButton,
	secondButton,
	thirdButton,
	error,
};

//------------------------------------------------------------------------
struct AlertBoxConfig
{
	UTF8String headline;
	UTF8String description;
	UTF8String defaultButton {"OK"};
	UTF8String secondButton;
	UTF8String thirdButton;
};

//------------------------------------------------------------------------
struct AlertBoxForWindowConfig : AlertBoxConfig
{
	using Callback = std::function<void (AlertResult)>;

	WindowPtr window;
	Callback callback;
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
