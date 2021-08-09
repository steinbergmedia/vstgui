// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "window.h"
#include "application.h"
#include "../../lib/cframe.h"
#include "../../lib/controls/coptionmenu.h"
#include "../../lib/dispatchlist.h"
#include "../../lib/events.h"
#include "../../uidescription/icontroller.h"
#include "../include/iapplication.h"
#include "../include/icommand.h"
#include "../include/ipreference.h"
#include "../include/iwindowcontroller.h"
#include "platform/iplatformwindow.h"

#include <sstream>
#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Detail {
namespace /*anonymous*/ {

//------------------------------------------------------------------------
UTF8String strFromPositionAndSize (const CPoint& pos, const CPoint& size)
{
	std::stringstream str;
	str << pos.x << "," << pos.y << "," << size.x << "," << size.y;
	return UTF8String (str.str ());
}

//------------------------------------------------------------------------
struct PosAndSize
{
	CPoint pos;
	CPoint size;
};

static CPoint nullPoint {0, 0};

//------------------------------------------------------------------------
PosAndSize positionAndSizeFromString (const UTF8String& str)
{
	PosAndSize r;
	std::vector<std::string> elements;
	std::stringstream stream (str.getString ());
	std::string item;
	while (std::getline (stream, item, ','))
		elements.emplace_back (item);

	if (elements.size () != 4)
		return r;
	r.pos.x = UTF8StringView (elements[0].data ()).toDouble ();
	r.pos.y = UTF8StringView (elements[1].data ()).toDouble ();
	r.size.x = UTF8StringView (elements[2].data ()).toDouble ();
	r.size.y = UTF8StringView (elements[3].data ()).toDouble ();

	return r;
}

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
class Window : public IPlatformWindowAccess,
               public Platform::IWindowDelegate,
               public IMouseObserver,
               public std::enable_shared_from_this<Window>
{
public:
	bool init (const WindowConfiguration& config, const WindowControllerPtr& controller);

	// IWindow
	const WindowControllerPtr& getController () const override { return controller; }
	CPoint getSize () const override { return platformWindow->getSize (); }
	CPoint getPosition () const override { return platformWindow->getPosition (); }
	double getScaleFactor () const override { return platformWindow->getScaleFactor (); }
	CRect getFocusViewRect () const override;
	const UTF8String& getTitle () const override { return title; }
	WindowType getType () const override { return windowType; }
	WindowStyle getStyle () const override { return windowStyle; }
	const UTF8String& getAutoSaveFrameName () const override { return autoSaveFrameName; }
	void setSize (const CPoint& newSize) override;
	void setPosition (const CPoint& newPosition) override
	{
		platformWindow->setPosition (newPosition);
	}
	void setTitle (const UTF8String& newTitle) override
	{
		title = newTitle;
		platformWindow->setTitle (newTitle);
	}
	void setContentView (const SharedPointer<CFrame>& newFrame) override;
	void setRepresentedPath (const UTF8String& path) override;
	WindowStyle changeStyle (WindowStyle stylesToAdd, WindowStyle stylesToRemove) override;
	void show () override;
	void hide () override { platformWindow->hide (); }
	void close () override { platformWindow->close (); }
	void activate () override { platformWindow->activate (); }
	void registerWindowListener (IWindowListener* listener) override;
	void unregisterWindowListener (IWindowListener* listener) override;

	// IPlatformWindowAccess
	InterfacePtr getPlatformWindow () const override { return platformWindow; }
	CFrame* getFrame () const override { return frame; }

	// Platform::IWindowDelegate
	CPoint constraintSize (const CPoint& newSize) override;
	void onSizeChanged (const CPoint& newSize) override;
	void onPositionChanged (const CPoint& newPosition) override;
	void onShow () override;
	void onHide () override;
	void onClosed () override;
	bool canClose () override;
	void onActivated () override;
	void onDeactivated () override;
	// ICommandHandler
	bool canHandleCommand (const Command& command) override;
	bool handleCommand (const Command& command) override;

	// IMouseObserver
	void onMouseEntered (CView*, CFrame* ) override {};
	void onMouseExited (CView*, CFrame* ) override {};
	void onMouseEvent (MouseEvent& event, CFrame*) override;

private:
	WindowControllerPtr controller;
	WindowStyle windowStyle;
	WindowType windowType;
	Platform::WindowPtr platformWindow;
	SharedPointer<CFrame> frame;
	UTF8String autoSaveFrameName;
	UTF8String title;
	DispatchList<IWindowListener*> windowListeners;
};

//------------------------------------------------------------------------
bool Window::init (const WindowConfiguration& config, const WindowControllerPtr& inController)
{
	title = config.title;
	windowStyle = config.style;
	windowType = config.type;
	platformWindow = Platform::makeWindow (config, *this);
	if (platformWindow)
	{
		if (!config.autoSaveFrameName.empty ())
		{
			autoSaveFrameName = config.autoSaveFrameName;
		}
		controller = inController;
	}
	return platformWindow != nullptr;
}

//------------------------------------------------------------------------
void Window::setSize (const CPoint& newSize)
{
	CPoint size (newSize);
	if (controller)
		size = controller->constraintSize (*this, size);
	platformWindow->setSize (size);
}

//------------------------------------------------------------------------
void Window::show ()
{
	if (controller)
		controller->beforeShow (*this);
	bool positionChanged = false;
	if (!autoSaveFrameName.empty ())
	{
		if (auto frameName = IApplication::instance ().getPreferences ().get (autoSaveFrameName))
		{
			auto ps = positionAndSizeFromString (*frameName);
			if (ps.pos != nullPoint && ps.size != nullPoint)
			{
				setPosition (ps.pos);
				setSize (ps.size);
				positionChanged = true;
			}
		}
	}
	if (!positionChanged && windowStyle.isCentered ())
		platformWindow->center ();
	platformWindow->show ();
}

//------------------------------------------------------------------------
void Window::setContentView (const SharedPointer<CFrame>& newFrame)
{
	if (frame)
	{
		frame->unregisterMouseObserver (this);
		frame->close ();
	}
	frame = newFrame;
	if (!frame)
	{
		if (controller)
			controller->onSetContentView (*this, frame);
		return;
	}
	auto frameConfig =
	    controller ? controller->createPlatformFrameConfig (platformWindow->getPlatformType ()) :
	                 nullptr;
	frameConfig = platformWindow->prepareFrameConfig (std::move (frameConfig));
	frame->open (platformWindow->getPlatformHandle (), platformWindow->getPlatformType (),
	             frameConfig.get ());
	frame->registerMouseObserver (this);
	platformWindow->onSetContentView (frame);
	if (controller)
		controller->onSetContentView (*this, frame);
}

//------------------------------------------------------------------------
void Window::setRepresentedPath (const UTF8String& path)
{
	platformWindow->setRepresentedPath (path);
}

//------------------------------------------------------------------------
WindowStyle Window::changeStyle (WindowStyle stylesToAdd, WindowStyle stylesToRemove)
{
	windowStyle = platformWindow->changeStyle (stylesToAdd, stylesToRemove);
	return windowStyle;
}


//------------------------------------------------------------------------
CRect Window::getFocusViewRect () const
{
	CRect result;
	if (frame)
	{
		if (auto focusView = frame->getFocusView ())
		{
			result = focusView->getViewSize ();
			focusView->translateToGlobal (result);
		}
	}
	return result;
}

//------------------------------------------------------------------------
CPoint Window::constraintSize (const CPoint& newSize)
{
	return controller ? controller->constraintSize (*this, newSize) : newSize;
}

//------------------------------------------------------------------------
void Window::onSizeChanged (const CPoint& newSize)
{
	windowListeners.forEach (
	    [&] (IWindowListener* listener) { listener->onSizeChanged (*this, newSize); });
	if (controller)
		controller->onSizeChanged (*this, newSize);
}

//------------------------------------------------------------------------
void Window::onPositionChanged (const CPoint& newPosition)
{
	windowListeners.forEach (
	    [&] (IWindowListener* listener) { listener->onPositionChanged (*this, newPosition); });
	if (controller)
		controller->onPositionChanged (*this, newPosition);
}

//------------------------------------------------------------------------
void Window::onClosed ()
{
	auto self = shared_from_this (); // make sure we live as long as this method executes

	if (!autoSaveFrameName.empty ())
		IApplication::instance ().getPreferences ().set (
		    autoSaveFrameName, strFromPositionAndSize (getPosition (), getSize ()));

	windowListeners.forEach ([&] (IWindowListener* listener) {
		listener->onClosed (*this);
		windowListeners.remove (listener);
	});
	if (controller)
		controller->onClosed (*this);
	platformWindow->onSetContentView (nullptr);
	if (frame)
	{
		frame->unregisterMouseObserver (this);
		frame->remember ();
		frame->close ();
		frame = nullptr;
	}
	controller = nullptr;
	platformWindow = nullptr;
}

//------------------------------------------------------------------------
void Window::onShow ()
{
	windowListeners.forEach ([&] (IWindowListener* listener) { listener->onShow (*this); });
	if (controller)
		controller->onShow (*this);
}

//------------------------------------------------------------------------
void Window::onHide ()
{
	windowListeners.forEach ([&] (IWindowListener* listener) { listener->onHide (*this); });
	if (controller)
		controller->onHide (*this);
}

//------------------------------------------------------------------------
bool Window::canClose ()
{
	return controller ? controller->canClose (*this) : true;
}

//------------------------------------------------------------------------
void Window::onActivated ()
{
	windowListeners.forEach ([&] (IWindowListener* listener) { listener->onActivated (*this); });
	if (controller)
		controller->onActivated (*this);
}

//------------------------------------------------------------------------
void Window::onDeactivated ()
{
	windowListeners.forEach ([&] (IWindowListener* listener) { listener->onDeactivated (*this); });
	if (controller)
		controller->onDeactivated (*this);
}

//------------------------------------------------------------------------
void Window::registerWindowListener (IWindowListener* listener)
{
	windowListeners.add (listener);
}

//------------------------------------------------------------------------
void Window::unregisterWindowListener (IWindowListener* listener)
{
	windowListeners.remove (listener);
}

//------------------------------------------------------------------------
bool Window::canHandleCommand (const Command& command)
{
	if (command == Commands::CloseWindow)
		return controller->canClose (*this);
	if (auto commandHandler = dynamicPtrCast<ICommandHandler> (controller))
		return commandHandler->canHandleCommand (command);
	return false;
}

//------------------------------------------------------------------------
bool Window::handleCommand (const Command& command)
{
	if (command == Commands::CloseWindow)
	{
		close ();
		return true;
	}
	if (auto commandHandler = dynamicPtrCast<ICommandHandler> (controller))
		return commandHandler->handleCommand (command);
	return false;
}

//------------------------------------------------------------------------
struct WindowContextMenuCommandHandler : ICommandMenuItemTarget, NonAtomicReferenceCounted
{
	WindowContextMenuCommandHandler (Window* window) : window (window) {}
	bool validateCommandMenuItem (CCommandMenuItem* item) override
	{
		Command cmd = {item->getCommandCategory (), item->getCommandName ()};
		if (window->canHandleCommand (cmd) || getApplicationPlatformAccess ()->canHandleCommand (cmd))
			return true;
		item->setEnabled (false);
		return false;
	}
	bool onCommandMenuItemSelected (CCommandMenuItem* item) override
	{
		Command cmd = {item->getCommandCategory (), item->getCommandName ()};
		if (window->handleCommand (cmd))
			return true;
		return getApplicationPlatformAccess ()->handleCommand (cmd);
	}

