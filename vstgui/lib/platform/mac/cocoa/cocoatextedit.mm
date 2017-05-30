// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cocoatextedit.h"

#if MAC_COCOA

#import "cocoahelpers.h"
#import "autoreleasepool.h"
#import "../cfontmac.h"
#import "../macstring.h"
#import "../../../vstkeycode.h"

using namespace VSTGUI;

static Class textFieldClass = nullptr;
static Class secureTextFieldClass = nullptr;

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

		editFrameRect.origin.x = static_cast<CGFloat> (textInset.x/2. - 1.);
		editFrameRect.origin.y = static_cast<CGFloat> (textInset.y/2.);
		editFrameRect.size.width -= textInset.x/2.;
		editFrameRect.size.height -= textInset.y/2. - 1.;
		self = objc_msgSendSuper (SUPER, @selector(initWithFrame:), editFrameRect);
		if (!self)
		{
			[containerView release];
			return nil;
		}
		OBJC_SET_VALUE (self, _textEdit, textEdit);

		CoreTextFont* ctf = tec->platformGetFont ()->getPlatformFont ().cast<CoreTextFont> ();
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
		
		NSString* text = fromUTF8String<NSString*> (tec->platformGetText ());
		NSString* placeholder = fromUTF8String<NSString*> (tec->platformGetPlaceholderText ());

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
			editFrameRect.origin.y = static_cast<CGFloat> (offset / 2.);
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
		if (placeholder.length > 0)
		{
			CColor color = tec->platformGetFontColor ();
			color.alpha /= 2;
			NSMutableParagraphStyle* paragraphStyle = [[[NSMutableParagraphStyle alloc] init] autorelease];
			if (tec->platformGetHoriTxtAlign () == kCenterText)
				paragraphStyle.alignment = NSCenterTextAlignment;
			else if (tec->platformGetHoriTxtAlign () == kRightText)
				paragraphStyle.alignment = NSRightTextAlignment;
			NSDictionary* attrDict =
			    [NSDictionary dictionaryWithObjectsAndKeys:[self font], NSFontAttributeName,
			                                               nsColorFromCColor (color),
			                                               NSForegroundColorAttributeName,
														   paragraphStyle,
														   NSParagraphStyleAttributeName,
														   nil];
			NSAttributedString* as =
			    [[[NSAttributedString alloc] initWithString:placeholder attributes:attrDict]
			        autorelease];
			[cell setPlaceholderAttributedString:as];
		}

		[self setDelegate:self];
		[self setNextKeyView:frameView];


		if (auto tv = static_cast<NSTextView*> ([[self window] fieldEditor:YES forObject:self]))
			tv.insertionPointColor = nsColorFromCColor (tec->platformGetFontColor ());

		if ([frameView respondsToSelector:@selector (makeSubViewFirstResponder:)])
			[frameView performSelector:@selector (makeSubViewFirstResponder:)
							withObject:self];

		dispatch_after (dispatch_time (DISPATCH_TIME_NOW, (int64_t) (1 * NSEC_PER_MSEC)),
		                dispatch_get_main_queue (), ^{
				              [[self window] makeFirstResponder:self];
			            });

		if ([frameView wantsLayer])
		{
			double maxZPosition = -1.;
			for (CALayer* layer in frameView.layer.sublayers)
			{
				double zPosition = layer.zPosition;
				if (zPosition > maxZPosition)
					maxZPosition = zPosition;
			}
			[containerView layer].zPosition = static_cast<CGFloat> (maxZPosition + 1);
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
	OBJC_SET_VALUE (self, _textEdit, nullptr);
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
		if (tec->platformOnKeyDown (keyCode))
		{
			return YES;
		}
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
		if (tec->platformOnKeyDown (keyCode))
		{
			return YES;
		}
	}
	return NO;
}

namespace VSTGUI {

//-----------------------------------------------------------------------------
__attribute__((__destructor__)) static void cleanup_VSTGUI_NSTextField ()
{
	if (textFieldClass)
		objc_disposeClassPair (textFieldClass);
	if (secureTextFieldClass)
		objc_disposeClassPair (secureTextFieldClass);
}

//-----------------------------------------------------------------------------
void CocoaTextEdit::initClass ()
{
	if (textFieldClass == nullptr)
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

		NSMutableString* secureTextFieldClassName = [[[NSMutableString alloc] initWithString:@"VSTGUI_NSSecureTextField"] autorelease];
		secureTextFieldClass = generateUniqueClass (secureTextFieldClassName, [NSSecureTextField class]);
		VSTGUI_CHECK_YES(class_addMethod (secureTextFieldClass, @selector(initWithTextEdit:), IMP (VSTGUI_NSTextField_Init), "@@:@:^:"))
		VSTGUI_CHECK_YES(class_addMethod (secureTextFieldClass, @selector(syncSize), IMP (VSTGUI_NSTextField_SyncSize), "v@:@:"))
		VSTGUI_CHECK_YES(class_addMethod (secureTextFieldClass, @selector(removeFromSuperview), IMP (VSTGUI_NSTextField_RemoveFromSuperview), "v@:@:"))
		VSTGUI_CHECK_YES(class_addMethod (secureTextFieldClass, @selector(control:textView:doCommandBySelector:), IMP (VSTGUI_NSTextField_DoCommandBySelector), "B@:@:@:@::"))
		VSTGUI_CHECK_YES(class_addMethod (secureTextFieldClass, @selector(textDidChange:), IMP (VSTGUI_NSTextField_TextDidChange), "v@:@:@@:"))
		VSTGUI_CHECK_YES(class_addIvar (secureTextFieldClass, "_textEdit", sizeof (void*), (uint8_t)log2(sizeof(void*)), @encode(void*)))
		objc_registerClassPair (secureTextFieldClass);
	}
}

//-----------------------------------------------------------------------------
CocoaTextEdit::CocoaTextEdit (NSView* parent, IPlatformTextEditCallback* textEdit)
: IPlatformTextEdit (textEdit)
, platformControl (nullptr)
, parent (parent)
{
	initClass ();
	if (textEdit->platformIsSecureTextEdit ())
		platformControl = [[secureTextFieldClass alloc] initWithTextEdit:(id)this];
	else
		platformControl = [[textFieldClass alloc] initWithTextEdit:(id)this];
}

//-----------------------------------------------------------------------------
CocoaTextEdit::~CocoaTextEdit () noexcept
{
	[platformControl performSelector:@selector(removeFromSuperview)];
	[platformControl performSelector:@selector(autorelease)];
}

//-----------------------------------------------------------------------------
UTF8String CocoaTextEdit::getText ()
{
	return [[[platformControl stringValue] decomposedStringWithCanonicalMapping] UTF8String];
}

//-----------------------------------------------------------------------------
bool CocoaTextEdit::updateSize ()
{
	[platformControl performSelector:@selector(syncSize)];
	return true;
}

//-----------------------------------------------------------------------------
bool CocoaTextEdit::setText (const UTF8String& text)
{
	[platformControl setStringValue:fromUTF8String<NSString*> (text)];
	return true;
}

} // namespace

#endif // MAC_COCOA
