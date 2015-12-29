#pragma once

#include "fwd.h"
#include "interface.h"
#include "../lib/crect.h"
#include "../lib/cstring.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
struct WindowFlags
{
private:
	uint32_t flags {0};

	enum Style {
		kBorder = 1 << 0,
		kClose = 1 << 1,
		kSize = 1 << 2,

		kPopup = 1 << 8,
		kTransparent = 1 << 9,
	};
public:
	WindowFlags& border () { flags |= Style::kBorder; return *this; }
	WindowFlags& close () { flags |= Style::kClose; return *this; }
	WindowFlags& size () { flags |= Style::kSize; return *this; }
	WindowFlags& popup () { flags |= Style::kPopup; return *this; }
	WindowFlags& transparent () { flags |= Style::kTransparent; return *this; }

	bool hasBorder () const { return (flags & Style::kBorder) != 0; }
	bool canClose () const { return (flags & Style::kClose) != 0; }
	bool canSize () const { return (flags & Style::kSize) != 0; }
	bool isPopup () const { return (flags & Style::kPopup) != 0; }
	bool isTransparent () const { return (flags & Style::kTransparent) != 0; }
};

//------------------------------------------------------------------------
struct WindowConfiguration
{
	WindowFlags flags;
	CPoint size;
	UTF8String title;
	UTF8String autoSaveFrameName;
};

//------------------------------------------------------------------------
class IWindow : public Interface
{
public:
	virtual const WindowControllerPtr& getController () const = 0;
	
	virtual CPoint getSize () const = 0;
	virtual CPoint getPosition () const = 0;

	virtual void setSize (const CPoint& newSize) = 0;
	virtual void setPosition (const CPoint& newPosition) = 0;
	virtual void setTitle (const UTF8String& newTitle) = 0;
	virtual void setContentView (const SharedPointer<CFrame>& frame) = 0;

	virtual void show () = 0;
	virtual void hide () = 0;
	virtual void close () = 0;

	// window listeners are removed automatically when window is closed
	virtual void registerWindowListener (IWindowListener* listener) = 0;
	virtual void unregisterWindowListener (IWindowListener* listener) = 0;
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
