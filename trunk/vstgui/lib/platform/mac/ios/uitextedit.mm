//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins :
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

#import "uitextedit.h"
#import <UIKit/UIKit.h>

#if TARGET_OS_IPHONE

#import "../cfontmac.h"
#import "../macglobals.h"

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
	CoreTextFont* ctf = dynamic_cast<CoreTextFont*>(textEdit->platformGetFont ()->getPlatformFont ());
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
	delegate = nil;
#if !ARC_ENABLED
	[delegate release];
#endif
}

//------------------------------------------------------------------------------------
UTF8StringPtr UITextEdit::getText ()
{
	if (platformControl)
	{
		NSString* text = [platformControl text];
		return [text UTF8String];
	}
	return 0;
}

//------------------------------------------------------------------------------------
bool UITextEdit::setText (UTF8StringPtr text)
{
	if (platformControl)
	{
		NSString* t = [NSString stringWithUTF8String:text];
		[platformControl setText:t];
	}
	return false;
}

//------------------------------------------------------------------------------------
bool UITextEdit::updateSize ()
{
	return false;
}

} // namespace

#endif
