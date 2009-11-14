
#ifndef __macglobals__
#define __macglobals__

#include "../../vstguibase.h"

#if MAC

#if VSTGUI_PLATFORM_ABSTRACTION

#include <ApplicationServices/ApplicationServices.h>

BEGIN_NAMESPACE_VSTGUI

// TODO: This needs to be done a nicer fashion
extern void* gBundleRef;
inline CFBundleRef getBundleRef () { return (CFBundleRef)gBundleRef; }
extern CGColorSpaceRef GetGenericRGBColorSpace ();

END_NAMESPACE_VSTGUI

#endif // VSTGUI_PLATFORM_ABSTRACTION
#endif // MAC
#endif // __macglobals__