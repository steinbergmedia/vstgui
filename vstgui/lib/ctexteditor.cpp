// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "ccolor.h"
#include "cdrawcontext.h"
#include "cdropsource.h"
#include "cframe.h"
#include "controls/cscrollbar.h"
#include "cscrollview.h"
#include "ctexteditor.h"
#include "cgraphicspath.h"
#include "events.h"
#include "iviewlistener.h"
#include "cvstguitimer.h"
#include "finally.h"
#include "platform/iplatformfont.h"
#include "platform/iplatformframe.h"
#include "platform/platformfactory.h"
#include "animation/timingfunctions.h"
#include "animation/ianimationtarget.h"

#include <codecvt>
#include <locale>
#include <string>
#include <bitset>
#include <cwctype>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace TextEditor {

using CharT = char16_t;
#define STB_TEXTEDIT_CHARTYPE CharT
#define STB_TEXTEDIT_POSITIONTYPE int32_t
#define STB_TEXTEDIT_STRING TextEditorView
#define STB_TEXTEDIT_KEYTYPE uint32_t

#include "platform/common/stb_textedit.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4996)
#endif


using StringConvert = std::wstring_convert<std::codecvt_utf8_utf16<CharT>, CharT>;
//------------------------------------------------------------------------
inline std::u16string convert (const char* text, size_t numChars)
{
	return StringConvert {}.from_bytes (text, text + numChars);
}

//------------------------------------------------------------------------
inline std::u16string convert (const std::string& str) { return StringConvert {}.from_bytes (str); }

//------------------------------------------------------------------------
inline std::string convert (const char16_t* text, size_t numChars)
{
	return StringConvert {}.to_bytes (text, text + numChars);
}

//------------------------------------------------------------------------
inline std::string convert (const std::u16string& str) { return StringConvert {}.to_bytes (str); }

#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

//------------------------------------------------------------------------
struct Range
{
	size_t start {0};
	size_t length {0};

	explicit operator bool () const { return length > 0; }
	size_t end () const { return start + length; }
};

//------------------------------------------------------------------------
bool operator== (const Range& r1, const Range& r2)
{
	return r1.start == r2.start && r1.length == r2.length;
}

//------------------------------------------------------------------------
bool operator!= (const Range& r1, const Range& r2)
{
	return r1.start != r2.start || r1.length != r2.length;
}

//------------------------------------------------------------------------
inline Range makeRange (size_t start, size_t end)
{
	if (start > end)
		std::swap (start, end);
	return Range {static_cast<size_t> (start), static_cast<size_t> (end - start)};
}

//------------------------------------------------------------------------
inline Range makeRange (const STB_TexteditState& state)
{
	return makeRange (state.select_start, state.select_end);
}

using String = std::u16string;
using StringView = std::u16string_view;

//------------------------------------------------------------------------
struct Line
{
	Range range;
	UTF8String text;
	CCoord width {};
};
using Lines = std::vector<Line>;

//------------------------------------------------------------------------
struct TextModel
{
	String text;
	Lines lines;
};

//------------------------------------------------------------------------
struct NewLineProcessor
{
	static void update (Lines& lines, StringView text, size_t startRow);
};

struct LineNumberView;

static constexpr CPoint MouseOutsidePos = {std::numeric_limits<CCoord>::max (),
										   std::numeric_limits<CCoord>::max ()};

//------------------------------------------------------------------------
struct TextEditorView : public CView,
						public ITextEditor,
						public IFocusDrawing,
						public ViewEventListenerAdapter
{
	TextEditorView (ITextEditorController* controller);
	void beforeDelete () override;

	void drawRect (CDrawContext* pContext, const CRect& dirtyRect) override;
	bool attached (CView* parent) override;
	bool removed (CView* parent) override;
	void parentSizeChanged () override;

	void looseFocus () override;
	void takeFocus () override;

	void onKeyboardEvent (KeyboardEvent& event) override;
	void onMouseDownEvent (MouseDownEvent& event) override;
	void onMouseMoveEvent (MouseMoveEvent& event) override;
	void onMouseUpEvent (MouseUpEvent& event) override;
	void onMouseCancelEvent (MouseCancelEvent& event) override;
	void onMouseEnterEvent (MouseEnterEvent& event) override;
	void onMouseExitEvent (MouseExitEvent& event) override;

	void viewOnEvent (CView* view, Event& event) override;

	int32_t deleteChars (size_t pos, size_t num);
	int32_t insertChars (size_t pos, const CharT* text, size_t num);
	void layout (StbTexteditRow* row, size_t start_i) const;
	float getCharWidth (size_t row, size_t pos) const;
	CharT getChar (int32_t pos) const;
	int32_t getLength () const;
	size_t moveToWordPrevious (size_t pos) const;
	size_t moveToWordNext (size_t pos) const;

	// STB
	static int32_t deleteChars (TextEditorView* self, size_t pos, size_t num)
	{
		return self->deleteChars (pos, num);
	}
	static int32_t insertChars (TextEditorView* self, size_t pos, const CharT* text, size_t num)
	{
		return self->insertChars (pos, text, num);
	}
	static void layout (StbTexteditRow* row, TextEditorView* self, size_t start_i)
	{
		self->layout (row, start_i);
	}
	static float getCharWidth (TextEditorView* self, size_t row, size_t pos)
	{
		return self->getCharWidth (row, pos);
	}
	static CharT getChar (TextEditorView* self, int32_t pos) { return self->getChar (pos); }
	static int32_t getLength (TextEditorView* self) { return self->getLength (); }

	static int moveToWordPrevious (TextEditorView* self, size_t pos)
	{
		return static_cast<int> (self->moveToWordPrevious (pos));
	}
	static int moveToWordNext (TextEditorView* self, size_t pos)
	{
		return static_cast<int> (self->moveToWordNext (pos));
	}

protected:
	// IFocusDrawing
	bool drawFocusOnTop () override;
	bool getFocusPath (CGraphicsPath& outPath) override;

	// ITextEditor
	bool setPlainText (std::string_view utf8Text) const override;
	std::string getPlainText () const override;
	void resetController () const override;
	void setStyle (const Style& style) const override;
	bool canHandleCommand (Command cmd) const override;
	bool handleCommand (Command cmd) const override;
	bool setCommandKeyBinding (Command cmd, char16_t character, VirtualKey virt,
							   Modifiers modifiers) const override;

	// commandos
	void selectAll ();
	bool doCut ();
	bool doCopy ();
	bool doPaste ();
	bool useSelectionForFind ();
	bool doFind (bool forward = true);

private:
	template<typename Proc>
	bool callSTB (Proc proc);

	enum Dirty
	{
		UI = 1 << 0,
		Layout = 1 << 1,

		All = UI | Layout,
	};
	void validateLineStarts (Lines::const_iterator it, const Lines::const_iterator& end);
	void invalidateSingleLine (size_t pos, int32_t numChars);
	void invalidateSingleLine (Lines::iterator& line, int32_t numChars);
	void invalidateLines (size_t pos, int32_t numChars);
	void invalidate (Dirty what = Dirty::UI) const;
	void invalidLine (size_t index, bool completeWidth = false);
	void invalidLine (Lines::const_iterator it, bool completeWidth = false);
	void invalidSelectedLines ();
	CCoord updateLineText (Lines::iterator& line);
	CRect calculateLineRect (size_t index) const;
	CRect calculateLineRect (Lines::const_iterator it) const;
	CCoord calculateMaxWidth () const;
	void updateLineNumbersView () const;
	void layoutRows ();
	void onCursorChanged (int oldCursorPos, int newCursorPos);
	void onSelectionChanged (Range newSel, bool forceInvalidation = false);
	void selectOnDoubleClick (uint32_t clickCount);
	void updateSelectionOnDoubleClickMove (uint32_t clickCount);
	void insertNewLine ();
	template<typename T>
	T findLine (T begin, T end, size_t pos) const;
	CRect calculateCursorRect (int cursor) const;
	CRect invalidCursorRect ();
	void toggleCursorVisibility ();
	void restartBlinkTimer ();

	//------------------------------------------------------------------------
	mutable TextModel model;

	mutable std::shared_ptr<Style> style {std::make_shared<Style> ()};
	mutable ITextEditorController* controller {nullptr};
	mutable const IFontPainter* fontPainer {nullptr};

	CScrollView* scrollView {nullptr};
	mutable SharedPointer<LineNumberView> lineNumberView;

	SharedPointer<CVSTGUITimer> blinkTimer;

	CRect cursorRect {};
	CRect lastDrawnCursorRect {};
	Range selectedLines {};
	CPoint lastMouse {MouseOutsidePos};
	CCoord maxHeight {0};
	CCoord maxWidth {200.};
	mutable CCoord lineHeight {style->font->getSize () + style->lineSpacing};
	mutable CCoord fontAscent {0.};
	mutable CCoord fontDescent {0.};

	STB_TexteditState editState {};
	STB_TexteditState editStateOnMouseDown {};
	bool mouseIsDown {false};
	bool cursorIsVisible {false};
	float cursorAlpha {1.f};

	mutable Lines::const_iterator stbInternalIterator;

	String findString;

	struct Key
	{
		char16_t character;
		VirtualKey virt;
		Modifiers modifiers;

		bool operator== (const KeyboardEvent& event) const
		{
			return event.character == character && event.virt == virt &&
				   event.modifiers == modifiers;
		}
	};
	mutable std::array<Key, static_cast<size_t> (Command::UseSelectionForFind) + 1> commandKeys;
};

