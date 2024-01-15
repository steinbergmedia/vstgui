// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#import "../lib/platform/mac/cocoa/objcclassbuilder.h"
#import "../lib/iexternalview.h"
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
 *	to create it call [ExternalViewContainerNSView::alloc () initWithFrame: rect]
 */
struct ExternalViewContainerNSView : RuntimeObjCClass<ExternalViewContainerNSView>
{
	static constexpr auto TookFocusCallbackVarName = "TookFocusCallback";

	static Class CreateClass ()
	{
		return ObjCClassBuilder ()
			.init ("ExternalViewContainerNSView", [NSView class])
			.addMethod (@selector (isFlipped), isFlipped)
			.addMethod (@selector (viewWillMoveToWindow:), viewWillMoveToWindow)
			.addMethod (@selector (observeValueForKeyPath:ofObject:change:context:),
						observeValueForKeyPath)
			.addIvar<IView::TookFocusCallback> (TookFocusCallbackVarName)
			.finalize ();
	}

	static BOOL isFlipped (id self, SEL cmd) { return YES; }
	static void viewWillMoveToWindow (id self, SEL _cmd, NSWindow* window)
	{
		if ([self window] && [self window] != window)
		{
			[[self window] removeObserver:self forKeyPath:@"firstResponder"];
		}
		if (window)
		{
			[window addObserver:self forKeyPath:@"firstResponder" options:0 context:nullptr];
		}
	}

	static void observeValueForKeyPath (id self, SEL cmd, NSString* keyPath, id object,
										NSDictionary<NSKeyValueChangeKey, id>* change,
										void* context)
	{
		if ([keyPath isEqualToString:@"firstResponder"])
		{
			auto view = [self window].firstResponder;
			if ([view isKindOfClass:[NSView class]] &&
				[static_cast<NSView*> (view) isDescendantOf:self])
			{
				if (auto var = makeInstance (self).getVariable<IView::TookFocusCallback> (
						TookFocusCallbackVarName))
				{
					if (var.value ().get ())
						var.value ().get () ();
				}
			}
		}
	}
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
struct ExternalNSViewBase : ViewAdapter
{
	using Base = ExternalNSViewBase<ViewType>;
	using PlatformViewType = ExternalView::PlatformViewType;
	using IntRect = ExternalView::IntRect;

	NSView* container {[ExternalViewContainerNSView::alloc () initWithFrame: {0., 0., 10., 10.}]};
	ViewType* view {nullptr};

	ExternalNSViewBase (ViewType* inView) : view (inView)
	{
		if (@available (macOS 14, *))
		{
#ifdef MAC_OS_VERSION_14_0
			// only available when building with the mac os sdk 14.0
			container.clipsToBounds = YES;
#else
			// but necessary to set to YES on macOS 14 even when not building with Xcode 15
			if ([container respondsToSelector:@selector (setClipsToBounds:)])
			{
				BOOL clipsToBounds = YES;
				auto* signature = [[container class]
					instanceMethodSignatureForSelector:@selector (setClipsToBounds:)];
				auto* invocation = [NSInvocation invocationWithMethodSignature:signature];
				invocation.target = container;
				invocation.selector = @selector (setClipsToBounds:);
				[invocation setArgument:&clipsToBounds atIndex:2];
				[invocation invoke];
			}
#endif
		}
		[container addSubview:view];
	}

#if !__has_feature(objc_arc)
	virtual ~ExternalNSViewBase () noexcept
	{
		[container release];
		[view release];
	}
#endif

	bool platformViewTypeSupported (PlatformViewType type) override
	{
		return type == PlatformViewType::NSView;
	}

	bool attach (void* parent, PlatformViewType parentViewType) override
	{
		if (!parent || parentViewType != PlatformViewType::NSView)
			return false;
		auto parentNSView = (__bridge NSView*)parent;
		[parentNSView addSubview:container];
		return true;
	}

	bool remove () override
	{
		[container removeFromSuperview];
		return true;
	}

	void setViewSize (IntRect frame, IntRect visible) override
	{
		container.frame = toNSRect (visible);
		frame.origin.x -= visible.origin.x;
		frame.origin.y -= visible.origin.y;
		view.frame = toNSRect (frame);
	}

	void setContentScaleFactor (double scaleFactor) override {}

	void setMouseEnabled (bool state) override
	{
		if ([view respondsToSelector:@selector (setEnabled:)])
			[(id)view setEnabled:state];
	}

	void takeFocus () override
	{
		if (view.acceptsFirstResponder)
		{
			if (auto window = view.window)
				[window makeFirstResponder:view];
		}
	}

	void looseFocus () override
	{
		if (auto window = view.window)
			[window makeFirstResponder:container.superview];
	}

	void setTookFocusCallback (const TookFocusCallback& callback) override
	{
		if (auto var = ObjCInstance (container).getVariable<TookFocusCallback> (
				ExternalViewContainerNSView::TookFocusCallbackVarName))
		{
			var->set (callback);
		}
	}
};

//------------------------------------------------------------------------
} // ExternalView
} // VSTGUI
