// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "nsviewdraggingsession.h"

#if MAC_COCOA

#import "../cgbitmap.h"
#import "cocoahelpers.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
SharedPointer<NSViewDraggingSession> NSViewDraggingSession::create (
    NSView* nsView, const DragDescription& desc, const SharedPointer<IDragCallback>& callback)
{
	NSEvent* event = [NSApp currentEvent];
	if (event == nullptr || !([event type] == MacEventType::LeftMouseDown ||
	                          [event type] == MacEventType::LeftMouseDragged))
		return nullptr;

	NSPoint bitmapOffset = {static_cast<CGFloat> (desc.bitmapOffset.x),
	                        static_cast<CGFloat> (desc.bitmapOffset.y)};

	auto bitmap = desc.bitmap;
	NSPoint nsLocation = [event locationInWindow];
	NSImage* nsImage = nil;
	if ((nsImage = nsImageForDragOperation (bitmap)))
	{
		nsLocation = [nsView convertPoint:nsLocation fromView:nil];
		bitmapOffset.x += nsLocation.x;
		bitmapOffset.y += nsLocation.y + [nsImage size].height;
	}

	NSMutableArray* dragItems = [[NSMutableArray new] autorelease];
	for (uint32_t index = 0, count = desc.data->getCount (); index < count; ++index)
	{
		const void* buffer = nullptr;
		IDataPackage::Type type {};
		auto size = desc.data->getData (index, buffer, type);
		if (size == 0)
			continue;
		NSDraggingItem* item = nil;
		switch (type)
		{
			case IDataPackage::kFilePath:
			{
				if (auto fileUrl = [NSURL
				        fileURLWithPath:[NSString
				                            stringWithUTF8String:reinterpret_cast<const char*> (
				                                                     buffer)]])
				{
					item = [[[NSDraggingItem alloc] initWithPasteboardWriter:fileUrl] autorelease];
				}
				break;
			}
			case IDataPackage::kText:
			{
				if (auto string =
				        [NSString stringWithUTF8String:reinterpret_cast<const char*> (buffer)])
				{
					item = [[[NSDraggingItem alloc] initWithPasteboardWriter:string] autorelease];
				}
				break;
			}
			case IDataPackage::kBinary: {
#if 0
				// TODO: write an object implementing NSPasteboardWriting to provide NSData
				if (auto data = [NSData dataWithBytes:buffer length:size])
				{
					item = [[[NSDraggingItem alloc] initWithPasteboardWriter:data] autorelease];
				}
#else
				vstgui_assert (false, "Not yet implemented");
#endif
				break;
			}
			case IDataPackage::kError: { continue;
			}
		}
		if (item)
		{
			if (nsImage && [dragItems count] == 0)
			{
				NSRect r;
				r.size.width = bitmap->getWidth ();
				r.size.height = bitmap->getHeight ();
				r.origin = bitmapOffset;
				r.origin.y -= r.size.height;
				[item setDraggingFrame:r contents:nsImage];
			}
			else
				item.draggingFrame.origin = bitmapOffset;
			[dragItems addObject:item];
		}
	}
	NSView<NSDraggingSource>* draggingSource = (NSView<NSDraggingSource>*)nsView;
	if (auto session =
	        [nsView beginDraggingSessionWithItems:dragItems event:event source:draggingSource])
	{
		session.animatesToStartingPositionsOnCancelOrFail = YES;
		return makeOwned<NSViewDraggingSession> (session, desc, callback);
	}

	return nullptr;
}

//------------------------------------------------------------------------
NSViewDraggingSession::NSViewDraggingSession (NSDraggingSession* session,
                                              const DragDescription& desc,
                                              const SharedPointer<IDragCallback>& callback)
: session (session), desc (desc), callback (callback)
{
}

//------------------------------------------------------------------------
bool NSViewDraggingSession::setBitmap (const SharedPointer<CBitmap>& bitmap, CPoint offset)
{
	[session enumerateDraggingItemsWithOptions:0
	                                   forView:nil
	                                   classes:[NSArray arrayWithObject:[NSPasteboardItem class]]
	                             searchOptions:[NSDictionary<NSPasteboardReadingOptionKey, id> new]
	                                usingBlock:[&] (NSDraggingItem* _Nonnull draggingItem,
	                                                NSInteger idx, BOOL* _Nonnull stop) {
		                                if (idx != 0)
			                                return;
		                                if (auto nsImage = nsImageForDragOperation (bitmap))
		                                {
			                                NSRect r;
			                                r.origin = nsPointFromCPoint (offset);
			                                r.origin.y -= nsImage.size.height;
			                                r.size = nsImage.size;
			                                [draggingItem setDraggingFrame:r contents:nsImage];
		                                }
		                                else
		                                {
			                                [draggingItem
			                                    setDraggingFrame:draggingItem.draggingFrame
			                                            contents:nil];
										}
		                                *stop = YES;
	                                }];
	desc.bitmap = bitmap;
	desc.bitmapOffset = offset;
	return true;
}

//------------------------------------------------------------------------
void NSViewDraggingSession::dragWillBegin (CPoint pos)
{
	if (!callback)
		return;
	callback->dragWillBegin (this, pos);
}

//------------------------------------------------------------------------
void NSViewDraggingSession::dragMoved (CPoint pos)
{
	if (!callback)
		return;
	callback->dragMoved (this, pos);
}

//------------------------------------------------------------------------
void NSViewDraggingSession::dragEnded (CPoint pos, DragResult result)
{
	if (!callback)
		return;
	callback->dragEnded (this, pos, result);
}

//-----------------------------------------------------------------------------
NSImage* NSViewDraggingSession::nsImageForDragOperation (CBitmap* bitmap)
{
	if (!bitmap)
		return nil;
	auto platformBitmap = bitmap->getPlatformBitmap ();
	if (!platformBitmap)
		return nil;
	auto cgBitmap = platformBitmap.cast<CGBitmap> ();
	if (!cgBitmap)
		return nil;
	auto cgImage = cgBitmap->getCGImage ();
	if (!cgImage)
		return nil;
	auto scaleFactor = platformBitmap->getScaleFactor ();
	return [imageFromCGImageRef (cgImage, scaleFactor) autorelease];
}

//------------------------------------------------------------------------
} // VSTGUI

#endif
