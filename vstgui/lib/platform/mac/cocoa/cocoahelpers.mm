// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cocoahelpers.h"

#if MAC_COCOA

#include "../../../vstkeycode.h"
#include "../../../events.h"
#include "../../../cview.h"
#include "../../../cbitmap.h"
#include "../cgbitmap.h"

//------------------------------------------------------------------------------------
HIDDEN Class generateUniqueClass (NSMutableString* className, Class baseClass)
{
	NSString* _className = [NSString stringWithString:className];
	int32_t iteration = 0;
	while (objc_lookUpClass ([className UTF8String]) != nil)
	{
		iteration++;
		[className setString:[NSString stringWithFormat:@"%@_%d", _className, iteration]];
	}
	Class resClass = objc_allocateClassPair (baseClass, [className UTF8String], 0);
	return resClass;
}

using namespace VSTGUI;

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
HIDDEN bool CreateKeyboardEventFromNSEvent (NSEvent* theEvent, KeyboardEvent& event)
{
	if (theEvent.type == NSEventTypeKeyUp)
		event.type = EventType::KeyUp;
	else if (theEvent.type == NSEventTypeKeyDown || theEvent.type == NSEventTypeFlagsChanged)
	{
		event.type = EventType::KeyDown;
		if (theEvent.ARepeat)
			event.isRepeat = true;
	}
	else
		return false;
    NSString *s = [theEvent charactersIgnoringModifiers];
    if ([s length] == 1)
	{
		char32_t utf32Char = {};
		if (![s getBytes:&utf32Char
		             maxLength:sizeof (utf32Char)
		            usedLength:nullptr
		              encoding:NSUTF32StringEncoding
		               options:0
		                 range:NSMakeRange (0, 1)
		        remainingRange:nullptr])
		{
			utf32Char = [s characterAtIndex:0];
		}
		switch (utf32Char)
		{
			case 8: case 0x7f:				event.virt = VirtualKey::Back; break;
			case 9:	case 0x19:				event.virt = VirtualKey::Tab; break;
			case NSClearLineFunctionKey:	event.virt = VirtualKey::Clear; break;
			case 0xd:						event.virt = VirtualKey::Return; break;
			case NSPauseFunctionKey:		event.virt = VirtualKey::Pause; break;
			case 0x1b:						event.virt = VirtualKey::Escape; break;
			case ' ':						event.virt = VirtualKey::Space; break;
			case NSNextFunctionKey:			event.virt = VirtualKey::Next; break;
			case NSEndFunctionKey:			event.virt = VirtualKey::End; break;
			case NSHomeFunctionKey:			event.virt = VirtualKey::Home; break;

			case NSLeftArrowFunctionKey:	event.virt = VirtualKey::Left; break;
			case NSUpArrowFunctionKey:		event.virt = VirtualKey::Up; break;
			case NSRightArrowFunctionKey:	event.virt = VirtualKey::Right; break;
			case NSDownArrowFunctionKey:	event.virt = VirtualKey::Down; break;
			case NSPageUpFunctionKey:		event.virt = VirtualKey::PageUp; break;
			case NSPageDownFunctionKey:		event.virt = VirtualKey::PageDown; break;
			
			case NSSelectFunctionKey:		event.virt = VirtualKey::Select; break;
			case NSPrintFunctionKey:		event.virt = VirtualKey::Print; break;
			// VirtualKey::ENTER
			// VirtualKey::SNAPSHOT
			case NSInsertFunctionKey:		event.virt = VirtualKey::Insert; break;
			case NSDeleteFunctionKey:		event.virt = VirtualKey::Delete; break;
			case NSHelpFunctionKey:			event.virt = VirtualKey::Help; break;


			case NSF1FunctionKey:			event.virt = VirtualKey::F1; break;
			case NSF2FunctionKey:			event.virt = VirtualKey::F2; break;
			case NSF3FunctionKey:			event.virt = VirtualKey::F3; break;
			case NSF4FunctionKey:			event.virt = VirtualKey::F4; break;
			case NSF5FunctionKey:			event.virt = VirtualKey::F5; break;
			case NSF6FunctionKey:			event.virt = VirtualKey::F6; break;
			case NSF7FunctionKey:			event.virt = VirtualKey::F7; break;
			case NSF8FunctionKey:			event.virt = VirtualKey::F8; break;
			case NSF9FunctionKey:			event.virt = VirtualKey::F9; break;
			case NSF10FunctionKey:			event.virt = VirtualKey::F10; break;
			case NSF11FunctionKey:			event.virt = VirtualKey::F11; break;
			case NSF12FunctionKey:			event.virt = VirtualKey::F12; break;
			default:
			{
				switch ([theEvent keyCode])
				{
					case 82:				event.virt = VirtualKey::NumPad0; break;
					case 83:				event.virt = VirtualKey::NumPad1; break;
					case 84:				event.virt = VirtualKey::NumPad2; break;
					case 85:				event.virt = VirtualKey::NumPad3; break;
					case 86:				event.virt = VirtualKey::NumPad4; break;
					case 87:				event.virt = VirtualKey::NumPad5; break;
					case 88:				event.virt = VirtualKey::NumPad6; break;
					case 89:				event.virt = VirtualKey::NumPad7; break;
					case 91:				event.virt = VirtualKey::NumPad8; break;
					case 92:				event.virt = VirtualKey::NumPad9; break;
					case 67:				event.virt = VirtualKey::Multiply; break;
					case 69:				event.virt = VirtualKey::Add; break;
					case 78:				event.virt = VirtualKey::Subtract; break;
					case 65:				event.virt = VirtualKey::Decimal; break;
					case 75:				event.virt = VirtualKey::Divide; break;
					case 76:				event.virt = VirtualKey::Enter; break;
					default:
					{
						if ((utf32Char >= 'A') && (utf32Char <= 'Z'))
							utf32Char += ('a' - 'A');
						else
							utf32Char = static_cast<char32_t> (tolower (utf32Char));
						event.character = utf32Char;
						break;
					}
				}
			}
		}
    }

	NSUInteger modifiers = [theEvent modifierFlags];
	if (modifiers & MacEventModifier::ShiftKeyMask)
		event.modifiers.add (ModifierKey::Shift);
	if (modifiers & MacEventModifier::CommandKeyMask)
		event.modifiers.add (ModifierKey::Control);
	if (modifiers & MacEventModifier::AlternateKeyMask)
		event.modifiers.add (ModifierKey::Alt);
	if (modifiers & MacEventModifier::ControlKeyMask)
		event.modifiers.add (ModifierKey::Super);

	return true;
}

