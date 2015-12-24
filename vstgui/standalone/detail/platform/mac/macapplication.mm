//
//  application.mm
//  vstgui
//
//  Created by Arne Scheffler on 20.12.15.
//
//

#import <Cocoa/Cocoa.h>
#import "VSTGUICommand.h"
#import "../../application.h"
#import "../../../iappdelegate.h"
#import "../../../iapplication.h"
#import "../../../../lib/platform/mac/macstring.h"

//------------------------------------------------------------------------
@interface VSTGUIApplicationDelegate : NSObject<NSApplicationDelegate>
@end

using namespace VSTGUI::Standalone;
using VSTGUI::Standalone::Detail::IApplicationPlatformAccess;
using VSTGUI::Standalone::Detail::CommandWithKey;
using CommandWithKeyList = VSTGUI::Standalone::Detail::IApplicationPlatformAccess::CommandWithKeyList;

//------------------------------------------------------------------------
static IApplicationPlatformAccess* getApplicationPlatformAccess ()
{
	return dynamic_cast<IApplicationPlatformAccess*> (&IApplication::instance ());
}

//------------------------------------------------------------------------
static const CommandWithKeyList* getCommandList (const char* group)
{
	for (auto& e : getApplicationPlatformAccess ()->getCommandList ())
	{
		if (e.first == group)
			return &e.second;
	}
	return nullptr;
}

//------------------------------------------------------------------------
static NSString* stringFromUTF8String (const VSTGUI::UTF8String& str)
{
	auto macStr = dynamic_cast<VSTGUI::MacString*> (str.getPlatformString ());
	if (macStr && macStr->getCFString ())
	{
		return (__bridge NSString*)macStr->getCFString();
	}
	return [NSString stringWithUTF8String:str.get ()];
}

//------------------------------------------------------------------------
@implementation VSTGUIApplicationDelegate

//------------------------------------------------------------------------
- (instancetype) init
{
	self =[super init];
	if (self)
	{
		auto app = getApplicationPlatformAccess ();
		app->init ();
	}
	return self;
}

//------------------------------------------------------------------------
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
	if (IApplication::instance ().getDelegate ()->canQuit ())
		return NSTerminateNow;
	return NSTerminateCancel;
}

//------------------------------------------------------------------------
- (IBAction)showAboutDialog:(id)sender
{
	if (IApplication::instance ().getDelegate ()->hasAboutDialog ())
		IApplication::instance ().getDelegate ()->showAboutDialog ();
	else
		[NSApp orderFrontStandardAboutPanel:sender];
}

//------------------------------------------------------------------------
- (IBAction)showPreferenceDialog:(id)sender
{
	IApplication::instance ().getDelegate ()->showPreferenceDialog ();
}

//------------------------------------------------------------------------
- (IBAction)processCommand:(id)sender
{
	VSTGUICommand* command = [sender representedObject];
	if (command)
	{
		auto commandHandler = dynamic_cast<VSTGUI::Standalone::ICommandHandler*> (IApplication::instance ().getDelegate ());
		if (commandHandler)
			commandHandler->handleCommand ([command command]);
	}
}

//------------------------------------------------------------------------
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
	if (menuItem.action == @selector(showPreferenceDialog:))
	{
		if (!IApplication::instance ().getDelegate ()->hasPreferenceDialog ())
		{
			return NO;
		}
	}
	else if (menuItem.action == @selector(showAboutDialog:))
	{
		return YES;
	}
	else if (VSTGUICommand* command = menuItem.representedObject)
	{
		auto commandHandler = dynamic_cast<VSTGUI::Standalone::ICommandHandler*> (IApplication::instance ().getDelegate ());
		if (commandHandler)
			return commandHandler->canHandleCommand ([command command]);
	}
	return NO;
}

//------------------------------------------------------------------------
- (SEL)selectorForCommand:(const CommandWithKey&)command
{
	if (command == Commands::CloseWindow)
		return @selector(performClose:);
	else if (command == Commands::Undo)
		return @selector(undo);
	else if (command == Commands::Redo)
		return @selector(redo);
	else if (command == Commands::Cut)
		return @selector(cut:);
	else if (command == Commands::Copy)
		return @selector(copy:);
	else if (command == Commands::Paste)
		return @selector(paste:);
	return @selector(processCommand:);
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
		item.keyEquivalent = [NSString stringWithCharacters:reinterpret_cast<const unichar*>(&command.defaultKey) length:1];
	}
	VSTGUICommand* representedObject = [VSTGUICommand new];
	representedObject.cmd = command;
	item.representedObject = representedObject;
	return item;
}

