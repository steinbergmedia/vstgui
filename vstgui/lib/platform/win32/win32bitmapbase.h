// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __win32bitmapbase__
#define __win32bitmapbase__

#include "../iplatformbitmap.h"

#if WINDOWS

#include <windows.h>
#include <objidl.h>

namespace VSTGUI {

class Win32BitmapBase : public IPlatformBitmap
{
public:
	virtual HBITMAP createHBitmap () = 0;
	virtual bool loadFromStream (IStream* stream) = 0;
	virtual PNGBitmapBuffer createMemoryPNGRepresentation () = 0;
};

} // namespace

#endif // WINDOWS

#endif // __win32bitmapbase__
