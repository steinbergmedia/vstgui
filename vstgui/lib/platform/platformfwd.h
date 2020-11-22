// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include <vector>
#include <functional>
#include <string>

#if WINDOWS
using HINSTANCE = struct HINSTANCE__*;
#elif MAC
using CFBundleRef = struct __CFBundle*;
#endif

//------------------------------------------------------------------------
namespace VSTGUI {

using PNGBitmapBuffer = std::vector<uint8_t>;
using FontFamilyCallback = std::function<bool (const std::string&)>;

class LinuxFactory;
class MacFactory;
class Win32Factory;

#if WINDOWS
using PlatformInstanceHandle = HINSTANCE;
#elif MAC
using PlatformInstanceHandle = CFBundleRef;
#elif LINUX
using PlatformInstanceHandle = void*;
#else
static_assert (false, "unknown platform");
#endif

//------------------------------------------------------------------------
} // VSTGUI

