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
struct ExternalTimerHandler
{
	VSTGUI::X11::ITimerHandler* timerHandler{nullptr};
	GSource* source{nullptr};
};

//------------------------------------------------------------------------
struct RunLoop::Impl
{
	using EventHandlerVector = std::vector<std::unique_ptr<ExternalEventHandler>>;
	using TimerHandlerVector = std::vector<std::unique_ptr<ExternalTimerHandler>>;

	GMainContext * mainContext{nullptr};
	EventHandlerVector eventHandlers;
	TimerHandlerVector timerHandlers;
};

//------------------------------------------------------------------------
RunLoop::RunLoop ()
{
	impl = std::unique_ptr<Impl> (new Impl);
	impl->mainContext = g_main_context_ref (g_main_context_default ());
}

//------------------------------------------------------------------------
RunLoop::~RunLoop () noexcept
{
	g_main_context_unref (impl->mainContext);
}

//------------------------------------------------------------------------
static gboolean eventHandlerProc (GIOChannel* channel, GIOCondition condition, gpointer userData)
{
	auto handler = static_cast<VSTGUI::X11::IEventHandler*> (userData);
	handler->onEvent ();
	return G_SOURCE_CONTINUE;
};

//------------------------------------------------------------------------
bool RunLoop::registerEventHandler (int fd, IEventHandler* handler)
{
	std::unique_ptr<ExternalEventHandler> eventHandler (new ExternalEventHandler);
	eventHandler->eventHandler = handler;
	eventHandler->ioChannel = g_io_channel_unix_new (fd);
	eventHandler->source = g_io_create_watch (
		eventHandler->ioChannel, static_cast<GIOCondition> (G_IO_IN | G_IO_ERR | G_IO_HUP));
	g_source_set_callback (eventHandler->source, reinterpret_cast<GSourceFunc> (eventHandlerProc),
						   handler, nullptr);
	g_source_attach (eventHandler->source, impl->mainContext);
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
	std::unique_ptr<ExternalTimerHandler> timerHandler (new ExternalTimerHandler);
	timerHandler->timerHandler = handler;
	timerHandler->source = g_timeout_source_new (interval);
	g_source_set_callback (timerHandler->source,
						   [](gpointer userData) -> gboolean {
							   auto handler = reinterpret_cast<ITimerHandler*> (userData);
							   handler->onTimer ();
							   return 1;
						   },
						   handler, nullptr);
	g_source_attach (timerHandler->source, impl->mainContext);
	impl->timerHandlers.emplace_back (std::move (timerHandler));
	return true;
}

//------------------------------------------------------------------------
bool RunLoop::unregisterTimer (ITimerHandler* handler)
{
	auto it = std::find_if (impl->timerHandlers.begin (), impl->timerHandlers.end (), [&] (const auto& p) {
		return p->timerHandler == handler;
	});
	if (it != impl->timerHandlers.end ())
	{
		g_source_destroy ((*it)->source);
		impl->timerHandlers.erase (it);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
} // GDK
} // Platform
} // Standalone
} // VSTGUI
