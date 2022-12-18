// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#import "externalview_nsview.h"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <functional>
#import <mutex>

//------------------------------------------------------------------------
using VSTGUIMetalLayerDelegateDrawCallback = std::function<void ()>;
using VSTGUIMetalViewScreenChangedCallack = std::function<void (NSScreen*)>;

@interface NSObject ()
- (void)setDrawCallback:(const VSTGUIMetalLayerDelegateDrawCallback&)callback;
- (void)setScreenChangedCallback:(const VSTGUIMetalViewScreenChangedCallack&)callback;
@end

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ExternalView {

//------------------------------------------------------------------------
struct IMetalView
{
	virtual ~IMetalView () noexcept = default;

	virtual void render () = 0;
};

//------------------------------------------------------------------------
/** metal render interface to be used as the renderer of the MetalView
 *
 *	The renderer has to set the metal device of the metal layer before it can draw to it.
 */
struct IMetalRenderer
{
	virtual ~IMetalRenderer () noexcept = default;

	virtual bool init (IMetalView* metalView, CAMetalLayer* metalLayer) = 0;
	virtual void draw (id<CAMetalDrawable> drawable) = 0;
	virtual void onSizeUpdate (int32_t width, int32_t height, double scaleFactor) = 0;
	virtual void onAttached () = 0;
	virtual void onRemoved () = 0;
	virtual void onScreenChanged (NSScreen* screen) = 0;
};

using MetalRendererPtr = std::shared_ptr<IMetalRenderer>;

//------------------------------------------------------------------------
struct MetalLayerDelegate : RuntimeObjCClass<MetalLayerDelegate>
{
	static constexpr auto CallbackVarName = "callback";

	static Class CreateClass ()
	{
		return ObjCClassBuilder ()
			.init ("MetalLayerDelegate", [NSObject class])
			.addMethod (@selector (displayLayer:), displayLayer)
			.addMethod (@selector (actionForLayer:forKey:), actionForLayer)
			.addMethod (@selector (setDrawCallback:), setCallback)
			.addProtocol ("CALayerDelegate")
			.addIvar<VSTGUIMetalLayerDelegateDrawCallback> (CallbackVarName)
			.finalize ();
	}

	static void setCallback (id self, SEL cmd, VSTGUIMetalLayerDelegateDrawCallback callback)
	{
		auto instance = makeInstance (self);
		if (auto var = instance.getVariable<VSTGUIMetalLayerDelegateDrawCallback> (CallbackVarName))
			var->set (callback);
	}

	static void displayLayer (id self, SEL cmd, CALayer* layer)
	{
		auto instance = makeInstance (self);
		if (auto var = instance.getVariable<VSTGUIMetalLayerDelegateDrawCallback> (CallbackVarName))
		{
			if (auto callback = var->get ())
				callback ();
		}
	}

	static id<CAAction> actionForLayer (CALayer* layer, NSString* key) { return [NSNull null]; }
};

//------------------------------------------------------------------------
struct MetalNSView : RuntimeObjCClass<MetalNSView>
{
	static constexpr auto CallbackVarName = "callback";

	static Class CreateClass ()
	{
		return ObjCClassBuilder ()
			.init ("MetalNSView", [NSView class])
			.addIvar<VSTGUIMetalViewScreenChangedCallack> (CallbackVarName)
			.addMethod (@selector (viewDidMoveToWindow), viewDidMoveToWindow)
			.addMethod (@selector (viewWillMoveToWindow:), viewWillMoveToWindow)
			.addMethod (@selector (windowDidChangeScreen:), windowDidChangeScreen)
			.addMethod (@selector (setScreenChangedCallback:), setCallback)
			.finalize ();
	}

	static void setCallback (id self, SEL cmd, VSTGUIMetalViewScreenChangedCallack callback)
	{
		auto instance = makeInstance (self);
		if (auto var = instance.getVariable<VSTGUIMetalViewScreenChangedCallack> (CallbackVarName))
			var->set (callback);
	}

	static void viewDidMoveToWindow (id self, SEL cmd)
	{
		windowDidChangeScreen (self, cmd, nullptr);
		makeInstance (self).callSuper<void ()> (cmd);
	}

	static void viewWillMoveToWindow (id self, SEL cmd, NSWindow* window)
	{
		if (auto prevWindow = [self window])
		{
			[NSNotificationCenter.defaultCenter removeObserver:self];
		}
		if (window)
		{
			[NSNotificationCenter.defaultCenter addObserver:self
												   selector:@selector (windowDidChangeScreen:)
													   name:NSWindowDidChangeScreenNotification
													 object:window];
		}
		makeInstance (self).callSuper<void (NSWindow*)> (cmd, window);
	}

	static void windowDidChangeScreen (id self, SEL cmd, NSNotification* n)
	{
		if (NSScreen* screen = [[self window] screen])
		{
			auto instance = makeInstance (self);
			if (auto var =
					instance.getVariable<VSTGUIMetalViewScreenChangedCallack> (CallbackVarName))
			{
				if (auto callback = var->get ())
					callback (screen);
			}
		}
	}
};

//------------------------------------------------------------------------
struct MetalView : ExternalNSViewBase<NSView>,
				   IMetalView
{
	/** make a new metal view.
	 *
	 *	The metal view can render on a background thread (only use one thread for rendering) or on
	 *	the main thread.
	 *	Rendering and view resizing is automatically guarded by a mutex.
	 *	The view will automatically trigger a rendering when the view is resized.
	 */
	static std::shared_ptr<MetalView> make (const MetalRendererPtr& renderer)
	{
		if (!renderer)
			return {};
		if (auto metalView = std::shared_ptr<MetalView> (new MetalView (renderer)))
		{
			if (renderer->init (metalView.get (), metalView->metalLayer))
				return metalView;
		}
		return {};
	}

	/** immediately render the view [thread safe] */
	void render () override
	{
		doLocked ([&] () { renderer->draw (metalLayer.nextDrawable); });
	}

	/** do something locked [thread safe] */
	template<typename Proc>
	void doLocked (Proc proc)
	{
		LockGuard g (mutex);
		@autoreleasepool
		{
			proc ();
		}
	}

private:
	CAMetalLayer* metalLayer {nullptr};
	id metalLayerDelegate {nullptr};
	double contentScaleFactor {1.};
	using Mutex = std::recursive_mutex;
	using LockGuard = std::lock_guard<Mutex>;
	Mutex mutex;
	MetalRendererPtr renderer;

	MetalView (const MetalRendererPtr& renderer)
	: Base ([MetalNSView::alloc () init]), renderer (renderer)
	{
		metalLayerDelegate = [MetalLayerDelegate::alloc () init];
		metalLayer = [CAMetalLayer new];
		metalLayer.delegate = metalLayerDelegate;
		view.layer = metalLayer;
		metalLayer.needsDisplayOnBoundsChange = YES;
		metalLayer.geometryFlipped = YES;
		metalLayer.opaque = NO;
		metalLayer.contentsGravity = kCAGravityBottomLeft;
		[metalLayerDelegate setDrawCallback:[this] () {
			render ();
		}];
		[view setScreenChangedCallback:[this] (NSScreen* screen) {
			this->renderer->onScreenChanged (screen);
		}];
	}

	bool attach (void* parent, PlatformViewType parentViewType) override
	{
		if (Base::attach (parent, parentViewType))
		{
			renderer->onAttached ();
			return true;
		}
		return false;
	}

	bool remove () override
	{
		if (Base::remove ())
		{
			renderer->onRemoved ();
			return true;
		}
		return false;
	}

	void setContentScaleFactor (double scaleFactor) override
	{
		contentScaleFactor = scaleFactor;
		metalLayer.contentsScale = scaleFactor;
		[metalLayer setNeedsDisplay];
		onSizeUpdate ();
	}

	void setViewSize (IntRect frame, IntRect visible) override
	{
		Base::setViewSize (frame, visible);
		onSizeUpdate ();
	}

	void onSizeUpdate ()
	{
		doLocked ([this] () {
			auto size = view.frame.size;
			metalLayer.drawableSize =
				NSMakeSize (size.width * contentScaleFactor, size.height * contentScaleFactor);
			renderer->onSizeUpdate (size.width, size.height, contentScaleFactor);
		});
	}

#if !__has_feature(objc_arc)
public:
	~MetalView () noexcept override
	{
		[metalLayerDelegate release];
		[metalLayer release];
	}
#endif
};

//------------------------------------------------------------------------
} // ExternalView
} // VSTGUI
