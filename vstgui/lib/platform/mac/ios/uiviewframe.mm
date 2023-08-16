// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "uiviewframe.h"

#if TARGET_OS_IPHONE
#import "../../../cfileselector.h"
#import "../../../idatapackage.h"
#import "../../iplatformoptionmenu.h"
#import "../coregraphicsdevicecontext.h"
#import "../cgbitmap.h"
#import "../quartzgraphicspath.h"
#import "../caviewlayer.h"
#import "uitouchevent.h"
#import "uitextedit.h"
#import "uiopenglview.h"
#import <vector>

#if __has_feature(objc_arc) && __clang_major__ >= 3
#define ARC_ENABLED 1
#endif // __has_feature(objc_arc)

using namespace VSTGUI;

//-----------------------------------------------------------------------------
@interface VSTGUI_UIView : UIView
//-----------------------------------------------------------------------------
{
	UIViewFrame* uiViewFrame;
	UITouchEvent touchEvent;
}
- (id)initWithUIViewFrame:(UIViewFrame*)viewFrame parent:(UIView*)parent size:(const CRect*)size;

@end

//-----------------------------------------------------------------------------
@implementation VSTGUI_UIView
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
- (id)initWithUIViewFrame:(UIViewFrame*)viewFrame parent:(UIView*)parent size:(const CRect*)size
{
	self = [super initWithFrame:CGRectFromCRect (*size)];
	if (self)
	{
		self.multipleTouchEnabled = YES;
		self.exclusiveTouch = YES;
		uiViewFrame = viewFrame;
		[parent addSubview:self];
	}
	return self;
}

//-----------------------------------------------------------------------------
- (void)drawRect:(CGRect)rect
{
	auto device = getPlatformFactory ().getGraphicsDeviceFactory ().getDeviceForScreen (
		DefaultScreenIdentifier);
	if (!device)
		return;
	auto cgDevice = std::static_pointer_cast<CoreGraphicsDevice> (device);
	auto deviceContext = std::make_shared<CoreGraphicsDeviceContext> (
		*cgDevice.get (), UIGraphicsGetCurrentContext ());

	uiViewFrame->getFrame ()->platformDrawRects (deviceContext, self.layer.contentsScale,
												 {CRectFromCGRect (rect)});
}

//-----------------------------------------------------------------------------
- (BOOL)canBecomeFirstResponder
{
	return YES;
}

//-----------------------------------------------------------------------------
- (void)didMoveToWindow
{
	uiViewFrame->getFrame ()->platformOnActivate (self.window ? true : false);
}

//-----------------------------------------------------------------------------
- (void)updateTouchEvent:(NSSet*)touches
{
	ITouchEvent::TouchMap& touchMap = touchEvent.getTouchMap ();
	for (UITouch* touch in touches)
	{
		auto it = touchEvent.nativeTouches.find (touch);
		if (it != touchEvent.nativeTouches.end ())
		{
			auto iTouch = touchMap.find (it->second);
			vstgui_assert (iTouch != touchMap.end ());
			if (touch.phase == UITouchPhaseStationary)
			{
				iTouch->second.state = ITouchEvent::kNoChange;
			}
			else
			{
				iTouch->second.location = CPointFromCGPoint ([touch locationInView:self]);
				switch (touch.phase)
				{
					case UITouchPhaseMoved: iTouch->second.state = ITouchEvent::kMoved; break;
					case UITouchPhaseEnded: iTouch->second.state = ITouchEvent::kEnded; break;
					case UITouchPhaseCancelled: iTouch->second.state = ITouchEvent::kCanceled; break;
					default: iTouch->second.state = ITouchEvent::kNoChange; break;
				}
			}
		}
		else if (touch.phase == UITouchPhaseBegan)
		{
			int32_t touchID = touchEvent.touchCounter++;
			touchEvent.nativeTouches.insert (std::make_pair (touch, touchID));
			ITouchEvent::Touch t;
			t.timeStamp = touchEvent.currentTime;
			t.state = ITouchEvent::kBegan;
			t.location = CPointFromCGPoint ([touch locationInView:self]);
			t.tapCount = static_cast<uint32_t> (touch.tapCount);
			touchMap.insert (std::make_pair (touchID, t));
		}
	}
}

//-----------------------------------------------------------------------------
- (void)removeTouches:(NSSet*)touches
{
	ITouchEvent::TouchMap& touchMap = touchEvent.getTouchMap ();
	for (UITouch* touch in touches)
	{
		auto it = touchEvent.nativeTouches.find (touch);
		vstgui_assert (it != touchEvent.nativeTouches.end ());
		if (it != touchEvent.nativeTouches.end ())
		{
			touchMap.erase (it->second);
			touchEvent.nativeTouches.erase (it);
		}
	}
}

