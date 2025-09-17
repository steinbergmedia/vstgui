// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "enumbitset.h"
#include "cview.h"
#include "ccolor.h"
#include "cfont.h"
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
 *
 *	@param r size of the text editor
 *	@param controller the controller of the text editor
 *	@return a new text editor view, the caller owns the returned object
 *
 *	@ingroup new_in_4_15
 */
CView* createNewTextEditor (const CRect& r, ITextEditorController* controller);

//------------------------------------------------------------------------
/** text editor controller interface
 *
 *	@ingroup new_in_4_15
 */
struct ITextEditorController
{
	virtual ~ITextEditorController () noexcept = default;

	virtual void onTextEditorCreated (const ITextEditor& te) = 0;
	virtual void onTextEditorDestroyed (const ITextEditor& te) = 0;
	virtual void onTextEditorTextChanged (const ITextEditor& te) = 0;
};

//------------------------------------------------------------------------
/** text editor interface
 *
 *	@ingroup new_in_4_15
 */
struct ITextEditor
{
	virtual ~ITextEditor () noexcept = default;

	/** Set the plain text content of the editor.
	 *
	 * @param utf8Text The new text to set, as a UTF-8 string view.
	 * @param clearSelection If true, clears the current selection after setting the text.
	 * @return True if the text was set successfully, false otherwise.
	 */
	virtual bool setPlainText (std::string_view utf8Text, bool clearSelection = false) const = 0;

	/** Get the plain text content of the editor.
	 *
	 * @return The current text as a UTF-8 string.
	 */
	virtual std::string getPlainText () const = 0;

	/** Reset the controller associated with this text editor. */
	virtual void resetController () const = 0;

	/** Style configuration for the text editor. */
	struct Style
	{
		/** Style flags for the text editor. */
		enum class Flags : uint32_t
		{
			ShowLineNumbers,	 ///< Show line numbers in the editor.
			HighlightCursorLine, ///< Highlight the line with the cursor.
			ReadOnlyMode		 ///< Enable read-only mode.
		};
		using FlagsBitset = EnumBitset<Flags>;

		SharedPointer<CFontDesc> font {kNormalFont};
		SharedPointer<CFontDesc> lineNumbersFont {kNormalFontSmall};
		CColor textColor {kBlackCColor};
		CColor backColor {kWhiteCColor};
		CColor cursorColor {kBlackCColor};
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
		uint32_t cursorBlinkTime {500}; ///< Cursor blink time in milliseconds.
		uint32_t tabWidth {4};			///< Tab width in spaces.
		FlagsBitset flags {{Flags::ShowLineNumbers, Flags::HighlightCursorLine}};
	};

	/** Set the style of the text editor.
	 *
	 * @param style The style configuration to apply.
	 */
	virtual void setStyle (const Style& style) const = 0;

	/** Editor command enumeration. */
	enum class Command : uint32_t
	{
		Undo,				 ///< Undo the last action.
		Redo,				 ///< Redo the last undone action.
		Cut,				 ///< Cut the selected text.
		Copy,				 ///< Copy the selected text.
		Paste,				 ///< Paste text from the clipboard.
		FindNext,			 ///< Find the next occurrence.
		FindPrevious,		 ///< Find the previous occurrence.
		SelectAll,			 ///< Select all text.
		UseSelectionForFind, ///< Use the current selection for find.
		ShiftLeft,			 ///< Shift selection left.
		ShiftRight,			 ///< Shift selection right.
		ShowFindPanel,		 ///< Show the find panel.
		TakeFocus			 ///< Take keyboard focus.
	};

	/** Query if a command can be handled by the editor.
	 *
	 * @param cmd The command to query.
	 * @return True if the command can be handled, false otherwise.
	 */
	virtual bool canHandleCommand (Command cmd) const = 0;

	/** Handle a command in the editor.
	 *
	 * @param cmd The command to handle.
	 * @return True if the command was handled, false otherwise.
	 */
	virtual bool handleCommand (Command cmd) const = 0;

	/** Set a key binding for a command.
	 *
	 * @param cmd The command to bind.
	 * @param character The character to bind.
	 * @param virt The virtual key code.
	 * @param modifiers The key modifiers.
	 * @return True if the key binding was set, false otherwise.
	 */
	virtual bool setCommandKeyBinding (Command cmd, char32_t character, VirtualKey virt,
									   Modifiers modifiers) const = 0;

	/** Find options for searching text. */
	enum class FindOption : uint32_t
	{
		CaseSensitive, ///< Match case when searching.
		WholeWords,	   ///< Match whole words only.
	};
	using FindOptions = EnumBitset<FindOption>;

	/** Set the find options for searching text.
	 *
	 * @param opt The find options to set.
	 */
	virtual void setFindOptions (FindOptions opt) const = 0;

	/** Set the string to search for.
	 *
	 * @param utf8Text The search string as a UTF-8 string view.
	 */
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
/** Extension to ITextEditor, use a dynamic_cast to get it from an ITextEditor
 *
 *	@ingroup new_in_4_15
 */
struct IEditorExt
{
	using ReadCallbackFunc = std::function<void (std::u32string_view text)>;
	/** Get access to the internal string buffer of the text editor
	 *
	 *	The callback is called synchronously within this call. The text passed to the callback is
	 *	only valid inside the callback.
	 *
	 *	@param startOffset Offset into the buffer in number of characters
	 *	@param length Number of characters
	 *	@param callback The callback which receives the text
	 *	@return True if the callback was called, false otherwise
	 */
	virtual bool readText (size_t startOffset, size_t length,
						   const ReadCallbackFunc& callback) const = 0;
	/** Get the length of the text
	 *
	 *	@return number of characters
	 */
	virtual size_t getTextLength () const = 0;
};

//------------------------------------------------------------------------
/** Extension to ITextEditorController
 *
 *	@ingroup new_in_4_15
 */
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

	/** Notification that drawing begins
	 *
	 *	@param editor reference to the editor
	 */
	virtual void beginDraw (const IEditorExt& editor) = 0;
	/** Get the styles of the text
	 *
	 *	The text editor calls this while drawing to get the styles for the text range.
	 *	The returned styles must be orded from front to back.
	 *	If ranges are missing in the styles, they are rendered with the default style.
	 *
	 *	@param editor reference to the editor
	 *	@param beginOffset offset into the text buffer in number of characters
	 *	@param length number of characters
	 *	@return vector of styles for the range
	 */
	virtual Styles getStyles (const IEditorExt& editor, size_t beginOffset, size_t length) = 0;
	/** Notification that drawing has ended
	 *
	 *	@param editor reference to the editor
	 */
	virtual void endDraw (const IEditorExt& editor) = 0;
};

//------------------------------------------------------------------------
} // TextEditorColorization
} // VSTGUI
