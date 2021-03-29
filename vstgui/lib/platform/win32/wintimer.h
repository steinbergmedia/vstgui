// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformtimer.h"

#if WINDOWS

//------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
class WinTimer final : public IPlatformTimer
{
public:
	WinTimer (IPlatformTimerCallback* callback);
	~WinTimer () noexcept;

	bool start (uint32_t fireTime) override;
	bool stop () override;
private:

	unsigned long long timer;
	IPlatformTimerCallback* callback;
};

//------------------------------------------------------------------------
} // VSTGUI

#endif // WINDOWS
