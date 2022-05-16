// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "nsviewoptionmenu.h"

#if MAC_COCOA

#import "../../../cbitmap.h"
#import "../../../cframe.h"
#import "../../../controls/coptionmenu.h"
#import "../cgbitmap.h"
#import "../macstring.h"
#import "cocoahelpers.h"
#import "nsviewframe.h"
#import "objcclassbuilder.h"

@interface NSObject (VSTGUI_NSMenu_Private)
- (id)initWithOptionMenu:(id)menu;
@end

namespace VSTGUI {

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
struct VSTGUI_NSMenu
{
	//------------------------------------------------------------------------------------
	static id alloc () { return [instance ().menuClass alloc]; }

private:
	static constexpr const auto privateVarName = "_private";

	Class menuClass {nullptr};
	int32_t menuClassCount = 0;

	//------------------------------------------------------------------------------------
	VSTGUI_NSMenu ()
	{
		menuClass = ObjCClassBuilder ()
						.init ("VSTGUI_NSMenu", [NSMenu class])
						.addMethod (@selector (initWithOptionMenu:), Init)
						.addMethod (@selector (dealloc), Dealloc)
						.addMethod (@selector (validateMenuItem:), ValidateMenuItem)
						.addMethod (@selector (menuItemSelected:), MenuItemSelected)
						.addMethod (@selector (optionMenu), OptionMenu)
						.addMethod (@selector (selectedMenu), SelectedMenu)
						.addMethod (@selector (selectedItem), SelectedItem)
						.addMethod (@selector (setSelectedMenu:), SetSelectedMenu)
						.addMethod (@selector (setSelectedItem:), SetSelectedItem)
						.addIvar<VSTGUI_NSMenu::Var*> (privateVarName)
						.finalize ();
	}

	//------------------------------------------------------------------------------------
	~VSTGUI_NSMenu () noexcept
	{
		objc_disposeClassPair (menuClass);
		vstgui_assert (menuClassCount == 0);
	}

	//------------------------------------------------------------------------------------
	struct Var
	{
		COptionMenu* _optionMenu {nullptr};
		COptionMenu* _selectedMenu {nullptr};
		int32_t _selectedItem {0};
	};

	//------------------------------------------------------------------------------------
	static VSTGUI_NSMenu& instance ()
	{
		static VSTGUI_NSMenu gInstance;
		return gInstance;
	}

