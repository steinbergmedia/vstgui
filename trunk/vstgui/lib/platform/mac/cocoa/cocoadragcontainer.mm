//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2011, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "cocoadragcontainer.h"

#if MAC_COCOA

#import <Cocoa/Cocoa.h>

namespace VSTGUI {

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
CocoaDragContainer::CocoaDragContainer (NSPasteboard* platformDrag)
: pb (platformDrag)
, nbItems (0)
, iterator (0)
, lastItem (0)
{
	NSArray *supportedTypes = [NSArray arrayWithObjects: NSStringPboardType, nil];
	NSString* hasString = [pb availableTypeFromArray: supportedTypes];
	if (hasString)
		nbItems = 1;
	else
	{
		supportedTypes = [NSArray arrayWithObjects: NSFilenamesPboardType, nil];
		NSString* hasFilenames = [pb availableTypeFromArray: supportedTypes];
		if (hasFilenames)
		{
			NSArray* fileNames = [pb propertyListForType:hasFilenames];
			nbItems = (int32_t)[fileNames count];
		}
		else
			nbItems = (int32_t)[[pb types] count];
	}
}

//------------------------------------------------------------------------------------
CocoaDragContainer::~CocoaDragContainer ()
{
	if (lastItem)
		free (lastItem);
}

//-----------------------------------------------------------------------------
void* CocoaDragContainer::first (int32_t& size, int32_t& type)
{
	iterator = 0;
	return next (size, type);
}

//------------------------------------------------------------------------------------
void* CocoaDragContainer::next (int32_t& size, int32_t& type)
{
	if (lastItem)
	{
		free (lastItem);
		lastItem = 0;
	}
	size = 0;
	iterator++;
	NSArray *supportedTypes = [NSArray arrayWithObjects: NSStringPboardType, nil];
	NSString* hasString = [pb availableTypeFromArray: supportedTypes];
	if (hasString)
	{
		if (iterator-1 == 0)
		{
			NSString* unicodeText = [pb stringForType:NSStringPboardType];
			if (unicodeText)
			{
				const char* utf8Text = [unicodeText UTF8String];
				char* data = (char*)malloc (strlen (utf8Text) + 1);
				strcpy (data, utf8Text);
				type = CDragContainer::kUnicodeText;
				size = (int32_t)strlen (utf8Text);
				lastItem = data;
				return data;
			}
		}
		type = CDragContainer::kError;
		return 0;
	}
		
	supportedTypes = [NSArray arrayWithObjects: NSFilenamesPboardType, nil];
	NSString* hasFilenames = [pb availableTypeFromArray: supportedTypes];
	if (hasFilenames)
	{
		NSArray* fileNames = [pb propertyListForType:hasFilenames];
		if (iterator-1 < (int32_t)[fileNames count])
		{
			NSString* filename = [fileNames objectAtIndex:iterator-1];
			if (filename)
			{
				const char* utf8Text = [filename UTF8String];
				char* data = (char*)malloc (strlen (utf8Text) + 1);
				strcpy (data, utf8Text);
				type = CDragContainer::kFile;
				size = (int32_t)strlen (utf8Text);
				lastItem = data;
				return data;
			}
		}
		type = CDragContainer::kError;
		return 0;
	}
	else if ([pb availableTypeFromArray:[NSArray arrayWithObject:NSColorPboardType]])
	{
		NSColor* nsColor = [NSColor colorFromPasteboard:pb];
		if (nsColor)
		{
			nsColor = [nsColor colorUsingColorSpaceName:NSDeviceRGBColorSpace];
			int32_t red = [nsColor redComponent] * 255.;
			int32_t green = [nsColor greenComponent] * 255.;
			int32_t blue = [nsColor blueComponent] * 255.;
			int32_t alpha = [nsColor alphaComponent] * 255.;
			char* data = (char*)malloc (9 * sizeof (int8_t));
			sprintf (data, "#%x%x%x%x", red, green, blue, alpha);
			size = (int32_t)strlen (data);
			type = CDragContainer::kUnicodeText;
			lastItem = data;
			return data;
		}
	}
	else
	{
		NSData* nsData = [pb dataForType:[[pb types] objectAtIndex:iterator-1]];
		if (nsData)
		{
			char* data = (char*)malloc ([nsData length]);
			memcpy (data, [nsData bytes], [nsData length]);
			type = CDragContainer::kUnknown;
			size = (int32_t)[nsData length];
			lastItem = data;
			return data;
		}
	}
	
	type = CDragContainer::kError;
	return 0;
}

//------------------------------------------------------------------------------------
int32_t CocoaDragContainer::getType (int32_t idx) const
{
	NSArray *supportedTypes = [NSArray arrayWithObjects: NSStringPboardType, nil];
	NSString* hasString = [pb availableTypeFromArray: supportedTypes];
	if (hasString)
		return idx == 0 ? CDragContainer::kUnicodeText : CDragContainer::kError;
		
	supportedTypes = [NSArray arrayWithObjects: NSFilenamesPboardType, nil];
	NSString* hasFilenames = [pb availableTypeFromArray: supportedTypes];
	if (hasFilenames)
	{
		NSArray* fileNames = [pb propertyListForType:hasFilenames];
		if ((int32_t)[fileNames count] > idx)
			return CDragContainer::kFile;
	}

	return CDragContainer::kUnknown;
}

} // namespace

#endif // MAC_COCOA
