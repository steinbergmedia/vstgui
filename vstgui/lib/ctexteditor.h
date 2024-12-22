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

struct ITextEditor;
struct ITextEditorController;

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
		enum class Flags : uint32_t
		{
			ShowLineNumbers,
			HighlightCursorLine
		};
		using FlagsBitset = EnumBitset<Flags>;

		SharedPointer<CFontDesc> font {kNormalFont};
		SharedPointer<CFontDesc> lineNumbersFont {kNormalFontSmall};
		CColor textColor {kBlackCColor};
		CColor backColor {kWhiteCColor};
		CColor highlightCursorLineColor {0, 0, 0, 10};
		CColor selectionBackColor {kGreyCColor};
		CColor frameColor {kGreyCColor};
		CColor lineNumberTextColor {127, 127, 127, 100};
		CColor lineNumberTextSelectedColor {100, 100, 100, 255};
		CColor lineNumberLine {127, 127, 127, 100};
		CCoord lineSpacing {2.5};
		CCoord leftMargin {4.};
		CCoord lineNumberLeftMargin {2.};
		CCoord lineNumberRightMargin {2.};
		uint32_t cursorBlinkTime {500}; // in milliseconds
		uint32_t tabWidth {4};
		FlagsBitset flags {{Flags::ShowLineNumbers, Flags::HighlightCursorLine}};
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
	virtual bool setCommandKeyBinding (Command cmd, char32_t character, VirtualKey virt,
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
namespace TextEditorColorization {

//------------------------------------------------------------------------
/** extension to ITextEditor, use a dynamic_cast to get it from an ITextEditor */
struct IEditorExt
{
	/** get access to the internal string buffer of the text editor
	 *
	 *	@param startOffset offset into the buffer in number of characters
	 *	@param length number of characters
	 *	@return a string view into the buffer
	 */
	virtual std::u32string_view readText (size_t startOffset, size_t length) const = 0;
	/** get the length of the text
	 *
	 *	@return number of characters
	 */
	virtual size_t getTextLength () const = 0;
};

//------------------------------------------------------------------------
/** extension to ITextEditorController */
struct IStyleProvider
{
	using IEditorExt = TextEditorColorization::IEditorExt;

	struct Style
	{
		size_t start;
		size_t length;
		CColor color;
	};
	using Styles = std::vector<Style>;

	/** notification that drawing begins
	 *
	 *	@param editor reference to the editor
	 */
	virtual void beginDraw (const IEditorExt& editor) = 0;
	/** get the styles of the text
	 *
	 *	The returned styles must be orded from front to back.
	 *	If ranges are missing in the styles, they are rendered with the default style.
	 *
	 *	@param editor reference to the editor
	 *	@param beginOffset offset into the text buffer in number of characters
	 *	@param length number of characters
	 *	@return vector of styles for the range
	 */
	virtual Styles getStyles (const IEditorExt& editor, size_t beginOffset, size_t length) = 0;
	/** notification that drawing has ended
	 *
	 *	@param editor reference to the editor
	 */
	virtual void endDraw (const IEditorExt& editor) = 0;
};

//------------------------------------------------------------------------
} // TextEditorColorization
} // VSTGUI
