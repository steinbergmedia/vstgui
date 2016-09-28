#pragma once

#include "fwd.h"
#include "../../lib/cstring.h"
#include "interface.h"
#include <functional>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
/** Alert result
 *
 *	@ingroup standalone
 */
enum class AlertResult
{
	defaultButton,
	secondButton,
	thirdButton,
	error,
};

//------------------------------------------------------------------------
/** Alertbox configuration
 *
 *	@ingroup standalone
 */
struct AlertBoxConfig
{
	UTF8String headline;
	UTF8String description;
	UTF8String defaultButton {"OK"};
	UTF8String secondButton;
	UTF8String thirdButton;
};

//------------------------------------------------------------------------
/** Alertbox for window configuration
 *
 *	@ingroup standalone
 */
struct AlertBoxForWindowConfig : AlertBoxConfig
{
	using Callback = std::function<void (AlertResult)>;

	WindowPtr window;
	Callback callback;
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
