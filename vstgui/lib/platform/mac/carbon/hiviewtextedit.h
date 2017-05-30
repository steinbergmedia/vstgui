// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
	~HIViewTextEdit () noexcept;
	
	UTF8String getText () override;
	bool setText (const UTF8String& text) override;
	bool updateSize () override;

	HIViewRef getPlatformControl () const { return platformControl; }
//-----------------------------------------------------------------------------
protected:
	void freeText ();

	HIViewRef platformControl;
	EventHandlerRef eventHandler;
	UTF8StringBuffer text;

	static pascal OSStatus CarbonEventsTextControlProc (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
};

} // namespace

#endif // MAC_CARBON

#endif // __hiviewtextedit__