//------------------------------------------------------------------------------------
HIDDEN NSString* GetVirtualKeyCodeString (int32_t virtualKeyCode)
{
	unichar character = 0;
	switch (virtualKeyCode)
	{
		case VKEY_BACK: character = NSDeleteCharacter; break;
		case VKEY_TAB: character = NSTabCharacter; break;
		case VKEY_CLEAR: character = NSClearLineFunctionKey; break;
		case VKEY_RETURN: character = NSCarriageReturnCharacter; break;
		case VKEY_PAUSE: character = NSPauseFunctionKey; break;
		case VKEY_ESCAPE: character = 0x1b; break;
		case VKEY_SPACE: character = ' '; break;
		case VKEY_NEXT: character = NSNextFunctionKey; break;
		case VKEY_END: character = NSEndFunctionKey; break;
		case VKEY_HOME: character = NSHomeFunctionKey; break;
		case VKEY_LEFT: character = NSLeftArrowFunctionKey; break;
		case VKEY_UP: character = NSUpArrowFunctionKey; break;
		case VKEY_RIGHT: character = NSRightArrowFunctionKey; break;
		case VKEY_DOWN: character = NSDownArrowFunctionKey; break;
		case VKEY_PAGEUP: character = NSPageUpFunctionKey; break;
		case VKEY_PAGEDOWN: character = NSPageDownFunctionKey; break;
		case VKEY_SELECT: character = NSSelectFunctionKey; break;
		case VKEY_PRINT: character = NSPrintFunctionKey; break;
		case VKEY_ENTER: character = NSEnterCharacter; break;
		case VKEY_SNAPSHOT: break;
		case VKEY_INSERT: character = NSInsertFunctionKey; break;
		case VKEY_DELETE: character = NSDeleteFunctionKey; break;
		case VKEY_HELP: character = NSHelpFunctionKey; break;
		case VKEY_NUMPAD0: break;
		case VKEY_NUMPAD1: break;
		case VKEY_NUMPAD2: break;
		case VKEY_NUMPAD3: break;
		case VKEY_NUMPAD4: break;
		case VKEY_NUMPAD5: break;
		case VKEY_NUMPAD6: break;
		case VKEY_NUMPAD7: break;
		case VKEY_NUMPAD8: break;
		case VKEY_NUMPAD9: break;
		case VKEY_MULTIPLY: break;
		case VKEY_ADD: break;
		case VKEY_SEPARATOR: break;
		case VKEY_SUBTRACT: break;
		case VKEY_DECIMAL: break;
		case VKEY_DIVIDE: break;
		case VKEY_F1: character = NSF1FunctionKey; break;
		case VKEY_F2: character = NSF2FunctionKey; break;
		case VKEY_F3: character = NSF3FunctionKey; break;
		case VKEY_F4: character = NSF4FunctionKey; break;
		case VKEY_F5: character = NSF5FunctionKey; break;
		case VKEY_F6: character = NSF6FunctionKey; break;
		case VKEY_F7: character = NSF7FunctionKey; break;
		case VKEY_F8: character = NSF8FunctionKey; break;
		case VKEY_F9: character = NSF9FunctionKey; break;
		case VKEY_F10: character = NSF10FunctionKey; break;
		case VKEY_F11: character = NSF11FunctionKey; break;
		case VKEY_F12: character = NSF12FunctionKey; break;
		case VKEY_NUMLOCK: break;
		case VKEY_SCROLL: break;
		case VKEY_EQUALS: break;
	}
	if (character != 0)
		return [NSString stringWithFormat:@"%C", character];
	return nil;
}

