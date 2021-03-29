// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "winstring.h"

#if WINDOWS

#include <algorithm>

namespace VSTGUI {

static int kMinWinStringBufferSize = 256;

//-----------------------------------------------------------------------------
WinString::WinString (UTF8StringPtr utf8String)
: wideString (nullptr)
, wideStringBufferSize (0)
{
	setUTF8String (utf8String);
}

//-----------------------------------------------------------------------------
WinString::~WinString () noexcept
{
	if (wideString)
		std::free (wideString);
}

//-----------------------------------------------------------------------------
void WinString::setUTF8String (UTF8StringPtr utf8String)
{
	if (utf8String)
	{
		int numChars = MultiByteToWideChar (CP_UTF8, 0, utf8String, -1, nullptr, 0);
		if ((numChars+1)*2 > wideStringBufferSize)
		{
			if (wideString)
				std::free (wideString);
			wideStringBufferSize = std::max<int> ((numChars+1)*2, kMinWinStringBufferSize);
			wideString = (WCHAR*)std::malloc (static_cast<size_t> (wideStringBufferSize));
		}
		if (wideString && MultiByteToWideChar (CP_UTF8, 0, utf8String, -1, wideString, numChars) == 0)
		{
			wideString[0] = 0;
		}
	}
	else if (wideString)
		wideString[0] = 0;
}

} // VSTGUI

#endif // WINDOWS
