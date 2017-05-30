// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __vstguidebug__
#define __vstguidebug__

#include "vstguibase.h"
#include <functional>

//------------------------------------------------------------------------
namespace VSTGUI {

using AssertionHandler = std::function<void (const char* filename, const char* line, const char* desc)>;
void setAssertionHandler (const AssertionHandler& handler);
bool hasAssertionHandler ();
void doAssert (const char* filename, const char* line, const char* desc = nullptr) noexcept (false);

#define vstgui_assert(x, ...) if (!(x)) VSTGUI::doAssert (__FILE__, VSTGUI_MAKE_STRING(__LINE__), ## __VA_ARGS__);

} // VSTGUI

#if DEBUG

#include <ctime>
#include <cassert>

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
	std::string name;
	std::clock_t startTime;
};

} // namespace

#else

#endif // DEBUG

#endif
