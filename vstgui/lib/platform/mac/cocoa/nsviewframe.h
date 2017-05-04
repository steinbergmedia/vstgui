// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __nsviewframe__
#define __nsviewframe__

#include "../../../vstguifwd.h"

#if MAC_COCOA

#include "../../../cview.h"
#include "../../iplatformframe.h"
#include "../../../idatapackage.h"
#include <list>

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#else
struct NSView;
struct NSRect;
#endif

namespace VSTGUI {
class CocoaTooltipWindow;

//-----------------------------------------------------------------------------
class CocoaFrameConfig : public IPlatformFrameConfig
{
public:
	enum Flags {
		kNoCALayer = 1 << 0,
	};
	uint32_t flags {0};
};

//-----------------------------------------------------------------------------
class NSViewFrame : public IPlatformFrame, public IPlatformFrameTouchBarExtension
{
public:
	NSViewFrame (IPlatformFrameCallback* frame, const CRect& size, NSView* parent, IPlatformFrameConfig* config);
	~NSViewFrame () noexcept override;

	NSView* getPlatformControl () const { return nsView; }
	IPlatformFrameCallback* getFrame () const { return frame; }
	void* makeTouchBar () const;
	
	void setLastDragOperationResult (DragResult result) { lastDragOperationResult = result; }
	void setIgnoreNextResignFirstResponder (bool state) { ignoreNextResignFirstResponder = state; }
	bool getIgnoreNextResignFirstResponder () const { return ignoreNextResignFirstResponder; }

	void setDragDataPackage (SharedPointer<IDataPackage>&& package) { dragDataPackage = std::move (package); }
	const SharedPointer<IDataPackage>& getDragDataPackage () const { return dragDataPackage; }

	void initTrackingArea ();
	void cursorUpdate ();
	virtual void drawRect (NSRect* rect);

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
	DragResult doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap) override;
	void setClipboard (const SharedPointer<IDataPackage>& data) override;
	SharedPointer<IDataPackage> getClipboard () override;
	PlatformType getPlatformType () const override { return PlatformType::kNSView; }

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

	DragResult lastDragOperationResult;
	bool ignoreNextResignFirstResponder;
	bool trackingAreaInitialized;
	bool inDraw;
	CCursorType cursor;
};

} // namespace

#endif // MAC_COCOA
#endif // __nsviewframe__