//------------------------------------------------------------------------
- (NSMenu*)createAppMenu
{
	NSDictionary<NSString *, id>* dict = [[NSBundle mainBundle] infoDictionary];
	NSString* appName = dict[(@"CFBundleName")];
	NSMenu* menu = [[NSMenu alloc] initWithTitle:appName];
	auto commandList = getCommandList (CommandGroup::Application);
	if (commandList)
	{
		for (auto& command : *commandList)
		{
			if (command == Commands::About)
			{
				[menu addItemWithTitle:[NSString stringWithFormat:@"About %@", appName] action:@selector(showAboutDialog:) keyEquivalent:@""];
				[menu addItem:[NSMenuItem separatorItem]];
			}
			else if (command == Commands::Preferences)
				[menu addItemWithTitle:@"Preferences..." action:@selector(showPreferenceDialog:) keyEquivalent:@","];
			else if (command == Commands::Quit)
			{
				[menu addItem:[NSMenuItem separatorItem]];
				[menu addItemWithTitle:[NSString stringWithFormat:@"Hide %@", appName] action:@selector(hide:) keyEquivalent:@"h"];
				[menu addItemWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@""];
				[menu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];
				[menu addItem:[NSMenuItem separatorItem]];
				[menu addItemWithTitle:[NSString stringWithFormat:@"Quit %@", appName] action:@selector(terminate:) keyEquivalent:@"q"];
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
	[menu addItemWithTitle:@"Minimize" action:@selector(performMiniaturize:) keyEquivalent:@"m"];
	[menu addItemWithTitle:@"Zoom" action:@selector(performZoom:) keyEquivalent:@""];
	NSMenuItem* item = [menu addItemWithTitle:@"Fullscreen" action:@selector(toggleFullScreen:) keyEquivalent:@"f"];
	item.keyEquivalentModifierMask = NSCommandKeyMask | NSControlKeyMask;
	[menu addItem:[NSMenuItem separatorItem]];
	return menu;
}

//------------------------------------------------------------------------
- (void)setupMainMenu
{
	NSMenu* mainMenu = NSApp.mainMenu;
	if (mainMenu == nil)
		mainMenu = [NSMenu new];
	else
		[mainMenu removeAllItems];

	NSMenuItem* appMenuItem = [[NSMenuItem alloc] initWithTitle:@"App" action:nil keyEquivalent:@""];
	appMenuItem.submenu = [self createAppMenu];
	[mainMenu addItem:appMenuItem];


	for (auto& e : getApplicationPlatformAccess ()->getCommandList ())
	{
		if (e.first != CommandGroup::Application)
		{
			NSMenu* menu = [[NSMenu alloc] initWithTitle:[NSString stringWithUTF8String:e.first.data ()]];
			[self fillMenu:menu fromCommandList:e.second];
			NSMenuItem* item = [NSMenuItem new];
			item.submenu = menu;
			[mainMenu addItem:item];
		}
	}

	NSMenuItem* item = [[NSMenuItem alloc] initWithTitle:@"Windows" action:NULL keyEquivalent:@""];
	[mainMenu addItem:item];

	NSApp.windowsMenu = [self createWindowsMenu];
	item.submenu = NSApp.windowsMenu;
	NSApp.mainMenu = mainMenu;
	
}

//------------------------------------------------------------------------
- (void)updateCommands
{
	[self setupMainMenu];
}

//------------------------------------------------------------------------
- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
	auto app = getApplicationPlatformAccess ();
	assert (app);

	app->setQuitFunction ([] () {
		[NSApp terminate:nil];
	});
	
	IApplication::instance ().getDelegate ()->finishLaunching ();
	[self setupMainMenu];
	VSTGUIApplicationDelegate* Self = self;
	app->setOnCommandUpdate ([Self] () {
		[Self updateCommands];
	});
}

@end

//------------------------------------------------------------------------
namespace VSTGUI {
void* gBundleRef = nullptr;
}

//------------------------------------------------------------------------
int main (int argc, const char* _Nonnull *argv)
{
	VSTGUI::gBundleRef = CFBundleGetMainBundle ();
	VSTGUIApplicationDelegate* delegate = [VSTGUIApplicationDelegate new];
	[NSApplication sharedApplication].delegate = delegate;
	return NSApplicationMain (argc, argv);
}
