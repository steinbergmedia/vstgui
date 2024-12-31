// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../vstguifwd.h"
#include <functional>
#include <string>

namespace VSTGUI {

//------------------------------------------------------------------------
using TextInputClientCancelCallback = std::function<void ()>;

//------------------------------------------------------------------------
struct ICocoaTextInputClient
{
	struct TextRange
	{
		size_t position;
		size_t length;
	};

	virtual void insertText (const std::u32string& string, TextRange range) = 0;
	virtual void setMarkedText (const std::u32string& string, TextRange selectedRange,
								TextRange replacementRange) = 0;
	virtual bool hasMarkedText () = 0;
	virtual void unmarkText () = 0;
	virtual TextRange getMarkedRange () = 0;
	virtual TextRange getSelectedRange () = 0;
	virtual CRect firstRectForCharacterRange (TextRange range, TextRange& actualRange) = 0;
	virtual std::u32string substringForRange (TextRange range, TextRange& actualRange) = 0;
	virtual size_t characterIndexForPoint (CPoint pos) = 0;

	virtual void setCancelCallback (const TextInputClientCancelCallback& callback) = 0;

	virtual ~ICocoaTextInputClient () noexcept = default;
};

//------------------------------------------------------------------------
struct IIMETextInputClient
{
	struct CharPosition
	{
		uint32_t characterPosition;
		CPoint position;
		double lineHeight;
		CRect documentRect;
	};

	virtual bool ime_queryCharacterPosition (CharPosition& cp) = 0;
	virtual void ime_setMarkedText (const std::u32string& string) = 0;
	virtual void ime_insertText (const std::u32string& string) = 0;
	virtual void ime_unmarkText () = 0;

	virtual ~IIMETextInputClient () noexcept = default;
};

//------------------------------------------------------------------------
} // VSTGUI
