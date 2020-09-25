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

#ifndef MAC_OS_X_VERSION_10_14
#define MAC_OS_X_VERSION_10_14      101400
#endif

namespace VSTGUI {
namespace MacClipboard {

#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14
//------------------------------------------------------------------------
class Pasteboard : public IDataPackage
{
public:
	Pasteboard (NSPasteboard* pb) : pb (pb) { entries.resize ([pb pasteboardItems].count); }

	uint32_t getCount () const override { return entries.size (); }
	uint32_t getDataSize (uint32_t index) const override
	{
		if (index >= getCount ())
			return 0;
		prepareEntryAtIndex (index);
		return entries[index].data.size ();
	}
	Type getDataType (uint32_t index) const override
	{
		if (index >= getCount ())
			return Type::kError;
		prepareEntryAtIndex (index);
		return entries[index].type;
	}
	uint32_t getData (uint32_t index, const void*& buffer, Type& type) const override
	{
		if (index >= getCount ())
			return 0;
		prepareEntryAtIndex (index);
		buffer = entries[index].data.data ();
		type = entries[index].type;
		return entries[index].data.size ();
	}

private:
	struct Entry
	{
		std::vector<uint8_t> data;
		Type type {Type::kError};
	};

	void prepareEntryAtIndex (uint32_t index) const
	{
		if (entries[index].type == Type::kError)
			entries[index] = std::move (makeEntry (pb.pasteboardItems[index]));
	}

	static Entry makeEntry (NSPasteboardItem* item)
	{
		auto DefaultPBItemTypes =
		    @[NSPasteboardTypeString, NSPasteboardTypeFileURL, NSPasteboardTypeColor];
		Entry result;
		if (auto availableType = [item availableTypeFromArray:DefaultPBItemTypes])
		{
			if ([availableType isEqualToString:NSPasteboardTypeFileURL])
			{
				result.type = Type::kFilePath;
				NSString* fileUrlStr = [item stringForType:NSPasteboardTypeFileURL];
				NSURL* url = [NSURL URLWithString:fileUrlStr];
				std::string pathStr = url.path.UTF8String;
				result.data.resize (pathStr.size ());
				memcpy (result.data.data (), pathStr.data (), pathStr.size ());
			}
			else if ([availableType isEqualToString:NSPasteboardTypeColor])
			{
				result.type = Type::kText;
				if (NSData* nsColorData = [item dataForType:NSPasteboardTypeColor])
				{
					if (NSColor* nsColor =
					        [NSKeyedUnarchiver unarchivedObjectOfClass:[NSColor class]
					                                          fromData:nsColorData
					                                             error:nil])
					{
						nsColor = [nsColor
						    colorUsingColorSpace:[[[NSColorSpace alloc]
						                             initWithCGColorSpace:GetCGColorSpace ()]
						                             autorelease]];
						int32_t red = static_cast<int32_t> ([nsColor redComponent] * 255.);
						int32_t green = static_cast<int32_t> ([nsColor greenComponent] * 255.);
						int32_t blue = static_cast<int32_t> ([nsColor blueComponent] * 255.);
						int32_t alpha = static_cast<int32_t> ([nsColor alphaComponent] * 255.);
						char str[10];
						sprintf (str, "#%02x%02x%02x%02x", red, green, blue, alpha);
						result.data.resize (10);
						memcpy (result.data.data (), str, 10);
					}
				}
			}
			else
			{
				assert ([availableType isEqualToString:NSPasteboardTypeString]);
				result.type = Type::kText;
				if (auto data = [item dataForType:availableType])
				{
					result.data.resize (data.length);
					memcpy (result.data.data (), data.bytes, data.length);
				}
			}
		}
		else
		{
			result.type = Type::kBinary;
			auto data = [item dataForType:item.types[0]];
			result.data.resize (data.length);
			memcpy (result.data.data (), data.bytes, data.length);
		}
		return result;
	}

	NSPasteboard* pb;
	mutable std::vector<Entry> entries;
};

#else

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
#endif

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
		[pb clearContents];

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
						[pb declareTypes:[NSArray arrayWithObject:NSPasteboardTypeString] owner:nil];
						[pb setString:[[[NSString alloc] initWithBytes:data length:length encoding:NSUTF8StringEncoding] autorelease] forType:NSPasteboardTypeString];
						return;
					}
					case IDataPackage::kFilePath:
					{
						if (fileArray == nullptr)
							fileArray = [[[NSMutableArray alloc] init] autorelease];
						auto fileStr =
						    [NSString stringWithUTF8String:static_cast<const char*> (data)];
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14
						[fileArray addObject:[NSURL fileURLWithPath:fileStr]];
#else
						[fileArray addObject:fileStr];
#endif
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
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14
			[pb writeObjects:fileArray];
#else
			[pb declareTypes:[NSArray arrayWithObject:NSFilenamesPboardType] owner:nil];
			[pb setPropertyList:fileArray forType:NSFilenamesPboardType];
#endif
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
