// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "iplatformframe.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace X11 {

//------------------------------------------------------------------------
class IEventHandler
{
public:
	virtual void onEvent () = 0;
};

//------------------------------------------------------------------------
class ITimerHandler
{
public:
	virtual void onTimer () = 0;
};

//------------------------------------------------------------------------
class IRunLoop : public virtual IReference
{
public:
	virtual bool registerEventHandler (int fd, IEventHandler* handler) = 0;
	virtual bool unregisterEventHandler (IEventHandler* handler) = 0;

	virtual bool registerTimer (uint64_t interval, ITimerHandler* handler) = 0;
	virtual bool unregisterTimer (ITimerHandler* handler) = 0;
};

//------------------------------------------------------------------------
class FrameConfig : public IPlatformFrameConfig
{
public:
	SharedPointer<IRunLoop> runLoop;
};

//------------------------------------------------------------------------
class IX11Frame
{
public:
	virtual uint32_t getX11WindowID () const = 0;
};

//------------------------------------------------------------------------
} // X11
} // VSTGUI
