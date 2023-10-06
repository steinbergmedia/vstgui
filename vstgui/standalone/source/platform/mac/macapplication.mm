// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "../../../../lib/platform/mac/cocoa/cocoahelpers.h"
#import "../../../../lib/platform/mac/macfactory.h"
#import "../../../../lib/vstguiinit.h"
#import "../../../include/iappdelegate.h"
#import "../../../include/iapplication.h"
#import "../../application.h"
#import "../../genericalertbox.h"
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

#define VSTGUI_STANDALONE_USE_GENERIC_ALERTBOX_ON_MACOS 0

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
	else if (menuItem.action == @selector (visualizeRedrawAreas:) || menuItem.action == @selector (useAsynchronousCALayerDrawing:))
	{
		return YES;
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

	[menu addItemWithTitle:[NSLocalizedString (@"About ", "Menu Item")
	                           stringByAppendingString:appName]
	                action:@selector (showAboutDialog:)
	         keyEquivalent:@""];
	[menu addItem:[NSMenuItem separatorItem]];

	[menu addItemWithTitle:NSLocalizedString (@"Preferences...", "Menu Item")
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
	[menu
	    addItemWithTitle:[NSLocalizedString (@"Hide ", "Menu Item") stringByAppendingString:appName]
	              action:@selector (hide:)
	       keyEquivalent:@"h"];
	[menu addItemWithTitle:NSLocalizedString (@"Hide Others", "Menu Item")
	                action:@selector (hideOtherApplications:)
	         keyEquivalent:@""];
	[menu addItemWithTitle:NSLocalizedString (@"Show All", "Menu Item")
	                action:@selector (unhideAllApplications:)
	         keyEquivalent:@""];
	[menu addItem:[NSMenuItem separatorItem]];
	[menu
	    addItemWithTitle:[NSLocalizedString (@"Quit ", "Menu Item") stringByAppendingString:appName]
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
	NSMenu* menu =
	    [[NSMenu alloc] initWithTitle:NSLocalizedString (@"Window", "Window Menu Title")];
	[menu addItemWithTitle:NSLocalizedString (@"Minimize", "Menu Item in Window Menu")
	                action:@selector (performMiniaturize:)
	         keyEquivalent:@"m"];
	[menu addItemWithTitle:NSLocalizedString (@"Zoom", "Menu Item in Window Menu")
	                action:@selector (performZoom:)
	         keyEquivalent:@""];
	NSMenuItem* item =
	    [menu addItemWithTitle:NSLocalizedString (@"Fullscreen", "Menu Item in Window Menu")
	                    action:@selector (toggleFullScreen:)
	             keyEquivalent:@"f"];
	item.keyEquivalentModifierMask =
	    MacEventModifier::CommandKeyMask | MacEventModifier::ControlKeyMask;
	[menu addItem:[NSMenuItem separatorItem]];

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12 && __clang_major__ >= 9
	if (@available (macOS 10.12, *))
	{
		item = [menu
		    addItemWithTitle:NSLocalizedString (@"Show Previous Tab", "Menu Item in Window Menu")
		              action:@selector (selectPreviousTab:)
		       keyEquivalent:@"\t"];
		item.keyEquivalentModifierMask =
		    MacEventModifier::ShiftKeyMask | MacEventModifier::ControlKeyMask;
		item =
		    [menu addItemWithTitle:NSLocalizedString (@"Show Next Tab", "Menu Item in Window Menu")
		                    action:@selector (selectNextTab:)
		             keyEquivalent:@"\t"];
		item.keyEquivalentModifierMask = MacEventModifier::ControlKeyMask;
		[menu addItemWithTitle:NSLocalizedString (@"Move Tab To New Window",
		                                          "Menu Item in Window Menu")
		                action:@selector (moveTabToNewWindow:)
		         keyEquivalent:@""];
		[menu addItemWithTitle:NSLocalizedString (@"Merge All Windows", "Menu Item in Window Menu")
		                action:@selector (mergeAllWindows:)
		         keyEquivalent:@""];
		item =
		    [menu addItemWithTitle:NSLocalizedString (@"Show All Tabs", "Menu Item in Window Menu")
		                    action:@selector (toggleTabOverview:)
		             keyEquivalent:@"\\"];
		item.keyEquivalentModifierMask =
		    MacEventModifier::ShiftKeyMask | MacEventModifier::CommandKeyMask;

		[menu addItem:[NSMenuItem separatorItem]];
	}
#endif
	[menu addItemWithTitle:NSLocalizedString (@"Bring All to Front", "Menu Item in Window Menu")
	                action:@selector (arrangeInFront:)
	         keyEquivalent:@""];
	[menu addItem:[NSMenuItem separatorItem]];
	return menu;
}

//------------------------------------------------------------------------
- (nonnull NSMenu*)createHelpMenu
{
	NSMenu* menu = [[NSMenu alloc] initWithTitle:NSLocalizedString (@"Help", "Help Menu Title")];
	return menu;
}

//------------------------------------------------------------------------
- (void)setupMainMenu
{
	NSMenu* mainMenu = [NSApp mainMenu];
	NSMenuItem* appMenuItem = nil;
	if (mainMenu == nil || mainMenu.itemArray.count == 1)
	{
		if (mainMenu == nil)
		{
			mainMenu = [NSMenu new];
			[NSApp setMainMenu:mainMenu];
			appMenuItem = [[NSMenuItem alloc] initWithTitle:@"App" action:nil keyEquivalent:@""];
			[mainMenu addItem:appMenuItem];
		}

		NSMenuItem* item =
		    [[NSMenuItem alloc] initWithTitle:NSLocalizedString (@"Window", "Menu Name")
		                               action:nullptr
		                        keyEquivalent:@""];
		NSMenu* windowsMenu = [self createWindowsMenu];
		[NSApp setWindowsMenu:windowsMenu];
		item.submenu = windowsMenu;
		[mainMenu addItem:item];

		item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString (@"Help", "Menu Name")
		                                  action:nullptr
		                           keyEquivalent:@""];
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
						item.title = [appName
						    stringByAppendingString:NSLocalizedString (@" Help", "Menu Item")];
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

	NSMenuItem* editMenu = [mainMenu itemWithTitle:NSLocalizedString (@"Edit", "Menu Name")];
	if (editMenu && editMenu.submenu)
	{
		NSMenuItem* showCharacterPanelItem =
		    [editMenu.submenu itemWithTitle:NSLocalizedString (@"Characters", "Menu Item")];
		if (showCharacterPanelItem == nil)
		{
			[editMenu.submenu addItem:[NSMenuItem separatorItem]];
			[editMenu.submenu
			    addItemWithTitle:NSLocalizedString (@"Emoji & Symbols", "Menu Item in Edit Menu")
			              action:@selector (orderFrontCharacterPalette:)
			       keyEquivalent:@""];
		}
	}

	NSMenuItem* debugMenu = [mainMenu itemWithTitle:@"Debug"];
	if (debugMenu && debugMenu.submenu && [debugMenu.submenu itemWithTitle:@"Color Panel"] == nil)
	{
		[debugMenu.submenu addItem:[NSMenuItem separatorItem]];
		[debugMenu.submenu addItemWithTitle:@"Color Panel"
		                             action:@selector (orderFrontColorPanel:)
		                      keyEquivalent:@""];
		[debugMenu.submenu addItem:[NSMenuItem separatorItem]];
		[debugMenu.submenu addItemWithTitle:@"Use Asynchronous CALayer Drawing"
		                             action:@selector (useAsynchronousCALayerDrawing:)
		                      keyEquivalent:@""];
		[debugMenu.submenu addItemWithTitle:@"Visualize Redraw Areas"
		                             action:@selector (visualizeRedrawAreas:)
		                      keyEquivalent:@""];
		[self updateDebugMenuItems];
	}

	// move Windows menu to the end
	if (auto* windowsMenuItem = [mainMenu itemWithTitle:NSLocalizedString (@"Window", "Menu Name")])
	{
		[mainMenu removeItem:windowsMenuItem];
		[mainMenu addItem:windowsMenuItem];
	}
	// move Help menu to the end
	if (auto* helpMenuItem = [mainMenu itemWithTitle:NSLocalizedString (@"Help", "Menu Name")])
	{
		[mainMenu removeItem:helpMenuItem];
		[mainMenu addItem:helpMenuItem];
	}
}

//------------------------------------------------------------------------
- (void)triggerSetupMainMenu
{
	if (self.hasTriggeredSetupMainMenu)
		return;
	self.hasTriggeredSetupMainMenu = YES;
	Async::schedule (Async::mainQueue (), [self] () {
		[self setupMainMenu];
		self.hasTriggeredSetupMainMenu = NO;
	});
}

//------------------------------------------------------------------------
- (void)visualizeRedrawAreas:(id)sender
{
	auto state = VSTGUI::getPlatformFactory ().asMacFactory ()->enableVisualizeRedrawAreas ();
	VSTGUI::getPlatformFactory ().asMacFactory ()->enableVisualizeRedrawAreas (!state);
	[self updateDebugMenuItems];
}

//------------------------------------------------------------------------
- (void)useAsynchronousCALayerDrawing:(id)sender
{
	auto state = VSTGUI::getPlatformFactory ().asMacFactory ()->getUseAsynchronousLayerDrawing ();
	VSTGUI::getPlatformFactory ().asMacFactory ()->setUseAsynchronousLayerDrawing (!state);
	[self updateDebugMenuItems];
}

//------------------------------------------------------------------------
- (void)updateDebugMenuItems
{
	NSMenuItem* debugMenu = [NSApp.mainMenu itemWithTitle:@"Debug"];
	if (debugMenu && debugMenu.submenu)
	{
		if (auto item = [debugMenu.submenu itemWithTitle:@"Visualize Redraw Areas"])
		{
			auto state =
			    VSTGUI::getPlatformFactory ().asMacFactory ()->enableVisualizeRedrawAreas ();
			item.state = state ? NSControlStateValueOn : NSControlStateValueOff;
		}
		if (auto item = [debugMenu.submenu itemWithTitle:@"Use Asynchronous CALayer Drawing"])
		{
			auto state =
			    VSTGUI::getPlatformFactory ().asMacFactory ()->getUseAsynchronousLayerDrawing ();
			item.state = state ? NSControlStateValueOn : NSControlStateValueOff;
		}
	}
}

#if !VSTGUI_STANDALONE_USE_GENERIC_ALERTBOX_ON_MACOS
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
#endif

//------------------------------------------------------------------------
- (AlertResult)showAlert:(const AlertBoxConfig&)config
{
#if VSTGUI_STANDALONE_USE_GENERIC_ALERTBOX_ON_MACOS
	AlertResult result = AlertResult::Error;
	auto alertWindow = Detail::createAlertBox (config, [&] (AlertResult r) {
		result = r;
		[NSApp abortModal];
	});
	auto platformAlertWindow = VSTGUI::dynamicPtrCast<IPlatformWindowAccess> (alertWindow);
	assert (platformAlertWindow);
	auto macAlertWindow =
	    VSTGUI::staticPtrCast<IMacWindow> (platformAlertWindow->getPlatformWindow ());
	assert (macAlertWindow);
	auto nsWindow = macAlertWindow->getNSWindow ();
	macAlertWindow->center ();
	macAlertWindow->show ();
	[NSApp runModalForWindow:nsWindow];
	return result;
#else
	NSAlert* alert = [self createAlert:config];
	NSModalResponse response = [alert runModal];
	if (response == NSAlertSecondButtonReturn)
		return AlertResult::SecondButton;
	if (response == NSAlertThirdButtonReturn)
		return AlertResult::ThirdButton;
	return AlertResult::DefaultButton;
#endif
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

	auto callback = std::move (config.callback);

#if VSTGUI_STANDALONE_USE_GENERIC_ALERTBOX_ON_MACOS
	struct Params
	{
		NSWindow* sheet {nullptr};
		NSWindow* parent {nullptr};
	};

	auto params = std::make_shared<Params> ();
	params->parent = macWindow->getNSWindow ();
	auto parentWindow = config.window;
	auto alertWindow = Detail::createAlertBox (config, [=] (AlertResult r) {
		if (callback)
			callback (r);
		[params->parent endSheet:params->sheet];
	});
	auto platformAlertWindow = VSTGUI::dynamicPtrCast<IPlatformWindowAccess> (alertWindow);
	assert (platformAlertWindow);
	auto macAlertWindow =
	    VSTGUI::staticPtrCast<IMacWindow> (platformAlertWindow->getPlatformWindow ());
	assert (macAlertWindow);
	params->sheet = macAlertWindow->getNSWindow ();

	[params->parent beginSheet:params->sheet completionHandler:^(NSModalResponse returnCode) {}];

#else

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
#endif
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
	IPlatformApplication::OpenFilesList openFilesList;
	if (auto filenames = self.startupOpenFiles)
	{
		openFilesList.reserve (filenames.count);
		for (NSString* filename in filenames)
		{
			openFilesList.emplace_back ([filename UTF8String]);
		}
		self.startupOpenFiles = nil;
	}
	self.hasFinishedLaunching = YES;
	app->init ({prefs, commonDirecories, std::move (cmdArgs), std::move (callbacks),
	            std::move (openFilesList)});
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
	VSTGUI::exit ();
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
int main (int argc, const char* _Nonnull* _Nonnull argv)
{
	VSTGUI::init (CFBundleGetMainBundle ());
	VSTGUIApplicationDelegate* delegate = [VSTGUIApplicationDelegate new];
	[NSApplication sharedApplication].delegate = delegate;
	return NSApplicationMain (argc, argv);
}
