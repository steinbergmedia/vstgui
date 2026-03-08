// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#ifndef VSTGUI_VERSION_MAJOR
#error "do not directly include this file, use vstguibase.h"
#endif

//------------------------------------------------------------------------
namespace VSTGUI {

using AssertionHandler = void (*) (const char* filename, const char* line, const char* condition,
								   const char* desc);
void setAssertionHandler (AssertionHandler handler);
bool hasAssertionHandler ();
void doAssert (const char* filename, const char* line, const char* condition,
			   const char* desc = nullptr) noexcept (false);

#define vstgui_assert(x, ...)                                                                      \
	if (!(x))                                                                                      \
		VSTGUI::doAssert (__FILE__, VSTGUI_MAKE_STRING (__LINE__), VSTGUI_MAKE_STRING (x),         \
						  ##__VA_ARGS__);

} // VSTGUI

#if DEBUG

#include <ctime>
#include <cassert>
#include <memory>
#include <vector>
#include <string>

namespace VSTGUI {

//-----------------------------------------------------------------------------
extern void DebugPrint (const char *format, ...);

//-----------------------------------------------------------------------------
class TimeWatch
{
public:
	TimeWatch (UTF8StringPtr name = nullptr, bool startNow = true);
	~TimeWatch () noexcept;
	
	void start ();
	void stop ();

protected:
	std::unique_ptr<char[]> name;
	std::clock_t startTime;
};

//------------------------------------------------------------------------
namespace Debug {

using Backtrace = std::vector<std::string>;
extern Backtrace backtrace (uint32_t maxFrames = 128);

//------------------------------------------------------------------------
} // Debug
} // VSTGUI

#else

#endif // DEBUG
