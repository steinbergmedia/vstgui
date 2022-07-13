#pragma once

#import "../lib/platform/mac/cocoa/objcclassbuilder.h"
#import <Cocoa/Cocoa.h>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ExternalView {

//------------------------------------------------------------------------
inline NSRect toNSRect (const IntRect& r)
{
	return NSMakeRect (r.origin.x, r.origin.y, r.size.width, r.size.height);
}

//------------------------------------------------------------------------
struct FlippedNSView : RuntimeObjCClass<FlippedNSView>
{
	static Class CreateClass ()
	{
		return ObjCClassBuilder ()
			.init ("FlippedNSView", [NSView class])
			.addMethod (@selector (isFlipped), isFlipped)
			.finalize ();
	}

	static BOOL isFlipped (id self, SEL cmd) { return YES; }
};

//------------------------------------------------------------------------
template<typename ViewType>
struct ExternalNSViewBase
{
	using Base = ExternalNSViewBase<ViewType>;

	NSView* container {[FlippedNSView::alloc () initWithFrame: {0., 0., 10., 10.}]};
	ViewType* view {nullptr};

	ExternalNSViewBase (ViewType* inView) : view (inView) { [container addSubview:view]; }

#if !__has_feature(objc_arc)
	virtual ~ExternalNSViewBase () noexcept
	{
		[container release];
		[view release];
	}
#endif

	bool platformViewTypeSupported (PlatformViewType type)
	{
		return type == PlatformViewType::NSView;
	}

	bool attach (void* parent, PlatformViewType parentViewType)
	{
		if (!parent || parentViewType != PlatformViewType::NSView)
			return false;
		auto parentNSView = (__bridge NSView*)parent;
		[parentNSView addSubview:container];
		return true;
	}

	bool remove ()
	{
		[container removeFromSuperview];
		return true;
	}

	void setViewSize (IntRect frame, IntRect visible)
	{
		container.frame = toNSRect (visible);
		frame.origin.x -= visible.origin.x;
		frame.origin.y -= visible.origin.y;
		view.frame = toNSRect (frame);
	}

	void setContentScaleFactor (double scaleFactor) {}

	void setMouseEnabled (bool state) { view.enabled = state; }

	void takeFocus ()
	{
		if (auto window = view.window)
			[window makeFirstResponder:view];
	}

	void looseFocus () {}
};

//------------------------------------------------------------------------
} // ExternalView
} // VSTGUI
