// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../../../lib/platform/linux/irunloop.h"
#include <memory>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace GDK {

//------------------------------------------------------------------------
class RunLoop : public VSTGUI::X11::IRunLoop
{
public:
	using IEventHandler = VSTGUI::X11::IEventHandler;
	using ITimerHandler = VSTGUI::X11::ITimerHandler;

	static RunLoop& instance ();

	RunLoop ();
	~RunLoop () noexcept;

	bool registerEventHandler (int fd, IEventHandler* handler) override;
	bool unregisterEventHandler (IEventHandler* handler) override;

	bool registerTimer (uint64_t interval, ITimerHandler* handler) override;
	bool unregisterTimer (ITimerHandler* handler) override;

private:
	void forget () override {}
	void remember () override {}

	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // GDK
} // Platform
} // Standalone
} // VSTGUI
