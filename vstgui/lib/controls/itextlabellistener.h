// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../vstguifwd.h"

namespace VSTGUI {

//------------------------------------------------------------------------
/** Listener for a text label
 *	@ingroup new_in_4_7
 */
class ITextLabelListener
{
public:
	/** the truncated text has changed */
	virtual void onTextLabelTruncatedTextChanged (CTextLabel* label) = 0;
};

//------------------------------------------------------------------------
class TextLabelListenerAdapter : public ITextLabelListener
{
public:
	void onTextLabelTruncatedTextChanged (CTextLabel* label) override {}
};

//------------------------------------------------------------------------
} // VSTGUI
