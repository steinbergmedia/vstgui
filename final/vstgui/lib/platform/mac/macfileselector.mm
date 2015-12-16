//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

/// @cond ignore

#import "../../cfileselector.h"
#import "../../cstring.h"
#import "../../cframe.h"

// the cocoa fileselector is also used for carbon
#import <Cocoa/Cocoa.h>
#import "cocoa/cocoahelpers.h"

#if MAC_COCOA
#import "cocoa/nsviewframe.h"

	#if MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_6

	static Class fileSelectorDelegateClass = 0;
	static id VSTGUI_FileSelector_Delegate_Init (id self, SEL _cmd, void* fileSelector);
	static void VSTGUI_FileSelector_Delegate_Dealloc (id self, SEL _cmd);
	static void VSTGUI_FileSelector_Delegate_OpenPanelDidEnd (id self, SEL _cmd, NSOpenPanel* openPanel, int32_t returnCode, void* contextInfo);

	@interface NSObject (VSTGUI_FileSelector_Private)
	-(id)initWithFileSelector:(id)fileSelector;
	@end
	#endif
#endif

namespace VSTGUI {

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CocoaFileSelector : public CNewFileSelector
{
public:
	CocoaFileSelector (CFrame* frame, Style style);
	~CocoaFileSelector ();

	void openPanelDidEnd (NSSavePanel* panel, NSInteger resultCode);
protected:
	static void initClass ();
	
	bool runInternal (CBaseObject* delegate) VSTGUI_OVERRIDE_VMETHOD;
	bool runModalInternal () VSTGUI_OVERRIDE_VMETHOD;
	void cancelInternal () VSTGUI_OVERRIDE_VMETHOD;

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
	#if MAC_COCOA && MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_6
	if (fileSelectorDelegateClass == 0)
	{
		NSMutableString* fileSelectorDelegateClassName = [[[NSMutableString alloc] initWithString:@"VSTGUI_FileSelector_Delegate"] autorelease];
		fileSelectorDelegateClass = generateUniqueClass (fileSelectorDelegateClassName, [NSObject class]);
		VSTGUI_CHECK_YES(class_addMethod (fileSelectorDelegateClass, @selector(initWithFileSelector:), IMP (VSTGUI_FileSelector_Delegate_Init), "@@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (fileSelectorDelegateClass, @selector(dealloc), IMP (VSTGUI_FileSelector_Delegate_Dealloc), "v@:@:"))
		VSTGUI_CHECK_YES(class_addMethod (fileSelectorDelegateClass, @selector(openPanelDidEnd:returnCode:contextInfo:), IMP (VSTGUI_FileSelector_Delegate_OpenPanelDidEnd), "v@:@:@:I:@:"))
		VSTGUI_CHECK_YES(class_addIvar (fileSelectorDelegateClass, "_fileSelector", sizeof (void*), (uint8_t)log2(sizeof(void*)), @encode(void*)))
		objc_registerClassPair (fileSelectorDelegateClass);
	}
	#endif
}

//-----------------------------------------------------------------------------
CocoaFileSelector::CocoaFileSelector (CFrame* frame, Style style)
: CNewFileSelector (frame)
, style (style)
, delegate (0)
{
	savePanel = nil;
	initClass ();
}

//-----------------------------------------------------------------------------
CocoaFileSelector::~CocoaFileSelector ()
{
}

//-----------------------------------------------------------------------------
void CocoaFileSelector::openPanelDidEnd (NSSavePanel* savePanel, NSInteger res)
{
	if (res == NSFileHandlingPanelOKButton)
	{
		if (style == kSelectSaveFile)
		{
			NSURL* url = [savePanel URL];
			const char* utf8Path = url ? [[url path] UTF8String] : 0;
			if (utf8Path)
			{
				UTF8StringBuffer path = String::newWithString (utf8Path);
				result.push_back (path);
			}
		}
		else
		{
			NSOpenPanel* openPanel = (NSOpenPanel*)savePanel;
			NSArray* urls = [openPanel URLs];
			for (NSUInteger i = 0; i < [urls count]; i++)
			{
				NSURL* url = [urls objectAtIndex:i];
				if (url == 0 || [url path] == 0)
					continue;
				const char* utf8Path = [[url path] UTF8String];
				if (utf8Path)
				{
					UTF8StringBuffer path = String::newWithString (utf8Path);
					result.push_back (path);
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
			parentWindow = nsViewFrame ? [(nsViewFrame->getPlatformControl ()) window] : 0;
		}
		#endif
		delegate = _delegate;
	}
	NSOpenPanel* openPanel = nil;
	NSMutableArray* typesArray = nil;
	if (extensions.empty () == false)
	{
		typesArray = [[[NSMutableArray alloc] init] autorelease];
		std::list<CFileExtension>::const_iterator it = extensions.begin ();
		while (it != extensions.end ())
		{
			NSString* uti = 0;
			if ((*it).getUTI ())
				uti = [[NSString stringWithCString: (*it).getUTI () encoding:NSUTF8StringEncoding] retain];
			if (uti == 0 && (*it).getMimeType ())
				uti = (NSString*)UTTypeCreatePreferredIdentifierForTag (kUTTagClassMIMEType, (CFStringRef)[NSString stringWithCString: (*it).getMimeType () encoding:NSUTF8StringEncoding], kUTTypeData);
			if (uti == 0 && (*it).getMacType ())
			{
				NSString* osType = (NSString*)UTCreateStringForOSType (static_cast<OSType> ((*it).getMacType ()));
				if (osType)
				{
					uti = (NSString*)UTTypeCreatePreferredIdentifierForTag (kUTTagClassOSType, (CFStringRef)osType, kUTTypeData);
					[osType release];
				}
			}
			if (uti == 0 && (*it).getExtension ())
				uti = [NSString stringWithUTF8String:(*it).getExtension ()];
			if (uti)
			{
				[typesArray addObject:uti];
				[uti release];
			}
			it++;
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
	if (title && savePanel)
		[savePanel setTitle:[NSString stringWithCString: title encoding:NSUTF8StringEncoding]];
	if (openPanel)
	{
	#if MAC_COCOA
		if (parentWindow)
		{
		#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_6
			setupInitalDir ();
			openPanel.allowedFileTypes = typesArray;
			remember ();
			[openPanel beginSheetModalForWindow:parentWindow completionHandler:^(NSInteger result) {
				openPanelDidEnd (openPanel, result);
				forget ();
			}];
		#else
			id fsdelegate = [[fileSelectorDelegateClass alloc] initWithFileSelector:(id)this];
			[openPanel beginSheetForDirectory:initialPath ? [NSString stringWithCString:initialPath encoding:NSUTF8StringEncoding] : nil file:nil types:typesArray modalForWindow:parentWindow modalDelegate:fsdelegate didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:) contextInfo:nil];
		#endif
		}
		else
	#endif
		{
		#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_6
			setupInitalDir ();
			openPanel.allowedFileTypes = typesArray;
			NSInteger res = [openPanel runModal];
		#else
			NSInteger res = [openPanel runModalForDirectory:initialPath ? [NSString stringWithCString:initialPath encoding:NSUTF8StringEncoding] : nil file:nil types:typesArray];
		#endif
			openPanelDidEnd (openPanel, res);
			return res == NSFileHandlingPanelOKButton;
		}
	}
	else if (savePanel)
	{
	#if MAC_COCOA
		if (parentWindow)
		{
		#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_6
			setupInitalDir ();
			savePanel.allowedFileTypes = typesArray;
			remember ();
			[savePanel beginSheetModalForWindow:parentWindow completionHandler:^(NSInteger result) {
				openPanelDidEnd (savePanel, result);
				forget ();
			}];
		#else
			id fsdelegate = [[fileSelectorDelegateClass alloc] initWithFileSelector:(id)this];
			[savePanel beginSheetForDirectory:initialPath ? [NSString stringWithCString:initialPath encoding:NSUTF8StringEncoding] : nil file:defaultSaveName ? [NSString stringWithCString:defaultSaveName encoding:NSUTF8StringEncoding] : nil modalForWindow:parentWindow modalDelegate:fsdelegate didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:) contextInfo:nil];
		#endif
		}
		else
	#endif
		{
		#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_6
			setupInitalDir ();
			savePanel.allowedFileTypes = typesArray;
			NSInteger res = [savePanel runModal];
		#else
			NSInteger res = [savePanel runModalForDirectory:initialPath ? [NSString stringWithCString:initialPath encoding:NSUTF8StringEncoding]:nil file:defaultSaveName ? [NSString stringWithCString:defaultSaveName encoding:NSUTF8StringEncoding] : nil];
		#endif
			openPanelDidEnd (savePanel, res);
			return res == NSFileHandlingPanelOKButton;
		}
	}
	
	return true;
}

//-----------------------------------------------------------------------------
void CocoaFileSelector::setupInitalDir ()
{
#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_6
	if (initialPath)
	{
		NSURL* dirURL = [NSURL fileURLWithPath:[NSString stringWithCString:initialPath encoding:NSUTF8StringEncoding]];
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
	if (defaultSaveName)
	{
		savePanel.nameFieldStringValue = [NSString stringWithCString:defaultSaveName encoding:NSUTF8StringEncoding];
	}
#endif
}

//-----------------------------------------------------------------------------
bool CocoaFileSelector::runModalInternal ()
{
	return runInternal (0);
}

}

#if MAC_COCOA && MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_6
using VSTGUI::CocoaFileSelector;

//-----------------------------------------------------------------------------
__attribute__((__destructor__)) static void cleanup_VSTGUI_FileSelector ()
{
	if (fileSelectorDelegateClass)
		objc_disposeClassPair (fileSelectorDelegateClass);
}

//-----------------------------------------------------------------------------
id VSTGUI_FileSelector_Delegate_Init (id self, SEL _cmd, void* fileSelector)
{
	__OBJC_SUPER(self)
	self = objc_msgSendSuper (SUPER, @selector(init)); // self = [super init];
	if (self)
	{
		((CocoaFileSelector*)fileSelector)->remember ();
		OBJC_SET_VALUE (self, _fileSelector, (id)fileSelector);
	}
	return self;
}

//-----------------------------------------------------------------------------
void VSTGUI_FileSelector_Delegate_Dealloc (id self, SEL _cmd)
{
	id fileSelector = OBJC_GET_VALUE(self, _fileSelector);
	if (fileSelector)
		((CocoaFileSelector*)fileSelector)->forget ();
	__OBJC_SUPER(self)
	objc_msgSendSuper (SUPER, @selector(dealloc)); // [super dealloc];
}

//-----------------------------------------------------------------------------
void VSTGUI_FileSelector_Delegate_OpenPanelDidEnd (id self, SEL _cmd, NSOpenPanel* openPanel, int32_t returnCode, void* contextInfo)
{
	id fileSelector = OBJC_GET_VALUE(self, _fileSelector);
	if (fileSelector)
	{
		((CocoaFileSelector*)fileSelector)->openPanelDidEnd (openPanel, returnCode);
	}
	[self autorelease];
}
#endif // MAC_COCOA

/// @endcond
