// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#import "externalview_nsview.h"
#import "../lib/vstguibase.h"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <functional>
#import <mutex>
#import <semaphore>
#import <AvailabilityMacros.h>

#if defined(MAC_OS_VERSION_14_0) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_VERSION_14_0
#define VSTGUI_CA_METAL_DISPLAY_LINK
#endif

#ifdef VSTGUI_CA_METAL_DISPLAY_LINK
#import <QuartzCore/CAMetalDisplayLink.h>
#define VSTGUI_CA_METAL_DISPLAY_LINK

//------------------------------------------------------------------------
using VSTGUIMetalDisplayLinkDelegateNeedsUpdateCallback =
	std::function<void (id<CAMetalDrawable>, CFTimeInterval targetTimestamp,
						CFTimeInterval targetPresentationTimestamp)>;

@interface NSObject ()
- (void)setUpdateCallback:(const VSTGUIMetalDisplayLinkDelegateNeedsUpdateCallback&)callback;
@end
//------------------------------------------------------------------------
#endif

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
	virtual void draw (id<CAMetalDrawable> drawable, CFTimeInterval targetTimestamp,
					   CFTimeInterval targetPresentationTimestamp) = 0;
	virtual void onSizeUpdate (int32_t width, int32_t height, double scaleFactor) = 0;
	virtual void onAttached () = 0;
	virtual void onRemoved () = 0;
	virtual void onScreenChanged (NSScreen* screen) = 0;
};

using MetalRendererPtr = std::shared_ptr<IMetalRenderer>;

//------------------------------------------------------------------------
namespace Detail {

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

#ifdef VSTGUI_CA_METAL_DISPLAY_LINK
//------------------------------------------------------------------------
struct MetalDisplayLinkDelegate : RuntimeObjCClass<MetalDisplayLinkDelegate>
{
	static constexpr auto CallbackVarName = "callback";

	static Class CreateClass ()
	{
		return ObjCClassBuilder ()
			.init ("MetalLayerDelegate", [NSObject class])
			.addMethod (@selector (metalDisplayLink:needsUpdate:), metalDisplayLinkNeedsUpdate)
			.addMethod (@selector (setUpdateCallback:), setUpdateCallback)
			.addProtocol ("CAMetalDisplayLinkDelegate")
			.addIvar<VSTGUIMetalDisplayLinkDelegateNeedsUpdateCallback> (CallbackVarName)
			.finalize ();
	}

	static void setUpdateCallback (id self, SEL cmd,
								   VSTGUIMetalDisplayLinkDelegateNeedsUpdateCallback callback)
	{
		auto instance = makeInstance (self);
		if (auto var = instance.getVariable<VSTGUIMetalDisplayLinkDelegateNeedsUpdateCallback> (
				CallbackVarName))
			var->set (callback);
	}

	static void metalDisplayLinkNeedsUpdate (id self, SEL cmd, CAMetalDisplayLink* link,
											 CAMetalDisplayLinkUpdate* update)
	{
		auto instance = makeInstance (self);
		if (auto var = instance.getVariable<VSTGUIMetalDisplayLinkDelegateNeedsUpdateCallback> (
				CallbackVarName))
		{
			if (auto callback = var->get ())
			{
				callback (update.drawable, update.targetTimestamp,
						  update.targetPresentationTimestamp);
			}
		}
	}
};
#endif

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
} // Detail

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
		if (auto metalView = std::shared_ptr<MetalView> (new MetalView (renderer, false)))
		{
			if (renderer->init (metalView.get (), metalView->metalLayer))
				return metalView;
		}
		return {};
	}

#ifdef VSTGUI_CA_METAL_DISPLAY_LINK
	/** make a metal view conditionally triggered via a display link and conditionally triggered on
	 * a background thread or the main thread.
	 *
	 * this is only available when the minimum deployment target is macOS 14.
	 */
	static std::shared_ptr<MetalView> make (const MetalRendererPtr& renderer, bool useDisplayLink,
											bool onMainThread = true)
	{
		if (!renderer)
			return {};
		if (auto metalView =
				std::shared_ptr<MetalView> (new MetalView (renderer, useDisplayLink, onMainThread)))
		{
			if (renderer->init (metalView.get (), metalView->metalLayer))
				return metalView;
		}
		return {};
	}

