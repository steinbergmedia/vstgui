// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "fwd.h"
#include "../../lib/crect.h"
#include "../../lib/cstring.h"
#include "interface.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
/** Window types
 *
 *	About window types:
 *
 *	There are two types of windows :
 *
 *	- Document
 *
 *	- Popup
 *
 *	There can be as many document windows visible as you wish, but only one popup can be visible at
 *	a time.
 *	A popup window will automatically close if it is deactivated.
 *
 *
 *	@ingroup standalone
 */
enum class WindowType
{
	Document,
	Popup,
};

//------------------------------------------------------------------------
/** Window style
 *
 *	Defines window style and behaviour.
 *
 *	Border: Adds border and title bar. If transparent is set, this is ignored.
 *
 *	Close: Adds a closebox if bordered and allows standard ways of closing the window.
 *
 *	Size: Allows user resizing.
 *
 *	Transparent: Window has no background and no operating system style window frame.
 *
 *	MovableByWindowBackground: User can move the window by its background.
 *
 *	Centered: Window will initially shown centered on screen.
 *
 *	@ingroup standalone
 */
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

	WindowStyle& operator+= (WindowStyle toAdd)
	{
		flags |= toAdd.flags;
		return *this;
	}

	WindowStyle& operator-= (WindowStyle toRemove)
	{
		flags &= ~(toRemove.flags);
		return *this;
	}

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
/** Window configuration
 *
 *	@ingroup standalone
 */
struct WindowConfiguration
{
	/** Type of window */
	WindowType type {WindowType::Document};
	/** Window style */
	WindowStyle style;
	/** Initial window size */
	CPoint size;
	/** Window title */
	UTF8String title;
	/** Window save frame name */
	UTF8String autoSaveFrameName;
	/** Window group identifier [optional]
	 *
	 *	Windows with the same group identifier can be grouped together on some platforms like on
	 *	macOS to tabs in a single window
	 */
	UTF8String groupIdentifier;
};

//------------------------------------------------------------------------
/** Window interface
 *
 *	Windows are created via IApplication::instance ().createWindow ()
 *
 *	Windows are automatically destroyed when they are closed.
 *
 *	@ingroup standalone
 */
class IWindow : public Interface
{
public:
	/** Get the window controller. Can be nullptr. */
	virtual const WindowControllerPtr& getController () const = 0;

	/** Get the size of the client area. */
	virtual CPoint getSize () const = 0;
	/** Get the position in global coordinates. */
	virtual CPoint getPosition () const = 0;
	/** Get the content scale factor. */
	virtual double getScaleFactor () const = 0;
	/** Get the rect of the current focus view in frame relative coordinates. */
	virtual CRect getFocusViewRect () const = 0;
	/** Get the title of the window. */
	virtual const UTF8String& getTitle () const = 0;
	/** Get the type of the window. */
	virtual WindowType getType () const = 0;
	/** Get the style of the window. */
	virtual WindowStyle getStyle () const = 0;
	/** Get the auto save frame name of the window. */
	virtual const UTF8String& getAutoSaveFrameName () const = 0;

	/** Set the size of the client area. */
	virtual void setSize (const CPoint& newSize) = 0;
	/** Set the position in global coordinates. */
	virtual void setPosition (const CPoint& newPosition) = 0;
	/** Set the window title. */
	virtual void setTitle (const UTF8String& newTitle) = 0;
	/** Set content view. */
	virtual void setContentView (const SharedPointer<CFrame>& frame) = 0;
	/** Set the path the contents of this window represents. */
	virtual void setRepresentedPath (const UTF8String& path) = 0;
	/** Change window style.
	 *	May not change every style. Depends on the platform.
	 *	Returns effective style.
	 */
	virtual WindowStyle changeStyle (WindowStyle stylesToAdd, WindowStyle stylesToRemove) = 0;

	/** Show the window. */
	virtual void show () = 0;
	/** Hide the window. */
	virtual void hide () = 0;
	/** Close the window. */
	virtual void close () = 0;

	/** Activate the window. */
	virtual void activate () = 0;

	/** Register a window listener.
	 *
	 *	There is no ownership involved here, so you have to make sure the listener is alive
	 *	as long as the window lives.
	 *	Listeners are automatically removed when the window is closed.
	 */
	virtual void registerWindowListener (IWindowListener* listener) = 0;
	/** Unregister a window listener. */
	virtual void unregisterWindowListener (IWindowListener* listener) = 0;
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
