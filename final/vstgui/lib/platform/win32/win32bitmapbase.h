/*
 *  win32bitmapbase.h
 *  VST3PlugIns
 *
 *  Created by Arne Scheffler on 4/9/10.
 *  Copyright 2010 Arne Scheffler. All rights reserved.
 *
 */

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
	virtual bool createMemoryPNGRepresentation (void** ptr, uint32_t& size) = 0;
};

} // namespace

#endif // WINDOWS

#endif // __win32bitmapbase__
