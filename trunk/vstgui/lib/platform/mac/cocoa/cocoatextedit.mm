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

#include "cocoatextedit.h"

#if MAC_COCOA

#import "cocoahelpers.h"
#import "autoreleasepool.h"
#import "../cfontmac.h"
#import "../../../vstkeycode.h"

using namespace VSTGUI;

static Class textFieldClass = 0;

@interface NSObject (VSTGUI_NSTextField_Private)
-(id)initWithTextEdit:(id)textEit;
@end

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
static id VSTGUI_NSTextField_Init (id self, SEL _cmd, void* textEdit)
{
	__OBJC_SUPER(self)
	if (self)
	{
		CocoaTextEdit* te = (CocoaTextEdit*)textEdit;
		IPlatformTextEditCallback* tec = te->getTextEdit ();
		NSView* frameView = te->getParent ();
		NSRect editFrameRect = nsRectFromCRect (tec->platformGetSize ());
		NSView* containerView = [[NSView alloc] initWithFrame:editFrameRect];
		[containerView setAutoresizesSubviews:YES];

		CPoint textInset = tec->platformGetTextInset ();

		editFrameRect.origin.x = textInset.x/2. - 1.;
		editFrameRect.origin.y = textInset.y/2.;
		editFrameRect.size.width -= textInset.x/2.;
		editFrameRect.size.height -= textInset.y/2. - 1.;
		self = objc_msgSendSuper (SUPER, @selector(initWithFrame:), editFrameRect);
		if (!self)
		{
			[containerView release];
			return nil;
		}
		OBJC_SET_VALUE (self, _textEdit, textEdit);

		CoreTextFont* ctf = dynamic_cast<CoreTextFont*> (tec->platformGetFont ()->getPlatformFont ());
		if (ctf)
		{
			CTFontRef fontRef = ctf->getFontRef ();
			if (fontRef)
			{
				CTFontDescriptorRef fontDesc = CTFontCopyFontDescriptor (fontRef);
				
				[self setFont:[NSFont fontWithDescriptor:(NSFontDescriptor *)fontDesc size:ctf->getSize ()]];
				CFRelease (fontDesc);
			}
		}
		
		NSString* text = [NSString stringWithCString:tec->platformGetText () encoding:NSUTF8StringEncoding];

		[self setTextColor:nsColorFromCColor (tec->platformGetFontColor ())];
		[self setBordered:NO];
		[self setAllowsEditingTextAttributes:NO];
		[self setImportsGraphics:NO];
		[self setStringValue:text];
		[self setFocusRingType:NSFocusRingTypeNone];
		[self sizeToFit];
		[(NSView*)self setAutoresizingMask:NSViewMinYMargin];
		if ([self frame].size.height < editFrameRect.size.height)
		{
			CGFloat offset = editFrameRect.size.height - [self frame].size.height;
			editFrameRect.origin.y = offset / 2.;
			editFrameRect.size.height = [self frame].size.height;
		}
		else
			editFrameRect.size.height = [self frame].size.height;
		[self setFrame:editFrameRect];
		
		[containerView addSubview:self];
		[self performSelector:@selector(syncSize)];
		[frameView addSubview:containerView];

		NSTextFieldCell* cell = [self cell];
		[cell setDrawsBackground:NO];
		[cell setLineBreakMode: NSLineBreakByClipping];
		[cell setScrollable:YES];
		if (tec->platformGetHoriTxtAlign () == kCenterText)
			[cell setAlignment:NSCenterTextAlignment];
		else if (tec->platformGetHoriTxtAlign () == kRightText)
			[cell setAlignment:NSRightTextAlignment];

		[self setDelegate:self];
		[self setNextKeyView:frameView];
		if ([frameView respondsToSelector:@selector(makeSubViewFirstResponder:)])
			[frameView performSelector:@selector(makeSubViewFirstResponder:) withObject:self];
		else
			[[self window] makeFirstResponder: self];

		if ([frameView wantsLayer])
		{
			double maxZPosition = -1.;
			for (CALayer* layer in frameView.layer.sublayers)
			{
				double zPosition = layer.zPosition;
				if (zPosition > maxZPosition)
					maxZPosition = zPosition;
			}
			[containerView layer].zPosition = maxZPosition + 1;
		}
		
	}
	return self;
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSTextField_SyncSize (id self, SEL _cmd)
{
	CocoaTextEdit* te = (CocoaTextEdit*)OBJC_GET_VALUE(self, _textEdit);
	if (!te)
		return;
	IPlatformTextEditCallback* tec = te->getTextEdit ();
	NSView* containerView = [self superview];
	CRect rect (tec->platformGetVisibleSize ());
	rect.makeIntegral ();

	[containerView setFrame:nsRectFromCRect (rect)];

	rect.extend (15,-15);
	[[containerView superview] setNeedsDisplayInRect:nsRectFromCRect (rect)];
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSTextField_RemoveFromSuperview (id self, SEL _cmd)
{
	OBJC_SET_VALUE (self, _textEdit, 0);
	NSView* containerView = [self superview];
	if (containerView)
	{
		[[containerView window] makeFirstResponder:[containerView superview]];
		[containerView removeFromSuperview];
		__OBJC_SUPER(self)
		objc_msgSendSuper (SUPER, @selector(removeFromSuperview)); // [super removeFromSuperview];
		[containerView release];
	}
}

//------------------------------------------------------------------------------------
static void VSTGUI_NSTextField_TextDidChange (id self, SEL _cmd, NSNotification* notification)
{
	CocoaTextEdit* te = (CocoaTextEdit*)OBJC_GET_VALUE(self, _textEdit);
	if (te && te->getTextEdit ())
	{
		te->getTextEdit ()->platformTextDidChange ();
	}
	__OBJC_SUPER(self)
	objc_msgSendSuper (SUPER, @selector(textDidChange:), notification);
}

//------------------------------------------------------------------------------------
static BOOL VSTGUI_NSTextField_DoCommandBySelector (id self, SEL _cmd, NSControl* control, NSTextView* textView, SEL commandSelector)
{
	CocoaTextEdit* te = (CocoaTextEdit*)OBJC_GET_VALUE(self, _textEdit);
	if (!te)
		return NO;
	IPlatformTextEditCallback* tec = te->getTextEdit ();
	if (commandSelector == @selector (insertNewline:))
	{
		VstKeyCode keyCode = {0};
		keyCode.virt = VKEY_RETURN;
		if (!tec->platformOnKeyDown (keyCode))
		{
			tec->platformLooseFocus (true);
		}
		return YES;
	}
	else if (commandSelector == @selector (insertTab:))
	{
		VstKeyCode keyCode = {0};
		keyCode.virt = VKEY_TAB;
		if (tec->platformOnKeyDown (keyCode))
		{
			return YES;
		}
	}
	else if (commandSelector == @selector (insertBacktab:))
	{
		VstKeyCode keyCode = {0};
		keyCode.virt = VKEY_TAB;
		keyCode.modifier = MODIFIER_SHIFT;
		if (tec->platformOnKeyDown (keyCode))
		{
			return YES;
		}
	}
	else if (commandSelector == @selector (cancelOperation:))
	{
		VstKeyCode keyCode = {0};
		keyCode.virt = VKEY_ESCAPE;
		if (!tec->platformOnKeyDown (keyCode))
		{
			tec->platformLooseFocus (false);
		}
		return YES; // return YES, otherwise it beeps !!!
	}
	return NO;
}

namespace VSTGUI {

//-----------------------------------------------------------------------------
__attribute__((__destructor__)) static void cleanup_VSTGUI_NSTextField ()
{
	if (textFieldClass)
		objc_disposeClassPair (textFieldClass);
}

//-----------------------------------------------------------------------------
void CocoaTextEdit::initClass ()
{
	if (textFieldClass == 0)
	{
		AutoreleasePool ap;
		NSMutableString* textFieldClassName = [[[NSMutableString alloc] initWithString:@"VSTGUI_NSTextField"] autorelease];
		textFieldClass = generateUniqueClass (textFieldClassName, [NSTextField class]);
		VSTGUI_CHECK_YES(class_addMethod (textFieldClass, @selector(initWithTextEdit:), IMP (VSTGUI_NSTextField_Init), "@@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (textFieldClass, @selector(syncSize), IMP (VSTGUI_NSTextField_SyncSize), "v@:@:"))
		VSTGUI_CHECK_YES(class_addMethod (textFieldClass, @selector(removeFromSuperview), IMP (VSTGUI_NSTextField_RemoveFromSuperview), "v@:@:"))
		VSTGUI_CHECK_YES(class_addMethod (textFieldClass, @selector(control:textView:doCommandBySelector:), IMP (VSTGUI_NSTextField_DoCommandBySelector), "B@:@:@:@::"))
		VSTGUI_CHECK_YES(class_addMethod (textFieldClass, @selector(textDidChange:), IMP (VSTGUI_NSTextField_TextDidChange), "v@:@:@@:"))
		VSTGUI_CHECK_YES(class_addIvar (textFieldClass, "_textEdit", sizeof (void*), (uint8_t)log2(sizeof(void*)), @encode(void*)))
		objc_registerClassPair (textFieldClass);
	}
}

//-----------------------------------------------------------------------------
CocoaTextEdit::CocoaTextEdit (NSView* parent, IPlatformTextEditCallback* textEdit)
: IPlatformTextEdit (textEdit)
, platformControl (0)
, parent (parent)
{
	initClass ();
	platformControl = [[textFieldClass alloc] initWithTextEdit:(id)this];
}

//-----------------------------------------------------------------------------
CocoaTextEdit::~CocoaTextEdit ()
{
	[platformControl performSelector:@selector(removeFromSuperview)];
	[platformControl performSelector:@selector(autorelease)];
}

//-----------------------------------------------------------------------------
UTF8StringPtr CocoaTextEdit::getText ()
{
	return [[platformControl stringValue] UTF8String];
}

//-----------------------------------------------------------------------------
bool CocoaTextEdit::updateSize ()
{
	[platformControl performSelector:@selector(syncSize)];
	return true;
}

//-----------------------------------------------------------------------------
bool CocoaTextEdit::setText (UTF8StringPtr text)
{
	NSString* nsText = [NSString stringWithCString:text encoding:NSUTF8StringEncoding];
	[platformControl setStringValue:nsText];
	return true;
}

} // namespace

#endif // MAC_COCOA
