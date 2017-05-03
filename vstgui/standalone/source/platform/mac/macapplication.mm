// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "../../../include/iappdelegate.h"
#import "../../../include/iapplication.h"
#import "../../application.h"
#import "../../shareduiresources.h"
#import "../../window.h"
#import "VSTGUICommand.h"
#import "macasync.h"
#import "maccommondirectories.h"
#import "macpreference.h"
#import "macutilities.h"
#import "macwindow.h"
#import <Cocoa/Cocoa.h>

#if __has_feature(nullability) == 0
static_assert (false, "Need newer clang compiler!");
#endif

//------------------------------------------------------------------------
@interface VSTGUIApplicationDelegate : NSObject <NSApplicationDelegate>
{
	VSTGUI::Standalone::Platform::Mac::MacPreference prefs;
	VSTGUI::Standalone::Platform::Mac::CommonDirectories commonDirecories;
}
@property NSArray<NSString*>* _Nullable startupOpenFiles;
@property BOOL hasFinishedLaunching;
@property BOOL hasTriggeredSetupMainMenu;
@end

using namespace VSTGUI::Standalone;
using VSTGUI::Standalone::Platform::Mac::IMacWindow;
using VSTGUI::Standalone::Detail::IPlatformApplication;
using VSTGUI::Standalone::Detail::CommandWithKey;
using VSTGUI::Standalone::Detail::IPlatformWindowAccess;
using CommandWithKeyList = VSTGUI::Standalone::Detail::IPlatformApplication::CommandWithKeyList;
using VSTGUI::Standalone::Detail::PlatformCallbacks;

//------------------------------------------------------------------------
static CommandWithKeyList getCommandList (const char* _Nonnull group)
{
	for (auto& e : Detail::getApplicationPlatformAccess ()->getCommandList ())
	{
		if (e.first == group)
			return e.second;
	}
	return {};
}

//------------------------------------------------------------------------
@implementation VSTGUIApplicationDelegate

//------------------------------------------------------------------------
- (instancetype _Nonnull)init
{
	self = [super init];
	if (self)
	{
	}
	return self;
}

//------------------------------------------------------------------------
- (NSApplicationTerminateReply)applicationShouldTerminate:(nonnull NSApplication*)sender
{
	if (Detail::getApplicationPlatformAccess ()->canQuit ())
		return NSTerminateNow;
	return NSTerminateCancel;
}

//------------------------------------------------------------------------
- (IBAction)showAboutDialog:(nullable id)sender
{
	if (IApplication::instance ().getDelegate ().hasAboutDialog ())
		IApplication::instance ().getDelegate ().showAboutDialog ();
	else
		[NSApp orderFrontStandardAboutPanel:sender];
}

//------------------------------------------------------------------------
- (IBAction)showPreferenceDialog:(nullable id)sender
{
	IApplication::instance ().getDelegate ().showPreferenceDialog ();
}

//------------------------------------------------------------------------
- (IBAction)processCommand:(nullable id)sender
{
	if (VSTGUICommand* command = [sender representedObject])
		Detail::getApplicationPlatformAccess ()->handleCommand ([command command]);
}

//------------------------------------------------------------------------
- (void)showHelp:(nullable id)sender
{
}

//------------------------------------------------------------------------
- (BOOL)validateMenuItem:(nonnull NSMenuItem*)menuItem
{
	if (menuItem.action == @selector (showPreferenceDialog:))
	{
		if (!IApplication::instance ().getDelegate ().hasPreferenceDialog ())
		{
			return NO;
		}
		return YES;
	}
	else if (menuItem.action == @selector (showAboutDialog:))
	{
		return YES;
	}
	else if (VSTGUICommand* command = menuItem.representedObject)
	{
		return Detail::getApplicationPlatformAccess ()->canHandleCommand ([command command]);
	}
	return NO;
}

