// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "macclipboard.h"
#import "macglobals.h"
#import "../../cdropsource.h"
#import <Cocoa/Cocoa.h>
#import <vector>
#import <string>

#if MAC_CARBON
#import <Carbon/Carbon.h>
#endif

namespace VSTGUI {
namespace MacClipboard {

//-----------------------------------------------------------------------------
class Pasteboard : public IDataPackage
{
public:
	Pasteboard (NSPasteboard* pb);
	~Pasteboard () noexcept override;

	uint32_t getCount () const override;
	uint32_t getDataSize (uint32_t index) const override;
	Type getDataType (uint32_t index) const override;
	uint32_t getData (uint32_t index, const void*& buffer, Type& type) const override;
protected:
	NSPasteboard* pb;
	uint32_t nbItems;
	bool stringsAreFiles;
	std::vector<std::string> strings;
	NSMutableArray* dataArray;
};

//-----------------------------------------------------------------------------
Pasteboard::Pasteboard (NSPasteboard* pb)
: pb (pb)
, nbItems (0)
, stringsAreFiles (false)
, dataArray (nullptr)
{
	NSArray *supportedTypes = [NSArray arrayWithObjects: NSStringPboardType, nil];
	NSString* hasString = [pb availableTypeFromArray: supportedTypes];
	if (hasString)
	{
		nbItems = 1;
		NSString* unicodeText = [pb stringForType:NSStringPboardType];
		if (unicodeText)
		{
			strings.emplace_back ([unicodeText UTF8String]);
		}
	}
	else
	{
		supportedTypes = [NSArray arrayWithObjects: NSFilenamesPboardType, nil];
		NSString* hasFilenames = [pb availableTypeFromArray: supportedTypes];
		if (hasFilenames)
		{
			stringsAreFiles = true;
			NSArray* fileNames = [pb propertyListForType:hasFilenames];
			nbItems = static_cast<uint32_t> ([fileNames count]);
			for (uint32_t i = 0; i < nbItems; i++)
			{
				NSString* str = [fileNames objectAtIndex:i];
				if (str)
					strings.emplace_back ([str UTF8String]);
			}
		}
		else if ([pb availableTypeFromArray:[NSArray arrayWithObject:NSColorPboardType]])
		{
			NSColor* nsColor = [NSColor colorFromPasteboard:pb];
			if (nsColor)
			{
				nsColor = [nsColor colorUsingColorSpace:[[[NSColorSpace alloc] initWithCGColorSpace:GetCGColorSpace ()] autorelease]];
				int32_t red = static_cast<int32_t> ([nsColor redComponent] * 255.);
				int32_t green = static_cast<int32_t> ([nsColor greenComponent] * 255.);
				int32_t blue = static_cast<int32_t> ([nsColor blueComponent] * 255.);
				int32_t alpha = static_cast<int32_t> ([nsColor alphaComponent] * 255.);
				char str[10];
				sprintf (str, "#%02x%02x%02x%02x", red, green, blue, alpha);
				strings.emplace_back (str);
			}
		}
		else
		{
			nbItems = static_cast<uint32_t> ([[pb types] count]);
			dataArray = [[NSMutableArray alloc] initWithCapacity:nbItems];
			for (uint32_t i = 0; i < nbItems; i++)
			{
				NSData* nsData = [pb dataForType:[[pb types] objectAtIndex:i]];
				[dataArray addObject:nsData];
			}
		}
	}
}

//-----------------------------------------------------------------------------
Pasteboard::~Pasteboard () noexcept
{
	if (dataArray)
		[dataArray release];
}

//-----------------------------------------------------------------------------
uint32_t Pasteboard::getCount () const
{
	return nbItems;
}

//-----------------------------------------------------------------------------
uint32_t Pasteboard::getDataSize (uint32_t index) const
{
	if (dataArray)
	{
		return static_cast<uint32_t> ([[dataArray objectAtIndex:index] length]);
	}
	if (index < strings.size ())
	{
		return static_cast<uint32_t> (strings[index].length ());
	}
	return 0;
}

//-----------------------------------------------------------------------------
Pasteboard::Type Pasteboard::getDataType (uint32_t index) const
{
	if (dataArray)
		return kBinary;
	else if (index < strings.size ())
	{
		if (stringsAreFiles)
			return kFilePath;
		return kText;
	}
	return kError;
}

//-----------------------------------------------------------------------------
uint32_t Pasteboard::getData (uint32_t index, const void*& buffer, Pasteboard::Type& type) const
{
	if (dataArray)
	{
		buffer = [[dataArray objectAtIndex:index] bytes];
		type = kBinary;
		return static_cast<uint32_t> ([[dataArray objectAtIndex:index] length]);
	}
	if (index < strings.size ())
	{
		buffer = strings[index].c_str ();
		type = stringsAreFiles ? kFilePath : kText;
		return static_cast<uint32_t> (strings[index].length ());
	}
	return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
SharedPointer<IDataPackage> createClipboardDataPackage ()
{
	return makeOwned<Pasteboard> ([NSPasteboard generalPasteboard]);
}

//-----------------------------------------------------------------------------
SharedPointer<IDataPackage> createDragDataPackage (NSPasteboard* pasteboard)
{
	return makeOwned<Pasteboard> (pasteboard);
}

#if MAC_CARBON
//-----------------------------------------------------------------------------
SharedPointer<IDataPackage> createCarbonDragDataPackage (DragRef drag)
{
	PasteboardRef pr;
	if (GetDragPasteboard (drag, &pr) == noErr)
	{
		CFStringRef pasteboardName;
		if (PasteboardCopyName (pr, &pasteboardName) == noErr)
		{
			[(NSString*)pasteboardName autorelease];
			return makeOwned<Pasteboard> ([NSPasteboard pasteboardWithName:(NSString*)pasteboardName]);
		}
	}
	return nullptr;
}
#endif

//-----------------------------------------------------------------------------
void setClipboard (const SharedPointer<IDataPackage>& dataSource)
{
	NSPasteboard* pb = [NSPasteboard generalPasteboard];
	if (dataSource)
	{
		uint32_t nbItems = dataSource->getCount ();
		NSMutableArray* fileArray = nullptr;
		IDataPackage::Type type;
		const void* data;
		uint32_t length;
		for (uint32_t i = 0; i < nbItems; i++)
		{
			if ((length = dataSource->getData (i, data, type)) > 0)
			{
				switch (type)
				{
					case IDataPackage::kBinary:
					{
						[pb declareTypes:[NSArray arrayWithObject:[NSString stringWithCString:MacClipboard::getPasteboardBinaryType () encoding:NSASCIIStringEncoding]] owner:nil];
						[pb setData:[NSData dataWithBytes:data length:length] forType:[NSString stringWithCString:MacClipboard::getPasteboardBinaryType () encoding:NSASCIIStringEncoding]];
						return;
					}
					case IDataPackage::kText:
					{
						[pb declareTypes:[NSArray arrayWithObject:NSStringPboardType] owner:nil];
						[pb setString:[[[NSString alloc] initWithBytes:data length:length encoding:NSUTF8StringEncoding] autorelease] forType:NSStringPboardType];
						return;
					}
					case IDataPackage::kFilePath:
					{
						if (fileArray == nullptr)
							fileArray = [[[NSMutableArray alloc] init] autorelease];
						[fileArray addObject:[NSString stringWithCString:(const char*)data encoding:NSUTF8StringEncoding]];
						break;
					}
					case IDataPackage::kError:
					{
						continue;
					}
				}
			}
		}
		if (fileArray)
		{
			[pb declareTypes:[NSArray arrayWithObject:NSFilenamesPboardType] owner:nil];
			[pb setPropertyList:fileArray forType:NSFilenamesPboardType];
		}
	}
	else
	{
		[pb clearContents];
	}
}

//-----------------------------------------------------------------------------
const char* getPasteboardBinaryType ()
{
	return "net.sourceforge.vstgui.pasteboard.type.binary";
}

}} // namespaces
