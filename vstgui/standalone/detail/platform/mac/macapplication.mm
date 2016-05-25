#import "../../../iappdelegate.h"
#import "../../../iapplication.h"
#import "../../application.h"
#import "../../shareduiresources.h"
#import "../../window.h"
#import "VSTGUICommand.h"
#import "macpreference.h"
#import "macutilities.h"
#import "macwindow.h"
#import <Cocoa/Cocoa.h>

//------------------------------------------------------------------------
@interface VSTGUIApplicationDelegate : NSObject <NSApplicationDelegate>
{
	VSTGUI::Standalone::Platform::Mac::MacPreference prefs;
}
@end

using namespace VSTGUI::Standalone;
using VSTGUI::Standalone::Platform::Mac::IMacWindow;
using VSTGUI::Standalone::Detail::IApplicationPlatformAccess;
using VSTGUI::Standalone::Detail::CommandWithKey;
using VSTGUI::Standalone::Detail::IPlatformWindowAccess;
using CommandWithKeyList =
    VSTGUI::Standalone::Detail::IApplicationPlatformAccess::CommandWithKeyList;
using VSTGUI::Standalone::Detail::PlatformCallbacks;

//------------------------------------------------------------------------
static const CommandWithKeyList* getCommandList (const char* group)
{
	for (auto& e : Detail::getApplicationPlatformAccess ()->getCommandList ())
	{
		if (e.first == group)
			return &e.second;
	}
	return nullptr;
}

//------------------------------------------------------------------------
@implementation VSTGUIApplicationDelegate

//------------------------------------------------------------------------
- (instancetype)init
{
	self = [super init];
	if (self)
	{
	}
	return self;
}

//------------------------------------------------------------------------
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender
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
	VSTGUICommand* command = [sender representedObject];
	if (command)
	{
		if (auto commandHandler =
		        IApplication::instance ().getDelegate ().dynamicCast<ICommandHandler> ())
			commandHandler->handleCommand ([command command]);
	}
}

//------------------------------------------------------------------------
- (BOOL)validateMenuItem:(NSMenuItem*)menuItem
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
		if (auto commandHandler =
		        IApplication::instance ().getDelegate ().dynamicCast<ICommandHandler> ())
			return commandHandler->canHandleCommand ([command command]);
	}
	return NO;
}

//------------------------------------------------------------------------
- (SEL)selectorForCommand:(const CommandWithKey&)command
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
	else if (command == Commands::SelectAll)
		return @selector (selectAll:);
	return @selector (processCommand:);
}

//------------------------------------------------------------------------
- (NSMenuItem*)createMenuItemFromCommand:(const CommandWithKey&)command
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
- (NSMenu*)createAppMenu
{
	NSDictionary* dict = [[NSBundle mainBundle] infoDictionary];
	NSString* appName = dict[(@"CFBundleName")];
	NSMenu* menu = [[NSMenu alloc] initWithTitle:appName];
	auto commandList = getCommandList (CommandGroup::Application);
	if (commandList)
	{
		for (auto& command : *commandList)
		{
			if (command == Commands::About)
			{
				[menu addItemWithTitle:[NSString stringWithFormat:@"About %@", appName]
				                action:@selector (showAboutDialog:)
				         keyEquivalent:@""];
				[menu addItem:[NSMenuItem separatorItem]];
			}
			else if (command == Commands::Preferences)
				[menu addItemWithTitle:@"Preferences..."
				                action:@selector (showPreferenceDialog:)
				         keyEquivalent:@","];
			else if (command == Commands::Quit)
			{
				[menu addItem:[NSMenuItem separatorItem]];
				[menu addItemWithTitle:[NSString stringWithFormat:@"Hide %@", appName]
				                action:@selector (hide:)
				         keyEquivalent:@"h"];
				[menu addItemWithTitle:@"Hide Others"
				                action:@selector (hideOtherApplications:)
				         keyEquivalent:@""];
				[menu addItemWithTitle:@"Show All"
				                action:@selector (unhideAllApplications:)
				         keyEquivalent:@""];
				[menu addItem:[NSMenuItem separatorItem]];
				[menu addItemWithTitle:[NSString stringWithFormat:@"Quit %@", appName]
				                action:@selector (terminate:)
				         keyEquivalent:@"q"];
			}
			else
			{
				[menu addItem:[self createMenuItemFromCommand:command]];
			}
		}
	}
	return menu;
}

//------------------------------------------------------------------------
- (void)fillMenu:(NSMenu*)menu fromCommandList:(const CommandWithKeyList&)commandList
{
	for (auto& command : commandList)
	{
		[menu addItem:[self createMenuItemFromCommand:command]];
	}
}

