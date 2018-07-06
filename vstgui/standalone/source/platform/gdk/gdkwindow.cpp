// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "gdkwindow.h"
#include "../../application.h"
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
		if (auto platformFrame = frame->getPlatformFrame ())
		{
			window = reinterpret_cast<GdkWindow*> (platformFrame->getPlatformRepresentation ());
			getWindows ().push_back (this);
		}
	}
	~FrameChildWindow () noexcept override
	{
		auto& windows = getWindows ();
		auto it = std::find (windows.begin (), windows.end (), this);
		if (it != windows.end ())
			windows.erase (it);
	}

	bool isGdkWindow (GdkWindow* _window) override { return _window == window; }
	bool handleEvent (GdkEvent* event) override
	{
#if 0
		auto gdkFrame = dynamic_cast<VSTGUI::GDK::Frame*> (frame->getPlatformFrame ());
		return gdkFrame->handleEvent (event);
#else
		return false;
#endif
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
	bool handleEvent (GdkEvent* event) override;

private:
	void updateGeometryHints ();
	void handleEventConfigure (GdkEventConfigure* event);

	CPoint lastPos;
	CPoint lastSize;

	WindowStyle style;
	WindowType type;
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
bool Window::init (const WindowConfiguration& config, IWindowDelegate& inDelegate)
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
	if (config.type == WindowType::Document)
		attributes.type_hint = GDK_WINDOW_TYPE_HINT_NORMAL;
	else if (config.type == WindowType::Popup)
		attributes.type_hint = GDK_WINDOW_TYPE_HINT_POPUP_MENU; // TODO: correct ?
	attributes.event_mask = GDK_STRUCTURE_MASK | GDK_EXPOSURE_MASK | GDK_FOCUS_CHANGE_MASK;
	gdkWindow = Gdk::Window::create (Gdk::Window::get_default_root_window (), &attributes,
									 GDK_WA_VISUAL | GDK_WA_TYPE_HINT);

	if (!gdkWindow)
		return false;

	style = config.style;
	type = config.type;
	lastSize = config.size;

	delegate = &inDelegate;

	gdkWindow->set_accept_focus (true);
	gdkWindow->set_focus_on_map (true);

	uint32_t windowDeco = 0;
	uint32_t windowFunctions = Gdk::FUNC_MOVE;

	if (config.type == WindowType::Document)
	{
		windowFunctions |= Gdk::FUNC_MINIMIZE;
		windowDeco |= Gdk::DECOR_MINIMIZE;
	}

	if (style.canClose ())
	{
		windowFunctions |= Gdk::FUNC_CLOSE;
	}
	if (style.canSize ())
	{
		windowFunctions |= Gdk::FUNC_RESIZE;
		windowFunctions |= Gdk::FUNC_MAXIMIZE;
		windowDeco |= Gdk::DECOR_MAXIMIZE;
		windowDeco |= Gdk::DECOR_RESIZEH;
	}
	if (style.isTransparent ())
	{
	}
	if (!style.hasBorder ())
	{
		windowDeco = 0;
	}
	if (!config.title.empty ())
		windowDeco |= Gdk::DECOR_TITLE;

	gdkWindow->set_functions (static_cast<Gdk::WMFunction> (windowFunctions));
	gdkWindow->set_decorations (static_cast<Gdk::WMDecoration> (windowDeco));

#if 0
	Gdk::RGBA color;
	color.set_rgba (0., 1., 0.);
	gdkWindow->set_background (color);
#endif
	return true;
}

//------------------------------------------------------------------------
void Window::updateGeometryHints ()
{
	Gdk::Geometry geometry;
	uint32_t geometryHints = Gdk::HINT_USER_POS | Gdk::HINT_USER_SIZE;
	if (style.canSize ())
	{
		auto minSize = delegate->constraintSize ({0, 0});
		geometry.min_width = minSize.x;
		geometry.min_height = minSize.y;
		auto maxSize = delegate->constraintSize (
			{std::numeric_limits<CCoord>::max (), std::numeric_limits<CCoord>::max ()});
		geometry.max_width = maxSize.x;
		geometry.max_height = maxSize.y;
		geometryHints |= Gdk::HINT_MIN_SIZE;
		geometryHints |= Gdk::HINT_MAX_SIZE;
	}
	else
	{
		geometry.min_width = lastSize.x;
		geometry.min_height = lastSize.y;
		geometry.max_width = lastSize.x;
		geometry.max_height = lastSize.y;
		geometryHints |= Gdk::HINT_MIN_SIZE;
		geometryHints |= Gdk::HINT_MAX_SIZE;
	}

	gdkWindow->set_geometry_hints (geometry, static_cast<Gdk::WindowHints> (geometryHints));
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
	updateGeometryHints ();
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
	if (!gdkWindow)
		return;
	gdkWindow->withdraw ();
	delegate->onClosed ();
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
bool Window::handleEvent (GdkEvent* ev)
{
	switch (ev->type)
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
			vstgui_assert (false, "this should never be called from GDK.");
			break;
		}
		case GDK_FOCUS_CHANGE:
		{
			if (frameChild)
				frameChild->handleEvent (ev);
			auto event = reinterpret_cast<GdkEventFocus*> (ev);
			auto hasFocus = event->in != 0;
			if (hasFocus)
				delegate->onActivated ();
			else
			{
				delegate->onDeactivated ();
				if (type == WindowType::Popup)
				{
					if (!Detail::getApplicationPlatformAccess ()->dontClosePopupOnDeactivation (
							this))
						close ();
				}
			}
			break;
		}
		case GDK_KEY_PRESS:
		{
			if (frameChild)
				frameChild->handleEvent (ev);
			break;
		}
		case GDK_KEY_RELEASE:
		{
			if (frameChild)
				frameChild->handleEvent (ev);
			break;
		}
		case GDK_BUTTON_PRESS:
		{
			if (style.isMovableByWindowBackground ())
			{
				auto event = reinterpret_cast<GdkEventButton*> (ev);
				gdkWindow->begin_move_drag (event->button, event->x_root, event->y_root,
											event->time);
			}
			break;
		}
	}
	return true;
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
