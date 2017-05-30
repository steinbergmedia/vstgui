// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __macstring__
#define __macstring__

#include "../iplatformstring.h"

#if MAC
#include "../../cstring.h"
#include <CoreFoundation/CoreFoundation.h>
#include "cfontmac.h"

#if defined(__OBJC__)
#import <Foundation/Foundation.h>
#else
struct NSString;
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
class MacString : public IPlatformString
{
public:
	MacString (UTF8StringPtr utf8String);
	~MacString () noexcept override;
	
	void setUTF8String (UTF8StringPtr utf8String) override;

	CFStringRef getCFString () const { return cfString; }

	CTLineRef getCTLine () const { return ctLine; }
	const void* getCTLineFontRef () const { return ctLineFontRef; }
	const CColor& getCTLineColor () const { return ctLineColor; }

	void setCTLine (CTLineRef line, const void* fontRef, const CColor& color);
//-----------------------------------------------------------------------------
protected:
	CFStringRef cfString;
	CTLineRef ctLine;
	const void* ctLineFontRef;
	CColor ctLineColor;
};

//-----------------------------------------------------------------------------
template <typename T>
inline T fromUTF8String (const UTF8String& str)
{
	vstgui_assert (false);
	return nullptr;
}

//-----------------------------------------------------------------------------
template <>
inline CFStringRef fromUTF8String (const UTF8String& str)
{
	if (auto macString = dynamic_cast<MacString*> (str.getPlatformString ()))
		return macString->getCFString ();
	return nullptr;
}

#ifdef __OBJC__
//-----------------------------------------------------------------------------
template <>
inline NSString* fromUTF8String (const UTF8String& str)
{
	if (auto macString = dynamic_cast<MacString*> (str.getPlatformString ()))
		return (__bridge NSString*) (macString->getCFString ());
	return nil;
}
#endif // __OBJC__

} // namespace

#endif // MAC

#endif // __macstring__
