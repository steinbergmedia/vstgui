// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "gdkrunloop.h"

#include <glib.h>
#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace GDK {

//------------------------------------------------------------------------
RunLoop& RunLoop::instance ()
{
	static RunLoop instance;
	return instance;
}

//------------------------------------------------------------------------
struct ExternalEventHandler
{
	VSTGUI::X11::IEventHandler* eventHandler{nullptr};
	GSource* source{nullptr};
	GIOChannel* ioChannel{nullptr};
};

//------------------------------------------------------------------------
struct RunLoop::Impl
{
	using EventHandlerVector = std::vector<std::unique_ptr<ExternalEventHandler>>;

	GMainLoop* mainLoop{nullptr};
	EventHandlerVector eventHandlers;
};

//------------------------------------------------------------------------
RunLoop::RunLoop ()
{
	impl = std::unique_ptr<Impl> (new Impl);
	impl->mainLoop = g_main_loop_new (g_main_context_default (), false);
}

//------------------------------------------------------------------------
RunLoop::~RunLoop () noexcept
{
	g_main_loop_unref (impl->mainLoop);
}

//------------------------------------------------------------------------
void RunLoop::run ()
{
	g_main_loop_run (impl->mainLoop);
}

//------------------------------------------------------------------------
void RunLoop::quit ()
{
	if (impl->mainLoop)
		g_main_loop_quit (impl->mainLoop);
}

//------------------------------------------------------------------------
bool RunLoop::registerEventHandler (int fd, IEventHandler* handler)
{
	std::unique_ptr<ExternalEventHandler> eventHandler (new ExternalEventHandler);
	eventHandler->eventHandler = handler;
	eventHandler->ioChannel = g_io_channel_unix_new (fd);
	eventHandler->source =
		g_io_create_watch (eventHandler->ioChannel,
						   static_cast<GIOCondition> (G_IO_IN | G_IO_OUT | G_IO_ERR | G_IO_HUP));
	g_source_set_callback (eventHandler->source,
						   [](gpointer userData) {
							   auto handler = static_cast<IEventHandler*> (userData);
							   handler->onEvent ();
							   return 1;
						   },
						   eventHandler.get (), nullptr);
	g_source_attach (eventHandler->source, g_main_loop_get_context (impl->mainLoop));
	impl->eventHandlers.emplace_back (std::move (eventHandler));
	return true;
}

//------------------------------------------------------------------------
bool RunLoop::unregisterEventHandler (IEventHandler* handler)
{
	auto it = std::find_if (impl->eventHandlers.begin (), impl->eventHandlers.end (),
							[&](const auto& p) { return p->eventHandler == handler; });
	if (it != impl->eventHandlers.end ())
	{
		g_source_destroy ((*it)->source);
		g_io_channel_unref ((*it)->ioChannel);
		impl->eventHandlers.erase (it);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool RunLoop::registerTimer (uint64_t interval, ITimerHandler* handler)
{
	return false;
}

//------------------------------------------------------------------------
bool RunLoop::unregisterTimer (ITimerHandler* handler)
{
	return false;
}

//------------------------------------------------------------------------
} // GDK
} // Platform
} // Standalone
} // VSTGUI
