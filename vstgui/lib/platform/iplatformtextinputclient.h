// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../vstguifwd.h"
#include <string>

namespace VSTGUI {

//------------------------------------------------------------------------
struct ICocoaTextInputClient
{
	struct TextRange
	{
		size_t position;
		size_t length;
	};

	virtual void insertText (const std::u16string& string, TextRange range) = 0;
	virtual void setMarkedText (const std::u16string& string, TextRange selectedRange,
								TextRange replacementRange) = 0;
	virtual bool hasMarkedText () = 0;
	virtual void unmarkText () = 0;
	virtual TextRange getMarkedRange () = 0;
	virtual TextRange getSelectedRange () = 0;
	virtual CRect firstRectForCharacterRange (TextRange range, TextRange& actualRange) = 0;

	virtual ~ICocoaTextInputClient () noexcept = default;
};

//------------------------------------------------------------------------
} // VSTGUI