//------------------------------------------------------------------------------------
HIDDEN int32_t eventButton (NSEvent* theEvent)
{
	if ([theEvent type] == MacEventType::MouseMoved)
	{
		return 0;
	}

	int32_t buttons = 0;
	switch ([theEvent buttonNumber])
	{
		case 0: buttons = ([theEvent modifierFlags] & MacEventModifier::ControlKeyMask) ? kRButton : kLButton; break;
		case 1: buttons = kRButton; break;
		case 2: buttons = kMButton; break;
		case 3: buttons = kButton4; break;
		case 4: buttons = kButton5; break;
	}
	return buttons;
}

//-----------------------------------------------------------------------------
HIDDEN void convertPointToGlobal (NSView* view, NSPoint& p)
{
	p = [view convertPoint:p toView:nil];
	if ([view window] == nil)
		return;

	NSRect r = {};
	r.origin = p;
	r = [[view window] convertRectToScreen:r];
	p = r.origin;
}

//-----------------------------------------------------------------------------
HIDDEN NSImage* bitmapToNSImage (CBitmap* bitmap)
{
	if (!bitmap)
		return nil;

	NSImage* image =
	    [[NSImage alloc] initWithSize:NSMakeSize (bitmap->getWidth (), bitmap->getHeight ())];
	for (auto& platformBitmap : *bitmap)
	{
		if (auto cgBitmap = dynamic_cast<CGBitmap*> (platformBitmap.get ()))
		{
			if (auto rep = [[NSBitmapImageRep alloc] initWithCGImage:cgBitmap->getCGImage ()])
			{
				[image addRepresentation:rep];
				[rep release];
			}
		}
	}
	return image;
}

#endif // MAC_COCOA
