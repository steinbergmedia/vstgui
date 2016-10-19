#import "maccommondirectories.h"
#import "macutilities.h"
#import "../../../include/iapplication.h"
#import "../../../include/iappdelegate.h"
#import <Cocoa/Cocoa.h>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Mac {

//------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------
UTF8String getPath (NSSearchPathDomainMask domain, NSSearchPathDirectory directory, const UTF8String& subDir, bool create, bool addAppURI)
{
	auto fileManager = [NSFileManager defaultManager];
	auto url = [fileManager URLForDirectory:directory
								   inDomain:domain
						  appropriateForURL:nil
									 create:create ? YES : NO
									  error:nil];
	if (url)
	{
		if (addAppURI)
		{
			auto name = IApplication::instance ().getDelegate ().getInfo ().uri;
			url = [url URLByAppendingPathComponent:stringFromUTF8String (name)];
		}
		if (!subDir.empty ())
		{
			url = [url URLByAppendingPathComponent:stringFromUTF8String (subDir)];
		}
		if (create)
		{
			if (![fileManager createDirectoryAtURL:url
					   withIntermediateDirectories:YES
										attributes:nil
											 error:nil])
			{
				return {};
			}
		}
		return UTF8String ([url.path UTF8String]) + "/";
	}
	return {};
}

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
UTF8String CommonDirectories::get (Location location, const UTF8String& subDir, bool create) const
{
	switch (location)
	{
		case Location::AppPath:
		{
			auto url = [[NSBundle mainBundle] bundleURL];
			return UTF8String ([url fileSystemRepresentation]);
		}
		case Location::AppPreferences:
		{
			break;
		}
		case Location::AppCaches:
		{
			return getPath (NSUserDomainMask, NSCachesDirectory, subDir, create, true);
		}
		case Location::UserDocuments:
		{
			break;
		}
	}
	return {};
}

//------------------------------------------------------------------------
} // Mac
} // Platform
} // Standalone
} // VSTGUI
