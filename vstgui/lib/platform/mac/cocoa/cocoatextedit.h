// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
	~CocoaTextEdit () noexcept override;
	
	UTF8String getText () override;
	bool setText (const UTF8String& text) override;
	bool updateSize () override;

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
