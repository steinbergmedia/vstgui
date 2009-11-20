
#ifndef __cocoatextedit__
#define __cocoatextedit__

#include "../../iplatformtextedit.h"

#if MAC_COCOA

#ifdef __OBJC__
@class NSView;
@class NSTextField;
#else
struct NSView;
struct NSTextField;
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
class CocoaTextEdit : public IPlatformTextEdit
{
public:
	CocoaTextEdit (NSView* parent, IPlatformTextEditCallback* textEdit);
	~CocoaTextEdit ();
	
	bool getText (char* text, long maxSize);
	bool setText (const char* text);
	bool updateSize ();

	NSTextField* getPlatformControl () const { return platformControl; }
	NSView* getParent () const { return parent; }
	IPlatformTextEditCallback* getTextEdit () const { return textEdit; }
//-----------------------------------------------------------------------------
protected:
	static void initClass ();

	NSTextField* platformControl;
	NSView* parent;
};

} // namespace

#endif // MAC_COCOA

#endif // __cocoatextedit__
