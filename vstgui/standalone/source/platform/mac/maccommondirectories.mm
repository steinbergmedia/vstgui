// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "maccommondirectories.h"
#import "../../../include/iappdelegate.h"
#import "../../../include/iapplication.h"
#import "macutilities.h"
#import <Cocoa/Cocoa.h>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Mac {

//------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------
UTF8String createAppPathString ()
{
	return IApplication::instance ().getDelegate ().getInfo ().uri;
}

//------------------------------------------------------------------------
NSURL* addSubDirs (NSURL* url, std::vector<const UTF8String*> subDirs)
{
	for (const auto& subDir : subDirs)
	{
		if (!subDir->empty ())
			url = [url URLByAppendingPathComponent:stringFromUTF8String (*subDir)];
	}
	return url;
}

//------------------------------------------------------------------------
Optional<UTF8String> getPath (NSSearchPathDomainMask domain, NSSearchPathDirectory directory,
                              std::vector<const UTF8String*> subDirs, bool create)
{
	auto fileManager = [NSFileManager defaultManager];
	auto url = [fileManager URLForDirectory:directory
	                               inDomain:domain
	                      appropriateForURL:nil
	                                 create:create ? YES : NO
	                                  error:nil];
	if (url)
	{
		url = addSubDirs (url, subDirs);
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
Optional<UTF8String> CommonDirectories::get (CommonDirectoryLocation location,
                                             const UTF8String& subDir, bool create) const
{
	switch (location)
	{
		case CommonDirectoryLocation::AppPath:
		{
			auto url = [[NSBundle mainBundle] bundleURL];
			return UTF8String ([url fileSystemRepresentation]);
		}
		case CommonDirectoryLocation::AppResourcesPath:
		{
			auto url = [[NSBundle mainBundle] resourceURL];
			url = addSubDirs (url, {&subDir});
			return UTF8String ([url fileSystemRepresentation]) + "/";
		}
		case CommonDirectoryLocation::AppPreferencesPath:
		{
			auto appPath = createAppPathString ();
			UTF8String prefPath ("Preferences");
			return getPath (NSUserDomainMask, NSLibraryDirectory, {&prefPath, &appPath, &subDir},
			                create);
		}
		case CommonDirectoryLocation::AppCachesPath:
		{
			auto appPath = createAppPathString ();
			return getPath (NSUserDomainMask, NSCachesDirectory, {&appPath, &subDir}, create);
		}
		case CommonDirectoryLocation::UserDocumentsPath:
		{
			return getPath (NSUserDomainMask, NSDocumentDirectory, {&subDir}, create);
		}
	}
	return {};
}

//------------------------------------------------------------------------
} // Mac
} // Platform
} // Standalone
} // VSTGUI