//------------------------------------------------------------------------
- (nonnull SEL)selectorForCommand:(const CommandWithKey&)command
{
	if (command == Commands::CloseWindow)
		return @selector (performClose:);
	else if (command == Commands::Undo)
		return @selector (undo);
	else if (command == Commands::Redo)
		return @selector (redo);
	else if (command == Commands::Cut)
		return @selector (cut:);
	else if (command == Commands::Copy)
		return @selector (copy:);
	else if (command == Commands::Paste)
		return @selector (paste:);
	else if (command == Commands::Delete)
		return @selector (delete:);
	else if (command == Commands::SelectAll)
		return @selector (selectAll:);
	return @selector (processCommand:);
}

//------------------------------------------------------------------------
- (nonnull NSMenuItem*)createMenuItemFromCommand:(const CommandWithKey&)command
{
	if (command.name == CommandName::MenuSeparator)
		return [NSMenuItem separatorItem];

	NSMenuItem* item = [NSMenuItem new];
	item.title = stringFromUTF8String (command.name);
	item.action = [self selectorForCommand:command];
	if (command.defaultKey)
	{
		item.keyEquivalent =
		    [NSString stringWithCharacters:reinterpret_cast<const unichar*> (&command.defaultKey)
		                            length:1];
	}
	VSTGUICommand* representedObject = [VSTGUICommand new];
	representedObject.cmd = command;
	item.representedObject = representedObject;
	return item;
}

//------------------------------------------------------------------------
- (nonnull NSString*)appName
{
	NSDictionary* dict = [[NSBundle mainBundle] infoDictionary];
	return dict[(@"CFBundleName")];
}

//------------------------------------------------------------------------
- (nonnull NSMenu*)createAppMenu
{
	NSString* appName = [self appName];
	NSMenu* menu = [[NSMenu alloc] initWithTitle:appName];

	[menu addItemWithTitle:[NSString stringWithFormat:@"About %@", appName]
	                action:@selector (showAboutDialog:)
	         keyEquivalent:@""];
	[menu addItem:[NSMenuItem separatorItem]];

	[menu addItemWithTitle:@"Preferences..."
	                action:@selector (showPreferenceDialog:)
	         keyEquivalent:@","];

	[menu addItem:[NSMenuItem separatorItem]];

	auto commandList = getCommandList (CommandGroup::Application);
	if (!commandList.empty ())
	{
		for (auto& command : commandList)
		{
			if (command != Commands::About && command != Commands::Preferences &&
			    command != Commands::Quit && command != Commands::Help)
			{
				[menu addItem:[self createMenuItemFromCommand:command]];
			}
		}
	}

	[menu addItem:[NSMenuItem separatorItem]];
	[menu addItemWithTitle:[NSString stringWithFormat:@"Hide %@", appName]
	                action:@selector (hide:)
	         keyEquivalent:@"h"];
	[menu addItemWithTitle:@"Hide Others"
	                action:@selector (hideOtherApplications:)
	         keyEquivalent:@""];
	[menu addItemWithTitle:@"Show All" action:@selector (unhideAllApplications:) keyEquivalent:@""];
	[menu addItem:[NSMenuItem separatorItem]];
	[menu addItemWithTitle:[NSString stringWithFormat:@"Quit %@", appName]
	                action:@selector (terminate:)
	         keyEquivalent:@"q"];

	return menu;
}

//------------------------------------------------------------------------
- (void)fillMenu:(nonnull NSMenu*)menu fromCommandList:(const CommandWithKeyList&)commandList
{
	for (auto& command : commandList)
	{
		[menu addItem:[self createMenuItemFromCommand:command]];
	}
}

//------------------------------------------------------------------------
- (nonnull NSMenu*)createWindowsMenu
{
	NSMenu* menu = [[NSMenu alloc] initWithTitle:@"Window"];
	[menu addItemWithTitle:@"Minimize" action:@selector (performMiniaturize:) keyEquivalent:@"m"];
	[menu addItemWithTitle:@"Zoom" action:@selector (performZoom:) keyEquivalent:@""];
	NSMenuItem* item = [menu addItemWithTitle:@"Fullscreen"
	                                   action:@selector (toggleFullScreen:)
	                            keyEquivalent:@"f"];
	item.keyEquivalentModifierMask = NSCommandKeyMask | NSControlKeyMask;
	[menu addItem:[NSMenuItem separatorItem]];
	[menu addItemWithTitle:@"Bring All to Front"
	                action:@selector (arrangeInFront:)
	         keyEquivalent:@""];
	[menu addItem:[NSMenuItem separatorItem]];
	return menu;
}

