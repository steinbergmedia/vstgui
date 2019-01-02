// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "autoreleasepool.h"

#if MAC

#import <Foundation/Foundation.h>

namespace VSTGUI {

//-----------------------------------------------------------------------------
AutoreleasePool::AutoreleasePool ()
{
	pool = [[NSAutoreleasePool alloc] init];
}

//-----------------------------------------------------------------------------
AutoreleasePool::~AutoreleasePool () noexcept
{
	[pool drain];
}

} // VSTGUI

#endif // MAC
