// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "iplatformframe.h"
#include "platform_linux.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
namespace X11 {
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
