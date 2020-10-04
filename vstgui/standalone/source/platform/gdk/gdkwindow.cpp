// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "gdkwindow.h"
#include "gdkapplication.h"
#include "gdkrunloop.h"
#include "../../application.h"
#include "../../../../lib/cframe.h"
#include "../../../../lib/platform/platform_x11.h"
#include <gtkmm.h>
#include <gdk/gdkx.h>
#include <cassert>
#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace GDK {

//------------------------------------------------------------------------
namespace {

/* XEMBED messages */
enum class XEmbedMessage
{
	EMBEDDED_NOTIFY = 0,
	WINDOW_ACTIVATE = 1,
	WINDOW_DEACTIVATE = 2,
	REQUEST_FOCUS = 3,
	FOCUS_IN = 4,
	FOCUS_OUT = 5,
	FOCUS_NEXT = 6,
	FOCUS_PREV = 7,
	/* 8-9 were used for GRAB_KEY/UNGRAB_KEY */
	MODALITY_ON = 10,
	MODALITY_OFF = 11,
	REGISTER_ACCELERATOR = 12,
	UNREGISTER_ACCELERATOR = 13,
	ACTIVATE_ACCELERATOR = 14,
};

//------------------------------------------------------------------------
void sendXEmbedProtocolMessage (::Window receiver,
								::Window parentWindow,
								XEmbedMessage message,
								uint32_t detail = 0,
								uint32_t xEmbedVersion = 1)
{
	auto xDisplay = gdk_x11_display_get_xdisplay (gdk_display_get_default ());

	auto xEmbedAtom = XInternAtom (xDisplay, "_XEMBED", true);
	if (xEmbedAtom == None)
		return;

	XEvent ev{};
	ev.xclient.type = ClientMessage;
	ev.xclient.window = receiver;
	ev.xclient.message_type = xEmbedAtom;
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = CurrentTime;
	ev.xclient.data.l[1] = static_cast<long> (message);
	ev.xclient.data.l[2] = detail;
	ev.xclient.data.l[3] = parentWindow;
	ev.xclient.data.l[4] = xEmbedVersion;
	XSendEvent (xDisplay, receiver, False, NoEventMask, &ev);
	XSync (xDisplay, False);
}

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
class Window
	: public IGdkWindow
	, public IWindow
{
public:
	~Window () noexcept override;
	bool init (const WindowConfiguration& config, IWindowDelegate& delegate);

	CPoint getSize () const override;
	CPoint getPosition () const override;
	double getScaleFactor () const override;

	void setSize (const CPoint& newSize) override;
	void setPosition (const CPoint& newPosition) override;
	void setTitle (const UTF8String& newTitle) override;
	void setRepresentedPath (const UTF8String& path) override;
	WindowStyle changeStyle (WindowStyle stylesToAdd, WindowStyle stylesToRemove) override;

	void show () override;
	void hide () override;
	void close () override;
	void activate () override;
	void center () override;

	PlatformType getPlatformType () const override;
	void* getPlatformHandle () const override;

	PlatformFrameConfigPtr prepareFrameConfig (PlatformFrameConfigPtr&& controllerConfig) override;
	void onSetContentView (CFrame* frame) override;

private:
	void updateGeometryHints ();
	void handleEventConfigure (GdkEventConfigure* event);
	void sendXEmbedMessage (XEmbedMessage msg, uint32_t data = 0);

	static GdkFilterReturn xEventFilter (GdkXEvent* xevent, GdkEvent* event, gpointer data);

	CPoint lastPos;
	CPoint lastSize;

	WindowStyle style;
	WindowType type;
	IWindowDelegate* delegate{nullptr};
	Gtk::ApplicationWindow gtkWindow;
	CFrame* contentView{nullptr};
};

//------------------------------------------------------------------------
Window::~Window () noexcept
{
	gtkApp ()->remove_window (gtkWindow);
}

//------------------------------------------------------------------------
bool Window::init (const WindowConfiguration& config, IWindowDelegate& inDelegate)
{
	gtkWindow.set_events (Gdk::ALL_EVENTS_MASK);

	gtkWindow.set_decorated (config.style.hasBorder ());
	gtkWindow.set_deletable (config.style.canClose ());
	if (config.type == WindowType::Document)
	{
		gtkWindow.set_type_hint (Gdk::WINDOW_TYPE_HINT_NORMAL);
		gtkWindow.set_skip_taskbar_hint (false);
		gtkWindow.set_show_menubar (config.style.hasBorder ());
	}
	else
	{
		gtkWindow.set_type_hint (Gdk::WINDOW_TYPE_HINT_POPUP_MENU);
		gtkWindow.set_skip_taskbar_hint (true);
	}

	gtkWindow.set_can_focus (true);
	gtkWindow.set_title (config.title.getString ());

	gtkWindow.signal_map ().connect ([this]() { delegate->onShow (); });

	gtkWindow.signal_configure_event ().connect (
		[this](GdkEventConfigure* event) {
			handleEventConfigure (event);
			return false;
		},
		false);

	gtkWindow.signal_delete_event ().connect ([this](GdkEventAny*) {
		if (delegate->canClose ())
		{
			close ();
			return true;
		}
		return false;
	});

	gtkWindow.signal_focus_in_event ().connect ([this](GdkEventFocus*) {
		delegate->onActivated ();
		sendXEmbedMessage (XEmbedMessage::WINDOW_ACTIVATE);
		sendXEmbedMessage (XEmbedMessage::FOCUS_IN);
		return true;
	});

	gtkWindow.signal_focus_out_event ().connect ([this](GdkEventFocus*) {
		delegate->onDeactivated ();
		sendXEmbedMessage (XEmbedMessage::FOCUS_OUT);
		sendXEmbedMessage (XEmbedMessage::WINDOW_DEACTIVATE);
		if (type == WindowType::Popup)
		{
			if (!Detail::getApplicationPlatformAccess ()->dontClosePopupOnDeactivation (this))
				close ();
		}
		return true;
	});

	if (style.isMovableByWindowBackground ())
	{
		gtkWindow.signal_button_press_event ().connect ([this](GdkEventButton* event) {
			gtkWindow.begin_move_drag (event->button, event->x_root, event->y_root, event->time);
			return true;
		});
	}

	gtkApp ()->add_window (gtkWindow);

	style = config.style;
	type = config.type;
	lastSize = config.size;

	delegate = &inDelegate;

	auto widget = reinterpret_cast<GtkWidget*> (gtkWindow.gobj ());
	gtk_widget_realize (widget);

	auto gdkWindow = gtkWindow.get_window ();
	vstgui_assert (gdkWindow);
	gdkWindow->add_filter (xEventFilter, this);

	gtkWindow.set_data ("VSTGUIWindow", this);
	return true;
}

//------------------------------------------------------------------------
GdkFilterReturn Window::xEventFilter (GdkXEvent* xevent, GdkEvent* event, gpointer data)
{
	GdkFilterReturn result = GDK_FILTER_CONTINUE;
	auto e = static_cast<XEvent*> (xevent);
	switch (e->type)
	{
		case CreateNotify:
		{
			auto self = reinterpret_cast<Window*> (data);
			::Window topLevelWindow = reinterpret_cast<::Window> (self->getPlatformHandle ());
			if (e->xcreatewindow.window == topLevelWindow)
				break;
			auto childWindow = e->xcreatewindow.window;
			auto xDisplay = gdk_x11_display_get_xdisplay (gdk_display_get_default ());
			XMapWindow (xDisplay, childWindow);
			sendXEmbedProtocolMessage (childWindow, topLevelWindow, XEmbedMessage::EMBEDDED_NOTIFY);
#if 0 // Do not commit
			XUnmapWindow (xDisplay, childWindow);
#endif
			break;
		}
	}
	return result;
}

//------------------------------------------------------------------------
void Window::updateGeometryHints () {}

//------------------------------------------------------------------------
CPoint Window::getSize () const
{
	CPoint size;
	size.x = gtkWindow.get_width ();
	size.y = gtkWindow.get_height ();
	return size;
}

//------------------------------------------------------------------------
CPoint Window::getPosition () const
{
	int x, y;
	gtkWindow.get_position (x, y);
	return CPoint (x, y);
}

//------------------------------------------------------------------------
double Window::getScaleFactor () const
{
	return static_cast<double> (gtkWindow.get_scale_factor ());
}

//------------------------------------------------------------------------
void Window::setSize (const CPoint& newSize)
{
	gtkWindow.resize (newSize.x, newSize.y);
}

//------------------------------------------------------------------------
void Window::setPosition (const CPoint& newPosition)
{
	gtkWindow.move (newPosition.x, newPosition.y);
}

//------------------------------------------------------------------------
void Window::setTitle (const UTF8String& newTitle)
{
	gtkWindow.set_title (newTitle.getString ());
}

//------------------------------------------------------------------------
void Window::setRepresentedPath (const UTF8String& path) {}

//------------------------------------------------------------------------
WindowStyle Window::changeStyle (WindowStyle stylesToAdd, WindowStyle stylesToRemove)
{
	// TODO: Implementation
	return style;
}

//------------------------------------------------------------------------
void Window::show ()
{
	updateGeometryHints ();
	gtkWindow.show_all ();
	activate ();
#if 0
	if (type == WindowType::Popup)
		gtkWindow.focus (0);
#endif
}

//------------------------------------------------------------------------
void Window::hide ()
{
	gtkWindow.hide ();
}

//------------------------------------------------------------------------
void Window::close ()
{
	gtkWindow.close ();
	delegate->onClosed ();
}

//------------------------------------------------------------------------
void Window::activate ()
{
	gtkWindow.present ();
}

//------------------------------------------------------------------------
void Window::center () {}

//------------------------------------------------------------------------
PlatformType Window::getPlatformType () const
{
	return PlatformType::kX11EmbedWindowID;
}

//------------------------------------------------------------------------
void* Window::getPlatformHandle () const
{
	if (auto gdkWindow = gtkWindow.get_window ())
	{
		auto ptr = const_cast<GdkWindow*> (gdkWindow->gobj ());
		return reinterpret_cast<void*> (gdk_x11_window_get_xid (ptr));
	}
	return nullptr;
}

//------------------------------------------------------------------------
PlatformFrameConfigPtr Window::prepareFrameConfig (PlatformFrameConfigPtr&& controllerConfig)
{
	if (controllerConfig)
	{
		if (auto config = dynamicPtrCast<X11::FrameConfig> (controllerConfig))
		{
			config->runLoop = &RunLoop::instance ();
			return std::move (config);
		}
	}
	auto config = std::make_shared<X11::FrameConfig> ();
	config->runLoop = &RunLoop::instance ();
	return config;
}

//------------------------------------------------------------------------
void Window::onSetContentView (CFrame* newFrame)
{
	contentView = newFrame;
}

//------------------------------------------------------------------------
void Window::sendXEmbedMessage (XEmbedMessage msg, uint32_t data)
{
	if (auto x11Frame = dynamic_cast<X11::IX11Frame*> (contentView->getPlatformFrame ()))
	{
		sendXEmbedProtocolMessage (x11Frame->getX11WindowID (),
								   reinterpret_cast<::Window> (getPlatformHandle ()), msg, data);
	}
}

//------------------------------------------------------------------------
void Window::handleEventConfigure (GdkEventConfigure* event)
{
	CPoint newPos (event->x, event->y);
	CPoint newSize (event->width, event->height);
	CPoint constraintSize = delegate->constraintSize (newSize);
	if (constraintSize != newSize)
	{
		setSize (constraintSize);
		newSize = constraintSize;
	}
	if (newPos != lastPos)
	{
		lastPos = newPos;
		delegate->onPositionChanged (newPos);
	}
	if (newSize != lastSize)
	{
		lastSize = newSize;
		delegate->onSizeChanged (newSize);
		if (contentView)
			contentView->setSize (newSize.x, newSize.y);
	}
}

//------------------------------------------------------------------------
} // GDK

//------------------------------------------------------------------------
WindowPtr makeWindow (const WindowConfiguration& config, IWindowDelegate& delegate)
{
	auto result = std::make_shared<GDK::Window> ();
	if (result->init (config, delegate))
		return result;
	return nullptr;
}

} // Platform
} // Standalone
} // VSTGUI
