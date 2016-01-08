#pragma once

#import "../../../../lib/platform/mac/macstring.h"

#import <Foundation/Foundation.h>

//------------------------------------------------------------------------
inline NSString* stringFromUTF8String (const VSTGUI::UTF8String& str)
{
	auto macStr = dynamic_cast<VSTGUI::MacString*> (str.getPlatformString ());
	if (macStr && macStr->getCFString ())
	{
		return (__bridge NSString*)macStr->getCFString ();
	}
	return [NSString stringWithUTF8String:str.get ()];
}
