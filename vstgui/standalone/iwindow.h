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
	enum Style {
		kBorder = 1 << 0,
		kClose = 1 << 1,
		kSize = 1 << 2,
		kAutoSaveFrame = 1 << 4,
	};
	
	WindowFlags& border () { flags |= Style::kBorder; return *this; }
	WindowFlags& close () { flags |= Style::kClose; return *this; }
	WindowFlags& size () { flags |= Style::kSize; return *this; }
	WindowFlags& autoSaveFrame () { flags |= Style::kAutoSaveFrame; return *this; }

	bool hasBorder () const { return flags & Style::kBorder; }
	bool canClose () const { return flags & Style::kClose; }
	bool canSize () const { return flags & Style::kSize; }
	bool doesAutoSaveFrame () const { return flags & Style::kAutoSaveFrame; }

private:
	uint32_t flags {0};
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
class IWindowListener : public Interface
{
public:
	virtual void onSizeChanged (const IWindow& window, const CPoint& newSize) = 0;
	virtual void onPositionChanged (const IWindow& window, const CPoint& newPosition) = 0;
	virtual void onShow (const IWindow& window) = 0;
	virtual void onHide (const IWindow& window) = 0;
	virtual void onClosed (const IWindow& window) = 0;
};

//------------------------------------------------------------------------
class IWindowController : public IWindowListener
{
public:
	virtual CPoint constraintSize (const IWindow& window, const CPoint& newSize) = 0;
	virtual bool canClose (const IWindow& window) const = 0;
};

//------------------------------------------------------------------------
using WindowControllerPtr = std::shared_ptr<IWindowController>;

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
	virtual void addWindowListener (IWindowListener* listener) = 0;
	virtual void removeWindowListener (IWindowListener* listener) = 0;
};

//------------------------------------------------------------------------
using WindowPtr = std::shared_ptr<IWindow>;

//------------------------------------------------------------------------
class WindowControllerAdapter : public IWindowController
{
public:
	void onSizeChanged (const IWindow& window, const CPoint& newSize) {}
	void onPositionChanged (const IWindow& window, const CPoint& newPosition) {}
	void onShow (const IWindow& window) {}
	void onHide (const IWindow& window) {}
	void onClosed (const IWindow& window) {}
	CPoint constraintSize (const IWindow& window, const CPoint& newSize) { return newSize; }
	bool canClose (const IWindow& window) const { return true; }
};
//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
