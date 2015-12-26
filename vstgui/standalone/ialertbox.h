#pragma once

#include "fwd.h"
#include "interface.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

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
enum class AlertResult
{
	defaultButton,
	secondButton,
	thirdButton,
	error,
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
