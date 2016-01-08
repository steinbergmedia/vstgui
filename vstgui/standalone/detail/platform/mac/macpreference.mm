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
UTF8String MacPreference::get (const UTF8String& key)
{
	NSString* value =
	    [[NSUserDefaults standardUserDefaults] stringForKey:stringFromUTF8String (key)];
	if (value != nil)
		return UTF8String ([value UTF8String]);
	return UTF8String ();
}

//------------------------------------------------------------------------
} // Mac
} // Platform
} // Standalone
} // VSTGUI
