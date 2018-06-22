// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "gdkwindow.h"
#include "../../../../lib/platform/linux/gdkframe.h"
#include "../../../../lib/cframe.h"
#include <gtkmm.h>
#include <cassert>
#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace GDK {

//------------------------------------------------------------------------
namespace {

using WindowVector = std::vector<IGdkWindow*>;

//------------------------------------------------------------------------
WindowVector& getWindows ()
{
	static WindowVector instance;
	return instance;
}

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
IGdkWindow* IGdkWindow::find (GdkWindow* gdkWindow)
{
	auto& windows = getWindows ();
	auto it = std::find_if (windows.begin (), windows.end (), [&](auto& w) {
		if (w->isGdkWindow (gdkWindow))
			return true;
		return false;
	});
	return it == windows.end () ? nullptr : *it;
}

//------------------------------------------------------------------------
class FrameChildWindow : public IGdkWindow
{
public:
	FrameChildWindow (CFrame* frame) : frame (frame)
	{
		window = reinterpret_cast<GdkWindow*> (
			frame->getPlatformFrame ()->getPlatformRepresentation ()); // TODO: correct access
		getWindows ().push_back (this);
	}
	~FrameChildWindow () noexcept override
	{
		auto& windows = getWindows ();
		auto it = std::find (windows.begin (), windows.end (), this);
		if (it != windows.end ())
			windows.erase (it);
	}

	bool isGdkWindow (GdkWindow* _window) override { return _window == window; }
	void handleEvent (GdkEvent* event) override
	{
		auto gdkFrame = dynamic_cast<VSTGUI::GDK::Frame*> (frame->getPlatformFrame ());
		gdkFrame->handleEvent (event);
	}

	CFrame* getFrame () const { return frame; }

private:
	CFrame* frame{nullptr};
	GdkWindow* window{nullptr};
};

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

	void show () override;
	void hide () override;
	void close () override;
	void activate () override;
	void center () override;

	PlatformType getPlatformType () const override;
	void* getPlatformHandle () const override;

	void onSetContentView (CFrame* frame) override;

	bool isGdkWindow (GdkWindow* window) override;
	void handleEvent (GdkEvent* event) override;

private:
	void handleEventConfigure (GdkEventConfigure* event);

	CPoint lastPos;
	CPoint lastSize;

	IWindowDelegate* delegate{nullptr};
	Glib::RefPtr<Gdk::Window> gdkWindow;
	std::unique_ptr<FrameChildWindow> frameChild;
};

//------------------------------------------------------------------------
Window::~Window () noexcept
{
	auto& windows = getWindows ();
	auto it = std::find (windows.begin (), windows.end (), this);
	if (it != windows.end ())
		windows.erase (it);
}

//------------------------------------------------------------------------
bool Window::init (const WindowConfiguration& config, IWindowDelegate& delegate)
{
	getWindows ().push_back (this);

	auto screen = gdk_screen_get_default ();
	auto visual = gdk_screen_get_system_visual (screen);

	GdkWindowAttr attributes{};
	attributes.x = 0;
	attributes.y = 0;
	attributes.width = config.size.x;
	attributes.height = config.size.y;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.visual = visual;
	attributes.window_type = GDK_WINDOW_TOPLEVEL;
	attributes.cursor = nullptr;
	attributes.wmclass_name = nullptr;
	attributes.wmclass_class = nullptr;
	attributes.override_redirect = false;
	attributes.type_hint = GDK_WINDOW_TYPE_HINT_NORMAL;
	attributes.event_mask = GDK_STRUCTURE_MASK | GDK_EXPOSURE_MASK | GDK_FOCUS_CHANGE_MASK;
	gdkWindow = Gdk::Window::create (Gdk::Window::get_default_root_window (), &attributes,
									 GDK_WA_VISUAL | GDK_WA_TYPE_HINT);

	if (!gdkWindow)
		return false;

	lastSize = config.size;

	this->delegate = &delegate;

	gdkWindow->set_accept_focus (true);
	gdkWindow->set_focus_on_map (true);

#if 0
	Gdk::RGBA color;
	color.set_rgba (0., 1., 0.);
	gdkWindow->set_background (color);
#endif
	return true;
}

