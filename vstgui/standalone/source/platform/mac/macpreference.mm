// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "macpreference.h"
#import "macutilities.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Mac {

//------------------------------------------------------------------------
bool MacPreference::set (const UTF8String& key, const UTF8String& value)
{
	[[NSUserDefaults standardUserDefaults] setObject:stringFromUTF8String (value)
	                                          forKey:stringFromUTF8String (key)];
	return true;
}

//------------------------------------------------------------------------
Optional<UTF8String> MacPreference::get (const UTF8String& key)
{
	NSString* value =
	    [[NSUserDefaults standardUserDefaults] stringForKey:stringFromUTF8String (key)];
	if (value != nil)
		return Optional<UTF8String> (UTF8String ([value UTF8String]));
	return {};
}

//------------------------------------------------------------------------
} // Mac
} // Platform
} // Standalone
} // VSTGUI
