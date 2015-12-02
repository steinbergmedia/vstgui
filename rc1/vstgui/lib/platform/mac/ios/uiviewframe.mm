//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins :
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

#import "uiviewframe.h"

#if TARGET_OS_IPHONE
#import "../../../cfileselector.h"
#import "../cgdrawcontext.h"
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
	CGDrawContext drawContext (UIGraphicsGetCurrentContext (), CRectFromCGRect (self.bounds));
	uiViewFrame->getFrame ()->platformDrawRect (&drawContext, CRectFromCGRect (rect));
}

//-----------------------------------------------------------------------------
- (BOOL)canBecomeFirstResponder
{
	return YES;
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
CNewFileSelector* CNewFileSelector::create (CFrame* frame, Style style)
{
#if DEBUG
	DebugPrint ("No iOS implementation for CNewFileSelector\n");
#endif
	return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
IPlatformFrame* IPlatformFrame::createPlatformFrame (IPlatformFrameCallback* frame, const CRect& size, void* parent, PlatformType platformType)
{
	return new UIViewFrame (frame, size, (__bridge UIView*)parent);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UIViewFrame::UIViewFrame (IPlatformFrameCallback* frame, const CRect& size, UIView* parent)
: IPlatformFrame (frame)
, uiView (0)
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
IPlatformTextEdit* UIViewFrame::createPlatformTextEdit (IPlatformTextEditCallback* textEdit)
{
	return new UITextEdit (uiView, textEdit);
}

//-----------------------------------------------------------------------------
IPlatformOptionMenu* UIViewFrame::createPlatformOptionMenu ()
{
	return nullptr;
}

//-----------------------------------------------------------------------------
COffscreenContext* UIViewFrame::createOffscreenContext (CCoord width, CCoord height, double scaleFactor)
{
	CGBitmap* bitmap = new CGBitmap (CPoint (width * scaleFactor, height * scaleFactor));
	bitmap->setScaleFactor (scaleFactor);
	CGDrawContext* context = new CGDrawContext (bitmap);
	bitmap->forget ();
	if (context->getCGContext ())
		return context;
	context->forget ();
	return 0;
}

#if VSTGUI_OPENGL_SUPPORT
//-----------------------------------------------------------------------------
IPlatformOpenGLView* UIViewFrame::createPlatformOpenGLView ()
{
	return new GLKitOpenGLView (uiView);
}
#endif

//-----------------------------------------------------------------------------
IPlatformViewLayer* UIViewFrame::createPlatformViewLayer (IPlatformViewLayerDelegate* drawDelegate, IPlatformViewLayer* parentLayer)
{
	CAViewLayer* parentViewLayer = dynamic_cast<CAViewLayer*> (parentLayer);
	CAViewLayer* layer = new CAViewLayer (parentViewLayer ? parentViewLayer->getLayer () : [uiView layer]);
	layer->init (drawDelegate);
	return layer;
}

}

#endif // TARGET_OS_IPHONE
