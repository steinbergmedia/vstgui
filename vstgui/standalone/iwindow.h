#pragma once

#include "fwd.h"
#include "../lib/crect.h"
#include "../lib/cstring.h"
#include "interface.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
/** About window types:
 *
 *	There are two types of windows :
 *
 *	- Document
 *
 *	- Popup
 *
 *	There can be as many document windows visible as you wish, but only one popup can be visible at
 *	a time.
 *	Popup windows will be closed when they get deactivated.
 *
 */
enum class WindowType
{
	Document,
	Popup,
};

//------------------------------------------------------------------------
struct WindowStyle
{
private:
	uint32_t flags {0};

	enum Style
	{
		Border = 1 << 0,
		Close = 1 << 1,
		Size = 1 << 2,
		Transparent = 1 << 3,
		MovableByWindowBackground = 1 << 4,
		Centered = 1 << 5,
	};

public:
	WindowStyle () = default;

	WindowStyle& border ()
	{
		flags |= Style::Border;
		return *this;
	}
	WindowStyle& close ()
	{
		flags |= Style::Close;
		return *this;
	}
	WindowStyle& size ()
	{
		flags |= Style::Size;
		return *this;
	}
	WindowStyle& transparent ()
	{
		flags |= Style::Transparent;
		return *this;
	}
	WindowStyle& movableByWindowBackground ()
	{
		flags |= Style::MovableByWindowBackground;
		return *this;
	}
	WindowStyle& centered ()
	{
		flags |= Style::Centered;
		return *this;
	}

	bool hasBorder () const { return (flags & Style::Border) != 0; }
	bool canClose () const { return (flags & Style::Close) != 0; }
	bool canSize () const { return (flags & Style::Size) != 0; }
	bool isTransparent () const { return (flags & Style::Transparent) != 0; }
	bool isMovableByWindowBackground () const
	{
		return (flags & Style::MovableByWindowBackground) != 0;
	}
	bool isCentered () const { return (flags & Style::Centered) != 0; }
};

//------------------------------------------------------------------------
struct WindowConfiguration
{
	WindowType type {WindowType::Document};
	WindowStyle style;
	CPoint size;
	UTF8String title;
	UTF8String autoSaveFrameName;
};

//------------------------------------------------------------------------
/** Window interface
 *
 *	Windows are created via IApplication::instance ().createWindow ()
 *
 *	Windows are automatically destroyed when they are closed.
 *
 */
class IWindow : public Interface
{
public:
	virtual const WindowControllerPtr& getController () const = 0;

	virtual CPoint getSize () const = 0;
	virtual CPoint getPosition () const = 0;
	virtual double getScaleFactor () const = 0;
	virtual CRect getFocusViewRect () const = 0;

	virtual void setSize (const CPoint& newSize) = 0;
	virtual void setPosition (const CPoint& newPosition) = 0;
	virtual void setTitle (const UTF8String& newTitle) = 0;
	virtual void setContentView (const SharedPointer<CFrame>& frame) = 0;

	virtual void show () = 0;
	virtual void hide () = 0;
	virtual void close () = 0;

	virtual void activate () = 0;

	/** Register a window listener.
	 *
	 *	There is no ownership involved here, so you have to make sure the listener is alive
	 *	as long as the window lives.
	 *	Listeners are automatically removed when the window is closed.
	 */
	virtual void registerWindowListener (IWindowListener* listener) = 0;
	/** Unregister a window listener */
	virtual void unregisterWindowListener (IWindowListener* listener) = 0;
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