//------------------------------------------------------------------------
- (nonnull NSMenu*)createHelpMenu
{
	NSMenu* menu = [[NSMenu alloc] initWithTitle:@"Help"];
	return menu;
}

//------------------------------------------------------------------------
- (void)setupMainMenu
{
	NSMenu* mainMenu = [NSApp mainMenu];
	NSMenuItem* appMenuItem = nil;
	if (mainMenu == nil)
	{
		mainMenu = [NSMenu new];
		[NSApp setMainMenu:mainMenu];

		appMenuItem = [[NSMenuItem alloc] initWithTitle:@"App" action:nil keyEquivalent:@""];
		[mainMenu addItem:appMenuItem];

		NSMenuItem* item =
		    [[NSMenuItem alloc] initWithTitle:@"Window" action:nullptr keyEquivalent:@""];
		NSMenu* windowsMenu = [self createWindowsMenu];
		[NSApp setWindowsMenu:windowsMenu];
		item.submenu = windowsMenu;
		[mainMenu addItem:item];

		item = [[NSMenuItem alloc] initWithTitle:@"Help" action:nullptr keyEquivalent:@""];
		NSMenu* helpMenu = [self createHelpMenu];
		[NSApp setHelpMenu:helpMenu];
		item.submenu = helpMenu;
		[mainMenu addItem:item];
	}
	else
	{
		appMenuItem = [mainMenu itemAtIndex:0];
	}

	appMenuItem.submenu = [self createAppMenu];

	auto commandList = Detail::getApplicationPlatformAccess ()->getCommandList ();
	for (auto& e : commandList)
	{
		if (e.first == CommandGroup::Window)
		{
			NSMenu* windowsMenu = [NSApp windowsMenu];
			for (auto& command : e.second)
			{
				NSString* title = stringFromUTF8String (command.name);
				NSMenuItem* item = [windowsMenu itemWithTitle:title];
				if (!item)
					[windowsMenu addItem:[self createMenuItemFromCommand:command]];
			}
		}
		else if (e.first == CommandGroup::Application)
		{
			for (auto& cmd : e.second)
			{
				if (cmd.name == CommandName::Help)
				{
					NSMenu* helpMenu = [NSApp helpMenu];
					NSMenuItem* item = [helpMenu itemWithTitle:stringFromUTF8String (cmd.name)];
					if (!item)
					{
						item = [self createMenuItemFromCommand:cmd];
						NSString* appName = [self appName];
						item.title = [appName stringByAppendingString:@" Help"];
						[helpMenu addItem:item];
					}
				}
			}
		}
		else
		{
			NSString* title = stringFromUTF8String (e.first);
			NSMenuItem* item = [mainMenu itemWithTitle:title];
			if (!item)
			{
				item = [[NSMenuItem alloc] initWithTitle:title action:nil keyEquivalent:@""];
				[mainMenu addItem:item];
				NSMenu* menu = [[NSMenu alloc] initWithTitle:title];
				item.submenu = menu;
			}
			else
				[item.submenu removeAllItems];
			[self fillMenu:item.submenu fromCommandList:e.second];
		}
	}

	NSMenuItem* editMenu = [mainMenu itemWithTitle:@"Edit"];
	if (editMenu && editMenu.submenu)
	{
		NSMenuItem* showCharacterPanelItem = [editMenu.submenu itemWithTitle:@"Characters"];
		if (showCharacterPanelItem == nil)
		{
			[editMenu.submenu addItem:[NSMenuItem separatorItem]];
			[editMenu.submenu addItemWithTitle:@"Emoji & Symbols"
			                            action:@selector (orderFrontCharacterPalette:)
			                     keyEquivalent:@""];
		}
	}

	// move Windows menu to the end
	NSMenuItem* windowsMenuItem = [mainMenu itemWithTitle:@"Window"];
	[mainMenu removeItem:windowsMenuItem];
	[mainMenu addItem:windowsMenuItem];
	// move Help menu to the end
	NSMenuItem* helpMenuItem = [mainMenu itemWithTitle:@"Help"];
	[mainMenu removeItem:helpMenuItem];
	[mainMenu addItem:helpMenuItem];
}

