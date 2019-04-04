// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../../vstguibase.h"

#if MAC

#ifdef __OBJC__
@class NSAutoreleasePool;
#else
struct NSAutoreleasePool;
#endif // __OBJC__

namespace VSTGUI {

//------------------------------------------------------------------------------------
class AutoreleasePool
{
public:
	AutoreleasePool ();
	~AutoreleasePool () noexcept;

//------------------------------------------------------------------------------------
protected:
	NSAutoreleasePool* pool;
};

} // VSTGUI

#endif // MAC
