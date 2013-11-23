//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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
	~Pasteboard ();

	int32_t getCount () VSTGUI_OVERRIDE_VMETHOD;
	int32_t getDataSize (int32_t index) VSTGUI_OVERRIDE_VMETHOD;
	Type getDataType (int32_t index) VSTGUI_OVERRIDE_VMETHOD;
	int32_t getData (int32_t index, const void*& buffer, Type& type) VSTGUI_OVERRIDE_VMETHOD;
protected:
	NSPasteboard* pb;
	int32_t nbItems;
	bool stringsAreFiles;
	std::vector<std::string> strings;
	NSMutableArray* dataArray;
};

//-----------------------------------------------------------------------------
Pasteboard::Pasteboard (NSPasteboard* pb)
: pb (pb)
, nbItems (0)
, stringsAreFiles (false)
, dataArray (0)
{
	NSArray *supportedTypes = [NSArray arrayWithObjects: NSStringPboardType, nil];
	NSString* hasString = [pb availableTypeFromArray: supportedTypes];
	if (hasString)
	{
		nbItems = 1;
		NSString* unicodeText = [pb stringForType:NSStringPboardType];
		if (unicodeText)
		{
			strings.push_back ([unicodeText UTF8String]);
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
			nbItems = (int32_t)[fileNames count];
			for (int32_t i = 0; i < nbItems; i++)
			{
				NSString* str = [fileNames objectAtIndex:i];
				if (str)
					strings.push_back ([str UTF8String]);
			}
		}
		else if ([pb availableTypeFromArray:[NSArray arrayWithObject:NSColorPboardType]])
		{
			NSColor* nsColor = [NSColor colorFromPasteboard:pb];
			if (nsColor)
			{
				nsColor = [nsColor colorUsingColorSpace:[[[NSColorSpace alloc] initWithCGColorSpace:GetCGColorSpace ()] autorelease]];
				int32_t red = [nsColor redComponent] * 255.;
				int32_t green = [nsColor greenComponent] * 255.;
				int32_t blue = [nsColor blueComponent] * 255.;
				int32_t alpha = [nsColor alphaComponent] * 255.;
				char str[10];
				sprintf (str, "#%02x%02x%02x%02x", red, green, blue, alpha);
				strings.push_back (str);
			}
		}
		else
		{
			nbItems = (int32_t)[[pb types] count];
			dataArray = [[NSMutableArray alloc] initWithCapacity:nbItems];
			for (int32_t i = 0; i < nbItems; i++)
			{
				NSData* nsData = [pb dataForType:[[pb types] objectAtIndex:i]];
				[dataArray addObject:nsData];
			}
		}
	}
}

//-----------------------------------------------------------------------------
Pasteboard::~Pasteboard ()
{
	if (dataArray)
		[dataArray release];
}

//-----------------------------------------------------------------------------
int32_t Pasteboard::getCount ()
{
	return nbItems;
}

//-----------------------------------------------------------------------------
int32_t Pasteboard::getDataSize (int32_t index)
{
	if (dataArray)
	{
		return (int32_t)[[dataArray objectAtIndex:index] length];
	}
	if (index < (int32_t)strings.size ())
	{
		return (int32_t)strings[index].length ();
	}
	return 0;
}

//-----------------------------------------------------------------------------
Pasteboard::Type Pasteboard::getDataType (int32_t index)
{
	if (dataArray)
		return kBinary;
	else if (index < (int32_t)strings.size ())
	{
		if (stringsAreFiles)
			return kFilePath;
		return kText;
	}
	return kError;
}

//-----------------------------------------------------------------------------
int32_t Pasteboard::getData (int32_t index, const void*& buffer, Pasteboard::Type& type)
{
	if (dataArray)
	{
		buffer = [[dataArray objectAtIndex:index] bytes];
		type = kBinary;
		return (int32_t)[[dataArray objectAtIndex:index] length];
	}
	if (index < (int32_t)strings.size ())
	{
		buffer = strings[index].c_str ();
		type = stringsAreFiles ? kFilePath : kText;
		return (int32_t)strings[index].length ();
	}
	return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
IDataPackage* createClipboardDataPackage ()
{
	return new Pasteboard ([NSPasteboard generalPasteboard]);
}

//-----------------------------------------------------------------------------
IDataPackage* createDragDataPackage (NSPasteboard* pasteboard)
{
	return new Pasteboard (pasteboard);
}

#if MAC_CARBON
//-----------------------------------------------------------------------------
IDataPackage* createCarbonDragDataPackage (DragRef drag)
{
	PasteboardRef pr;
	if (GetDragPasteboard (drag, &pr) == noErr)
	{
		CFStringRef pasteboardName;
		if (PasteboardCopyName (pr, &pasteboardName) == noErr)
		{
			[(NSString*)pasteboardName autorelease];
			return new Pasteboard ([NSPasteboard pasteboardWithName:(NSString*)pasteboardName]);
		}
	}
	return 0;
}
#endif

//-----------------------------------------------------------------------------
void setClipboard (IDataPackage* dataSource)
{
	NSPasteboard* pb = [NSPasteboard generalPasteboard];
	if (dataSource)
	{
		int32_t nbItems = dataSource->getCount ();
		NSMutableArray* fileArray = 0;
		IDataPackage::Type type;
		const void* data;
		int32_t length;
		for (int32_t i = 0; i < nbItems; i++)
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
						if (fileArray == 0)
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
		[pb declareTypes:nil owner:nil];
	}
}

//-----------------------------------------------------------------------------
const char* getPasteboardBinaryType ()
{
	return "net.sourceforge.vstgui.pasteboard.type.binary";
}

}} // namespaces
