// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

/// @cond ignore

#import "../../cstring.h"
#import "cocoa/cocoahelpers.h"
#import "macfileselector.h"
#import "macstring.h"

#pragma clang diagnostic push

#if defined(VSTGUI_USE_OBJC_UTTYPE)
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#else
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace VSTGUI {

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CocoaFileSelector
: public IPlatformFileSelector
, public std::enable_shared_from_this<CocoaFileSelector>
{
public:
	CocoaFileSelector (PlatformFileSelectorStyle style, NSViewFrame* frame);
	~CocoaFileSelector () override = default;

	bool run (const PlatformFileSelectorConfig& config) override;
	bool cancel () override;

	void openPanelDidEnd (NSSavePanel* panel, NSInteger resultCode);

protected:
	static void initClass ();

	void setupInitalDir (const PlatformFileSelectorConfig& config);

	PlatformFileSelectorStyle style;
	NSViewFrame* frame {nullptr};
	NSSavePanel* savePanel {nullptr};
	PlatformFileSelectorConfig::CallbackFunc callback;
};

//-----------------------------------------------------------------------------
PlatformFileSelectorPtr createCocoaFileSelector (PlatformFileSelectorStyle style,
												 NSViewFrame* frame)
{
	return std::make_shared<CocoaFileSelector> (style, frame);
}

//-----------------------------------------------------------------------------
void CocoaFileSelector::initClass ()
{
}

//-----------------------------------------------------------------------------
CocoaFileSelector::CocoaFileSelector (PlatformFileSelectorStyle style, NSViewFrame* frame)
: style (style), frame (frame)
{
	initClass ();
}

//-----------------------------------------------------------------------------
void CocoaFileSelector::openPanelDidEnd (NSSavePanel* panel, NSInteger res)
{
	std::vector<UTF8String> result;
	if (res == NSModalResponseOK)
	{
		if (style == PlatformFileSelectorStyle::SelectSaveFile)
		{
			NSURL* url = [panel URL];
			const char* utf8Path = url ? [[url path] UTF8String] : nullptr;
			if (utf8Path)
			{
				result.emplace_back (utf8Path);
			}
		}
		else
		{
			NSOpenPanel* openPanel = (NSOpenPanel*)panel;
			NSArray* urls = [openPanel URLs];
			for (NSUInteger i = 0; i < [urls count]; i++)
			{
				NSURL* url = [urls objectAtIndex:i];
				if (url == nullptr || [url path] == nullptr)
					continue;
				const char* utf8Path = [[url path] UTF8String];
				if (utf8Path)
				{
					result.emplace_back (utf8Path);
				}
			}
		}
	}
	if (callback)
		callback (std::move (result));
}

//-----------------------------------------------------------------------------
bool CocoaFileSelector::cancel ()
{
	if (savePanel)
	{
		[savePanel cancel:nil];
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool CocoaFileSelector::run (const PlatformFileSelectorConfig& config)
{
	NSWindow* parentWindow = nil;

	if (!hasBit (config.flags, PlatformFileSelectorFlags::RunModal) && frame)
		parentWindow = [(frame->getNSView ()) window];

	callback = config.doneCallback;

	NSOpenPanel* openPanel = nil;
	NSMutableArray* typesArray = nil;
	if (config.extensions.empty () == false)
	{
		typesArray = [[[NSMutableArray alloc] init] autorelease];
		for (auto& ext : config.extensions)
		{
#ifdef VSTGUI_USE_OBJC_UTTYPE
			UTType* uti = nullptr;
			if (ext.uti.empty () == false)
				uti = [UTType typeWithIdentifier:fromUTF8String<NSString*> (ext.uti)];
			if (uti == nullptr && ext.mimeType.empty() == false)
				uti = [UTType typeWithMIMEType:fromUTF8String<NSString*> (ext.mimeType)];
			if (uti == nullptr && ext.extension.empty () == false)
				uti = [UTType typeWithFilenameExtension:fromUTF8String<NSString*> (ext.extension)];
			if (uti)
				[typesArray addObject:uti];
#else
			NSString* uti = nullptr;
			if (ext.uti.empty () == false)
				uti = [fromUTF8String<NSString*> (ext.uti) retain];
			if (uti == nullptr && ext.mimeType.empty () == false)
				uti = (NSString*)UTTypeCreatePreferredIdentifierForTag (
					kUTTagClassMIMEType, fromUTF8String<CFStringRef> (ext.mimeType), kUTTypeData);
			if (uti == nullptr && ext.macType)
			{
				NSString* osType =
					(NSString*)UTCreateStringForOSType (static_cast<OSType> (ext.macType));
				if (osType)
				{
					uti = (NSString*)UTTypeCreatePreferredIdentifierForTag (
						kUTTagClassOSType, (CFStringRef)osType, kUTTypeData);
					[osType release];
				}
			}
			if (uti == nullptr && ext.extension.empty () == false)
				uti = [fromUTF8String<NSString*> (ext.extension) retain];
			if (uti)
			{
				[typesArray addObject:uti];
				[uti release];
			}
#endif
		}
	}
	if (style == PlatformFileSelectorStyle::SelectSaveFile)
	{
		savePanel = [NSSavePanel savePanel];
		if (typesArray)
		{
#ifdef VSTGUI_USE_OBJC_UTTYPE
			[savePanel setAllowedContentTypes:typesArray];
#else
			[savePanel setAllowedFileTypes:typesArray];
#endif
		}
	}
	else
	{
		savePanel = openPanel = [NSOpenPanel openPanel];
		if (style == PlatformFileSelectorStyle::SelectFile)
		{
			bool allowMultiFileSelection =
				hasBit (config.flags, PlatformFileSelectorFlags::MultiFileSelection);
			[openPanel setAllowsMultipleSelection:allowMultiFileSelection ? YES : NO];
		}
		else
		{
			[openPanel setCanChooseDirectories:YES];
		}
	}
	if (!config.title.empty () && savePanel)
	{
#if 0 // Apple broke this again with macOS 12. Disable this now and always use the message to
	  // display the title.
		if (@available (macOS 11, *))
		{
			if (parentWindow)
				[savePanel setMessage:fromUTF8String<NSString*> (config.title)];
			else
				[savePanel setTitle:fromUTF8String<NSString*> (config.title)];
		}
		else
#endif
		if (@available (macOS 10.11, *))
		{
			[savePanel setMessage:fromUTF8String<NSString*> (config.title)];
		}
		else
		{
			[savePanel setTitle:fromUTF8String<NSString*> (config.title)];
		}
	}
	if (openPanel)
	{
#ifdef VSTGUI_USE_OBJC_UTTYPE
		openPanel.allowedContentTypes = typesArray;
#else
		openPanel.allowedFileTypes = typesArray;
#endif
		if (parentWindow)
		{
			setupInitalDir (config);
			auto This = shared_from_this ();
			[openPanel beginSheetModalForWindow:parentWindow
							  completionHandler:^(NSInteger result) {
								  This->openPanelDidEnd (openPanel, result);
							  }];
		}
		else
		{
			setupInitalDir (config);
			NSInteger res = [openPanel runModal];
			openPanelDidEnd (openPanel, res);
			return res == NSModalResponseOK;
		}
	}
	else if (savePanel)
	{
#ifdef VSTGUI_USE_OBJC_UTTYPE
		savePanel.allowedContentTypes = typesArray;
#else
		savePanel.allowedFileTypes = typesArray;
#endif
		if (parentWindow)
		{
			setupInitalDir (config);
			auto This = shared_from_this ();
			[savePanel beginSheetModalForWindow:parentWindow
							  completionHandler:^(NSInteger result) {
								  This->openPanelDidEnd (savePanel, result);
							  }];
		}
		else
		{
			setupInitalDir (config);
			NSInteger res = [savePanel runModal];
			openPanelDidEnd (savePanel, res);
			return res == NSModalResponseOK;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
void CocoaFileSelector::setupInitalDir (const PlatformFileSelectorConfig& config)
{
	if (!config.initialPath.empty ())
	{
		NSURL* dirURL = [NSURL fileURLWithPath:fromUTF8String<NSString*> (config.initialPath)];
		NSNumber* isDir;
		if ([dirURL getResourceValue:&isDir forKey:NSURLIsDirectoryKey error:nil])
		{
			if ([isDir boolValue] == NO)
			{
				savePanel.nameFieldStringValue =
					[[dirURL.path lastPathComponent] stringByDeletingPathExtension];
				dirURL = [NSURL fileURLWithPath:[dirURL.path stringByDeletingLastPathComponent]];
			}
			savePanel.directoryURL = dirURL;
		}
	}
	if (!config.defaultSaveName.empty ())
	{
		savePanel.nameFieldStringValue = fromUTF8String<NSString*> (config.defaultSaveName);
	}
}

} // VSTGUI

#pragma clang diagnostic pop

/// @endcond