//-----------------------------------------------------------------------------
- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
	if (event.type == UIEventTypeTouches)
	{
		touchEvent.currentTime = event.timestamp;
		[self updateTouchEvent:[event allTouches]];
		uiViewFrame->getFrame()->platformOnTouchEvent (touchEvent);
	}
}

//-----------------------------------------------------------------------------
- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event
{
	if (event.type == UIEventTypeTouches)
	{
		touchEvent.currentTime = event.timestamp;
		[self updateTouchEvent:[event allTouches]];
		uiViewFrame->getFrame()->platformOnTouchEvent (touchEvent);
	}
}

//-----------------------------------------------------------------------------
- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event
{
	if (event.type == UIEventTypeTouches)
	{
		touchEvent.currentTime = event.timestamp;
		[self updateTouchEvent:[event allTouches]];
		uiViewFrame->getFrame()->platformOnTouchEvent (touchEvent);
		[self removeTouches:touches];
	}
}

//-----------------------------------------------------------------------------
- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event
{
	if (event.type == UIEventTypeTouches)
	{
		touchEvent.currentTime = event.timestamp;
		[self updateTouchEvent:[event allTouches]];
		uiViewFrame->getFrame()->platformOnTouchEvent (touchEvent);
		[self removeTouches:touches];
	}
}

@end

namespace VSTGUI {

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UIViewFrame::UIViewFrame (IPlatformFrameCallback* frame, const CRect& size, UIView* parent)
: IPlatformFrame (frame)
, uiView (nullptr)
{
	uiView = [[VSTGUI_UIView alloc] initWithUIViewFrame:this parent:parent size:&size];
}

//-----------------------------------------------------------------------------
UIViewFrame::~UIViewFrame ()
{
	[uiView removeFromSuperview];
#if !ARC_ENABLED
	[uiView release];
#endif
}

//-----------------------------------------------------------------------------
bool UIViewFrame::getGlobalPosition (CPoint& pos) const
{
	if (uiView)
	{
		CGPoint p = [uiView convertPoint:[uiView bounds].origin toView:nil];
		pos.x = p.x;
		pos.y = p.y;
		// TODO: check if this is correct in all cases
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool UIViewFrame::setSize (const CRect& newSize)
{
	if (uiView)
	{
		uiView.frame = CGRectFromCRect (newSize);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool UIViewFrame::getSize (CRect& size) const
{
	if (uiView)
	{
		size = CRectFromCGRect (uiView.frame);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool UIViewFrame::invalidRect (const CRect& rect)
{
	[uiView setNeedsDisplayInRect:CGRectFromCRect (rect)];
	return true;
}

//-----------------------------------------------------------------------------
bool UIViewFrame::scrollRect (const CRect& src, const CPoint& distance)
{
	return false;
}

//-----------------------------------------------------------------------------
SharedPointer<IPlatformTextEdit> UIViewFrame::createPlatformTextEdit (IPlatformTextEditCallback* textEdit)
{
	return owned <IPlatformTextEdit> (new UITextEdit (uiView, textEdit));
}

//-----------------------------------------------------------------------------
SharedPointer<IPlatformOptionMenu> UIViewFrame::createPlatformOptionMenu ()
{
	return nullptr;
}

#if VSTGUI_OPENGL_SUPPORT
//-----------------------------------------------------------------------------
SharedPointer<IPlatformOpenGLView> UIViewFrame::createPlatformOpenGLView ()
{
	return owned<IPlatformOpenGLView> (new GLKitOpenGLView (uiView));
}
#endif

//-----------------------------------------------------------------------------
SharedPointer<IPlatformViewLayer> UIViewFrame::createPlatformViewLayer (IPlatformViewLayerDelegate* drawDelegate, IPlatformViewLayer* parentLayer)
{
	CAViewLayer* parentViewLayer = dynamic_cast<CAViewLayer*> (parentLayer);
	auto layer = owned (new CAViewLayer (parentViewLayer ? parentViewLayer->getCALayer () : [uiView layer]));
	layer->init (drawDelegate);
	return shared<IPlatformViewLayer> (layer);
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
DragResult UIViewFrame::doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap) { return kDragError; }
#endif
bool UIViewFrame::doDrag (const DragDescription& dragDescription, const SharedPointer<IDragCallback>& callback) { return false; }

}

#endif // TARGET_OS_IPHONE
