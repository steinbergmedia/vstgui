// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformtimer.h"
#include "x11frame.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace X11 {

//------------------------------------------------------------------------
class Timer : public IPlatformTimer, public ITimerHandler
{
public:
	Timer (IPlatformTimerCallback* callback);
	~Timer () noexcept;

	bool start (uint32_t periodMs) override;
	bool stop () override;

	void onTimer () override;

private:
	IPlatformTimerCallback* callback = nullptr;
};

//------------------------------------------------------------------------
} // X11
} // VSTGUI
