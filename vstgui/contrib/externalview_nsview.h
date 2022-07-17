// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
/** a NSView that has a flipped coordinate system (top-left is {0, 0}, not AppKits default which is
 *	bottom-left)
 *
 *	to create it call [FlippedNSView::alloc () initWithFrame: rect]
 */
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
/** a template helper class for embedding NSViews into VSTGUI via ExternalView::IView
 *
 *	Example to add a simple NSView:
 *
 *	// Header: ExampleNSView.h
 *
 *	class ExampleNSView : IView
 *	{
 *	public:
 *  	ExampleNSView ();
 *  	~ExampleNSView () noexcept;
 *
 *	private:
 *  	bool platformViewTypeSupported (PlatformViewType type) override;
 *  	bool attach (void* parent, PlatformViewType parentViewType) override;
 *  	bool remove () override;
 *  	void setViewSize (IntRect frame, IntRect visible) override;
 *  	void setContentScaleFactor (double scaleFactor) override;
 *  	void setMouseEnabled (bool state) override;
 *  	void takeFocus () override;
 *  	void looseFocus () override;
 *
 *  	struct Impl;
 *  	std::unique_ptr<Impl> impl;
 *	};
 *
 *	// Source: ExampleNSView.mm
 *
 *	#import "ExampleNSView.h"
 *	#import "externalview_nsview.h"
 *
 *	struct ExampleNSView::Impl : ExternalNSViewBase<NSView>
 *	{
 *  	Impl () : Base ([NSView new])
 *  	{
 *  		// configure the view here
 *  		view.alphaValue = 0.5;
 *  	}
 *	};
 *
 *	ExampleNSView::ExampleNSView () { impl = std::make_unique<Impl> (); }
 *	ExampleNSView::~ExampleNSView () noexcept = default;
 *	bool ExampleNSView::platformViewTypeSupported (PlatformViewType type)
 *	{
 *  	return impl->platformViewTypeSupported (type);
 *	}
 *	bool ExampleNSView::attach (void* parent, PlatformViewType parentViewType)
 *	{
 *  	return impl->attach (parent, parentViewType);
 *	}
 *	bool ExampleNSView::remove () { return impl->remove (); }
 *	void ExampleNSView::setViewSize (IntRect frame, IntRect visible)
 *	{
 *  	impl->setViewSize (frame, visible);
 *	}
 *	void ExampleNSView::setContentScaleFactor (double scaleFactor)
 *	{
 *  	impl->setContentScaleFactor (scaleFactor);
 *	}
 *	void ExampleNSView::setMouseEnabled (bool state) { impl->setMouseEnabled (state); }
 *	void ExampleNSView::takeFocus () { impl->takeFocus (); }
 *	void ExampleNSView::looseFocus () { impl->looseFocus (); }
 *
 */
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
