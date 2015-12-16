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

#import "nsviewoptionmenu.h"

#if MAC_COCOA

#import "cocoahelpers.h"
#import "nsviewframe.h"
#import "../../../cbitmap.h"
#import "../../../cframe.h"
#import "../../../controls/coptionmenu.h"
#import "../cgbitmap.h"

@interface NSObject (VSTGUI_NSMenu_Private)
-(id)initWithOptionMenu:(id)menu;
@end

namespace VSTGUI {

#if DEBUG
static int32_t menuClassCount = 0;
#endif

static Class menuClass = 0;

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
	self = objc_msgSendSuper (SUPER, @selector(init));
	if (self)
	{
		NSMenu* nsMenu = (NSMenu*)self;
		COptionMenu* menu = (COptionMenu*)_menu;
		VSTGUI_NSMenu_Var* var = new VSTGUI_NSMenu_Var;
		var->_optionMenu = menu;
		var->_selectedItem = 0;
		var->_selectedMenu = 0;
		OBJC_SET_VALUE(self, _private, var);

		int32_t index = -1;
		bool multipleCheck = menu->getStyle () & (kMultipleCheckStyle & ~kCheckStyle);
		CConstMenuItemIterator it = menu->getItems ()->begin ();
		while (it != menu->getItems ()->end ())
		{
			CMenuItem* item = (*it);
			it++;
			index++;
			NSMenuItem* nsItem = 0;
			NSMutableString* itemTitle = [[[NSMutableString alloc] initWithCString:item->getTitle () encoding:NSUTF8StringEncoding] autorelease];
			if (menu->getPrefixNumbers ())
			{
				NSString* prefixString = 0;
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
					[nsItem setState:NSOnState];
				else
					[nsItem setState:NSOffState];
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
					[nsItem setState:NSOnState];
				else
					[nsItem setState:NSOffState];
				NSString* keyEquivalent = nil;
				if (item->getKeycode ())
				{
					keyEquivalent = [NSString stringWithCString:item->getKeycode () encoding:NSUTF8StringEncoding];
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
						keyModifiers |= NSCommandKeyMask;
					if (item->getKeyModifiers () & kShift)
						keyModifiers |= NSShiftKeyMask;
					if (item->getKeyModifiers () & kAlt)
						keyModifiers |= NSAlternateKeyMask;
					if (item->getKeyModifiers () & kApple)
						keyModifiers |= NSControlKeyMask;
					[nsItem setKeyEquivalentModifierMask:keyModifiers];
				}
			}
			if (nsItem && item->getIcon ())
			{
				IPlatformBitmap* platformBitmap = item->getIcon ()->getPlatformBitmap ();
				CGBitmap* cgBitmap = platformBitmap ? dynamic_cast<CGBitmap*> (platformBitmap) : 0;
				CGImageRef image = cgBitmap ? cgBitmap->getCGImage () : 0;
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
	objc_msgSendSuper (SUPER, @selector(dealloc)); // [super dealloc];
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
	return var ? var->_optionMenu : 0;
}

//------------------------------------------------------------------------------------
static void* VSTGUI_NSMenu_SelectedMenu (id self, SEL _cmd)
{
	VSTGUI_NSMenu_Var* var = (VSTGUI_NSMenu_Var*)OBJC_GET_VALUE(self, _private);
	return var ? var->_selectedMenu : 0;
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
	if (menuClass == 0)
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
	return menuClass != 0;
}

//-----------------------------------------------------------------------------
PlatformOptionMenuResult NSViewOptionMenu::popup (COptionMenu* optionMenu)
{
	PlatformOptionMenuResult result = {0};

	if (!initClass ())
		return result;

	CFrame* frame = optionMenu->getFrame ();
	if (!frame || !frame->getPlatformFrame ())
		return result;
	NSViewFrame* nsViewFrame = dynamic_cast<NSViewFrame*> (frame->getPlatformFrame ());

	CRect globalSize = optionMenu->translateToGlobal (optionMenu->getViewSize ());

	bool multipleCheck = optionMenu->getStyle () & (kMultipleCheckStyle & ~kCheckStyle);
	NSView* view = nsViewFrame->getPlatformControl ();
	NSMenu* nsMenu = [[menuClass alloc] initWithOptionMenu:(id)optionMenu];
	CPoint p = globalSize.getTopLeft ();
	NSRect cellFrameRect = {{0}};
	cellFrameRect.origin = nsPointFromCPoint (p);
	cellFrameRect.size.width = globalSize.getWidth ();
	cellFrameRect.size.height = globalSize.getHeight ();
	if (!(optionMenu->getStyle () & kPopupStyle))
	{
		NSMenuItem* item = [nsMenu insertItemWithTitle:@"" action:nil keyEquivalent:@"" atIndex:0];
		[item setTag:-1];
	}
	if (!multipleCheck && optionMenu->getStyle () & kCheckStyle)
		[[nsMenu itemWithTag:(NSInteger)optionMenu->getValue ()] setState:NSOnState];

	NSView* cellContainer = [[NSView alloc] initWithFrame:cellFrameRect];
	[view addSubview:cellContainer];
	cellFrameRect.origin.x = 0;
	cellFrameRect.origin.y = 0;

	NSPopUpButtonCell* cell = [[NSPopUpButtonCell alloc] initTextCell:@"" pullsDown:optionMenu->getStyle () & kPopupStyle ? NO : YES];
	[cell setAltersStateOfSelectedItem: NO];
	[cell setAutoenablesItems:NO];
	[cell setMenu:nsMenu];
	if (optionMenu->getStyle () & kPopupStyle)
		[cell selectItemWithTag:(NSInteger)optionMenu->getValue ()];
	[cell performClickWithFrame:cellFrameRect inView:cellContainer];
	[cellContainer removeFromSuperviewWithoutNeedingDisplay];
	[cellContainer release];
	result.menu = (COptionMenu*)[nsMenu performSelector:@selector(selectedMenu)];
	result.index = (int32_t)(intptr_t)[nsMenu performSelector:@selector(selectedItem)];
	[cell release];
	[nsMenu release];
	
	return result;
}


} // namespace

#endif // MAC_COCOA
