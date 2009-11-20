
#ifndef __hiviewtextedit__
#define __hiviewtextedit__

#include "../../iplatformtextedit.h"

#if MAC_CARBON

#include <Carbon/Carbon.h>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class HIViewTextEdit : public IPlatformTextEdit
{
public:
	HIViewTextEdit (HIViewRef parent, IPlatformTextEditCallback* textEdit);
	~HIViewTextEdit ();
	
	bool getText (char* text, long maxSize);
	bool setText (const char* text);
	bool updateSize ();

	HIViewRef getPlatformControl () const { return platformControl; }
//-----------------------------------------------------------------------------
protected:
	HIViewRef platformControl;
	EventHandlerRef eventHandler;

	static pascal OSStatus CarbonEventsTextControlProc (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
};

} // namespace

#endif // MAC_CARBON

#endif // __hiviewtextedit__
