#ifndef __autoreleasepool__
#define __autoreleasepool__

#include "../../../vstguibase.h"

#ifdef __OBJC__
@class NSAutoreleasePool;
#else
struct NSAutoreleasePool;
#endif

BEGIN_NAMESPACE_VSTGUI

class AutoreleasePool
{
public:
	AutoreleasePool ();
	~AutoreleasePool ();
protected:
	NSAutoreleasePool* pool;
};

END_NAMESPACE_VSTGUI

#endif
