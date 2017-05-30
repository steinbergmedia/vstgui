// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __uitextfield__
#define __uitextfield__

#include "../../iplatformtextedit.h"

#if TARGET_OS_IPHONE

#ifdef __OBJC__
@class UIView;
@class UITextField;
@class VSTGUI_UITextFieldDelegate;
#else
struct UIView;
struct UITextField;
struct VSTGUI_UITextFieldDelegate;
#endif

namespace VSTGUI {

class UITextEdit : public IPlatformTextEdit
{
public:
	UITextEdit (UIView* parent, IPlatformTextEditCallback* textEdit);
	~UITextEdit ();

	UTF8String getText () override;
	bool setText (const UTF8String& text) override;
	bool updateSize () override;

protected:
	UITextField* platformControl;
	VSTGUI_UITextFieldDelegate* delegate;
	UIView* parent;
};

} // namespace

#endif // TARGET_OS_IPHONE

#endif // __uitextfield__
