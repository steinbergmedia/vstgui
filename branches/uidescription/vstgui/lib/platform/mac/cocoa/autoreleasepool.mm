
#include "autoreleasepool.h"

#import <Foundation/Foundation.h>

BEGIN_NAMESPACE_VSTGUI

AutoreleasePool::AutoreleasePool ()
{
	pool = [[NSAutoreleasePool alloc] init];
}

AutoreleasePool::~AutoreleasePool ()
{
	[pool release];
}

END_NAMESPACE_VSTGUI
