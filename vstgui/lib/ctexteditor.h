// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "enumbitset.h"
#include "cview.h"
#include "ccolor.h"
#include <string_view>

//------------------------------------------------------------------------
namespace VSTGUI {

struct ITextEditorController;
struct ITextEditor;

//------------------------------------------------------------------------
/** Create a new texteditor
 *
 *	To handle the life time between the controller and the texteditor it is necessary that the
 *	controller listens to the destroy call of the text editor and then reset the controller on the
 *	texteditor, or when the controller is destroyed first to also reset the controller on the
 *	texteditor.
 */
CView* createNewTextEditor (const CRect& r, ITextEditorController* controller);

//------------------------------------------------------------------------
struct ITextEditorController
{
	virtual ~ITextEditorController () noexcept = default;

	virtual void onTextEditorCreated (const ITextEditor& te) = 0;
	virtual void onTextEditorDestroyed (const ITextEditor& te) = 0;
	virtual void onTextEditorTextChanged (const ITextEditor& te) = 0;
};

//------------------------------------------------------------------------
struct ITextEditor
{
	virtual ~ITextEditor () noexcept = default;

	virtual bool setPlainText (std::string_view utf8Text) const = 0;
	virtual std::string getPlainText () const = 0;

	virtual void resetController () const = 0;

	struct Style
	{
		SharedPointer<CFontDesc> font {kNormalFont};
		SharedPointer<CFontDesc> lineNumbersFont {kNormalFontSmall};
		CColor textColor {kBlackCColor};
		CColor backColor {kWhiteCColor};
		CColor selectionBackColor {kGreyCColor};
		CColor frameColor {kGreyCColor};
		CColor lineNumberTextColor {127, 127, 127, 100};
		CColor lineNumberTextSelectedColor {100, 100, 100, 255};
		CColor lineNumberLine {127, 127, 127, 100};
		CCoord lineSpacing {2.5};
		CCoord leftMargin {4.};
		CCoord lineNumerLeftMargin {2.};
		CCoord lineNumerRightMargin {2.};
		uint32_t cursorBlinkTime {500}; // in milliseconds
		uint32_t tabWidth {4};
		bool showLineNumbers {true};
	};

	virtual void setStyle (const Style& style) const = 0;

	enum class Command : uint32_t
	{
		Undo,
		Redo,
		Cut,
		Copy,
		Paste,
		FindNext,
		FindPrevious,
		SelectAll,
		UseSelectionForFind,
		ShiftLeft,
		ShiftRight,
		ShowFindPanel,
		TakeFocus,
	};

	virtual bool canHandleCommand (Command cmd) const = 0;
	virtual bool handleCommand (Command cmd) const = 0;
	virtual bool setCommandKeyBinding (Command cmd, char16_t character, VirtualKey virt,
									   Modifiers modifiers) const = 0;

	enum class FindOption : uint32_t
	{
		CaseSensitive,
		WholeWords,
	};
	using FindOptions = EnumBitset<FindOption>;

	virtual void setFindOptions (FindOptions opt) const = 0;
	virtual void setFindString (std::string_view utf8Text) const = 0;
};

//------------------------------------------------------------------------
struct TextEditorControllerAdapter : ITextEditorController
{
	void onTextEditorCreated (const ITextEditor& te) override {}
	void onTextEditorDestroyed (const ITextEditor& te) override {}
	void onTextEditorTextChanged (const ITextEditor& te) override {}
};

//------------------------------------------------------------------------
} // VSTGUI