//------------------------------------------------------------------------
- (NSMenu*)createWindowsMenu
{
	NSMenu* menu = [[NSMenu alloc] initWithTitle:@"Windows"];
	[menu addItemWithTitle:@"Minimize" action:@selector (performMiniaturize:) keyEquivalent:@"m"];
	[menu addItemWithTitle:@"Zoom" action:@selector (performZoom:) keyEquivalent:@""];
	NSMenuItem* item = [menu addItemWithTitle:@"Fullscreen"
	                                   action:@selector (toggleFullScreen:)
	                            keyEquivalent:@"f"];
	item.keyEquivalentModifierMask = NSCommandKeyMask | NSControlKeyMask;
	[menu addItem:[NSMenuItem separatorItem]];
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
		    [[NSMenuItem alloc] initWithTitle:@"Windows" action:nullptr keyEquivalent:@""];
		NSMenu* windowsMenu = [self createWindowsMenu];
		[NSApp setWindowsMenu:windowsMenu];
		item.submenu = windowsMenu;
		[mainMenu addItem:item];
	}
	else
	{
		appMenuItem = [mainMenu itemAtIndex:0];
	}

	appMenuItem.submenu = [self createAppMenu];

	auto& commandList = Detail::getApplicationPlatformAccess ()->getCommandList ();
	for (auto& e : commandList)
	{
		if (e.first == CommandGroup::Windows)
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
		else if (e.first != CommandGroup::Application)
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
	NSMenuItem* windowsMenuItem = [mainMenu itemWithTitle:@"Windows"];
	[mainMenu removeItem:windowsMenuItem];
	[mainMenu addItem:windowsMenuItem];
}

//------------------------------------------------------------------------
- (NSAlert*)createAlert:(const AlertBoxConfig&)config
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
		return AlertResult::secondButton;
	if (response == NSAlertThirdButtonReturn)
		return AlertResult::thirdButton;
	return AlertResult::defaultButton;
}

//------------------------------------------------------------------------
- (void)showAlertForWindow:(const AlertBoxForWindowConfig&)config
{
	auto platformWindowAccess = config.window->dynamicCast<IPlatformWindowAccess> ();
	if (!platformWindowAccess)
		return;
	auto platformWindow = platformWindowAccess->getPlatformWindow ();
	if (!platformWindow)
		return;
	auto macWindow = platformWindow->dynamicCast<IMacWindow> ();
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
			            AlertResult result = AlertResult::error;
			            if (returnCode == NSAlertFirstButtonReturn)
				            result = AlertResult::defaultButton;
			            else if (returnCode == NSAlertSecondButtonReturn)
				            result = AlertResult::secondButton;
			            else if (returnCode == NSAlertThirdButtonReturn)
				            result = AlertResult::thirdButton;
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
		NSLog (@"CFBundleName is not equal to Application::Info::name");
		return NO;
	}
	infoPlistString = dict[(@"CFBundleShortVersionString")];
	if (![stringFromUTF8String (appInfo.version) isEqualToString:infoPlistString])
	{
		NSLog (@"CFBundleShortVersionString is not equal to Application::Info::version");
		return NO;
	}
	infoPlistString = dict[(@"CFBundleIdentifier")];
	if (![stringFromUTF8String (appInfo.uri) isEqualToString:infoPlistString])
	{
		NSLog (@"CFBundleIdentifier is not equal to Application::Info::uri");
		return NO;
	}
	return YES;
}

//------------------------------------------------------------------------
- (void)applicationDidFinishLaunching:(NSNotification*)notification
{
	if ([self verifyInfoPlistEntries] == NO)
	{
		[NSApp terminate:nil];
		return;
	}

	IApplication::CommandLineArguments cmdArgs;
	NSArray* args = [[NSProcessInfo processInfo] arguments];
	for (NSString* str in args)
	{
		cmdArgs.push_back ([str UTF8String]);
	}

	VSTGUIApplicationDelegate* Self = self;
	PlatformCallbacks callbacks;
	callbacks.quit = [] () {
		[NSApp performSelector:@selector (terminate:) withObject:nil afterDelay:0];
	};
	callbacks.onCommandUpdate = [Self] () { [Self setupMainMenu]; };
	callbacks.showAlert = [Self] (const AlertBoxConfig& config) { return [Self showAlert:config]; };
	callbacks.showAlertForWindow = [Self] (const AlertBoxForWindowConfig& config) {
		return [Self showAlertForWindow:config];
	};
	auto app = Detail::getApplicationPlatformAccess ();
	vstgui_assert (app);
	app->init (prefs, std::move (cmdArgs), std::move (callbacks));
	[self setupMainMenu];
}

//------------------------------------------------------------------------
- (void)applicationWillTerminate:(NSNotification*)notification
{
	IApplication::instance ().getDelegate ().onQuit ();
	Detail::cleanupSharedUIResources ();
}

@end

//------------------------------------------------------------------------
namespace VSTGUI {
void* gBundleRef = nullptr;
}

//------------------------------------------------------------------------
int main (int argc, const char** argv)
{
	VSTGUI::gBundleRef = CFBundleGetMainBundle ();
	VSTGUIApplicationDelegate* delegate = [VSTGUIApplicationDelegate new];
	[NSApplication sharedApplication].delegate = delegate;
	return NSApplicationMain (argc, argv);
}
