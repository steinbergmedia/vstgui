// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cocoatextedit.h"

#if MAC_COCOA

#import "cocoahelpers.h"
#import "objcclassbuilder.h"
#import "autoreleasepool.h"
#import "../cfontmac.h"
#import "../macstring.h"
#import "../../../events.h"

//------------------------------------------------------------------------------------
@interface NSObject (VSTGUI_NSTextField_Private)
-(id)initWithTextEdit:(id)textEit;
@end

//------------------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------------------
namespace MacTextAlignment {

#ifdef MAC_OS_X_VERSION_10_12
static constexpr auto Right = ::NSTextAlignmentRight;
static constexpr auto Center = ::NSTextAlignmentCenter;
#else
static constexpr auto Right = ::NSRightTextAlignment;
static constexpr auto Center = ::NSCenterTextAlignment;
#endif

//------------------------------------------------------------------------------------
} // MacTextAlignment

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
template<bool SecureT>
struct VSTGUI_NSTextFieldT : RuntimeObjCClass<VSTGUI_NSTextFieldT<SecureT>>
{
	using Base = RuntimeObjCClass<VSTGUI_NSTextFieldT<SecureT>>;

	static constexpr const auto textEditVarName = "_textEdit";

	static Class CreateClass ()
	{
		if constexpr (SecureT)
		{
			return ObjCClassBuilder ()
				.init ("VSTGUI_NSSecureTextField", [NSSecureTextField class])
				.addMethod (@selector (initWithTextEdit:), Init)
				.addMethod (@selector (syncSize), SyncSize)
				.addMethod (@selector (removeFromSuperview), RemoveFromSuperview)
				.addMethod (@selector (control:textView:doCommandBySelector:), DoCommandBySelector)
				.addMethod (@selector (textDidChange:), TextDidChange)
				.addIvar<IPlatformTextEditCallback*> (textEditVarName)
				.finalize ();
		}
		else
		{
			return ObjCClassBuilder ()
				.init ("VSTGUI_NSTextField", [NSTextField class])
				.addMethod (@selector (initWithTextEdit:), Init)
				.addMethod (@selector (syncSize), SyncSize)
				.addMethod (@selector (removeFromSuperview), RemoveFromSuperview)
				.addMethod (@selector (control:textView:doCommandBySelector:), DoCommandBySelector)
				.addMethod (@selector (textDidChange:), TextDidChange)
				.addIvar<CocoaTextEdit*> (textEditVarName)
				.finalize ();
		}
	}

