// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "vstguidebug.h"
#include <cstdarg>

#if DEBUG

#include "cstring.h"

#if WINDOWS
	#include "platform/win32/win32support.h"
#endif

#include <cstdio>

namespace VSTGUI {

//-----------------------------------------------------------------------------
TimeWatch::TimeWatch (UTF8StringPtr name, bool startNow)
: startTime (0)
{
	this->name = name ? name : "";
	if (startNow)
		start ();
}

//-----------------------------------------------------------------------------
TimeWatch::~TimeWatch () noexcept
{
	stop ();
}

//-----------------------------------------------------------------------------
void TimeWatch::start ()
{
	startTime = std::clock ();
}

//-----------------------------------------------------------------------------
void TimeWatch::stop ()
{
	if (startTime > 0)
	{
		clock_t stopTime = std::clock ();
		DebugPrint ("%s took %d\n", name.data (), stopTime - startTime);
		startTime = 0;
	}
}

//-----------------------------------------------------------------------------
void DebugPrint (const char *format, ...)
{
	char string[300];
	std::va_list marker;
	va_start (marker, format);
	std::vsprintf (string, format, marker);
	if (string[0] == 0)
		std::strcpy (string, "Empty string\n");
	#if WINDOWS
	UTF8StringHelper debugString (string);
	OutputDebugString (debugString);
	#else
	std::fprintf (stderr, "%s", string);
	#endif
}

} // namespace

#endif // DEBUG

namespace VSTGUI {

static AssertionHandler assertionHandler {};

//------------------------------------------------------------------------
void setAssertionHandler (const AssertionHandler& handler)
{
	assertionHandler = handler;
}

//------------------------------------------------------------------------
bool hasAssertionHandler ()
{
	return assertionHandler ? true : false;
}

//------------------------------------------------------------------------
void doAssert (const char* filename, const char* line, const char* desc) noexcept (false)
{
#if NDEBUG
	if (!hasAssertionHandler ())
		return;
#endif
	if (hasAssertionHandler ())
	{
		try {
			assertionHandler (filename, line, desc);
		} catch (...)
		{
		 std::rethrow_exception (std::current_exception());
		}
	}
#if DEBUG
	else
	{
		DebugPrint ("\nassert at %s:%s: %s\n", filename, line, desc ? desc : "unknown");
		assert (false);
	}
#endif // DEBUG
}
	
} // namespace
