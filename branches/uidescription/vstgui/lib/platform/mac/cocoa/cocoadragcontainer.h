#ifndef __cocoadragcontainer__
#define __cocoadragcontainer__

#include "../../../cframe.h"

#if MAC_COCOA && VSTGUI_PLATFORM_ABSTRACTION

#ifdef __OBJC__
@class NSPasteboard;
#else
struct NSPasteboard;
#endif

BEGIN_NAMESPACE_VSTGUI

//------------------------------------------------------------------------------------
class CocoaDragContainer : public CDragContainer
{
public:
	CocoaDragContainer (NSPasteboard* platformDrag);
	~CocoaDragContainer ();
	
	void* first (long& size, long& type);
	void* next (long& size, long& type);
	long getType (long idx) const;
	long getCount () const { return nbItems; }

protected:
	NSPasteboard* pb;

	long nbItems;
	
	long iterator;
	void* lastItem;
};

END_NAMESPACE_VSTGUI

#endif

#endif
