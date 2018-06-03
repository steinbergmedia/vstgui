// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "gdkwindow.h"
#include <gtkmm.h>
#include <cassert>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace GDK {

//------------------------------------------------------------------------
class Window : public IGdkWindow
{
public:
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

private:
	IWindowDelegate* delegate {nullptr};
	Glib::RefPtr<Gdk::Window> gdkWindow;
};

//------------------------------------------------------------------------
bool Window::init (const WindowConfiguration& config, IWindowDelegate& delegate)
{
	GdkWindowAttr attributes;
	gdkWindow = Gdk::Window::create (Gdk::Window::get_default_root_window (), &attributes, 0);
	if (gdkWindow)
	{
		this->delegate = &delegate;
		return true;
	}
	return false;
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
	gdkWindow->withdraw ();
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
	return kGtkWindow;
}

//------------------------------------------------------------------------
void* Window::getPlatformHandle () const
{
	assert (false);
	return nullptr;
}

void Window::onSetContentView (CFrame* frame) {}

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
