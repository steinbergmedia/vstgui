// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "macstring.h"

#if MAC

namespace VSTGUI {

//-----------------------------------------------------------------------------
SharedPointer<IPlatformString> IPlatformString::createWithUTF8String (UTF8StringPtr utf8String)
{
	return makeOwned<MacString> (utf8String);
}

//-----------------------------------------------------------------------------
MacString::MacString (UTF8StringPtr utf8String)
: cfString (nullptr)
, ctLine (nullptr)
, ctLineFontRef (nullptr)
{
	if (utf8String)
		cfString = CFStringCreateWithCString (nullptr, utf8String, kCFStringEncodingUTF8);
}

//-----------------------------------------------------------------------------
MacString::~MacString () noexcept
{
	if (ctLine)
		CFRelease (ctLine);
	if (cfString)
		CFRelease (cfString);
}

//-----------------------------------------------------------------------------
void MacString::setUTF8String (UTF8StringPtr utf8String)
{
	if (cfString)
		CFRelease (cfString);

	if (ctLine)
		CFRelease (ctLine);
	ctLine = nullptr;
	ctLineFontRef = nullptr;

	if (utf8String)
		cfString = CFStringCreateWithCString (nullptr, utf8String, kCFStringEncodingUTF8);
	else
		cfString = nullptr;
}

//-----------------------------------------------------------------------------
void MacString::setCTLine (CTLineRef line, const void* fontRef, const CColor& color)
{
	if (ctLine)
		CFRelease (ctLine);
	ctLine = line;
	if (ctLine)
		CFRetain (ctLine);
	ctLineFontRef = fontRef;
	ctLineColor = color;
}

} // namespace

#endif // MAC
