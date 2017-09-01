// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../../lib/platform/iplatformframe.h"
#include "../../include/icommand.h"
#include "../../include/iwindow.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {

//------------------------------------------------------------------------
class IWindowDelegate : public ICommandHandler
{
public:
	virtual CPoint constraintSize (const CPoint& newSize) = 0;
	virtual void onSizeChanged (const CPoint& newSize) = 0;
	virtual void onPositionChanged (const CPoint& newPosition) = 0;
	virtual void onShow () = 0;
	virtual void onHide () = 0;
	virtual void onClosed () = 0;
	virtual bool canClose () = 0;
	virtual void onActivated () = 0;
	virtual void onDeactivated () = 0;
};

//------------------------------------------------------------------------
class IWindow : public Interface
{
public:
	virtual CPoint getSize () const = 0;
	virtual CPoint getPosition () const = 0;
	virtual double getScaleFactor () const = 0;

	virtual void setSize (const CPoint& newSize) = 0;
	virtual void setPosition (const CPoint& newPosition) = 0;
	virtual void setTitle (const UTF8String& newTitle) = 0;

	virtual void show () = 0;
	virtual void hide () = 0;
	virtual void close () = 0;
	virtual void activate () = 0;
	virtual void center () = 0;

	virtual PlatformType getPlatformType () const = 0;
	virtual void* getPlatformHandle () const = 0;

	virtual void onSetContentView (CFrame* frame) = 0;
};

//------------------------------------------------------------------------
using WindowPtr = std::shared_ptr<IWindow>;

//------------------------------------------------------------------------
WindowPtr makeWindow (const WindowConfiguration& config, IWindowDelegate& delegate);

//------------------------------------------------------------------------
} // Platform
} // Standalone
} // VSTGUI