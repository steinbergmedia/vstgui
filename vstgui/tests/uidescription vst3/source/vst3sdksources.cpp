// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#if MAC
#include <CoreFoundation/CoreFoundation.h>
#endif
#include "pluginterfaces/base/fplatform.h"

#if MAC
#include "public.sdk/source/main/macmain.cpp"
#endif
#if WINDOWS
#include "public.sdk/source/main/dllmain.cpp"
#endif
