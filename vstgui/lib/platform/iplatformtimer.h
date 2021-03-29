// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../vstguibase.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
class IPlatformTimerCallback
{
public:
	virtual void fire () = 0;
};

//-----------------------------------------------------------------------------
class IPlatformTimer : public AtomicReferenceCounted
{
public:
	virtual bool start (uint32_t fireTime) = 0;
	virtual bool stop () = 0;
};

} // VSTGUI
