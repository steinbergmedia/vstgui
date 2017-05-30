// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
	DefaultButton,
	SecondButton,
	ThirdButton,
	Error,
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
