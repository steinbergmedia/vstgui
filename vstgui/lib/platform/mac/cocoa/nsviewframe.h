// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../../vstguifwd.h"

#if MAC_COCOA

#include "../../platform_macos.h"
#include "../../../cview.h"
#include "../../../idatapackage.h"
#include "nsviewdraggingsession.h"
#include <list>

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#else
struct NSView;
struct NSRect;
struct NSDraggingSession;
struct NSEvent;
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
	IPlatformFrameCallback* getFrame () const { return frame; }
	void* makeTouchBar () const;
	NSViewDraggingSession* getDraggingSession () const { return draggingSession; }
	void clearDraggingSession () { draggingSession = nullptr; }
	
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	void setLastDragOperationResult (DragResult result) { lastDragOperationResult = result; }
#endif
	void setIgnoreNextResignFirstResponder (bool state) { ignoreNextResignFirstResponder = state; }
	bool getIgnoreNextResignFirstResponder () const { return ignoreNextResignFirstResponder; }

	void setDragDataPackage (SharedPointer<IDataPackage>&& package) { dragDataPackage = std::move (package); }
	const SharedPointer<IDataPackage>& getDragDataPackage () const { return dragDataPackage; }

	void initTrackingArea ();
	void scaleFactorChanged (double newScaleFactor);
	void cursorUpdate ();
	virtual void drawRect (NSRect* rect);
	bool onMouseDown (NSEvent* evt);
	bool onMouseUp (NSEvent* evt);
	bool onMouseMoved (NSEvent* evt);

	// IPlatformFrame
	bool getGlobalPosition (CPoint& pos) const override;
	bool setSize (const CRect& newSize) override;
	bool getSize (CRect& size) const override;
	bool getCurrentMousePosition (CPoint& mousePosition) const override;
	bool getCurrentMouseButtons (CButtonState& buttons) const override;
	bool setMouseCursor (CCursorType type) override;
	bool invalidRect (const CRect& rect) override;
	bool scrollRect (const CRect& src, const CPoint& distance) override;
	bool showTooltip (const CRect& rect, const char* utf8Text) override;
	bool hideTooltip () override;
	void* getPlatformRepresentation () const override { return nsView; }
	SharedPointer<IPlatformTextEdit> createPlatformTextEdit (IPlatformTextEditCallback* textEdit) override;
	SharedPointer<IPlatformOptionMenu> createPlatformOptionMenu () override;
#if VSTGUI_OPENGL_SUPPORT
	SharedPointer<IPlatformOpenGLView> createPlatformOpenGLView () override;
#endif
	SharedPointer<IPlatformViewLayer> createPlatformViewLayer (IPlatformViewLayerDelegate* drawDelegate, IPlatformViewLayer* parentLayer = nullptr) override;
	SharedPointer<COffscreenContext> createOffscreenContext (CCoord width, CCoord height, double scaleFactor) override;
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	DragResult doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap) override;
#endif
	bool doDrag (const DragDescription& dragDescription, const SharedPointer<IDragCallback>& callback) override;

	void setClipboard (const SharedPointer<IDataPackage>& data) override;
	SharedPointer<IDataPackage> getClipboard () override;
	PlatformType getPlatformType () const override { return PlatformType::kNSView; }
	void onFrameClosed () override {}
	Optional<UTF8String> convertCurrentKeyEventToText () override;
	bool setupGenericOptionMenu (bool use, GenericOptionMenuTheme* theme = nullptr) override;

	// IPlatformFrameTouchBarExtension
	void setTouchBarCreator (const SharedPointer<ITouchBarCreator>& creator) override;
	void recreateTouchBar () override;

//-----------------------------------------------------------------------------
protected:
	static void initClass ();

	NSView* nsView;
	CocoaTooltipWindow* tooltipWindow;
	SharedPointer<IDataPackage> dragDataPackage;
	SharedPointer<ITouchBarCreator> touchBarCreator;
	SharedPointer<NSViewDraggingSession> draggingSession;
	std::unique_ptr<GenericOptionMenuTheme> genericOptionMenuTheme;
	
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	DragResult lastDragOperationResult;
#endif
	bool ignoreNextResignFirstResponder;
	bool trackingAreaInitialized;
	bool inDraw;
	CCursorType cursor;
	CButtonState mouseDownButtonState {};
};

} // VSTGUI

#endif // MAC_COCOA
