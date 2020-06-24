// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "nsviewoptionmenu.h"

#if MAC_COCOA

#import "cocoahelpers.h"
#import "nsviewframe.h"
#import "../../../cbitmap.h"
#import "../../../cframe.h"
#import "../../../controls/coptionmenu.h"
#import "../cgbitmap.h"
#import "../macstring.h"

@interface NSObject (VSTGUI_NSMenu_Private)
-(id)initWithOptionMenu:(id)menu;
@end

namespace VSTGUI {

#if DEBUG
static int32_t menuClassCount = 0;
#endif

static Class menuClass = nullptr;

#ifndef MAC_OS_X_VERSION_10_14
#define MAC_OS_X_VERSION_10_14 101400
#endif

#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_14
static constexpr auto NSControlStateValueOn = NSOnState;
static constexpr auto NSControlStateValueOff = NSOffState;
#endif

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
struct VSTGUI_NSMenu_Var
{
	COptionMenu* _optionMenu;
	COptionMenu* _selectedMenu;
	int32_t _selectedItem;
};

//------------------------------------------------------------------------------------
static id VSTGUI_NSMenu_Init (id self, SEL _cmd, void* _menu)
{
#if DEBUG
	menuClassCount++;
#endif
	__OBJC_SUPER(self)
	self = SuperInit (SUPER, @selector(init));
	if (self)
	{
		NSMenu* nsMenu = (NSMenu*)self;
		COptionMenu* menu = (COptionMenu*)_menu;
		VSTGUI_NSMenu_Var* var = new VSTGUI_NSMenu_Var;
		var->_optionMenu = menu;
		var->_selectedItem = 0;
		var->_selectedMenu = nullptr;
		OBJC_SET_VALUE(self, _private, var);

		int32_t index = -1;
		bool multipleCheck = menu->isMultipleCheckStyle ();
		CConstMenuItemIterator it = menu->getItems ()->begin ();
		while (it != menu->getItems ()->end ())
		{
			CMenuItem* item = (*it);
			it++;
			index++;
			NSMenuItem* nsItem = nullptr;
			NSMutableString* itemTitle = [[[NSMutableString alloc] initWithString:fromUTF8String<NSString*> (item->getTitle ())] autorelease];
			if (menu->getPrefixNumbers ())
			{
				NSString* prefixString = nullptr;
				switch (menu->getPrefixNumbers ())
				{
					case 2:	prefixString = [NSString stringWithFormat:@"%1d ", index+1]; break;
					case 3: prefixString = [NSString stringWithFormat:@"%02d ", index+1]; break;
					case 4: prefixString = [NSString stringWithFormat:@"%03d ", index+1]; break;
				}
				[itemTitle insertString:prefixString atIndex:0];
			}
			if (item->getSubmenu ())
			{
				nsItem = [nsMenu addItemWithTitle:itemTitle action:nil keyEquivalent:@""];
				NSMenu* subMenu = [[[menuClass alloc] initWithOptionMenu:(id)item->getSubmenu ()] autorelease];
				[nsMenu setSubmenu: subMenu forItem:nsItem];
				if (multipleCheck && item->isChecked ())
					[nsItem setState:NSControlStateValueOn];
				else
					[nsItem setState:NSControlStateValueOff];
			}
			else if (item->isSeparator ())
			{
				[nsMenu addItem:[NSMenuItem separatorItem]];
			}
			else
			{
				nsItem = [nsMenu addItemWithTitle:itemTitle action:@selector(menuItemSelected:) keyEquivalent:@""];
				if (item->isTitle ())
					[nsItem setIndentationLevel:1];
				[nsItem setTarget:nsMenu];
				[nsItem setTag: index];
				if (multipleCheck && item->isChecked ())
					[nsItem setState:NSControlStateValueOn];
				else
					[nsItem setState:NSControlStateValueOff];
				NSString* keyEquivalent = nil;
				if (!item->getKeycode ().empty ())
				{
					keyEquivalent = fromUTF8String<NSString*> (item->getKeycode ());
				}
				else if (item->getVirtualKeyCode ())
				{
					keyEquivalent = GetVirtualKeyCodeString (item->getVirtualKeyCode ());
				}
				if (keyEquivalent)
				{
					[nsItem setKeyEquivalent:keyEquivalent];
					uint32_t keyModifiers = 0;
					if (item->getKeyModifiers () & kControl)
						keyModifiers |= MacEventModifier::CommandKeyMask;
					if (item->getKeyModifiers () & kShift)
						keyModifiers |= MacEventModifier::ShiftKeyMask;
					if (item->getKeyModifiers () & kAlt)
						keyModifiers |= MacEventModifier::AlternateKeyMask;
					if (item->getKeyModifiers () & kApple)
						keyModifiers |= MacEventModifier::ControlKeyMask;
					[nsItem setKeyEquivalentModifierMask:keyModifiers];
				}
			}
			if (nsItem && item->getIcon ())
			{
				IPlatformBitmap* platformBitmap = item->getIcon ()->getPlatformBitmap ();
				CGBitmap* cgBitmap = platformBitmap ? dynamic_cast<CGBitmap*> (platformBitmap) : nullptr;
				CGImageRef image = cgBitmap ? cgBitmap->getCGImage () : nullptr;
				if (image)
				{
					NSImage* nsImage = imageFromCGImageRef (image);
					if (nsImage)
					{
						[nsItem setImage:nsImage];
						[nsImage release];
					}
				}
			}
		}
	}
	return self;
}

//-----------------------------------------------------------------------------
static void VSTGUI_NSMenu_Dealloc (id self, SEL _cmd)
{
#if DEBUG
	menuClassCount--;
#endif
	VSTGUI_NSMenu_Var* var = (VSTGUI_NSMenu_Var*)OBJC_GET_VALUE(self, _private);
	if (var)
		delete var;
	__OBJC_SUPER(self)
	SuperDealloc (SUPER, @selector(dealloc)); // [super dealloc];
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSMenu_ValidateMenuItem (id self, SEL _cmd, id item)
{
	VSTGUI_NSMenu_Var* var = (VSTGUI_NSMenu_Var*)OBJC_GET_VALUE(self, _private);
	if (var && var->_optionMenu)
	{
		CMenuItem* menuItem = var->_optionMenu->getEntry ((int32_t)[item tag]);
		if (!menuItem->isEnabled () || menuItem->isTitle ())
			return NO;
	}
	return YES;
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSMenu_MenuItemSelected (id self, SEL _cmd, id item)
{
	VSTGUI_NSMenu_Var* var = (VSTGUI_NSMenu_Var*)OBJC_GET_VALUE(self, _private);
	if (var)
	{
		id menu = self;
		while ([menu supermenu]) menu = [menu supermenu];
		[menu performSelector:@selector (setSelectedMenu:) withObject: (id)var->_optionMenu];
		[menu performSelector:@selector (setSelectedItem:) withObject: (id)[item tag]];
	}
}

//------------------------------------------------------------------------------------
static void* VSTGUI_NSMenu_OptionMenu (id self, SEL _cmd)
{
	VSTGUI_NSMenu_Var* var = (VSTGUI_NSMenu_Var*)OBJC_GET_VALUE(self, _private);
	return var ? var->_optionMenu : nullptr;
}

//------------------------------------------------------------------------------------
static void* VSTGUI_NSMenu_SelectedMenu (id self, SEL _cmd)
{
	VSTGUI_NSMenu_Var* var = (VSTGUI_NSMenu_Var*)OBJC_GET_VALUE(self, _private);
	return var ? var->_selectedMenu : nullptr;
}

//------------------------------------------------------------------------------------
static int32_t VSTGUI_NSMenu_SelectedItem (id self, SEL _cmd)
{
	VSTGUI_NSMenu_Var* var = (VSTGUI_NSMenu_Var*)OBJC_GET_VALUE(self, _private);
	return var ? var->_selectedItem : 0;
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSMenu_SetSelectedMenu (id self, SEL _cmd, void* menu)
{
	VSTGUI_NSMenu_Var* var = (VSTGUI_NSMenu_Var*)OBJC_GET_VALUE(self, _private);
	if (var)
		var->_selectedMenu = (COptionMenu*)menu;
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSMenu_SetSelectedItem (id self, SEL _cmd, int32_t item)
{
	VSTGUI_NSMenu_Var* var = (VSTGUI_NSMenu_Var*)OBJC_GET_VALUE(self, _private);
	if (var)
		var->_selectedItem = item;
}

//-----------------------------------------------------------------------------
__attribute__((__destructor__)) static void cleanup_VSTGUI_NSMenu ()
{
	if (menuClass)
		objc_disposeClassPair (menuClass);
}

//-----------------------------------------------------------------------------
bool NSViewOptionMenu::initClass ()
{
	if (menuClass == nullptr)
	{
		NSMutableString* menuClassName = [[[NSMutableString alloc] initWithString:@"VSTGUI_NSMenu"] autorelease];
		menuClass = generateUniqueClass (menuClassName, [NSMenu class]);
		VSTGUI_CHECK_YES (class_addMethod (menuClass, @selector(initWithOptionMenu:), IMP (VSTGUI_NSMenu_Init), "@@:@:^:"))
		VSTGUI_CHECK_YES (class_addMethod (menuClass, @selector(dealloc), IMP (VSTGUI_NSMenu_Dealloc), "v@:@:"))
		VSTGUI_CHECK_YES (class_addMethod (menuClass, @selector(validateMenuItem:), IMP (VSTGUI_NSMenu_ValidateMenuItem), "B@:@:@:"))
		VSTGUI_CHECK_YES (class_addMethod (menuClass, @selector(menuItemSelected:), IMP (VSTGUI_NSMenu_MenuItemSelected), "v@:@:@:"))
		VSTGUI_CHECK_YES (class_addMethod (menuClass, @selector(optionMenu), IMP (VSTGUI_NSMenu_OptionMenu), "^@:@:"))
		VSTGUI_CHECK_YES (class_addMethod (menuClass, @selector(selectedMenu), IMP (VSTGUI_NSMenu_SelectedMenu), "^@:@:"))
		VSTGUI_CHECK_YES (class_addMethod (menuClass, @selector(selectedItem), IMP (VSTGUI_NSMenu_SelectedItem), "i@:@:"))
		VSTGUI_CHECK_YES (class_addMethod (menuClass, @selector(setSelectedMenu:), IMP (VSTGUI_NSMenu_SetSelectedMenu), "^@:@:^:"))
		VSTGUI_CHECK_YES (class_addMethod (menuClass, @selector(setSelectedItem:), IMP (VSTGUI_NSMenu_SetSelectedItem), "^@:@:i:"))
		VSTGUI_CHECK_YES (class_addIvar (menuClass, "_private", sizeof (VSTGUI_NSMenu_Var*), (uint8_t)log2(sizeof(VSTGUI_NSMenu_Var*)), @encode(VSTGUI_NSMenu_Var*)))
		objc_registerClassPair (menuClass);
	}
	return menuClass != nullptr;
}

//-----------------------------------------------------------------------------
void NSViewOptionMenu::popup (COptionMenu* optionMenu, const Callback& callback)
{
	vstgui_assert (optionMenu && callback, "arguments are required");

	PlatformOptionMenuResult result = {};

	CFrame* frame = optionMenu->getFrame ();
	if (!initClass () || !frame || !frame->getPlatformFrame ())
	{
		callback (optionMenu, result);
		return;
	}

	NSViewFrame* nsViewFrame = dynamic_cast<NSViewFrame*> (frame->getPlatformFrame ());
	nsViewFrame->setMouseCursor (kCursorDefault);

	CRect globalSize = optionMenu->translateToGlobal (optionMenu->getViewSize ());
	globalSize.offset (-frame->getViewSize ().getTopLeft ());

	bool multipleCheck = optionMenu->isMultipleCheckStyle ();
	NSView* view = nsViewFrame->getNSView ();
	NSMenu* nsMenu = [[menuClass alloc] initWithOptionMenu:(id)optionMenu];
	CPoint p = globalSize.getTopLeft ();
	NSRect cellFrameRect = {};
	cellFrameRect.origin = nsPointFromCPoint (p);
	cellFrameRect.size.width = static_cast<CGFloat> (globalSize.getWidth ());
	cellFrameRect.size.height = static_cast<CGFloat> (globalSize.getHeight ());
	if (!optionMenu->isPopupStyle ())
		cellFrameRect.origin.y += cellFrameRect.size.height;
	if (!multipleCheck && optionMenu->isCheckStyle ())
	{
		[[nsMenu itemWithTag:static_cast<NSInteger> (optionMenu->getCurrentIndex (true))]
		    setState:NSControlStateValueOn];
	}
	nsMenu.minimumWidth = cellFrameRect.size.width;
	
	NSView* menuContainer = [[NSView alloc] initWithFrame:cellFrameRect];
	[view addSubview:menuContainer];

	NSMenuItem* selectedItem = nil;
	if (optionMenu->isPopupStyle ())
		selectedItem = [nsMenu itemAtIndex:optionMenu->getValue ()];
	[nsMenu popUpMenuPositioningItem:selectedItem
	                      atLocation:NSMakePoint (0, menuContainer.frame.size.height)
	                          inView:menuContainer];

	[menuContainer removeFromSuperviewWithoutNeedingDisplay];
	[menuContainer release];
	result.menu = (COptionMenu*)[nsMenu performSelector:@selector(selectedMenu)];
	result.index = (int32_t)(intptr_t)[nsMenu performSelector:@selector(selectedItem)];
	[nsMenu release];

	callback (optionMenu, result);
}


} // VSTGUI

#endif // MAC_COCOA
