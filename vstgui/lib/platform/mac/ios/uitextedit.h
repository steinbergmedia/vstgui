// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

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
	bool drawsPlaceholder () const override { return true; }

protected:
	UITextField* platformControl;
	VSTGUI_UITextFieldDelegate* delegate;
	UIView* parent;
};

} // VSTGUI

#endif // TARGET_OS_IPHONE
