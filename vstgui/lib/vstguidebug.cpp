// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "vstguidebug.h"
#include <cstdarg>
#include <exception>

#if DEBUG

#include "cstring.h"

#if WINDOWS
	#include "platform/win32/win32support.h"
#endif

#include <cstdio>

namespace VSTGUI {

//-----------------------------------------------------------------------------
TimeWatch::TimeWatch (UTF8StringPtr inName, bool startNow)
: startTime (0)
{
	if (inName)
	{
		auto len = std::strlen (inName);
		name = std::unique_ptr<char[]> (new char[len + 1]);
		std::strcpy (name.get (), inName);
	}
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
		if (name)
			DebugPrint ("%s took %d\n", name.get (), stopTime - startTime);
		else
			DebugPrint ("it took %d\n", stopTime - startTime);
		startTime = 0;
	}
}

//-----------------------------------------------------------------------------
void DebugPrint (const char *format, ...)
{
	static constexpr auto BufferSize = 1024u;
	char string[BufferSize];
	std::va_list marker;
	va_start (marker, format);
	if (std::vsnprintf (string, BufferSize, format, marker) == 0)
		std::strcpy (string, "Empty string\n");
#if WINDOWS
	UTF8StringHelper debugString (string);
	OutputDebugString (debugString);
#else
	std::fprintf (stderr, "%s", string);
#endif
}

} // VSTGUI

#endif // DEBUG

namespace VSTGUI {

static AssertionHandler assertionHandler = nullptr;

//------------------------------------------------------------------------
void setAssertionHandler (AssertionHandler handler)
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
			std::rethrow_exception (std::current_exception ());
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
	
} // VSTGUI
