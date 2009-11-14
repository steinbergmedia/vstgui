
#include "cocoatextedit.h"

#if MAC_COCOA && VSTGUI_PLATFORM_ABSTRACTION

#import "cocoahelpers.h"
#import "autoreleasepool.h"
#import "../../../cfontmac.h"
#import "../../../vstkeycode.h"

USING_NAMESPACE_VSTGUI

static Class textFieldClass = 0;

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

		editFrameRect.origin.x = textInset.x/2.;
		editFrameRect.origin.y = textInset.y/2.;
		editFrameRect.size.width -= textInset.x/2.;
		editFrameRect.size.height -= textInset.y/2.;
		self = objc_msgSendSuper (SUPER, @selector(initWithFrame:), editFrameRect);
		if (!self)
		{
			[containerView release];
			return nil;
		}
		OBJC_SET_VALUE (self, _textEdit, textEdit);

		#if VSTGUI_USES_CORE_TEXT
		CoreTextFont* ctf = dynamic_cast<CoreTextFont*> (tec->platformGetFont ()->getPlatformFont ());
		if (ctf)
		{
			CTFontRef fontRef = ctf->getFontRef ();
			if (fontRef)
			{
				CTFontDescriptorRef fontDesc = CTFontCopyFontDescriptor (fontRef);
				
				[self setFont:[NSFont fontWithDescriptor:(NSFontDescriptor *)fontDesc size:0]];
				CFRelease (fontDesc);
			}
		}
		#endif
		
		NSString* text = [NSString stringWithCString:tec->platformGetText () encoding:NSUTF8StringEncoding];

		[self setTextColor:nsColorFromCColor (tec->platformGetFontColor ())];
		[self setBordered:NO];
		[self setAllowsEditingTextAttributes:NO];
		[self setImportsGraphics:NO];
		[self setStringValue:text];
		[self setFocusRingType:NSFocusRingTypeNone];
		[self sizeToFit];
		if ([self frame].size.height < editFrameRect.size.height)
		{
			CGFloat offset = editFrameRect.size.height - [self frame].size.height;
			editFrameRect.origin.y = offset / 2.;
			editFrameRect.size.height = [self frame].size.height;
			[self setFrame:editFrameRect];
		}
		
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
		[[self window] makeFirstResponder: self];
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

	[containerView setFrame:nsRectFromCRect (rect)];

	rect.inset (-15, -15);
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
static BOOL VSTGUI_NSTextField_DoCommandBySelector (id self, SEL _cmd, NSControl* control, NSTextView* textView, SEL commandSelector)
{
	CocoaTextEdit* te = (CocoaTextEdit*)OBJC_GET_VALUE(self, _textEdit);
	if (!te)
		return NO;
	IPlatformTextEditCallback* tec = te->getTextEdit ();
	if (commandSelector == @selector (insertNewline:))
	{
		tec->platformLooseFocus (true);
		return YES;
	}
	else if (commandSelector == @selector (insertTab:))
	{
		VstKeyCode keyCode = {0};
		keyCode.virt = VKEY_TAB;
		if (tec->platformOnKeyDown (keyCode))
			return YES;
	}
	else if (commandSelector == @selector (insertBacktab:))
	{
		VstKeyCode keyCode = {0};
		keyCode.virt = VKEY_TAB;
		keyCode.modifier = MODIFIER_SHIFT;
		if (tec->platformOnKeyDown (keyCode))
			return YES;
	}
	else if (commandSelector == @selector (cancelOperation:))
	{
		VstKeyCode keyCode = {0};
		keyCode.virt = VKEY_ESCAPE;
		if (!tec->platformOnKeyDown (keyCode))
			tec->platformLooseFocus (false);
		return YES; // return YES, otherwise it beeps !!!
	}
	return NO;
}

namespace VSTGUI {

//-----------------------------------------------------------------------------
__attribute__((__destructor__)) void cleanup_VSTGUI_NSTextField ()
{
	if (textFieldClass)
		objc_disposeClassPair (textFieldClass);
}

//-----------------------------------------------------------------------------
void CocoaTextEdit::initClass ()
{
	if (textFieldClass == 0)
	{
		AutoreleasePool ap ();
		NSMutableString* textFieldClassName = [[[NSMutableString alloc] initWithString:@"VSTGUI_NSTextField"] autorelease];
		textFieldClass = generateUniqueClass (textFieldClassName, [NSTextField class]);
		BOOL res = class_addMethod (textFieldClass, @selector(initWithTextEdit:), IMP (VSTGUI_NSTextField_Init), "@@:@:^:");
		res = class_addMethod (textFieldClass, @selector(syncSize), IMP (VSTGUI_NSTextField_SyncSize), "v@:@:");
		res = class_addMethod (textFieldClass, @selector(removeFromSuperview), IMP (VSTGUI_NSTextField_RemoveFromSuperview), "v@:@:");
		res = class_addMethod (textFieldClass, @selector(control:textView:doCommandBySelector:), IMP (VSTGUI_NSTextField_DoCommandBySelector), "B@:@:@:@::");
		res = class_addIvar (textFieldClass, "_textEdit", sizeof (void*), (uint8_t)log2(sizeof(void*)), @encode(void*));
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
	platformControl = [[textFieldClass alloc] performSelector:@selector (initWithTextEdit:) withObject:(id)this];
}

//-----------------------------------------------------------------------------
CocoaTextEdit::~CocoaTextEdit ()
{
	[platformControl performSelector:@selector(removeFromSuperview)];
	[platformControl performSelector:@selector(autorelease)];
}

//-----------------------------------------------------------------------------
bool CocoaTextEdit::getText (char* text, long maxSize)
{
	return [[platformControl stringValue] getCString:text maxLength:maxSize encoding:NSUTF8StringEncoding] == YES ? true : false;
}

//-----------------------------------------------------------------------------
bool CocoaTextEdit::updateSize ()
{
	[platformControl performSelector:@selector(syncSize)];
}

//-----------------------------------------------------------------------------
bool CocoaTextEdit::setText (const char* text)
{
	NSString* nsText = [NSString stringWithCString:text encoding:NSUTF8StringEncoding];
	[platformControl setStringValue:nsText];
	return true;
}

} // namespace

#endif
