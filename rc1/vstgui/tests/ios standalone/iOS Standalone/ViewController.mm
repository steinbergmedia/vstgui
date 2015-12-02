//
//  ViewController.m
//  iOS Standalone
//
//  Created by Arne Scheffler on 12/09/14.
//
//

#import "ViewController.h"
#import "vstgui/vstgui.h"
#import "vstgui/vstgui_uidescription.h"
#import "vstgui/lib/platform/mac/macglobals.h"

using namespace VSTGUI;

//------------------------------------------------------------------------
@interface ViewController ()
{
	SharedPointer<CFrame> frame;
	SharedPointer<UIDescription> uiDesc;
}
@end

//------------------------------------------------------------------------
@implementation ViewController
//------------------------------------------------------------------------
- (std::string)templateViewName
{
	CRect size = CRectFromCGRect (self.view.bounds);
	std::stringstream str;
	str << static_cast<int> (size.getWidth ()) << "x" << static_cast<int> (size.getHeight ());
	return str.str ();
}

//------------------------------------------------------------------------
- (void)viewDidLoad
{
    [super viewDidLoad];

	uiDesc = owned (new UIDescription (CResourceDescription ("ios_test.uidesc")));
	if (!uiDesc->parse ())
	{
		@throw [NSException exceptionWithName:@"" reason:@"" userInfo:nil];
	}

	CRect size = CRectFromCGRect (self.view.bounds);
	frame = owned (new CFrame (size, nullptr));
	frame->open ((__bridge void*) (self.view));

	[self didRotateFromInterfaceOrientation:self.interfaceOrientation];
}

//------------------------------------------------------------------------
- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
	frame->setSize (self.view.bounds.size.width, self.view.bounds.size.height);
	frame->removeAll ();

	SharedPointer<CView> view = uiDesc->createView ([self templateViewName].c_str (), nullptr);
	if (view == nullptr)
		@throw [NSException exceptionWithName:@"" reason:@"" userInfo:nil];
	
	view->setMouseableArea (frame->getViewSize ());
	view->setViewSize (frame->getViewSize ());
	frame->addView (view);
}

@end
