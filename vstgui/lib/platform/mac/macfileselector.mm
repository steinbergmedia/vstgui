// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

/// @cond ignore

#import "../../cfileselector.h"
#import "../../cstring.h"
#import "../../cframe.h"

// the cocoa fileselector is also used for carbon
#import <Cocoa/Cocoa.h>
#import "cocoa/cocoahelpers.h"
#import "macstring.h"

#if MAC_COCOA
#import "cocoa/nsviewframe.h"
#endif

namespace VSTGUI {

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CocoaFileSelector : public CNewFileSelector
{
public:
	CocoaFileSelector (CFrame* frame, Style style);
	~CocoaFileSelector () override = default;

	void openPanelDidEnd (NSSavePanel* panel, NSInteger resultCode);
protected:
	static void initClass ();
	
	bool runInternal (CBaseObject* delegate) override;
	bool runModalInternal () override;
	void cancelInternal () override;

	void setupInitalDir ();

	Style style;
	SharedPointer<CBaseObject> delegate;
	NSSavePanel* savePanel;
};

//-----------------------------------------------------------------------------
CNewFileSelector* CNewFileSelector::create (CFrame* frame, Style style)
{
	return new CocoaFileSelector (frame, style);
}

//-----------------------------------------------------------------------------
void CocoaFileSelector::initClass ()
{
}

//-----------------------------------------------------------------------------
CocoaFileSelector::CocoaFileSelector (CFrame* frame, Style style)
: CNewFileSelector (frame)
, style (style)
, delegate (nullptr)
{
	savePanel = nil;
	initClass ();
}

//-----------------------------------------------------------------------------
void CocoaFileSelector::openPanelDidEnd (NSSavePanel* savePanel, NSInteger res)
{
	if (res == NSFileHandlingPanelOKButton)
	{
		if (style == kSelectSaveFile)
		{
			NSURL* url = [savePanel URL];
			const char* utf8Path = url ? [[url path] UTF8String] : nullptr;
			if (utf8Path)
			{
				result.emplace_back (utf8Path);
			}
		}
		else
		{
			NSOpenPanel* openPanel = (NSOpenPanel*)savePanel;
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
	if (delegate)
		delegate->notify (this, CNewFileSelector::kSelectEndMessage);
}

//-----------------------------------------------------------------------------
void CocoaFileSelector::cancelInternal ()
{
	if (savePanel)
		[savePanel cancel:nil];
}

//-----------------------------------------------------------------------------
bool CocoaFileSelector::runInternal (CBaseObject* _delegate)
{
	CBaseObjectGuard lifeGuard (this);

	NSWindow* parentWindow = nil;
	if (_delegate)
	{
		#if MAC_COCOA
		if (frame && frame->getPlatformFrame ())
		{
			NSViewFrame* nsViewFrame = static_cast<NSViewFrame*> (frame->getPlatformFrame ());
			parentWindow = nsViewFrame ? [(nsViewFrame->getPlatformControl ()) window] : nullptr;
		}
		#endif
		delegate = _delegate;
	}
	NSOpenPanel* openPanel = nil;
	NSMutableArray* typesArray = nil;
	if (extensions.empty () == false)
	{
		typesArray = [[[NSMutableArray alloc] init] autorelease];
		for (auto& ext : extensions)
		{
			NSString* uti = nullptr;
			if (ext.getUTI ().empty () == false)
				uti = [fromUTF8String<NSString*> (ext.getUTI ()) retain];
			if (uti == nullptr && ext.getMimeType ().empty () == false)
				uti = (NSString*)UTTypeCreatePreferredIdentifierForTag (kUTTagClassMIMEType, fromUTF8String<CFStringRef> (ext.getMimeType ()), kUTTypeData);
			if (uti == nullptr && ext.getMacType ())
			{
				NSString* osType = (NSString*)UTCreateStringForOSType (static_cast<OSType> (ext.getMacType ()));
				if (osType)
				{
					uti = (NSString*)UTTypeCreatePreferredIdentifierForTag (kUTTagClassOSType, (CFStringRef)osType, kUTTypeData);
					[osType release];
				}
			}
			if (uti == nullptr && ext.getExtension ().empty () == false)
				uti = [fromUTF8String<NSString*> (ext.getExtension ()) retain];
			if (uti)
			{
				[typesArray addObject:uti];
				[uti release];
			}
		}
	}
	if (style == kSelectSaveFile)
	{
		savePanel = [NSSavePanel savePanel];
		if (typesArray)
			[savePanel setAllowedFileTypes:typesArray];
	}
	else
	{
		savePanel = openPanel = [NSOpenPanel openPanel];
		if (style == kSelectFile)
		{
			[openPanel setAllowsMultipleSelection:allowMultiFileSelection ? YES : NO];
		}
		else
		{
			[openPanel setCanChooseDirectories:YES];
		}
	}
	if (!title.empty () && savePanel)
		[savePanel setTitle:fromUTF8String<NSString*>(title)];
	if (openPanel)
	{
	#if MAC_COCOA
		if (parentWindow)
		{
			setupInitalDir ();
			openPanel.allowedFileTypes = typesArray;
			remember ();
			[openPanel beginSheetModalForWindow:parentWindow completionHandler:^(NSInteger result) {
				openPanelDidEnd (openPanel, result);
				forget ();
			}];
		}
		else
	#endif
		{
			setupInitalDir ();
			openPanel.allowedFileTypes = typesArray;
			NSInteger res = [openPanel runModal];
			openPanelDidEnd (openPanel, res);
			return res == NSFileHandlingPanelOKButton;
		}
	}
	else if (savePanel)
	{
	#if MAC_COCOA
		if (parentWindow)
		{
			setupInitalDir ();
			savePanel.allowedFileTypes = typesArray;
			remember ();
			[savePanel beginSheetModalForWindow:parentWindow completionHandler:^(NSInteger result) {
				openPanelDidEnd (savePanel, result);
				forget ();
			}];
		}
		else
	#endif
		{
			setupInitalDir ();
			savePanel.allowedFileTypes = typesArray;
			NSInteger res = [savePanel runModal];
			openPanelDidEnd (savePanel, res);
			return res == NSFileHandlingPanelOKButton;
		}
	}
	
	return true;
}

//-----------------------------------------------------------------------------
void CocoaFileSelector::setupInitalDir ()
{
	if (!initialPath.empty ())
	{
		NSURL* dirURL = [NSURL fileURLWithPath:fromUTF8String<NSString*> (initialPath)];
		NSNumber* isDir;
		if ([dirURL getResourceValue:&isDir forKey:NSURLIsDirectoryKey error:nil])
		{
			if ([isDir boolValue] == NO)
			{
				savePanel.nameFieldStringValue = [[dirURL.path lastPathComponent] stringByDeletingPathExtension];
				dirURL = [NSURL fileURLWithPath:[dirURL.path stringByDeletingLastPathComponent]];
			}
			savePanel.directoryURL = dirURL;
		}
	}
	if (!defaultSaveName.empty ())
	{
		savePanel.nameFieldStringValue = fromUTF8String<NSString*> (defaultSaveName);
	}
}

//-----------------------------------------------------------------------------
bool CocoaFileSelector::runModalInternal ()
{
	return runInternal (nullptr);
}

} // VSTGUI

/// @endcond