#define VIRTUAL_KEY_BIT 0x80000000
#define STB_TEXTEDIT_K_SHIFT 0x40000000
#define STB_TEXTEDIT_K_CONTROL 0x20000000
#define STB_TEXTEDIT_K_ALT 0x10000000
// key-bindings
#define STB_TEXTEDIT_K_LEFT (VIRTUAL_KEY_BIT | VKEY_LEFT)
#define STB_TEXTEDIT_K_RIGHT (VIRTUAL_KEY_BIT | VKEY_RIGHT)
#define STB_TEXTEDIT_K_UP (VIRTUAL_KEY_BIT | VKEY_UP)
#define STB_TEXTEDIT_K_DOWN (VIRTUAL_KEY_BIT | VKEY_DOWN)
#if MAC
#define STB_TEXTEDIT_K_LINESTART (STB_TEXTEDIT_K_CONTROL | STB_TEXTEDIT_K_LEFT)
#define STB_TEXTEDIT_K_LINEEND (STB_TEXTEDIT_K_CONTROL | STB_TEXTEDIT_K_RIGHT)
#define STB_TEXTEDIT_K_WORDLEFT (STB_TEXTEDIT_K_ALT | STB_TEXTEDIT_K_LEFT)
#define STB_TEXTEDIT_K_WORDRIGHT (STB_TEXTEDIT_K_ALT | STB_TEXTEDIT_K_RIGHT)
#define STB_TEXTEDIT_K_TEXTSTART (STB_TEXTEDIT_K_CONTROL | STB_TEXTEDIT_K_UP)
#define STB_TEXTEDIT_K_TEXTEND (STB_TEXTEDIT_K_CONTROL | STB_TEXTEDIT_K_DOWN)
#else
#define STB_TEXTEDIT_K_LINESTART (VIRTUAL_KEY_BIT | VKEY_HOME)
#define STB_TEXTEDIT_K_LINEEND (VIRTUAL_KEY_BIT | VKEY_END)
#define STB_TEXTEDIT_K_WORDLEFT (STB_TEXTEDIT_K_LEFT | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_WORDRIGHT (STB_TEXTEDIT_K_RIGHT | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_TEXTSTART (STB_TEXTEDIT_K_LINESTART | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_TEXTEND (STB_TEXTEDIT_K_LINEEND | STB_TEXTEDIT_K_CONTROL)
#endif
#define STB_TEXTEDIT_K_DELETE (VIRTUAL_KEY_BIT | VKEY_DELETE)
#define STB_TEXTEDIT_K_BACKSPACE (VIRTUAL_KEY_BIT | VKEY_BACK)
#define STB_TEXTEDIT_K_UNDO (STB_TEXTEDIT_K_CONTROL | 'z')
#define STB_TEXTEDIT_K_REDO (STB_TEXTEDIT_K_CONTROL | STB_TEXTEDIT_K_SHIFT | 'z')
#define STB_TEXTEDIT_K_INSERT (VIRTUAL_KEY_BIT | VKEY_INSERT)
#define STB_TEXTEDIT_K_PGUP (VIRTUAL_KEY_BIT | VKEY_PAGEUP)
#define STB_TEXTEDIT_K_PGDOWN (VIRTUAL_KEY_BIT | VKEY_PAGEDOWN)
// functions
#define STB_TEXTEDIT_STRINGLEN(tc) TextEditorView::getLength (tc)
#define STB_TEXTEDIT_LAYOUTROW TextEditorView::layout
#define STB_TEXTEDIT_GETWIDTH(tc, n, i) TextEditorView::getCharWidth (tc, n, i)
#define STB_TEXTEDIT_KEYTOTEXT(key)                                                                \
	((key & VIRTUAL_KEY_BIT) ? 0 : ((key & STB_TEXTEDIT_K_CONTROL) ? 0 : (key & (~0xF0000000))));
#define STB_TEXTEDIT_GETCHAR(tc, i) TextEditorView::getChar (tc, i)
#define STB_TEXTEDIT_NEWLINE '\n'
#define STB_TEXTEDIT_DELETECHARS TextEditorView::deleteChars
#define STB_TEXTEDIT_INSERTCHARS TextEditorView::insertChars
#define STB_TEXTEDIT_MOVEWORDLEFT TextEditorView::moveToWordPrevious
#define STB_TEXTEDIT_MOVEWORDRIGHT TextEditorView::moveToWordNext
#define STB_TEXTEDIT_GETWIDTH_NEWLINE -1.f

#define STB_TEXTEDIT_IMPLEMENTATION
#include "platform/common/stb_textedit.h"
#undef STB_TEXTEDIT_IMPLEMENTATION

//------------------------------------------------------------------------
struct LineNumberView : CView,
						ViewListenerAdapter
{
	LineNumberView (CView* textEditorView);
	~LineNumberView () noexcept override;

	void drawRect (CDrawContext* context, const CRect& dirtyRect) override;

	void setStyle (const std::shared_ptr<ITextEditor::Style>& newStyle, CCoord newLineHeight);
	void setNumLines (size_t numberOfLines);
	void setSelectedLines (Range range);

private:
	void viewSizeChanged (CView* view, const CRect& oldSize) override;
	void viewWillDelete (CView* view) override;

	std::shared_ptr<ITextEditor::Style> style;
	CCoord lineHeight {};
	size_t numLines {0};
	Range selectedLines {};
	CView* textEditorView {nullptr};
};

static constexpr size_t Index (ITextEditor::Command cmd) { return static_cast<size_t> (cmd); }