	//------------------------------------------------------------------------------------
	static id Init (id self, SEL _cmd, void* _menu)
	{
		instance ().menuClassCount++;
		ObjCInstance obj (self);
		self = obj.callSuper<id (id, SEL), id> (@selector (init));
		if (self)
		{
			NSMenu* nsMenu = (NSMenu*)self;
			COptionMenu* menu = (COptionMenu*)_menu;
			Var* var = new Var;
			var->_optionMenu = menu;
			setVar (self, var);

			int32_t index = -1;
			bool multipleCheck = menu->isMultipleCheckStyle ();
			CConstMenuItemIterator it = menu->getItems ()->begin ();
			while (it != menu->getItems ()->end ())
			{
				CMenuItem* item = (*it);
				it++;
				index++;
				NSMenuItem* nsItem = nullptr;
				NSMutableString* itemTitle = [[[NSMutableString alloc]
					initWithString:fromUTF8String<NSString*> (item->getTitle ())] autorelease];
				if (menu->getPrefixNumbers ())
				{
					NSString* prefixString = nullptr;
					switch (menu->getPrefixNumbers ())
					{
						case 2:
							prefixString = [NSString stringWithFormat:@"%1d ", index + 1];
							break;
						case 3:
							prefixString = [NSString stringWithFormat:@"%02d ", index + 1];
							break;
						case 4:
							prefixString = [NSString stringWithFormat:@"%03d ", index + 1];
							break;
					}
					[itemTitle insertString:prefixString atIndex:0];
				}
				if (item->getSubmenu ())
				{
					nsItem = [nsMenu addItemWithTitle:itemTitle action:nil keyEquivalent:@""];
					NSMenu* subMenu = [[[[self class] alloc]
						initWithOptionMenu:(id)item->getSubmenu ()] autorelease];
					[nsMenu setSubmenu:subMenu forItem:nsItem];
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
					nsItem = [nsMenu addItemWithTitle:itemTitle
											   action:@selector (menuItemSelected:)
										keyEquivalent:@""];
					if (item->isTitle ())
						[nsItem setIndentationLevel:1];
					[nsItem setTarget:nsMenu];
					[nsItem setTag:index];
					if (multipleCheck && item->isChecked ())
						[nsItem setState:NSControlStateValueOn];
					else
						[nsItem setState:NSControlStateValueOff];
					NSString* keyEquivalent = nil;
					if (!item->getKeycode ().empty ())
					{
						keyEquivalent = fromUTF8String<NSString*> (item->getKeycode ());
					}
					else if (item->getVirtualKey () != VirtualKey::None)
					{
						keyEquivalent = GetVirtualKeyCodeString (item->getVirtualKey ());
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
				if (nsItem)
				{
					if (auto nsImage = bitmapToNSImage (item->getIcon ()))
					{
						[nsItem setImage:nsImage];
						[nsImage release];
					}
				}
			}
		}
		return self;
	}

	//-----------------------------------------------------------------------------
	static void setVar (id self, Var* var)
	{
		if (auto v = ObjCInstance (self).getVariable<Var*> (privateVarName))
		{
			if (auto old = v->get ())
				delete old;
			v->set (var);
		}
	}

	//-----------------------------------------------------------------------------
	static Var* getVar (id self)
	{
		if (auto v = ObjCInstance (self).getVariable<Var*> (privateVarName); v.has_value ())
			return v->get ();
		return nullptr;
	}

	//-----------------------------------------------------------------------------
	static void Dealloc (id self, SEL _cmd)
	{
		instance ().menuClassCount--;

		setVar (self, nullptr);
		ObjCInstance (self).callSuper<void (id, SEL)> (_cmd);
	}

	//------------------------------------------------------------------------------------
	static BOOL ValidateMenuItem (id self, SEL _cmd, id item)
	{
		Var* var = getVar (self);
		if (var && var->_optionMenu)
		{
			CMenuItem* menuItem = var->_optionMenu->getEntry ((int32_t)[item tag]);
			if (!menuItem->isEnabled () || menuItem->isTitle ())
				return NO;
		}
		return YES;
	}

	//------------------------------------------------------------------------------------
	static void MenuItemSelected (id self, SEL _cmd, id item)
	{
		Var* var = getVar (self);
		if (var)
		{
			id menu = self;
			while ([menu supermenu])
				menu = [menu supermenu];
			[menu performSelector:@selector (setSelectedMenu:) withObject:(id)var->_optionMenu];
			[menu performSelector:@selector (setSelectedItem:) withObject:(id)[item tag]];
		}
	}

	//------------------------------------------------------------------------------------
	static void* OptionMenu (id self, SEL _cmd)
	{
		Var* var = getVar (self);
		return var ? var->_optionMenu : nullptr;
	}

	//------------------------------------------------------------------------------------
	static void* SelectedMenu (id self, SEL _cmd)
	{
		Var* var = getVar (self);
		return var ? var->_selectedMenu : nullptr;
	}

	//------------------------------------------------------------------------------------
	static int32_t SelectedItem (id self, SEL _cmd)
	{
		Var* var = getVar (self);
		return var ? var->_selectedItem : 0;
	}

	//------------------------------------------------------------------------------------
	static void SetSelectedMenu (id self, SEL _cmd, void* menu)
	{
		Var* var = getVar (self);
		if (var)
			var->_selectedMenu = (COptionMenu*)menu;
	}

	//------------------------------------------------------------------------------------
	static void SetSelectedItem (id self, SEL _cmd, int32_t item)
	{
		Var* var = getVar (self);
		if (var)
			var->_selectedItem = item;
	}
};

//-----------------------------------------------------------------------------
void NSViewOptionMenu::popup (COptionMenu* optionMenu, const Callback& callback)
{
	vstgui_assert (optionMenu && callback, "arguments are required");

	PlatformOptionMenuResult result = {};

	CFrame* frame = optionMenu->getFrame ();
	if (!frame || !frame->getPlatformFrame ())
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
	NSMenu* nsMenu = [VSTGUI_NSMenu::alloc () initWithOptionMenu:(id)optionMenu];
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
	result.menu = (COptionMenu*)[nsMenu performSelector:@selector (selectedMenu)];
	result.index = (int32_t) (intptr_t)[nsMenu performSelector:@selector (selectedItem)];
	[nsMenu release];

	callback (optionMenu, result);
}

} // VSTGUI

#endif // MAC_COCOA