//------------------------------------------------------------------------
- (void)triggerSetupMainMenu
{
	if (self.hasTriggeredSetupMainMenu)
		return;
	self.hasTriggeredSetupMainMenu = YES;
	Async::perform (Async::Context::Main, [self] () {
		[self setupMainMenu];
		self.hasTriggeredSetupMainMenu = NO;
	});
}

//------------------------------------------------------------------------
- (nonnull NSAlert*)createAlert:(const AlertBoxConfig&)config
{
	NSAlert* alert = [NSAlert new];
	if (!config.headline.empty ())
		alert.messageText = stringFromUTF8String (config.headline);
	if (!config.description.empty ())
		alert.informativeText = stringFromUTF8String (config.description);
	[alert addButtonWithTitle:stringFromUTF8String (config.defaultButton)];
	if (!config.secondButton.empty ())
		[alert addButtonWithTitle:stringFromUTF8String (config.secondButton)];
	if (!config.thirdButton.empty ())
		[alert addButtonWithTitle:stringFromUTF8String (config.thirdButton)];
	return alert;
}

//------------------------------------------------------------------------
- (AlertResult)showAlert:(const AlertBoxConfig&)config
{
	NSAlert* alert = [self createAlert:config];
	NSModalResponse response = [alert runModal];
	if (response == NSAlertSecondButtonReturn)
		return AlertResult::SecondButton;
	if (response == NSAlertThirdButtonReturn)
		return AlertResult::ThirdButton;
	return AlertResult::DefaultButton;
}

//------------------------------------------------------------------------
- (void)showAlertForWindow:(const AlertBoxForWindowConfig&)config
{
	auto platformWindowAccess = VSTGUI::dynamicPtrCast<IPlatformWindowAccess> (config.window);
	if (!platformWindowAccess)
		return;
	auto macWindow = VSTGUI::staticPtrCast<IMacWindow> (platformWindowAccess->getPlatformWindow ());
	if (!macWindow)
		return;
	if (macWindow->isPopup ())
	{
		auto result = [self showAlert:config];
		if (config.callback)
			config.callback (result);
		return;
	}

	auto callback = config.callback;

	NSAlert* alert = [self createAlert:config];
	[alert beginSheetModalForWindow:macWindow->getNSWindow ()
	              completionHandler:^(NSModalResponse returnCode) {
		            if (callback)
		            {
			            AlertResult result = AlertResult::Error;
			            if (returnCode == NSAlertFirstButtonReturn)
				            result = AlertResult::DefaultButton;
			            else if (returnCode == NSAlertSecondButtonReturn)
				            result = AlertResult::SecondButton;
			            else if (returnCode == NSAlertThirdButtonReturn)
				            result = AlertResult::ThirdButton;
			            callback (result);
		            }
		          }];
}

//------------------------------------------------------------------------
- (BOOL)verifyInfoPlistEntries
{
	NSDictionary* dict = [[NSBundle mainBundle] infoDictionary];
	const auto& appInfo = IApplication::instance ().getDelegate ().getInfo ();
	NSString* infoPlistString = dict[(@"CFBundleName")];
	if (![stringFromUTF8String (appInfo.name) isEqualToString:infoPlistString])
	{
		NSLog (@"Warning: CFBundleName is not equal to Application::Info::name");
	}
	infoPlistString = dict[(@"CFBundleShortVersionString")];
	if (![stringFromUTF8String (appInfo.version) isEqualToString:infoPlistString])
	{
		NSLog (@"Warning: CFBundleShortVersionString is not equal to Application::Info::version");
	}
	infoPlistString = dict[(@"CFBundleIdentifier")];
	if (![stringFromUTF8String (appInfo.uri) isEqualToString:infoPlistString])
	{
		NSLog (@"Warning: CFBundleIdentifier is not equal to Application::Info::uri");
	}
	return YES;
}