#endif

	/** immediately render the view [thread safe] */
	void render () override
	{
#ifdef VSTGUI_CA_METAL_DISPLAY_LINK
		vstgui_assert (_displayLink == nullptr,
					   "When using a display link, your renderer is automatically called when a "
					   "new frame needs to be rendered.");
		if (_displayLink)
			return;
#endif
		doLocked ([&] () { renderer->draw (metalLayer.nextDrawable, 0, 0); });
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

#ifdef VSTGUI_CA_METAL_DISPLAY_LINK
	CAMetalDisplayLink* _displayLink {nullptr};
	id _displayLinkDelegate {nullptr};
	NSRunLoop* _displayLinkRunLoop {nullptr};
	struct BackgroundThread
	{
		static NSRunLoop* getRunLoop () { return instance ()._runLoop; }

	private:
		static BackgroundThread& instance ()
		{
			static BackgroundThread thread;
			return thread;
		}

		BackgroundThread ()
		{
			_thread = [[NSThread alloc] initWithBlock:^{
				@autoreleasepool
				{
					_runLoop = [NSRunLoop currentRunLoop];
					semaphore.release ();
					[_runLoop addPort:_keepAlivePort forMode:NSDefaultRunLoopMode];
					while (doRunning)
					{
						[_runLoop runMode:NSDefaultRunLoopMode beforeDate:NSDate.distantFuture];
					}
					semaphore.release ();
				}
			}];
			semaphore.acquire ();
			_thread.name = @"MetalDisplayLink Background";
			[_thread start];
			semaphore.acquire ();
		}
		~BackgroundThread () noexcept
		{
			doRunning = false;
			if (_runLoop)
			{
				[_runLoop removePort:_keepAlivePort forMode:NSDefaultRunLoopMode];
				[_runLoop performBlock:^{
					[_thread cancel];
				}];
			}
			semaphore.acquire ();
#if !__has_feature(objc_arc)
			[_thread release];
			[_keepAlivePort release];
#endif
		}
		std::atomic_bool doRunning {true};
		NSThread* _thread {nullptr};
		NSRunLoop* _runLoop {nullptr};
		NSPort* _keepAlivePort {[NSMachPort port]};
		std::binary_semaphore semaphore {1};
	};
#endif

	MetalView (const MetalRendererPtr& renderer, bool useDisplayLink, bool onMainThread = true)
	: Base ([Detail::MetalNSView::alloc () init]), renderer (renderer)
	{
		metalLayerDelegate = [Detail::MetalLayerDelegate::alloc () init];
		metalLayer = [CAMetalLayer new];
		metalLayer.delegate = metalLayerDelegate;
		view.layer = metalLayer;
		metalLayer.needsDisplayOnBoundsChange = YES;
		metalLayer.geometryFlipped = YES;
		metalLayer.opaque = NO;
		metalLayer.contentsGravity = kCAGravityBottomLeft;
#ifdef VSTGUI_CA_METAL_DISPLAY_LINK
		if (useDisplayLink)
		{
			_displayLink = [[CAMetalDisplayLink alloc] initWithMetalLayer:metalLayer];
			_displayLinkDelegate = [Detail::MetalDisplayLinkDelegate::alloc () init];
			[_displayLinkDelegate
				setUpdateCallback:[this] (auto drawable, auto time, auto presentTime) {
					doLocked ([&] () { this->renderer->draw (drawable, time, presentTime); });
				}];
			_displayLink.delegate = _displayLinkDelegate;
			if (onMainThread)
			{
				_displayLinkRunLoop = [NSRunLoop mainRunLoop];
			}
			else
			{
				_displayLinkRunLoop = BackgroundThread::getRunLoop ();
			}
		}
		else
#endif
		{
			[metalLayerDelegate setDrawCallback:[this] () {
				render ();
			}];
		}
		[view setScreenChangedCallback:[this] (NSScreen* screen) {
			this->renderer->onScreenChanged (screen);
		}];
	}

	bool attach (void* parent, PlatformViewType parentViewType) override
	{
		if (Base::attach (parent, parentViewType))
		{
			renderer->onAttached ();
#ifdef VSTGUI_CA_METAL_DISPLAY_LINK
			if (_displayLink)
			{
				[_displayLink addToRunLoop:_displayLinkRunLoop forMode:NSRunLoopCommonModes];
				_displayLink.paused = NO;
			}
#endif
			return true;
		}
		return false;
	}

	bool remove () override
	{
#ifdef VSTGUI_CA_METAL_DISPLAY_LINK
		if (_displayLink)
		{
			_displayLink.paused = YES;
			[_displayLink removeFromRunLoop:_displayLinkRunLoop forMode:NSRunLoopCommonModes];
		}
#endif
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
#ifdef VSTGUI_CA_METAL_DISPLAY_LINK
		if (!_displayLink)
		{
			[metalLayer setNeedsDisplay];
		}
#else
		[metalLayer setNeedsDisplay];
#endif
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

public:
	~MetalView () noexcept override
	{
#ifdef VSTGUI_CA_METAL_DISPLAY_LINK
		[_displayLink invalidate];
#if !__has_feature(objc_arc)
		[_displayLink release];
		[_displayLinkDelegate release];
#endif
#endif

#if !__has_feature(objc_arc)
		[metalLayerDelegate release];
		[metalLayer release];
#endif
	}
};

//------------------------------------------------------------------------
} // ExternalView
} // VSTGUI
