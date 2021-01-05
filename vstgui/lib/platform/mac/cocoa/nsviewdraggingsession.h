// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../../vstguifwd.h"

#if MAC_COCOA && !TARGET_OS_IPHONE

#include "../../../dragging.h"

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#else
struct NSView;
struct NSDraggingSession;
struct NSImage;
#endif

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
struct NSViewDraggingSession : public IDraggingSession, public NonAtomicReferenceCounted
{
	static SharedPointer<NSViewDraggingSession> create (
	    NSView* view, const DragDescription& desc, const SharedPointer<IDragCallback>& callback);

	NSViewDraggingSession (NSDraggingSession* session, const DragDescription& desc,
	                       const SharedPointer<IDragCallback>& callback);

	bool setBitmap (const SharedPointer<CBitmap>& bitmap, CPoint offset) override;

	void dragWillBegin (CPoint pos);
	void dragMoved (CPoint pos);
	void dragEnded (CPoint pos, DragOperation result);

private:
	static NSImage* nsImageForDragOperation (CBitmap* bitmap);

	NSDraggingSession* session;
	DragDescription desc;
	SharedPointer<IDragCallback> callback;
};

//------------------------------------------------------------------------
} // VSTGUI

#endif