//------------------------------------------------------------------------
- (void)applicationDidFinishLaunching:(nonnull NSNotification*)notification
{
	if ([self verifyInfoPlistEntries] == NO)
	{
		[NSApp terminate:nil];
		return;
	}

	// disable tab support in macOS 10.12 and above until we support it correctly
	if (auto m = [[NSWindow class] methodForSelector:@selector(setAllowsAutomaticWindowTabbing:)])
	{
		m ([NSWindow class], @selector(setAllowsAutomaticWindowTabbing:), NO);
	}

	IApplication::CommandLineArguments cmdArgs;
	NSArray* args = [[NSProcessInfo processInfo] arguments];
	cmdArgs.reserve ([args count]);
	for (NSString* str in args)
	{
		cmdArgs.emplace_back ([str UTF8String]);
	}

	VSTGUIApplicationDelegate* Self = self;
	PlatformCallbacks callbacks;
	callbacks.quit = [] () {
		[NSApp performSelector:@selector (terminate:) withObject:nil afterDelay:0];
	};
	callbacks.onCommandUpdate = [Self] () { [Self triggerSetupMainMenu]; };
	callbacks.showAlert = [Self] (const AlertBoxConfig& config) { return [Self showAlert:config]; };
	callbacks.showAlertForWindow = [Self] (const AlertBoxForWindowConfig& config) {
		return [Self showAlertForWindow:config];
	};
	auto app = Detail::getApplicationPlatformAccess ();
	vstgui_assert (app);
	[self setupMainMenu];
	app->init ({prefs, commonDirecories, std::move (cmdArgs), std::move (callbacks)});
	self.hasFinishedLaunching = YES;
	if (self.startupOpenFiles)
	{
		[self openFilesInternal:self.startupOpenFiles];
		self.startupOpenFiles = nil;
	}
}

//------------------------------------------------------------------------
- (void)applicationWillTerminate:(nonnull NSNotification*)notification
{
	IApplication::instance ().getDelegate ().onQuit ();
	for (NSWindow* window in [NSApp windows])
	{
		[window close];
	}
	Detail::cleanupSharedUIResources ();
	Async::waitAllTasksDone ();
}

//------------------------------------------------------------------------
- (BOOL)openFilesInternal:(nonnull NSArray<NSString*>*)filenames
{
	std::vector<VSTGUI::UTF8String> paths;
	paths.reserve (filenames.count);
	for (NSString* filename in filenames)
	{
		paths.emplace_back ([filename UTF8String]);
	}
	return IApplication::instance ().getDelegate ().openFiles (paths) ? YES : NO;
}

//------------------------------------------------------------------------
- (void)application:(nonnull NSApplication*)sender openFiles:(nonnull NSArray<NSString*>*)filenames
{
	if (!self.hasFinishedLaunching)
	{
		self.startupOpenFiles = filenames;
		return;
	}
	BOOL result = [self openFilesInternal:filenames];
	[sender replyToOpenOrPrint:result ? NSApplicationDelegateReplySuccess :
	                                    NSApplicationDelegateReplyFailure];
}

@end

//------------------------------------------------------------------------
namespace VSTGUI {
void* _Nullable gBundleRef = nullptr;
}

//------------------------------------------------------------------------
int main (int argc, const char* _Nonnull* _Nonnull argv)
{
	VSTGUI::gBundleRef = CFBundleGetMainBundle ();
	VSTGUIApplicationDelegate* delegate = [VSTGUIApplicationDelegate new];
	[NSApplication sharedApplication].delegate = delegate;
	return NSApplicationMain (argc, argv);
}
