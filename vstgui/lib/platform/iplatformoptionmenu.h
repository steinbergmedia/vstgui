// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

/// @cond ignore

#include "../vstguifwd.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
struct PlatformOptionMenuResult
{
	SharedPointer<COptionMenu> menu;
	int32_t index;
};

//-----------------------------------------------------------------------------
class IPlatformOptionMenu : public AtomicReferenceCounted
{
public:
	using Callback = std::function<void (const SharedPointer<COptionMenu>& optionMenu,
										 PlatformOptionMenuResult result)>;
	virtual void popup (const SharedPointer<COptionMenu>& optionMenu, const Callback& callback) = 0;
};

} // VSTGUI

/// @endcond
