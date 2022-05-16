// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "uitextedit.h"
#import <UIKit/UIKit.h>

#if TARGET_OS_IPHONE

#import "../cfontmac.h"
#import "../macglobals.h"
#import "../macstring.h"

#if __has_feature(objc_arc) && __clang_major__ >= 3
#define ARC_ENABLED 1
#endif // __has_feature(objc_arc)

//------------------------------------------------------------------------------------
@interface VSTGUI_UITextFieldDelegate : NSObject<UITextFieldDelegate>
//------------------------------------------------------------------------------------
{
	VSTGUI::IPlatformTextEditCallback* textEdit;
}
@end

//------------------------------------------------------------------------------------
@implementation VSTGUI_UITextFieldDelegate
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
- (id)initWithUITextEdit:(VSTGUI::IPlatformTextEditCallback*)_textEdit
{
	self = [super init];
	if (self)
		textEdit = _textEdit;
	return self;
}

//------------------------------------------------------------------------------------
- (void)looseFocus
{
	if (textEdit)
	{
		textEdit->platformLooseFocus (false);
	}
}

//------------------------------------------------------------------------------------
- (BOOL)textFieldShouldEndEditing:(UITextField *)textField
{
	if (textEdit)
	{
		[self performSelector:@selector(looseFocus) withObject:nil afterDelay:0];
	}
	return YES;
}

//------------------------------------------------------------------------------------
- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
	VSTGUI::IPlatformTextEditCallback* tmp = textEdit;
	textEdit = 0;
	tmp->platformLooseFocus (true);
	return YES;
}

@end

//------------------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------------------
UITextEdit::UITextEdit (UIView* parent, IPlatformTextEditCallback* textEdit)
: IPlatformTextEdit (textEdit)
, platformControl (0)
, parent (parent)
{
	delegate = [[VSTGUI_UITextFieldDelegate alloc] initWithUITextEdit:textEdit];

	CRect rect (textEdit->platformGetSize ());
	CPoint textInset = textEdit->platformGetTextInset ();
	CGRect r = CGRectFromCRect (rect);
	r.origin.x += textInset.x / 2.;
	r.origin.y += textInset.y / 2.;
	r.size.width -= textInset.x / 2;
	r.size.height -= textInset.y / 2;
	platformControl = [[UITextField alloc] initWithFrame:r];
	
	bool fontSet = false;
	CoreTextFont* ctf = textEdit->platformGetFont ()->getPlatformFont ().cast<CoreTextFont> ();
	if (ctf)
	{
		CTFontRef fontRef = ctf->getFontRef ();
		if (fontRef)
		{
			CTFontDescriptorRef fontDesc = CTFontCopyFontDescriptor (fontRef);
			[platformControl setFont:[UIFont fontWithDescriptor:(__bridge UIFontDescriptor *)fontDesc size:0]];
			CFRelease (fontDesc);
			fontSet = true;
		}
	}
	if (!fontSet)
	{
		NSString* fontName = [NSString stringWithCString:textEdit->platformGetFont ()->getName () encoding:NSUTF8StringEncoding];
		[platformControl setFont:[UIFont fontWithName:fontName size:static_cast<CGFloat> (textEdit->platformGetFont ()->getSize ())]];
	}
	CColor fontColor = textEdit->platformGetFontColor ();
	platformControl.textColor = [UIColor colorWithRed:fontColor.red / 255.f green:fontColor.green / 255.f blue:fontColor.red / 255.f alpha:fontColor.alpha / 255.f];
	platformControl.borderStyle = UITextBorderStyleNone;
	platformControl.opaque = NO;
	platformControl.clearsContextBeforeDrawing = YES;
	NSTextAlignment textAlignment;
	switch (textEdit->platformGetHoriTxtAlign ())
	{
		case kLeftText: textAlignment = NSTextAlignmentLeft; break;
		case kCenterText: textAlignment = NSTextAlignmentCenter; break;
		case kRightText:textAlignment = NSTextAlignmentRight; break;
	}
	platformControl.textAlignment = textAlignment;
	platformControl.contentVerticalAlignment = UIControlContentVerticalAlignmentCenter;
	platformControl.clearButtonMode = UITextFieldViewModeNever;
	platformControl.returnKeyType = UIReturnKeyDefault;
	platformControl.enablesReturnKeyAutomatically = YES;
	platformControl.delegate = delegate;

	setText (textEdit->platformGetText ());

	[parent addSubview:platformControl];

	[platformControl becomeFirstResponder];
}

//------------------------------------------------------------------------------------
UITextEdit::~UITextEdit ()
{
	if (platformControl)
	{
		[platformControl removeFromSuperview];
	#if !ARC_ENABLED
		[platformControl release];
	#endif
		platformControl = nil;
	}
#if !ARC_ENABLED
	[delegate release];
#endif
	delegate = nil;
}

//------------------------------------------------------------------------------------
UTF8String UITextEdit::getText ()
{
	if (platformControl)
	{
		NSString* text = [platformControl text];
		return [text UTF8String];
	}
	return 0;
}

//------------------------------------------------------------------------------------
bool UITextEdit::setText (const UTF8String& text)
{
	if (platformControl == nullptr)
		return false;
	if (NSString* t = fromUTF8String<NSString*> (text))
	{
		[platformControl setText:t];
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------------
bool UITextEdit::updateSize ()
{
	return false;
}

} // VSTGUI

#endif
