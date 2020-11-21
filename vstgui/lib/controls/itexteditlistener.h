// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../vstguifwd.h"

namespace VSTGUI {

//------------------------------------------------------------------------
/** Listener for a text edit
 *	@ingroup new_in_4_10
 */
class ITextEditListener
{
public:
	/** called when the native platform text edit control was created and started to listen for keyboard input. */
	virtual void onTextEditPlatformControlTookFocus (CTextEdit* textEdit) = 0;
	/** called when the natvie platform text edit control is going to be destroyed. */
	virtual void onTextEditPlatformControlLostFocus (CTextEdit* textEdit) = 0;
};

//------------------------------------------------------------------------
class TextEditListenerAdapter : public ITextEditListener
{
public:
	void onTextEditPlatformControlTookFocus (CTextEdit* textEdit) override {}
	void onTextEditPlatformControlLostFocus (CTextEdit* textEdit) override {}
};

//------------------------------------------------------------------------
} // VSTGUI
