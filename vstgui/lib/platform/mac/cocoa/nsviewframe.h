// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../../vstguifwd.h"

#if MAC_COCOA && !TARGET_OS_IPHONE

#include "../../platform_macos.h"
#include "../../iplatformtextinputclient.h"
#include "../../../cinvalidrectlist.h"
#include "../../../idatapackage.h"
#import "../coregraphicsdevicecontext.h"
#include "nsviewdraggingsession.h"
#include <list>

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#else
struct NSView;
struct NSRect;
struct NSDraggingSession;
struct NSEvent;
struct CALayer;
#endif

namespace VSTGUI {
class CocoaTooltipWindow;
struct NSViewDraggingSession;

//-----------------------------------------------------------------------------
class NSViewFrame : public IPlatformFrame, public ICocoaPlatformFrame, public IPlatformFrameTouchBarExtension
{
public:
	NSViewFrame (IPlatformFrameCallback* frame, const CRect& size, NSView* parent, IPlatformFrameConfig* config);
	~NSViewFrame () noexcept override;

	NSView* getNSView () const override { return nsView; }
	void setTextInputClient (ICocoaTextInputClient* client) override;
	ICocoaTextInputClient* getTextInputClient () const { return textInputClient; }

	CALayer* getCALayer () const { return caLayer; }
	IPlatformFrameCallback* getFrame () const { return frame; }
	void* makeTouchBar () const;
	SharedPointer<NSViewDraggingSession> getDraggingSession () const { return draggingSession; }
	void clearDraggingSession () { draggingSession = nullptr; }
	void setNeedsDisplayInRect (NSRect r);

	void setDragDataPackage (SharedPointer<IDataPackage>&& package) { dragDataPackage = std::move (package); }
	const SharedPointer<IDataPackage>& getDragDataPackage () const { return dragDataPackage; }

	void initTrackingArea ();
	void scaleFactorChanged (double newScaleFactor);
	void cursorUpdate ();
	void drawLayer (CALayer* layer, CGContextRef ctx);
	void drawRect (NSRect* rect);
	bool onMouseDown (NSEvent* evt);
	bool onMouseUp (NSEvent* evt);
	bool onMouseMoved (NSEvent* evt);

	// IPlatformFrame
	bool getGlobalPosition (CPoint& pos) const override;
	bool setSize (const CRect& newSize) override;
	bool getSize (CRect& size) const override;
	bool getCurrentMousePosition (CPoint& mousePosition) const override;
	bool getCurrentMouseButtons (CButtonState& buttons) const override;
	bool getCurrentModifiers (Modifiers& modifiers) const override;
	bool setMouseCursor (CCursorType type) override;
	bool invalidRect (const CRect& rect) override;
	bool showTooltip (const CRect& rect, const char* utf8Text) override;
	bool hideTooltip () override;
	void* getPlatformRepresentation () const override { return nsView; }
	SharedPointer<IPlatformTextEdit> createPlatformTextEdit (IPlatformTextEditCallback* textEdit) override;
	PlatformOptionMenuPtr createPlatformOptionMenu () override;
	PlatformViewLayerPtr
		createPlatformViewLayer (IPlatformViewLayerDelegate* drawDelegate,
								 IPlatformViewLayer* parentLayer = nullptr) override;
	bool doDrag (const DragDescription& dragDescription, const SharedPointer<IDragCallback>& callback) override;

	PlatformType getPlatformType () const override { return PlatformType::kNSView; }
	void onFrameClosed () override {}
	Optional<UTF8String> convertCurrentKeyEventToText () override;
	bool setupGenericOptionMenu (bool use, GenericOptionMenuTheme* theme = nullptr) override;

	// IPlatformFrameTouchBarExtension
	void setTouchBarCreator (const SharedPointer<ITouchBarCreator>& creator) override;
	void recreateTouchBar () override;

//-----------------------------------------------------------------------------
protected:
	void draw (CGContextRef context, CRect updateRect, double scaleFactor);
	void addDebugRedrawRect (CRect r, bool isClipBoundingBox = false);

	NSView* nsView {nullptr};
	CALayer* caLayer {nullptr};
	SharedPointer<CocoaTooltipWindow> tooltipWindow;
	ICocoaTextInputClient* textInputClient {nullptr};
	SharedPointer<IDataPackage> dragDataPackage;
	SharedPointer<ITouchBarCreator> touchBarCreator;
	SharedPointer<NSViewDraggingSession> draggingSession;
	std::unique_ptr<GenericOptionMenuTheme> genericOptionMenuTheme;

	bool trackingAreaInitialized;
	bool inDraw;
	bool useInvalidRects {false};

	CCursorType cursor;
	CButtonState mouseDownButtonState {};
	CInvalidRectList invalidRectList;
};

} // VSTGUI

#endif // MAC_COCOA
