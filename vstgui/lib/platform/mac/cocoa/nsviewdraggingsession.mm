// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "nsviewdraggingsession.h"

#if MAC_COCOA

#import "../cgbitmap.h"
#import "../macclipboard.h"
#import "cocoahelpers.h"
#import "objcclassbuilder.h"

//------------------------------------------------------------------------
@interface NSObject (VSTGUI_BinaryDataType_Private)
- (id)initWithData:(const void*)data andSize:(size_t)size;
@end

#ifndef MAC_OS_X_VERSION_10_13
#define MAC_OS_X_VERSION_10_13 101300
#endif

#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_13
typedef NSString *NSPasteboardType;
typedef NSString *NSPasteboardReadingOptionKey;
#endif

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
struct BinaryDataType : RuntimeObjCClass<BinaryDataType>
{
	static constexpr const auto dataVarName = "_data";

	static NSString* getCocoaPasteboardTypeString ()
	{
		return [NSString stringWithCString:VSTGUI::MacClipboard::getPasteboardBinaryType ()
		                          encoding:NSASCIIStringEncoding];
	}

	static id Init (id self, SEL, const void* buffer, size_t bufferSize)
	{
		auto obj = makeInstance (self);
		self = obj.callSuper<id (id, SEL), id> (@selector (init));
		if (self)
		{
			auto data = [[NSData alloc] initWithBytes:buffer length:bufferSize];
			if (auto var = obj.getVariable<NSData*> (dataVarName))
				var->set (data);
		}
		return self;
	}

	static void Dealloc (id self, SEL _cmd)
	{
		auto obj = makeInstance (self);
		if (auto var = obj.getVariable<NSData*> (dataVarName); var.has_value () && var->get ())
		{
			[var->get () release];
			var->set (nullptr);
		}
		obj.callSuper<void (id, SEL)> (_cmd);
	}

	static NSArray<NSPasteboardType>* WritableTypesForPasteboard (id, SEL, NSPasteboard*)
	{
		return @[getCocoaPasteboardTypeString ()];
	}

	static id PasteboardPropertyListForType (id self, SEL, NSPasteboardType)
	{
		auto obj = makeInstance (self);
		if (auto var = obj.getVariable<NSData*> (dataVarName); var.has_value () && var->get ())
			return var->get ();
		return nullptr;
	}

	static Class CreateClass ()
	{
		return ObjCClassBuilder ()
			.init ("VSTGUI_BinaryDataType", [NSObject class])
			.addProtocol ("NSPasteboardWriting")
			.addMethod (@selector (initWithData:andSize:), Init)
			.addMethod (@selector (dealloc), Dealloc)
			.addMethod (@selector (writableTypesForPasteboard:), WritableTypesForPasteboard)
			.addMethod (@selector (pasteboardPropertyListForType:), PasteboardPropertyListForType)
			.addIvar<NSData*> (dataVarName)
			.finalize ();
	}
};

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
			case IDataPackage::kBinary:
			{
				if (id data = [[BinaryDataType::alloc () initWithData:buffer
															  andSize:size] autorelease])
					item = [[[NSDraggingItem alloc] initWithPasteboardWriter:data] autorelease];
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
			{
				NSRect r;
				r.origin = bitmapOffset;
				r.size = NSMakeSize (1, 1);
				item.draggingFrame = r;
			}
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
void NSViewDraggingSession::dragEnded (CPoint pos, DragOperation result)
{
	if (!callback)
		return;
	callback->dragEnded (this, pos, result);
}

//-----------------------------------------------------------------------------
NSImage* NSViewDraggingSession::nsImageForDragOperation (CBitmap* bitmap)
{
	return bitmapToNSImage (bitmap);
}

//------------------------------------------------------------------------
} // VSTGUI

#endif