//------------------------------------------------------------------------
CPoint Window::getSize () const
{
	CPoint size;
	size.x = gdkWindow->get_width ();
	size.y = gdkWindow->get_height ();
	return size;
}

//------------------------------------------------------------------------
CPoint Window::getPosition () const
{
	int x, y;
	gdkWindow->get_position (x, y);
	return CPoint (x, y);
}

//------------------------------------------------------------------------
double Window::getScaleFactor () const
{
	return static_cast<double> (gdkWindow->get_scale_factor ());
}

//------------------------------------------------------------------------
void Window::setSize (const CPoint& newSize)
{
	gdkWindow->resize (newSize.x, newSize.y);
}

//------------------------------------------------------------------------
void Window::setPosition (const CPoint& newPosition)
{
	gdkWindow->move (newPosition.x, newPosition.y);
}

//------------------------------------------------------------------------
void Window::setTitle (const UTF8String& newTitle)
{
	gdkWindow->set_title (newTitle.getString ());
}

//------------------------------------------------------------------------
void Window::setRepresentedPath (const UTF8String& path) {}

//------------------------------------------------------------------------
void Window::show ()
{
	gdkWindow->show ();
}

//------------------------------------------------------------------------
void Window::hide ()
{
	gdkWindow->hide ();
}

//------------------------------------------------------------------------
void Window::close ()
{
	delegate->onClosed ();
	frameChild = nullptr;
	gdkWindow->withdraw ();
	gdkWindow.reset ();
}

//------------------------------------------------------------------------
void Window::activate ()
{
	gdkWindow->raise ();
}

//------------------------------------------------------------------------
void Window::center () {}

//------------------------------------------------------------------------
PlatformType Window::getPlatformType () const
{
	return kGdkWindow;
}

//------------------------------------------------------------------------
void* Window::getPlatformHandle () const
{
	if (gdkWindow)
	{
		auto ptr = gdkWindow->gobj ();
		return ptr;
	}
	return nullptr;
}

//------------------------------------------------------------------------
void Window::onSetContentView (CFrame* newFrame)
{
	frameChild = nullptr;
	if (newFrame)
		frameChild = std::unique_ptr<FrameChildWindow> (new FrameChildWindow (newFrame));
}

//------------------------------------------------------------------------
bool Window::isGdkWindow (GdkWindow* window)
{
	assert (window);
	return window == getPlatformHandle ();
}

//------------------------------------------------------------------------
void Window::handleEvent (GdkEvent* ev)
{
	auto type = ev->type;
	switch (type)
	{
		case GDK_EXPOSE:
		{
			break;
		}
		case GDK_CONFIGURE:
		{
			handleEventConfigure (reinterpret_cast<GdkEventConfigure*> (ev));
			break;
		}
		case GDK_MAP:
		{
			delegate->onShow ();
			break;
		}
		case GDK_UNMAP:
		{
			delegate->onHide ();
			break;
		}
		case GDK_DELETE:
		{
			if (delegate->canClose ())
			{
				close ();
			}
			break;
		}
		case GDK_DESTROY:
		{
			delegate->onClosed ();
			break;
		}
		case GDK_FOCUS_CHANGE:
		{
			auto event = reinterpret_cast<GdkEventFocus*> (ev);
			auto hasFocus = event->in != 0;
			if (hasFocus)
				delegate->onActivated ();
			else
				delegate->onDeactivated ();
			break;
		}
	}
}

//------------------------------------------------------------------------
void Window::handleEventConfigure (GdkEventConfigure* event)
{
	CPoint newPos (event->x, event->y);
	CPoint newSize (event->width, event->height);
	if (newPos != lastPos)
	{
		lastPos = newPos;
		delegate->onPositionChanged (newPos);
	}
	if (newSize != lastSize)
	{
		lastSize = newSize;
		delegate->onSizeChanged (newSize);
		if (frameChild)
			frameChild->getFrame ()->setSize (newSize.x, newSize.y);
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
