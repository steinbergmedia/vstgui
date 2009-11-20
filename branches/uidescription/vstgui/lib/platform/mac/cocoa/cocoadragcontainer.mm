
#include "cocoadragcontainer.h"

#if MAC_COCOA

#import <Cocoa/Cocoa.h>

BEGIN_NAMESPACE_VSTGUI

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
			nbItems = [fileNames count];
		}
	}
}

//------------------------------------------------------------------------------------
CocoaDragContainer::~CocoaDragContainer ()
{
	if (lastItem)
		free (lastItem);
}

//-----------------------------------------------------------------------------
void* CocoaDragContainer::first (long& size, long& type)
{
	iterator = 0;
	return next (size, type);
}

//------------------------------------------------------------------------------------
void* CocoaDragContainer::next (long& size, long& type)
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
				size = strlen (utf8Text);
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
		if (iterator-1 < (long)[fileNames count])
		{
			NSString* filename = [fileNames objectAtIndex:iterator-1];
			if (filename)
			{
				const char* utf8Text = [filename UTF8String];
				char* data = (char*)malloc (strlen (utf8Text) + 1);
				strcpy (data, utf8Text);
				type = CDragContainer::kFile;
				size = strlen (utf8Text);
				lastItem = data;
				return data;
			}
		}
		type = CDragContainer::kError;
		return 0;
	}
	
	type = CDragContainer::kError;
	return 0;
}

//------------------------------------------------------------------------------------
long CocoaDragContainer::getType (long idx) const
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
		if ((long)[fileNames count] > idx)
			return CDragContainer::kFile;
	}

	return CDragContainer::kUnknown;
}

END_NAMESPACE_VSTGUI

#endif // MAC_COCOA