	Window* window;
};


//------------------------------------------------------------------------
void Window::onMouseEvent (MouseEvent& event, CFrame* inFrame)
{
	if (event.type != EventType::MouseDown || !event.buttonState.isRight ())
		return;

	auto contextMenu = makeOwned<COptionMenu> ();

	CPoint where (event.mousePosition);
	inFrame->getTransform ().transform (where);

	CViewContainer::ViewList views;
	if (inFrame->getViewsAt (where, views, GetViewOptions ().deep ().includeViewContainer ()))
	{
		for (const auto& view : views)
		{
			auto viewController = getViewController (view);
			auto contextMenuController = dynamic_cast<IContextMenuController*> (viewController);
			auto contextMenuController2 = dynamic_cast<IContextMenuController2*> (viewController);
			if (contextMenuController == nullptr && contextMenuController2 == nullptr)
				continue;
			if (contextMenu->getNbEntries () != 0)
				contextMenu->addSeparator ();
			CPoint p (event.mousePosition);
			view->frameToLocal (p);
			if (contextMenuController2)
				contextMenuController2->appendContextMenuItems (*contextMenu, view, p);
			else if (contextMenuController)
				contextMenuController->appendContextMenuItems (*contextMenu, p);
		}
	}
	if (contextMenu->getNbEntries () == 0 &&
	    getApplicationPlatformAccess ()->getConfiguration ().showCommandsInWindowContextMenu)
	{
		auto commandList = getApplicationPlatformAccess ()->getCommandList (
		    staticPtrCast<Platform::IWindow> (getPlatformWindow ()).get ());
		if (!commandList.empty ())
		{
			auto menuHandler = makeOwned<WindowContextMenuCommandHandler> (this);
			for (const auto& cat : commandList)
			{
				auto item = new CMenuItem (cat.first);
				auto catMenu = new COptionMenu ();
				item->setSubmenu (catMenu);
				for (const auto& entry : cat.second)
				{
					if (entry.name == CommandName::MenuSeparator)
					{
						catMenu->addSeparator ();
					}
					else
					{
						auto catItem =
							new CCommandMenuItem ({entry.name, menuHandler, entry.group, entry.name});
						catMenu->addEntry (catItem);
					}
				}
				if (catMenu->getNbEntries () > 0)
					contextMenu->addEntry (item);
				else
					item->forget ();
			}
		}
	}

	if (contextMenu->getNbEntries () > 0)
	{
		contextMenu->cleanupSeparators (true);
		contextMenu->setStyle (COptionMenu::kPopupStyle | COptionMenu::kMultipleCheckStyle);
		contextMenu->popup (inFrame, event.mousePosition);
		event.consumed = true;
		castMouseDownEvent (event).ignoreFollowUpMoveAndUpEvents (true);
	}
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