//------------------------------------------------------------------------
TextEditorView::TextEditorView (ITextEditorController* controller)
: CView ({0, 0, 10, 10}), controller (controller)
{
	setWantsFocus (true);
	stb_textedit_initialize_state (&editState, false);
	commandKeys[Index (Command::SelectAll)] = {u'a', VirtualKey::None, {ModifierKey::Control}};
	commandKeys[Index (Command::Cut)] = {u'x', VirtualKey::None, {ModifierKey::Control}};
	commandKeys[Index (Command::Copy)] = {u'c', VirtualKey::None, {ModifierKey::Control}};
	commandKeys[Index (Command::Paste)] = {u'v', VirtualKey::None, {ModifierKey::Control}};
	commandKeys[Index (Command::Undo)] = {u'z', VirtualKey::None, {ModifierKey::Control}};
	commandKeys[Index (Command::Redo)] = {
		u'z', VirtualKey::None, {ModifierKey::Control, ModifierKey::Shift}};
#if MAC
	commandKeys[Index (Command::FindNext)] = {u'g', VirtualKey::None, {ModifierKey::Control}};
	commandKeys[Index (Command::FindPrevious)] = {
		u'g', VirtualKey::None, {ModifierKey::Control, ModifierKey::Shift}};
	commandKeys[Index (Command::UseSelectionForFind)] = {
		u'e', VirtualKey::None, {ModifierKey::Control}};
#else
	commandKeys[Index (Command::FindNext)] = {0, VirtualKey::F3, {}};
	commandKeys[Index (Command::FindPrevious)] = {0, VirtualKey::F3, {ModifierKey::Shift}};
	commandKeys[Index (Command::UseSelectionForFind)] = {0, VirtualKey::F3, {ModifierKey::Control}};
#endif
	controller->onTextEditorCreated (*this);
}

//------------------------------------------------------------------------
void TextEditorView::beforeDelete ()
{
	if (controller)
		controller->onTextEditorDestroyed (*this);
	CView::beforeDelete ();
}

