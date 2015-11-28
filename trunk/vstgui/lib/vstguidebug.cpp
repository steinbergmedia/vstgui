//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "vstguidebug.h"

#if DEBUG

#include "cstring.h"

#if WINDOWS
	#include "platform/win32/win32support.h"
#endif

#include <cstdarg>
#include <cstdio>

namespace VSTGUI {

//-----------------------------------------------------------------------------
TimeWatch::TimeWatch (UTF8StringPtr name, bool startNow)
: startTime (0)
{
	this->name = String::newWithString (name);
	if (startNow)
		start ();
}

//-----------------------------------------------------------------------------
TimeWatch::~TimeWatch ()
{
	stop ();
	String::free (name);
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
		DebugPrint ("%s took %d\n", name, stopTime - startTime);
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
