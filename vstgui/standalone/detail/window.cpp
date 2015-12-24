#include "window.h"
#include "platform/iplatformwindow.h"
#include "../../lib/cframe.h"
#include "../icommand.h"
#include "../iwindowcontroller.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Detail {

//------------------------------------------------------------------------
class Window : public IWindow, public Platform::IWindowDelegate, public std::enable_shared_from_this<Window>
{
public:
	bool init (const WindowConfiguration& config, const WindowControllerPtr& controller);

	const WindowControllerPtr& getController () const override { return controller; }

	// IWindow
	CPoint getSize () const override { return platformWindow->getSize (); }
	CPoint getPosition () const override { return platformWindow->getPosition (); }
	void setSize (const CPoint& newSize) override { platformWindow->setSize (newSize); }
	void setPosition (const CPoint& newPosition) override { platformWindow->setPosition (newPosition); }
	void setTitle (const UTF8String& newTitle) override { platformWindow->setTitle (newTitle); }
	void setContentView (const SharedPointer<CFrame>& newFrame) override;
	void show () override { platformWindow->show (); }
	void hide () override { platformWindow->hide (); }
	void close () override { platformWindow->close (); }
	void addWindowListener (IWindowListener* listener) override;
	void removeWindowListener (IWindowListener* listener) override;

	// Platform::IWindowDelegate
	CPoint constraintSize (const CPoint& newSize) override;
	void onSizeChanged (const CPoint& newSize) override;
	void onPositionChanged (const CPoint& newPosition) override;
	void onShow () override;
	void onHide () override;
	void onClosed () override;
	bool canClose () override;
	// ICommandHandler
	bool canHandleCommand (const Command& command) override;
	bool handleCommand (const Command& command) override;
private:
	template<typename Proc, typename ...Args>
	void forEachWindowListener (const Proc& proc, Args... args);

	WindowControllerPtr controller;
	Platform::WindowPtr platformWindow;
	SharedPointer<CFrame> frame;
	UTF8String autoSaveFrameName;
	std::vector<IWindowListener*> windowListeners;
};

//------------------------------------------------------------------------
bool Window::init (const WindowConfiguration& config, const WindowControllerPtr& inController)
{
	platformWindow = Platform::makeWindow (config, *this);
	if (platformWindow)
	{
		if (config.flags.doesAutoSaveFrame ())
			autoSaveFrameName = config.autoSaveFrameName;
		controller = inController;
	}
	return platformWindow != nullptr;
}

//------------------------------------------------------------------------
void Window::setContentView (const SharedPointer<CFrame>& newFrame)
{
	if (frame)
		frame->close ();
	frame = newFrame;
	if (!frame)
		return;
	frame->open (platformWindow->getPlatformHandle (), platformWindow->getPlatformType ());
}

//------------------------------------------------------------------------
template<typename Proc, typename ...Args>
void Window::forEachWindowListener (const Proc& proc, Args... args)
{
	std::for_each (windowListeners.begin (), windowListeners.end (), [&] (IWindowListener* listener) {
		proc (listener, std::forward<Args> (args)...);
	});
}

//------------------------------------------------------------------------
CPoint Window::constraintSize (const CPoint& newSize)
{
	return controller ? controller->constraintSize (*this, newSize) : newSize;
}

//------------------------------------------------------------------------
void Window::onSizeChanged (const CPoint& newSize)
{
	forEachWindowListener ([] (IWindowListener* listener, const IWindow* window, const CPoint& newSize) {
		listener->onSizeChanged (*window, newSize);
	}, this, newSize);
	if (controller)
		controller->onSizeChanged (*this, newSize);
	if (frame)
		frame->setSize (newSize.x, newSize.y);
}

//------------------------------------------------------------------------
void Window::onPositionChanged (const CPoint& newPosition)
{
	forEachWindowListener ([] (IWindowListener* listener, const IWindow* window, const CPoint& newPosition) {
		listener->onPositionChanged (*window, newPosition);
	}, this, newPosition);
	controller->onPositionChanged (*this, newPosition);
}

//------------------------------------------------------------------------
void Window::onClosed ()
{
	auto self = shared_from_this (); // make sure we live as long as the methos executes
	forEachWindowListener ([] (IWindowListener* listener, const IWindow* window) {
		listener->onClosed (*window);
	}, this);
	if (controller)
		controller->onClosed (*this);
	if (frame)
	{
		frame->remember ();
		frame->close ();
		frame = nullptr;
	}
	platformWindow = nullptr;
}

//------------------------------------------------------------------------
void Window::onShow ()
{
	forEachWindowListener ([] (IWindowListener* listener, const IWindow* window) {
		listener->onShow (*window);
	}, this);
	if (controller)
		controller->onShow (*this);
}

//------------------------------------------------------------------------
void Window::onHide ()
{
	forEachWindowListener ([] (IWindowListener* listener, const IWindow* window) {
		listener->onHide (*window);
	}, this);
	if (controller)
		controller->onHide (*this);
}

//------------------------------------------------------------------------
bool Window::canClose ()
{
	return controller ? controller->canClose (*this) : true;
}

//------------------------------------------------------------------------
void Window::addWindowListener (IWindowListener* listener)
{
	windowListeners.push_back (listener);
}

//------------------------------------------------------------------------
void Window::removeWindowListener (IWindowListener* listener)
{
	auto it = std::find (windowListeners.begin (), windowListeners.end (), listener);
	if (it != windowListeners.end ())
		windowListeners.erase (it);
}

//------------------------------------------------------------------------
bool Window::canHandleCommand (const Command& command)
{
	ICommandHandler* commandHandler = controller ? dynamic_cast<ICommandHandler*>(controller.get ()) : nullptr;
	return commandHandler ? commandHandler->canHandleCommand (command) : false;
}

//------------------------------------------------------------------------
bool Window::handleCommand (const Command& command)
{
	ICommandHandler* commandHandler = controller ? dynamic_cast<ICommandHandler*>(controller.get ()) : nullptr;
	return commandHandler ? commandHandler->handleCommand (command) : false;
}

//------------------------------------------------------------------------
WindowPtr makeWindow (const WindowConfiguration& config, const WindowControllerPtr& controller)
{
	auto window = std::make_shared<Detail::Window> ();
	if (!window->init (config, controller))
		return nullptr;
	return window;
}

//------------------------------------------------------------------------
} // Detail
} // Standalone
} // VSTGUI