//------------------------------------------------------------------------
bool TextEditorView::attached (CView* parent)
{
	if (CView::attached (parent))
	{
		if (auto sv = dynamic_cast<CScrollView*> (parent->getParentView ()))
		{
			scrollView = sv;
			layoutRows ();
			scrollView->registerViewEventListener (this);
			if (auto sb = scrollView->getVerticalScrollbar ())
				sb->registerViewEventListener (this);
			if (style->showLineNumbers)
			{
				lineNumberView = makeOwned<LineNumberView> (this);
				lineNumberView->setStyle (style, lineHeight);
				updateLineNumbersView ();
				scrollView->setEdgeView (CScrollView::Edge::Left, lineNumberView);
				lineNumberView->remember ();
			}
		}
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool TextEditorView::removed (CView* parent)
{
	if (scrollView)
	{
		scrollView->unregisterViewEventListener (this);
		if (auto sb = scrollView->getVerticalScrollbar ())
			sb->unregisterViewEventListener (this);
	}
	scrollView = nullptr;
	blinkTimer = nullptr;
	lineNumberView = nullptr;
	return CView::removed (parent);
}

//------------------------------------------------------------------------
void TextEditorView::takeFocus ()
{
	if (lastMouse != MouseOutsidePos)
		getFrame ()->setCursor (kCursorIBeam);
	restartBlinkTimer ();
}

//------------------------------------------------------------------------
void TextEditorView::looseFocus ()
{
	blinkTimer = nullptr;
	cursorIsVisible = true;
	toggleCursorVisibility ();
}

//------------------------------------------------------------------------
void TextEditorView::parentSizeChanged ()
{
	if (scrollView)
	{
		auto func = [this] () mutable {
			if (!scrollView)
				return;
			auto viewSize = getViewSize ();
			viewSize.setHeight (maxHeight);
			viewSize.setWidth (maxWidth);
			auto containerSize = scrollView->calculateOptimalContainerSize ();
			if (containerSize.getWidth () > viewSize.getWidth ())
			{
				if (viewSize.getHeight () > containerSize.getHeight ())
					viewSize.setWidth (containerSize.getWidth () -
									   scrollView->getScrollbarWidth ());
				else
					viewSize.setWidth (containerSize.getWidth ());
			}
			if (containerSize.getHeight () > viewSize.getHeight ())
			{
				if (viewSize.getWidth () > containerSize.getWidth ())
					viewSize.setHeight (containerSize.getHeight () -
										scrollView->getScrollbarWidth ());
				else
					viewSize.setHeight (containerSize.getHeight ());
			}
			setViewSize (viewSize);
		};
		auto frame = getFrame ();
		if (frame && frame->inEventProcessing ())
		{
			frame->doAfterEventProcessing (func);
			return;
		}
		else
		{
			Call::later (func, 1);
		}
	}
	else
	{
		auto viewSize = getViewSize ();
		viewSize.setHeight (maxHeight);
		viewSize.setWidth (maxWidth);
		setViewSize (viewSize);
	}
}

//------------------------------------------------------------------------
bool TextEditorView::drawFocusOnTop () { return true; }

//------------------------------------------------------------------------
bool TextEditorView::getFocusPath (CGraphicsPath& outPath)
{
	auto r = getVisibleViewSize ();
	outPath.addRect (r);
	CCoord focusWidth = getFrame ()->getFocusWidth ();
	r.inset (focusWidth, focusWidth);
	outPath.addRect (r);
	return true;
}

//------------------------------------------------------------------------
void TextEditorView::viewOnEvent (CView* view, Event& event)
{
	if (event.type != EventType::MouseWheel)
		return;
	auto& wheelEvent = castMouseWheelEvent (event);
	CPoint scrollOffset {};
	if (wheelEvent.flags & MouseWheelEvent::Flags::PreciseDeltas)
	{
		scrollOffset.x += std::ceil (wheelEvent.deltaX / 0.1);
		scrollOffset.y += std::ceil (wheelEvent.deltaY / 0.1);
	}
	else
	{
		scrollOffset.x += wheelEvent.deltaX * lineHeight;
		scrollOffset.y += wheelEvent.deltaY * lineHeight;
	}

	auto viewSize = getVisibleViewSize ();
	viewSize.offset (scrollOffset);

	scrollView->makeRectVisible (viewSize);
	event.consumed = true;
}

//------------------------------------------------------------------------
void TextEditorView::setStyle (const Style& newStyle) const
{
	if (!style->font)
		return;

	style = std::make_shared<Style> (newStyle);

	if (auto pf = style->font->getPlatformFont ())
	{
		fontAscent = pf->getAscent ();
		fontDescent = pf->getDescent ();
		lineHeight = std::round ((fontAscent + fontDescent + style->lineSpacing) * 2.) / 2.;
	}
	else
	{
		lineHeight = style->font->getSize () + style->lineSpacing;
	}

	fontPainer = style->font->getFontPainter ();

	if (lineNumberView)
	{
		if (style->showLineNumbers)
		{
			lineNumberView->setStyle (style, lineHeight);
			updateLineNumbersView ();
		}
		else if (scrollView)
		{
			scrollView->setEdgeView (CScrollView::Edge::Left, nullptr);
			lineNumberView = nullptr;
		}
	}
	else if (style->showLineNumbers && scrollView)
	{
		lineNumberView = makeOwned<LineNumberView> (const_cast<TextEditorView*> (this));
		lineNumberView->setStyle (style, lineHeight);
		updateLineNumbersView ();
		scrollView->setEdgeView (CScrollView::Edge::Left, lineNumberView);
		lineNumberView->remember ();
	}
	invalidate (Dirty::All);
}

//------------------------------------------------------------------------
bool TextEditorView::setPlainText (std::string_view utf8Text) const
{
	model.text = convert (utf8Text.data (), utf8Text.size ());
	invalidate (Dirty::All);
	return true;
}

//------------------------------------------------------------------------
std::string TextEditorView::getPlainText () const { return convert (model.text); }

//------------------------------------------------------------------------
void TextEditorView::resetController () const { controller = nullptr; }

//------------------------------------------------------------------------
bool TextEditorView::canHandleCommand (Command cmd) const
{
	switch (cmd)
	{
		case Command::SelectAll:
			return true;
		case Command::UseSelectionForFind:
			[[fallthrough]];
		case Command::Cut:
			[[fallthrough]];
		case Command::Copy:
		{
			return editState.select_start != editState.select_end;
		}
		case Command::Paste:
		{
			if (auto clipboard = getFrame ()->getClipboard ())
			{
				auto count = clipboard->getCount ();
				for (auto i = 0u; i < count; ++i)
				{
					if (clipboard->getDataType (i) == IDataPackage::kText)
						return true;
				}
			}
			return false;
		}
		case Command::Undo:
		{
			// TODO:
			return true;
		}
		case Command::Redo:
		{
			// TODO:
			return true;
		}
		case Command::FindNext:
			[[fallthrough]];
		case Command::FindPrevious:
		{
			return !findString.empty ();
		}
	}
	return false;
}

//------------------------------------------------------------------------
bool TextEditorView::handleCommand (Command cmd) const
{
	auto This = const_cast<TextEditorView*> (this);
	switch (cmd)
	{
		case Command::SelectAll:
			This->selectAll ();
			return true;
		case Command::Cut:
			return This->doCut ();
		case Command::Copy:
			return This->doCopy ();
		case Command::Paste:
			return This->doPaste ();
		case Command::Undo:
			return This->callSTB ([=] () { stb_text_undo (This, &This->editState); });
		case Command::Redo:
			return This->callSTB ([=] () { stb_text_redo (This, &This->editState); });
		case Command::FindNext:
			return This->doFind (true);
		case Command::FindPrevious:
			return This->doFind (false);
		case Command::UseSelectionForFind:
			return This->useSelectionForFind ();
	}
	return false;
}

//------------------------------------------------------------------------
bool TextEditorView::setCommandKeyBinding (Command cmd, char16_t character, VirtualKey virt,
										   Modifiers modifiers) const
{
	commandKeys[Index (cmd)] = {character, virt, modifiers};
	return true;
}

//------------------------------------------------------------------------
inline Range toLineSelection (const Range& line, size_t selStart, size_t selEnd)
{
	if (selStart == selEnd)
		return {};
	if (selStart > selEnd)
		std::swap (selStart, selEnd);
	if (selEnd < line.start)
		return {};
	if (selStart >= line.start + line.length)
		return {};
	size_t lineStart = line.start;
	size_t lineLength = selEnd - selStart;
	if (selStart > line.start)
		lineStart = selStart;
	else if (selStart < line.start)
		lineLength -= line.start - selStart;
	if (lineStart + lineLength > line.start + line.length)
		lineLength = line.start + line.length - lineStart;
	return {lineStart - line.start, lineLength};
}

//------------------------------------------------------------------------
void TextEditorView::drawRect (CDrawContext* context, const CRect& dirtyRect)
{
	context->setFillColor (style->backColor);
	context->setDrawMode (kAntiAliasing);
	context->drawRect (dirtyRect, kDrawFilled);

	CCoord x = getViewSize ().left + style->leftMargin;
	CCoord y = getViewSize ().top - style->lineSpacing;
	for (auto index = 0u; index < model.lines.size (); ++index)
	{
		const auto& line = model.lines[index];
		if (y > dirtyRect.bottom)
			break;
		y += lineHeight;
		if (y < dirtyRect.top)
			continue;
		context->setFontColor (style->textColor);
		context->setFont (style->font);
		auto lt = line.text;
		auto selRange = toLineSelection (line.range, editState.select_start, editState.select_end);
		if (selRange)
		{
			auto selX = x;
			if (selRange.start != 0)
			{
				auto nonSelectedText = lt.getString ().substr (0, selRange.start);
				selX += context->getStringWidth (nonSelectedText.data ());
			}
			CRect r (selX, y - fontAscent, selX, y + fontDescent);
			if (selRange.start + selRange.length >= line.range.length &&
				(line.range.length > 0 && model.text[line.range.end () - 1] == u'\n'))
				r.right = getViewSize ().right;
			else
			{
				auto selectedText = lt.getString ().substr (selRange.start, selRange.length);
				r.setWidth (context->getStringWidth (selectedText.data ()));
			}
			r.inset (0, -style->lineSpacing / 2.);
			context->setFillColor (style->selectionBackColor);
			context->drawRect (r, kDrawFilled);
		}
		context->drawString (lt.getPlatformString (), {x, y});
	}
	// cursor
	if ((cursorIsVisible || cursorAlpha != 0.f) && editState.select_start == editState.select_end)
	{
		auto cr = cursorRect;
		auto alpha = context->getGlobalAlpha ();
		context->setGlobalAlpha (alpha * cursorAlpha);
		cr.offset (getViewSize ().getTopLeft ());
		context->setFillColor (style->textColor);
		context->drawRect (cr, kDrawFilled);
		context->setGlobalAlpha (alpha);
		lastDrawnCursorRect = cr;
	}
	else
		lastDrawnCursorRect = {};
}

//------------------------------------------------------------------------
void TextEditorView::invalidLine (size_t index, bool completeWidth)
{
	auto rectOfLine = calculateLineRect (index);
	if (completeWidth)
		rectOfLine.right = getViewSize ().right;
	invalidRect (rectOfLine);
}

//------------------------------------------------------------------------
void TextEditorView::invalidLine (Lines::const_iterator it, bool completeWidth)
{
	auto rectOfLine = calculateLineRect (it);
	if (completeWidth)
		rectOfLine.right = getViewSize ().right;
	invalidRect (rectOfLine);
}

//------------------------------------------------------------------------
void TextEditorView::invalidSelectedLines ()
{
	for (auto index = selectedLines.start; index < selectedLines.end (); ++index)
		invalidLine (index, true);
}

//------------------------------------------------------------------------
CRect TextEditorView::calculateLineRect (size_t index) const
{
	auto width = index < model.lines.size () ? model.lines[index].width : maxWidth;
	CRect r {0., -style->lineSpacing, std::ceil (width + style->leftMargin), -style->lineSpacing};
	r.offset (getViewSize ().left, lineHeight * (index + 1) + getViewSize ().top);
	r.top = r.top - fontAscent;
	r.bottom = r.bottom + fontDescent;
	r.inset (0, -style->lineSpacing / 2.);
	return r;
}

//------------------------------------------------------------------------
CRect TextEditorView::calculateLineRect (Lines::const_iterator it) const
{
	auto lineIndex = std::distance (model.lines.cbegin (), it);
	return calculateLineRect (lineIndex);
}

//------------------------------------------------------------------------
CRect TextEditorView::calculateCursorRect (int cursor) const
{
	CRect r {0., -style->lineSpacing, 1., -style->lineSpacing};
	for (const auto& line : model.lines)
	{
		r.offset (0., lineHeight);
		if (static_cast<size_t> (cursor) <= line.range.end ())
		{
			if (static_cast<size_t> (cursor) == line.range.start)
				break;
			if (model.text[cursor - 1] != '\n')
			{
				auto lineTextToCursor =
					convert (model.text.data () + line.range.start, cursor - line.range.start);
				auto platformText = getPlatformFactory ().createString (lineTextToCursor.data ());
				auto width = fontPainer->getStringWidth (nullptr, platformText);
				r.offset (width, 0);
				break;
			}
		}
	}
	r.offset (style->leftMargin, 0.);
	r.top = r.top - fontAscent;
	r.bottom = r.bottom + fontDescent;
	r.inset (0, -style->lineSpacing / 2.);
	return r;
}

//------------------------------------------------------------------------
CRect TextEditorView::invalidCursorRect ()
{
	auto r = cursorRect;
	r.offset (getViewSize ().getTopLeft ());
	invalidRect (r);
	return r;
}

//------------------------------------------------------------------------
void TextEditorView::toggleCursorVisibility ()
{
	struct CursorAnimation : Animation::IAnimationTarget
	{
		CursorAnimation (TextEditorView& view) : view (view) {}
		void animationStart (CView*, IdStringPtr name) override {}
		void animationTick (CView*, IdStringPtr name, float pos) override
		{
			view.cursorAlpha = view.cursorIsVisible ? 1.f - pos : pos;
			view.invalidCursorRect ();
		}
		void animationFinished (CView*, IdStringPtr name, bool wasCanceled) override
		{
			if (!wasCanceled)
				view.cursorIsVisible = !view.cursorIsVisible;
			view.cursorAlpha = view.cursorIsVisible ? 1.f : 0.f;
			view.invalidCursorRect ();
		}
		TextEditorView& view;
	};
	addAnimation ("CursorAlphaBlend", new CursorAnimation (*this),
				  new Animation::CubicBezierTimingFunction (
					  Animation::CubicBezierTimingFunction::easy (style->cursorBlinkTime / 2.)));
}

//------------------------------------------------------------------------
void TextEditorView::restartBlinkTimer ()
{
	if (style->cursorBlinkTime)
	{
		removeAnimation ("CursorAlphaBlend");
		cursorIsVisible = true;
		cursorAlpha = 1.f;
		invalidCursorRect ();
		blinkTimer = makeOwned<CVSTGUITimer> ([this] (auto timer) { toggleCursorVisibility (); },
											  style->cursorBlinkTime);
	}
}

//------------------------------------------------------------------------
void TextEditorView::onKeyboardEvent (KeyboardEvent& event)
{
	if (event.type == EventType::KeyUp)
		return;

	if (event.character == 0 && event.virt == VirtualKey::None)
		return;

	auto doFinally = finally ([&] () {
		if (event.consumed)
		{
			if (getFrame ()->getFocusView () == this)
			{
				cursorIsVisible = true;
				restartBlinkTimer ();
			}
		}
	});

	for (auto index = 0u; index < commandKeys.size (); ++index)
	{
		const auto& cmd = commandKeys[index];
		if (cmd == event)
		{
			if (handleCommand (static_cast<Command> (index)))
			{
				event.consumed = true;
				return;
			}
		}
	}

	auto key = event.character;
	if (key)
	{
		if (auto txt = getFrame ()->getPlatformFrame ()->convertCurrentKeyEventToText ())
		{
			auto tmp = convert (txt->getString ());
			key = tmp[0];
		}
	}
	if (event.virt != VirtualKey::None)
	{
		switch (event.virt)
		{
			case VirtualKey::Space:
			{
				key = 0x20;
				break;
			}
			case VirtualKey::Tab:
			{
				key = '\t';
				break;
			}
			case VirtualKey::Return:
			{
				insertNewLine ();
				event.consumed = true;
				return;
			}
			default:
			{
				key = static_cast<uint32_t> (event.virt) | VIRTUAL_KEY_BIT;
				break;
			}
		}
	}
	if (event.modifiers.has (ModifierKey::Control))
		key |= STB_TEXTEDIT_K_CONTROL;
	if (event.modifiers.has (ModifierKey::Alt))
		key |= STB_TEXTEDIT_K_ALT;
	if (event.modifiers.has (ModifierKey::Shift))
		key |= STB_TEXTEDIT_K_SHIFT;
	if (callSTB ([&] () { stb_textedit_key (this, &editState, key); }))
		event.consumed = true;
}

//------------------------------------------------------------------------
void TextEditorView::onMouseDownEvent (MouseDownEvent& event)
{
	if (!event.buttonState.isLeft ())
		return;

	getFrame ()->setFocusView (this);

	mouseIsDown = true;
	editStateOnMouseDown = editState;

	if (event.clickCount > 1)
	{
		selectOnDoubleClick (event.clickCount);
		event.consumed = true;
		return;
	}

	CPoint pos (event.mousePosition);
	pos -= getViewSize ().getTopLeft ();
	auto tm = getGlobalTransform (true);
	pos.x /= tm.m11;
	pos.y /= tm.m22;

	callSTB ([&] () {
		stb_textedit_click (this, &editState, static_cast<float> (pos.x),
							static_cast<float> (pos.y));
	});

	if (event.modifiers.is (ModifierKey::Shift))
	{
		editState.select_start = editStateOnMouseDown.select_start;
		editState.select_end = editState.cursor;
		onSelectionChanged (makeRange (editState));
	}

	event.consumed = true;
}

//------------------------------------------------------------------------
void TextEditorView::onMouseMoveEvent (MouseMoveEvent& event)
{
	lastMouse = event.mousePosition;
	if (!mouseIsDown)
		return;

	CPoint pos (event.mousePosition);
	pos -= getViewSize ().getTopLeft ();
	auto tm = getGlobalTransform (true);
	pos.x /= tm.m11;
	pos.y /= tm.m22;

	callSTB ([&] () {
		stb_textedit_drag (this, &editState, static_cast<float> (pos.x),
						   static_cast<float> (pos.y));
	});
	if (event.clickCount > 1)
		updateSelectionOnDoubleClickMove (event.clickCount);
	event.consumed = true;
}

//------------------------------------------------------------------------
void TextEditorView::onMouseUpEvent (MouseUpEvent& event)
{
	if (!mouseIsDown)
		return;
	event.consumed = true;
	mouseIsDown = false;
}

//------------------------------------------------------------------------
void TextEditorView::onMouseCancelEvent (MouseCancelEvent& event)
{
	if (mouseIsDown)
		mouseIsDown = false;
}

//------------------------------------------------------------------------
void TextEditorView::onMouseEnterEvent (MouseEnterEvent& event)
{
	if (getFrame ()->getFocusView () == this)
		getFrame ()->setCursor (CCursorType::kCursorIBeam);
}

//------------------------------------------------------------------------
void TextEditorView::onMouseExitEvent (MouseExitEvent& event)
{
	getFrame ()->setCursor (CCursorType::kCursorDefault);
	lastMouse = MouseOutsidePos;
}

//------------------------------------------------------------------------
CCoord TextEditorView::calculateMaxWidth () const
{
	auto width = 0.;
	for (const auto& line : model.lines)
	{
		if (line.width > width)
			width = line.width;
	}
	return width;
}

//------------------------------------------------------------------------
void TextEditorView::invalidate (Dirty what) const
{
	auto This = const_cast<TextEditorView*> (this);
	if (what & Dirty::Layout)
	{
		This->layoutRows ();
	}
	if (what & Dirty::UI)
	{
		This->onCursorChanged (This->editState.cursor, This->editState.cursor);
		This->invalid ();
	}
}

//------------------------------------------------------------------------
CCoord TextEditorView::updateLineText (Lines::iterator& line)
{
	line->text = convert (model.text.data () + line->range.start, line->range.length);
	line->width = fontPainer->getStringWidth (nullptr, line->text.getPlatformString ());
	return line->width;
}

//------------------------------------------------------------------------
void TextEditorView::updateLineNumbersView () const
{
	if (!lineNumberView)
		return;

	UTF8String str;
	auto lineCount = model.lines.size ();
	while (lineCount > 0)
	{
		str += "9";
		lineCount /= 10;
	}
	auto lineNumbersWidth = style->lineNumbersFont->getFontPainter ()->getStringWidth (
		nullptr, str.getPlatformString ());
	lineNumbersWidth += (style->lineNumerLeftMargin + style->lineNumerRightMargin);
	lineNumbersWidth = std::ceil (lineNumbersWidth);
	auto vs = lineNumberView->getViewSize ();
	if (vs.getWidth () != lineNumbersWidth)
		vs.setWidth (lineNumbersWidth);
	lineNumberView->setViewSize (vs);
	lineNumberView->setNumLines (model.lines.size ());
}

//------------------------------------------------------------------------
void TextEditorView::layoutRows ()
{
	NewLineProcessor::update (model.lines, model.text, 0);
	CCoord width = 0.;
	for (auto it = model.lines.begin (); it != model.lines.end (); ++it)
	{
		auto w = updateLineText (it);
		if (w > width)
			width = w;
	}
	maxWidth = width + style->leftMargin;
	maxHeight = model.lines.size () * lineHeight + fontDescent;
	parentSizeChanged ();
}

//------------------------------------------------------------------------
void NewLineProcessor::update (Lines& lines, StringView text, size_t startRow)
{
	auto start = text.begin ();
	if (lines.empty ())
		lines.resize (1);
	if (startRow && startRow < lines.size ())
	{
		std::advance (start, lines[startRow].range.start);
	}
	auto end = text.end ();
	size_t startIndex = lines[startRow].range.start;
	size_t row = startRow;
	for (auto it = start; it != end; ++it)
	{
		if ((*it) == '\n')
		{
			if (row >= lines.size ())
				lines.emplace_back (Line ());
			lines[row].range = {startIndex, static_cast<size_t> (std::distance (start, it)) + 1};
			startIndex = lines[row].range.start + lines[row].range.length;
			std::advance (start, lines[row].range.length);
			++row;
		}
	}
	lines.resize (row + 1);
	lines[row].range = {startIndex, static_cast<size_t> (std::distance (start, end))};
}

//------------------------------------------------------------------------
void TextEditorView::insertNewLine ()
{
	auto cursor = editState.cursor;
	auto currentLine = findLine (model.lines.begin (), model.lines.end (), cursor);
	if (currentLine == model.lines.end ())
		return;
	std::u16string insertStr = u"\n";
	auto isWhiteSpace = [] (char16_t character) {
		return character == u'\t' || character == u' ';
	};
	for (auto index = currentLine->range.start; index < currentLine->range.end (); ++index)
	{
		if (isWhiteSpace (model.text[index]))
		{
			insertStr += model.text[index];
			continue;
		}
		break;
	}
	callSTB ([&] () {
		stb_textedit_paste (this, &editState, insertStr.data (),
							static_cast<int32_t> (insertStr.size ()));
	});
}

//------------------------------------------------------------------------
void TextEditorView::validateLineStarts (Lines::const_iterator it, const Lines::const_iterator& end)
{
#if DEBUG
	auto prev = it;
	while (++it != end)
	{
		vstgui_assert (it->range.start > prev->range.start);
		vstgui_assert (it->range.start == prev->range.end ());
		prev = it;
	}
#endif
}

//------------------------------------------------------------------------
void TextEditorView::invalidateSingleLine (size_t pos, int32_t numChars)
{
	auto line = findLine (model.lines.begin (), model.lines.end (), pos);
	invalidateSingleLine (line, numChars);
}

//------------------------------------------------------------------------
void TextEditorView::invalidateSingleLine (Lines::iterator& line, int32_t numChars)
{
	if (line == model.lines.end ())
		return;

	if (numChars < 0)
	{
		invalidLine (line, true);
	}

	line->range.length += numChars;
	auto width = updateLineText (line) + style->leftMargin;
	if (width > maxWidth)
	{
		maxWidth = width;
		parentSizeChanged ();
	}

	invalidLine (line);
	if (numChars)
	{
		auto it = line;
		++it;
		for (; it != model.lines.end (); ++it)
			it->range.start += numChars;
	}
}

//------------------------------------------------------------------------
void TextEditorView::invalidateLines (size_t pos, int32_t numChars)
{
	auto line = findLine (model.lines.begin (), model.lines.end (), pos);
	if (line == model.lines.end ())
		return;
	bool numLinesChanged = false;
	if (numChars > 0)
	{
		int32_t counter = 0;
		for (auto index = pos; index < pos + numChars; ++index)
		{
			if (model.text[index] == u'\n')
			{
				auto change = static_cast<int32_t> (index - line->range.start) -
							  static_cast<int32_t> (line->range.length);
				auto nextLineLength = -change + counter;
				if (line->range.length > 0)
				{
					line->range.length -= 1;
					change++;
				}
				invalidateSingleLine (line, change + 1);
				counter = 0;
				Range nextLineRange {line->range.end (), static_cast<size_t> (nextLineLength)};
				line = model.lines.emplace (++line, Line {nextLineRange, "", 0.});
				updateLineText (line);
				auto nextLine = line;
				auto prevLine = line;
				while (++nextLine != model.lines.end ())
				{
					nextLine->range.start = prevLine->range.end ();
					prevLine = nextLine;
				}
				numLinesChanged = true;
			}
			else
			{
				++counter;
			}
		}
		if (counter > 0)
			invalidateSingleLine (line, counter);
	}
	else
	{
		int32_t counter = std::abs (numChars);
		auto lineIndex = std::distance (model.lines.begin (), line);
		while (counter > 0)
		{
			auto numDel = std::min (counter, static_cast<int32_t> (line->range.end ()) -
												 static_cast<int32_t> (pos));
			vstgui_assert (model.text.size () > (pos + numDel - 1));
			bool isNewLine = model.text[pos + numDel - 1] == u'\n';
			model.text.erase (pos, numDel);
			invalidLine (lineIndex++, true);
			if (isNewLine)
			{
				auto nextLine = line;
				++nextLine;
				nextLine->range.length += line->range.length - numDel;
				nextLine->range.start = line->range.start;
				nextLine = model.lines.erase (line);
				numLinesChanged = true;
				invalidateSingleLine (nextLine, 0);
				auto prevLine = nextLine;
				while (++nextLine != model.lines.end ())
				{
					nextLine->range.start = prevLine->range.end ();
					prevLine = nextLine;
				}
			}
			else
			{
				invalidateSingleLine (pos, -static_cast<int32_t> (numDel));
			}
			counter -= numDel;
		}
		if (numLinesChanged)
		{
			auto width = calculateMaxWidth () + style->leftMargin;
			if (width != maxWidth)
				maxWidth = width;
		}
	}
	if (numLinesChanged)
	{
		maxHeight = model.lines.size () * lineHeight + fontDescent;
		updateLineNumbersView ();
		parentSizeChanged ();
	}
	validateLineStarts (model.lines.begin (), model.lines.end ());
}

//------------------------------------------------------------------------
int32_t TextEditorView::deleteChars (size_t pos, size_t num)
{
	if (pos >= model.text.size ())
		return false;

	invalidateLines (pos, -static_cast<int32_t> (num));

	if (controller)
		controller->onTextEditorTextChanged (*this);
	return true;
}

//------------------------------------------------------------------------
int32_t TextEditorView::insertChars (size_t pos, const CharT* chars, size_t num)
{
	if (pos >= model.text.size ())
		pos = model.text.size ();
	model.text.insert (pos, chars, num);
	invalidateLines (pos, static_cast<int32_t> (num));

	if (controller)
		controller->onTextEditorTextChanged (*this);
	return true;
}

//------------------------------------------------------------------------
template<typename T>
T TextEditorView::findLine (T begin, T end, size_t pos) const
{
	auto dist = std::distance (begin, end);
	auto halfdist = dist / 2;
	auto it = begin;
	std::advance (it, halfdist);
	if (it->range.start <= pos)
	{
		if (it->range.end () > pos)
			return it;
		if (halfdist == 0)
			return it;
		return findLine (it, end, pos);
	}
	if (halfdist == 0)
		return begin;
	return findLine (begin, it, pos);
}

//------------------------------------------------------------------------
void TextEditorView::layout (StbTexteditRow* row, size_t start_i) const
{
	*row = {};
	if (static_cast<size_t> (start_i) >= model.text.size ())
		return;
	if (start_i == stbInternalIterator->range.start)
	{
		row->num_chars = static_cast<int> (stbInternalIterator->range.length);
		row->baseline_y_delta = static_cast<float> (lineHeight);
		row->ymin = static_cast<float> (-fontDescent);
		row->ymax = static_cast<float> (lineHeight);
		row->x0 = static_cast<float> (style->leftMargin);
		row->x1 = static_cast<float> (getWidth ());
		++stbInternalIterator;
		return;
	}
	stbInternalIterator = model.lines.begin ();
	auto line = findLine (model.lines.begin (), model.lines.end (), start_i);
	if (line == model.lines.end ())
		return;
	row->num_chars = static_cast<int> (line->range.length);
	row->baseline_y_delta = static_cast<float> (lineHeight);
	row->ymin = static_cast<float> (-fontDescent);
	row->ymax = static_cast<float> (lineHeight);
	row->x0 = static_cast<float> (style->leftMargin);
	row->x1 = static_cast<float> (getWidth ());
}

//------------------------------------------------------------------------
float TextEditorView::getCharWidth (size_t row, size_t pos) const
{
	if (row + pos >= model.text.length ())
		return STB_TEXTEDIT_GETWIDTH_NEWLINE;

	auto platformFont = style->font->getPlatformFont ();
	vstgui_assert (platformFont);
	auto fontPainter = platformFont->getPainter ();
	vstgui_assert (fontPainter);

	auto c = model.text[row + pos];

	UTF8String str (convert (&c, 1));
	if (str[0] == '\n')
		return STB_TEXTEDIT_GETWIDTH_NEWLINE;
	auto width = fontPainter->getStringWidth (nullptr, str.getPlatformString (), true);
	return static_cast<float> (width / getGlobalTransform (true).m11);
}

//------------------------------------------------------------------------
CharT TextEditorView::getChar (int32_t pos) const
{
	if (pos < getLength ())
		return model.text[pos];
	return 0;
}

//------------------------------------------------------------------------
int32_t TextEditorView::getLength () const { return static_cast<int32_t> (model.text.length ()); }

//------------------------------------------------------------------------
void TextEditorView::onSelectionChanged (Range newSel, bool forceInvalidation)
{
	Range newSelectedLines = {};
	auto line = findLine (model.lines.begin (), model.lines.end (), newSel.start);
	if (line != model.lines.end ())
	{
		newSelectedLines.start = std::distance (model.lines.begin (), line);
		auto endLine = ++(findLine (model.lines.begin (), model.lines.end (), newSel.end ()));
		newSelectedLines.length = std::distance (line, endLine);
	}
	if (forceInvalidation || newSelectedLines != selectedLines)
	{
		invalidSelectedLines ();
		selectedLines = newSelectedLines;
		invalidSelectedLines ();
	}
	if (lineNumberView)
		lineNumberView->setSelectedLines (newSelectedLines);
}

//------------------------------------------------------------------------
inline bool isStopChar (char16_t character)
{
	return std::iswpunct (character) || std::iswcntrl (character) || std::iswspace (character);
};

//------------------------------------------------------------------------
template<bool forward, typename iterator_t>
inline bool findStopChar (iterator_t& it, iterator_t end)
{
	for (; it != end; forward ? ++it : --it)
	{
		auto cursorChar = *it;
		if (isStopChar (cursorChar))
			return true;
	}
	return false;
}

//------------------------------------------------------------------------
void TextEditorView::selectOnDoubleClick (uint32_t clickCount)
{
	auto cursor = static_cast<size_t> (editState.cursor);
	if (cursor >= model.text.size ())
		return;

	if (clickCount > 2)
	{
		auto currentLine = findLine (model.lines.begin (), model.lines.end (), editState.cursor);
		if (currentLine != model.lines.end ())
		{
			editState.select_start = static_cast<int> (currentLine->range.start);
			editState.select_end = static_cast<int> (currentLine->range.end ());
			onSelectionChanged (makeRange (editState), true);
			editStateOnMouseDown = editState;
		}
		return;
	}

	auto cursorChar = model.text[cursor];
	if (isStopChar (cursorChar))
		return;

	if (cursor > 0)
	{
		auto it = model.text.begin ();
		std::advance (it, cursor - 1);
		if (findStopChar<false> (it, model.text.begin ()))
			++it;
		editState.select_start = static_cast<int> (std::distance (model.text.begin (), it));
	}
	if (cursor < model.text.size ())
	{
		auto it = model.text.begin ();
		std::advance (it, cursor + 1);
		findStopChar<true> (it, model.text.end ());
		editState.select_end = static_cast<int> (std::distance (model.text.begin (), it));
	}
	editStateOnMouseDown = editState;
	onSelectionChanged (makeRange (editState), true);
}

//------------------------------------------------------------------------
void TextEditorView::updateSelectionOnDoubleClickMove (uint32_t clickCount)
{
	auto cursor = static_cast<size_t> (editState.cursor);
	if (cursor >= model.text.size ())
		return;

	if (clickCount > 2)
	{
		auto currentLine = findLine (model.lines.begin (), model.lines.end (), cursor);
		if (currentLine != model.lines.end ())
		{
			if (currentLine->range.start >= editStateOnMouseDown.select_start)
			{
				editState.select_start = editStateOnMouseDown.select_start;
				editState.select_end = static_cast<int> (currentLine->range.end ());
			}
			else
			{
				editState.select_start = static_cast<int> (currentLine->range.start);
				editState.select_end = editStateOnMouseDown.select_end;
			}
			onSelectionChanged (makeRange (editState), true);
		}
		return;
	}

	if (cursor < editStateOnMouseDown.select_start)
	{
		auto it = model.text.begin ();
		std::advance (it, cursor - 1);
		if (findStopChar<false> (it, model.text.begin ()))
			++it;
		auto start = static_cast<int> (std::distance (model.text.begin (), it));
		if (start < editStateOnMouseDown.select_start)
			editState.select_start = start;
		else
			editState.select_start = editStateOnMouseDown.select_start;
		editState.select_end = editStateOnMouseDown.select_end;
	}
	else
	{
		auto it = model.text.begin ();
		std::advance (it, cursor + 1);
		findStopChar<true> (it, model.text.end ());
		auto end = static_cast<int> (std::distance (model.text.begin (), it));
		if (end > editStateOnMouseDown.select_end)
			editState.select_end = end;
		else
			editState.select_end = editStateOnMouseDown.select_end;
		editState.select_start = editStateOnMouseDown.select_start;
	}
	onSelectionChanged (makeRange (editState), true);
}

//------------------------------------------------------------------------
void TextEditorView::onCursorChanged (int oldCursorPos, int newCursorPos)
{
	if (!lastDrawnCursorRect.isEmpty ())
		invalidRect (lastDrawnCursorRect);
	cursorRect = calculateCursorRect (newCursorPos);
	auto r = invalidCursorRect ();
	if (scrollView)
	{
		r.bottom += fontDescent;
		scrollView->makeRectVisible (r);
	}
}

//------------------------------------------------------------------------
size_t TextEditorView::moveToWordPrevious (size_t pos) const
{
	if (pos == 0)
		return 0;
	--pos;
	auto it = model.text.begin ();
	std::advance (it, pos);
	auto firstIsStopChar = isStopChar (*it);
	for (; it != model.text.begin (); --it)
	{
		auto cursorChar = *it;
		if (isStopChar (cursorChar) != firstIsStopChar)
		{
			++it;
			break;
		}
	}
	return std::distance (model.text.begin (), it);
}

//------------------------------------------------------------------------
size_t TextEditorView::moveToWordNext (size_t pos) const
{
	if (pos >= model.text.size ())
		return pos;
	++pos;
	auto it = model.text.begin ();
	std::advance (it, pos);
	auto firstIsStopChar = isStopChar (*it);
	for (; it != model.text.end (); ++it)
	{
		auto cursorChar = *it;
		if (isStopChar (cursorChar) != firstIsStopChar)
			break;
	}
	return std::distance (model.text.begin (), it);
}

//-----------------------------------------------------------------------------
template<typename Proc>
bool TextEditorView::callSTB (Proc proc)
{
	stbInternalIterator = model.lines.begin ();
	auto oldState = editState;
	proc ();
	if (memcmp (&oldState, &editState, sizeof (STB_TexteditState)) != 0)
	{
		if (oldState.select_start != editState.select_start ||
			oldState.select_end != editState.select_end)
			onSelectionChanged (makeRange (editState), true);
		if (oldState.cursor != editState.cursor)
		{
			onCursorChanged (oldState.cursor, editState.cursor);
			auto selRange = makeRange (editState);
			if (selRange.length == 0)
				onSelectionChanged (makeRange (editState.cursor, editState.cursor));
		}
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
void TextEditorView::selectAll ()
{
	editState.select_start = 0;
	editState.select_end = getLength ();
	onSelectionChanged (makeRange (editState), true);
}

//------------------------------------------------------------------------
bool TextEditorView::doCut ()
{
	if (doCopy ())
	{
		callSTB ([&] () { stb_textedit_cut (this, &editState); });
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool TextEditorView::doCopy ()
{
	if (editState.select_start == editState.select_end)
		return false;
	auto start = editState.select_start;
	auto end = editState.select_end;
	if (start > end)
		std::swap (start, end);
	auto txt = convert (model.text.data () + start, end - start);
	auto dataPackage =
		CDropSource::create (txt.data (), static_cast<uint32_t> (txt.size ()), IDataPackage::kText);
	getFrame ()->setClipboard (dataPackage);
	return true;
}

//------------------------------------------------------------------------
bool TextEditorView::doPaste ()
{
	if (auto clipboard = getFrame ()->getClipboard ())
	{
		auto count = clipboard->getCount ();
		for (auto i = 0u; i < count; ++i)
		{
			const void* buffer;
			IDataPackage::Type dataType;
			auto size = clipboard->getData (i, buffer, dataType);
			if (dataType == IDataPackage::kText)
			{
				auto txt = reinterpret_cast<const char*> (buffer);
				auto uText = convert (txt, size);
				callSTB ([&] () {
					stb_textedit_paste (this, &editState, uText.data (),
										static_cast<int32_t> (uText.size ()));
				});
				return true;
			}
		}
	}
	return false;
}

//------------------------------------------------------------------------
bool TextEditorView::useSelectionForFind ()
{
	if (auto range = makeRange (editState))
	{
		findString = model.text.substr (range.start, range.length);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool TextEditorView::doFind (bool forward)
{
	if (findString.empty ())
		return false;
	auto pos = String::npos;
	if (forward)
	{
		auto cursor = editState.select_end;
		if (cursor > model.text.length ())
			cursor = 0;
		auto text = StringView (model.text.data (), model.text.length ());
		pos = text.find (findString.data (), cursor, findString.length ());
		if (pos == String::npos && cursor != 0 /*wrap around*/)
			pos = text.find (findString.data (), 0, findString.length ());
	}
	else
	{
		auto cursor = editState.select_start - findString.length ();
		if (cursor < 0)
			cursor = String::npos;
		auto text = StringView (model.text.data (), model.text.length ());
		pos = text.rfind (findString.data (), cursor);
		if (pos == String::npos /*wrap around*/)
			pos = text.rfind (findString.data (), pos);
	}
	if (pos != String::npos)
	{
		auto oldCursor = editState.cursor;
		editState.cursor = static_cast<int> (pos);
		editState.select_start = editState.cursor;
		editState.select_end = editState.cursor + static_cast<int> (findString.length ());
		onSelectionChanged (makeRange (editState));
		onCursorChanged (oldCursor, editState.cursor);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
LineNumberView::LineNumberView (CView* textEditorView) : CView ({}), textEditorView (textEditorView)
{
	textEditorView->registerViewListener (this);
}

//------------------------------------------------------------------------
LineNumberView::~LineNumberView () noexcept
{
	if (textEditorView)
		textEditorView->unregisterViewListener (this);
}

//------------------------------------------------------------------------
void LineNumberView::viewSizeChanged (CView* view, const CRect& oldSize)
{
	vstgui_assert (view == textEditorView);

	invalid ();
}

//------------------------------------------------------------------------
void LineNumberView::viewWillDelete (CView* view)
{
	vstgui_assert (view == textEditorView);
	textEditorView->unregisterViewListener (this);
	textEditorView = nullptr;
}

//------------------------------------------------------------------------
void LineNumberView::drawRect (CDrawContext* context, const CRect& _dirtyRect)
{
	CRect dirtyRect = _dirtyRect;

	context->setFillColor (style->backColor);
	context->setDrawMode (kAntiAliasing);
	context->drawRect (dirtyRect, kDrawFilled);

	CCoord x = getViewSize ().right;
	CCoord y = getViewSize ().top - style->lineSpacing + textEditorView->getViewSize ().top;
	for (auto index = 0u; index < numLines; ++index)
	{
		if (y > dirtyRect.bottom)
			break;
		y += lineHeight;
		if (y < dirtyRect.top)
			continue;
		context->setFontColor (style->textColor);
		context->setFont (style->font);
		bool lineIsSelected = index >= selectedLines.start && index < selectedLines.end ();
		context->setFontColor (lineIsSelected ? style->lineNumberTextSelectedColor
											  : style->lineNumberTextColor);
		context->setFont (style->lineNumbersFont);
		auto lineNoStr = toString (index + 1);
		auto lineNoWidth = style->lineNumbersFont->getFontPainter ()->getStringWidth (
			context->getPlatformDeviceContext (), lineNoStr.getPlatformString ());
		context->drawString (lineNoStr.getPlatformString (),
							 {x - (lineNoWidth + style->lineNumerRightMargin), y});
	}

	auto cr = getViewSize ();
	cr.left = (cr.right - 1.);
	context->setFillColor (style->lineNumberLine);
	context->drawRect (cr, kDrawFilled);
}

//------------------------------------------------------------------------
void LineNumberView::setStyle (const std::shared_ptr<ITextEditor::Style>& newStyle,
							   CCoord newLineHeight)
{
	style = newStyle;
	lineHeight = newLineHeight;
	invalid ();
}

//------------------------------------------------------------------------
void LineNumberView::setNumLines (size_t numberOfLines)
{
	numLines = numberOfLines;
	invalid ();
}

//------------------------------------------------------------------------
void LineNumberView::setSelectedLines (Range range)
{
	if (range != selectedLines)
	{
		selectedLines = range;
		invalid ();
	}
}

//------------------------------------------------------------------------
} // TextEditor

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
CView* createNewTextEditor (const CRect& r, ITextEditorController* controller)
{
	vstgui_assert (controller != nullptr, "you need to call this with a controller");
	return new TextEditor::TextEditorView (controller);
}

//------------------------------------------------------------------------
} // VSTGUI

#undef STB_TEXTEDIT_CHARTYPE
#undef STB_TEXTEDIT_POSITIONTYPE
#undef STB_TEXTEDIT_STRING
#undef STB_TEXTEDIT_KEYTYPE
#undef INCLUDE_STB_TEXTEDIT_H

#undef VIRTUAL_KEY_BIT
#undef STB_TEXTEDIT_K_SHIFT
#undef STB_TEXTEDIT_K_CONTROL
#undef STB_TEXTEDIT_K_ALT
// key-bindings
#undef STB_TEXTEDIT_K_LEFT
#undef STB_TEXTEDIT_K_RIGHT
#undef STB_TEXTEDIT_K_UP
#undef STB_TEXTEDIT_K_DOWN
#undef STB_TEXTEDIT_K_LINESTART
#undef STB_TEXTEDIT_K_LINEEND
#undef STB_TEXTEDIT_K_WORDLEFT
#undef STB_TEXTEDIT_K_WORDRIGHT
#undef STB_TEXTEDIT_K_TEXTSTART
#undef STB_TEXTEDIT_K_TEXTEND
#undef STB_TEXTEDIT_K_DELETE
#undef STB_TEXTEDIT_K_BACKSPACE
#undef STB_TEXTEDIT_K_UNDO
#undef STB_TEXTEDIT_K_REDO
#undef STB_TEXTEDIT_K_INSERT
#undef STB_TEXTEDIT_K_PGUP
#undef STB_TEXTEDIT_K_PGDOWN
// functions
#undef STB_TEXTEDIT_STRINGLEN
#undef STB_TEXTEDIT_LAYOUTROW
#undef STB_TEXTEDIT_GETWIDTH
#undef STB_TEXTEDIT_KEYTOTEXT
#undef STB_TEXTEDIT_GETCHAR
#undef STB_TEXTEDIT_NEWLINE
#undef STB_TEXTEDIT_DELETECHARS
#undef STB_TEXTEDIT_INSERTCHARS
#undef STB_TEXTEDIT_MOVEWORDLEFT
#undef STB_TEXTEDIT_MOVEWORDRIGHT
#undef STB_TEXTEDIT_GETWIDTH_NEWLINE
