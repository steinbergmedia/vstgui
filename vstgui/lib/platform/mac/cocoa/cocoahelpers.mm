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

#include "cocoahelpers.h"

#if MAC_COCOA

#include "../../../vstkeycode.h"
#include "../../../cview.h"

//------------------------------------------------------------------------------------
HIDDEN Class generateUniqueClass (NSMutableString* className, Class baseClass)
{
	NSString* _className = [NSString stringWithString:className];
	int32_t iteration = 0;
	id cl = nil;
	while ((cl = objc_lookUpClass ([className UTF8String])) != nil)
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
HIDDEN VstKeyCode CreateVstKeyCodeFromNSEvent (NSEvent* theEvent)
{
	VstKeyCode kc = {0};
    NSString *s = [theEvent charactersIgnoringModifiers];
    if ([s length] == 1)
	{
		unichar c = [s characterAtIndex:0];
		switch (c)
		{
			case 8: case 0x7f:				kc.virt = VKEY_BACK; break;
			case 9:	case 0x19:				kc.virt = VKEY_TAB; break;
			case NSClearLineFunctionKey:	kc.virt = VKEY_CLEAR; break;
			case 0xd:						kc.virt = VKEY_RETURN; break;
			case NSPauseFunctionKey:		kc.virt = VKEY_PAUSE; break;
			case 0x1b:						kc.virt = VKEY_ESCAPE; break;
			case ' ':						kc.virt = VKEY_SPACE; break;
			case NSNextFunctionKey:			kc.virt = VKEY_NEXT; break;
			case NSEndFunctionKey:			kc.virt = VKEY_END; break;
			case NSHomeFunctionKey:			kc.virt = VKEY_HOME; break;

			case NSLeftArrowFunctionKey:	kc.virt = VKEY_LEFT; break;
			case NSUpArrowFunctionKey:		kc.virt = VKEY_UP; break;
			case NSRightArrowFunctionKey:	kc.virt = VKEY_RIGHT; break;
			case NSDownArrowFunctionKey:	kc.virt = VKEY_DOWN; break;
			case NSPageUpFunctionKey:		kc.virt = VKEY_PAGEUP; break;
			case NSPageDownFunctionKey:		kc.virt = VKEY_PAGEDOWN; break;
			
			case NSSelectFunctionKey:		kc.virt = VKEY_SELECT; break;
			case NSPrintFunctionKey:		kc.virt = VKEY_PRINT; break;
			// VKEY_ENTER
			// VKEY_SNAPSHOT
			case NSInsertFunctionKey:		kc.virt = VKEY_INSERT; break;
			case NSDeleteFunctionKey:		kc.virt = VKEY_DELETE; break;
			case NSHelpFunctionKey:			kc.virt = VKEY_HELP; break;


			case NSF1FunctionKey:			kc.virt = VKEY_F1; break;
			case NSF2FunctionKey:			kc.virt = VKEY_F2; break;
			case NSF3FunctionKey:			kc.virt = VKEY_F3; break;
			case NSF4FunctionKey:			kc.virt = VKEY_F4; break;
			case NSF5FunctionKey:			kc.virt = VKEY_F5; break;
			case NSF6FunctionKey:			kc.virt = VKEY_F6; break;
			case NSF7FunctionKey:			kc.virt = VKEY_F7; break;
			case NSF8FunctionKey:			kc.virt = VKEY_F8; break;
			case NSF9FunctionKey:			kc.virt = VKEY_F9; break;
			case NSF10FunctionKey:			kc.virt = VKEY_F10; break;
			case NSF11FunctionKey:			kc.virt = VKEY_F11; break;
			case NSF12FunctionKey:			kc.virt = VKEY_F12; break;
			default:
			{
				switch ([theEvent keyCode])
				{
					case 82:				kc.virt = VKEY_NUMPAD0; break;
					case 83:				kc.virt = VKEY_NUMPAD1; break;
					case 84:				kc.virt = VKEY_NUMPAD2; break;
					case 85:				kc.virt = VKEY_NUMPAD3; break;
					case 86:				kc.virt = VKEY_NUMPAD4; break;
					case 87:				kc.virt = VKEY_NUMPAD5; break;
					case 88:				kc.virt = VKEY_NUMPAD6; break;
					case 89:				kc.virt = VKEY_NUMPAD7; break;
					case 91:				kc.virt = VKEY_NUMPAD8; break;
					case 92:				kc.virt = VKEY_NUMPAD9; break;
					case 67:				kc.virt = VKEY_MULTIPLY; break;
					case 69:				kc.virt = VKEY_ADD; break;
					case 78:				kc.virt = VKEY_SUBTRACT; break;
					case 65:				kc.virt = VKEY_DECIMAL; break;
					case 75:				kc.virt = VKEY_DIVIDE; break;
					case 76:				kc.virt = VKEY_ENTER; break;
					default:
					{
						if ((c >= 'A') && (c <= 'Z'))
							c += ('a' - 'A');
						else
							c = static_cast<unichar> (tolower (c));
						kc.character = c;
						break;
					}
				}
			}
		}
    }

	NSUInteger modifiers = [theEvent modifierFlags];
	if (modifiers & NSShiftKeyMask)
		kc.modifier |= MODIFIER_SHIFT;
	if (modifiers & NSCommandKeyMask)
		kc.modifier |= MODIFIER_CONTROL;
	if (modifiers & NSAlternateKeyMask)
		kc.modifier |= MODIFIER_ALTERNATE;
	if (modifiers & NSControlKeyMask)
		kc.modifier |= MODIFIER_COMMAND;

	return kc;
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
	if ([theEvent type] == NSMouseMoved)
		return 0;
	int32_t buttons = 0;
	switch ([theEvent buttonNumber])
	{
		case 0: buttons = ([theEvent modifierFlags] & NSControlKeyMask) ? kRButton : kLButton; break;
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
#if MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_6
	p = [[view window] convertBaseToScreen:p];
#else
	#if MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_6
	if ([NSWindow instancesRespondToSelector:@selector(convertRectToScreen:)])
	{
	#endif

		NSRect r = {};
		r.origin = p;
		r = [[view window] convertRectToScreen:r];
		p = r.origin;

	#if MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_6
	}
	else
	{
		p = [[view window] convertBaseToScreen:p];
	}
	#endif
#endif
}


#endif // MAC_COCOA