	static id Init (id self, SEL _cmd, void* textEdit)
	{
		if (self)
		{
			CocoaTextEdit* te = (CocoaTextEdit*)textEdit;
			IPlatformTextEditCallback* tec = te->getTextEdit ();
			NSView* frameView = te->getParent ();
			NSRect editFrameRect = nsRectFromCRect (tec->platformGetSize ());
			NSView* containerView = [[NSView alloc] initWithFrame:editFrameRect];
			[containerView setAutoresizesSubviews:YES];

			if ([frameView wantsLayer])
			{
				containerView.wantsLayer = YES;
				double maxZPosition = -1.;
				for (CALayer* layer in frameView.layer.sublayers)
				{
					double zPosition = layer.zPosition;
					if (zPosition > maxZPosition)
						maxZPosition = zPosition;
				}
				[containerView layer].zPosition = static_cast<CGFloat> (maxZPosition + 1);
			}

			CPoint textInset = tec->platformGetTextInset ();

			editFrameRect.origin.x = static_cast<CGFloat> (textInset.x / 2. - 1.);
			editFrameRect.origin.y = static_cast<CGFloat> (textInset.y / 2.);
			editFrameRect.size.width -= textInset.x / 2.;
			editFrameRect.size.height -= textInset.y / 2. - 1.;
			self = Base::makeInstance (self).template callSuper<id (id, SEL, NSRect), id> (
				@selector (initWithFrame:), editFrameRect);
			if (!self)
			{
				[containerView release];
				return nil;
			}
			auto obj = Base::makeInstance (self);
			if (auto var = obj.template getVariable<IPlatformTextEditCallback*> (textEditVarName))
				var->set (tec);

			CoreTextFont* ctf = tec->platformGetFont ()->getPlatformFont ().cast<CoreTextFont> ();
			if (ctf)
			{
				CTFontRef fontRef = ctf->getFontRef ();
				if (fontRef)
				{
					CTFontDescriptorRef fontDesc = CTFontCopyFontDescriptor (fontRef);

					[self setFont:[NSFont fontWithDescriptor:(NSFontDescriptor*)fontDesc
														size:ctf->getSize ()]];
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
			[self performSelector:@selector (syncSize)];
			[frameView addSubview:containerView];

			NSTextFieldCell* cell = [self cell];
			[cell setDrawsBackground:NO];
			[cell setLineBreakMode:NSLineBreakByClipping];
			[cell setScrollable:YES];
			if (tec->platformGetHoriTxtAlign () == kCenterText)
				[cell setAlignment:MacTextAlignment::Center];
			else if (tec->platformGetHoriTxtAlign () == kRightText)
				[cell setAlignment:MacTextAlignment::Right];
			if (placeholder.length > 0)
			{
				CColor color = tec->platformGetFontColor ();
				color.alpha /= 2;
				NSMutableParagraphStyle* paragraphStyle =
					[[[NSMutableParagraphStyle alloc] init] autorelease];
				if (tec->platformGetHoriTxtAlign () == kCenterText)
					paragraphStyle.alignment = MacTextAlignment::Center;
				else if (tec->platformGetHoriTxtAlign () == kRightText)
					paragraphStyle.alignment = MacTextAlignment::Right;
				NSDictionary* attrDict = [NSDictionary
					dictionaryWithObjectsAndKeys:[self font], NSFontAttributeName,
												 nsColorFromCColor (color),
												 NSForegroundColorAttributeName, paragraphStyle,
												 NSParagraphStyleAttributeName, nil];
				NSAttributedString* as =
					[[[NSAttributedString alloc] initWithString:placeholder
													 attributes:attrDict] autorelease];
				[cell setPlaceholderAttributedString:as];
			}

			[self setDelegate:self];
			[self setNextKeyView:frameView];

			if (auto tv = static_cast<NSTextView*> ([[self window] fieldEditor:YES forObject:self]))
				tv.insertionPointColor = nsColorFromCColor (tec->platformGetFontColor ());

			if ([frameView respondsToSelector:@selector (makeSubViewFirstResponder:)])
				[frameView performSelector:@selector (makeSubViewFirstResponder:) withObject:self];

			dispatch_after (dispatch_time (DISPATCH_TIME_NOW, (int64_t) (1 * NSEC_PER_MSEC)),
							dispatch_get_main_queue (), ^{
								[[self window] makeFirstResponder:self];
							});
		}
		return self;
	}

	//------------------------------------------------------------------------------------
	static void SyncSize (id self, SEL _cmd)
	{
		if (auto te = Base::makeInstance (self).template getVariable<IPlatformTextEditCallback*> (
				textEditVarName))
		{
			auto textEdit = te->get ();
			if (!textEdit)
				return;
			NSView* containerView = [self superview];
			CRect rect (textEdit->platformGetVisibleSize ());
			rect.makeIntegral ();

			[containerView setFrame:nsRectFromCRect (rect)];

			rect.extend (15, -15);
			[[containerView superview] setNeedsDisplayInRect:nsRectFromCRect (rect)];
		}
	}

	//------------------------------------------------------------------------------------
	static void RemoveFromSuperview (id self, SEL _cmd)
	{
		auto obj = Base::makeInstance (self);
		if (auto var = obj.template getVariable<IPlatformTextEditCallback*> (textEditVarName))
			var->set (nullptr);
		NSView* containerView = [self superview];
		if (containerView)
		{
			[[containerView window] makeFirstResponder:[containerView superview]];
			[containerView removeFromSuperview];
			// [super removeFromSuperview];
			obj.template callSuper<void (id, SEL)> (_cmd);
			[containerView release];
		}
	}

	//------------------------------------------------------------------------------------
	static void TextDidChange (id self, SEL _cmd, NSNotification* notification)
	{
		auto obj = Base::makeInstance (self);
		if (auto var = obj.template getVariable<IPlatformTextEditCallback*> (textEditVarName))
		{
			if (auto te = var->get ())
				te->platformTextDidChange ();
		}
		obj.template callSuper<void (id, SEL, NSNotification*)> (_cmd, notification);
	}

	//------------------------------------------------------------------------------------
	static BOOL DoCommandBySelector (id self, SEL _cmd, NSControl* control, NSTextView* textView,
									 SEL commandSelector)
	{
		auto var = Base::makeInstance (self).template getVariable<IPlatformTextEditCallback*> (
			textEditVarName);
		if (!var)
			return NO;
		IPlatformTextEditCallback* tec = var->get ();
		if (!tec)
			return NO;
		if (commandSelector == @selector (insertNewline:))
		{
			KeyboardEvent event;
			event.type = EventType::KeyDown;
			event.virt = VirtualKey::Return;
			tec->platformOnKeyboardEvent (event);
			if (event.consumed)
				return YES;
		}
		else if (commandSelector == @selector (insertTab:))
		{
			KeyboardEvent event;
			event.type = EventType::KeyDown;
			event.virt = VirtualKey::Tab;
			tec->platformOnKeyboardEvent (event);
			if (event.consumed)
				return YES;
		}
		else if (commandSelector == @selector (insertBacktab:))
		{
			KeyboardEvent event;
			event.type = EventType::KeyDown;
			event.virt = VirtualKey::Tab;
			event.modifiers.add (ModifierKey::Shift);
			tec->platformOnKeyboardEvent (event);
			if (event.consumed)
				return YES;
		}
		else if (commandSelector == @selector (cancelOperation:))
		{
			KeyboardEvent event;
			event.type = EventType::KeyDown;
			event.virt = VirtualKey::Escape;
			tec->platformOnKeyboardEvent (event);
			if (event.consumed)
				return YES;
		}
		return NO;
	}
};

using VSTGUI_NSTextField = VSTGUI_NSTextFieldT<false>;
using VSTGUI_NSTextField_Secure = VSTGUI_NSTextFieldT<true>;

//-----------------------------------------------------------------------------
CocoaTextEdit::CocoaTextEdit (NSView* parent, IPlatformTextEditCallback* textEdit)
: IPlatformTextEdit (textEdit)
, platformControl (nullptr)
, parent (parent)
{
	if (textEdit->platformIsSecureTextEdit ())
		platformControl = [VSTGUI_NSTextField_Secure::alloc () initWithTextEdit:(id)this];
	else
		platformControl = [VSTGUI_NSTextField::alloc () initWithTextEdit:(id)this];
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

//-----------------------------------------------------------------------------
} // VSTGUI

#endif // MAC_COCOA
