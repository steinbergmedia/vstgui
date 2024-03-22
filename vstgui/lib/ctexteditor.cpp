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
#include "controls/cbuttons.h"
#include "controls/ctextedit.h"
#include "animation/timingfunctions.h"
#include "animation/animations.h"

#include <optional>
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
#define STB_TEXTEDIT_STRING const TextEditorView
#define STB_TEXTEDIT_KEYTYPE uint32_t
#define STB_TEXTEDIT_UNDOSTATECOUNT 0
#define STB_TEXTEDIT_UNDOCHARCOUNT 0

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

//------------------------------------------------------------------------
inline void replaceTabs (std::string& str, uint32_t tabWidth)
{
	if (tabWidth < 1)
		return;
	std::string replaceStr;
	while (--tabWidth)
		replaceStr += ' ';
	std::string::size_type pos = std::string::npos;
	while ((pos = str.find_first_of ('\t')) != std::string::npos)
	{
		str[pos] = ' ';
		str.replace (pos, 1, " ");
		str.insert (pos, replaceStr);
	}
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
struct FindPanelController;

static constexpr CPoint MouseOutsidePos = {std::numeric_limits<CCoord>::max (),
										   std::numeric_limits<CCoord>::max ()};

//------------------------------------------------------------------------
struct Key
{
	char16_t character;
	VirtualKey virt;
	Modifiers modifiers;

	bool operator== (const KeyboardEvent& event) const
	{
		return event.character == character && event.virt == virt && event.modifiers == modifiers;
	}
};
using CommandKeyArray = std::array<Key, static_cast<size_t> (ITextEditor::Command::TakeFocus) + 1>;

//------------------------------------------------------------------------
struct TextEditorView : public CView,
						public ITextEditor,
						public TextEditorColorization::IEditorExt,
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

	int32_t deleteChars (size_t pos, size_t num) const;
	int32_t insertChars (size_t pos, const CharT* text, size_t num) const;
	void layout (StbTexteditRow* row, size_t start_i) const;
	float getCharWidth (size_t row, size_t pos) const;
	CharT getChar (int32_t pos) const;
	int32_t getLength () const;
	size_t moveToWordPrevious (size_t pos) const;
	size_t moveToWordNext (size_t pos) const;

	// STB
	static int32_t deleteChars (const TextEditorView* self, size_t pos, size_t num)
	{
		return self->deleteChars (pos, num);
	}
	static int32_t insertChars (const TextEditorView* self, size_t pos, const CharT* text,
								size_t num)
	{
		return self->insertChars (pos, text, num);
	}
	static void layout (StbTexteditRow* row, const TextEditorView* self, size_t start_i)
	{
		self->layout (row, start_i);
	}
	static float getCharWidth (const TextEditorView* self, size_t row, size_t pos)
	{
		return self->getCharWidth (row, pos);
	}
	static CharT getChar (const TextEditorView* self, int32_t pos) { return self->getChar (pos); }
	static int32_t getLength (const TextEditorView* self) { return self->getLength (); }

	static int moveToWordPrevious (const TextEditorView* self, size_t pos)
	{
		return static_cast<int> (self->moveToWordPrevious (pos));
	}
	static int moveToWordNext (const TextEditorView* self, size_t pos)
	{
		return static_cast<int> (self->moveToWordNext (pos));
	}

	static CharT* createUndoRecord (const TextEditorView* self, size_t pos, size_t insert_len,
									size_t delete_len)
	{
		return self->createUndoRecord (pos, insert_len, delete_len);
	}
	static void undo (const TextEditorView* self) { self->doUndo (); }
	static void redo (const TextEditorView* self) { self->doRedo (); }

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
	void setFindOptions (FindOptions opt) const override;
	void setFindString (std::string_view utf8Text) const override;

	// TextEditorHighlighting::IEditorExt
	std::u16string_view readText (size_t startOffset, size_t length) const override;
	size_t getTextLength () const override;

	// commandos
	bool doShifting (bool right) const;
	void selectAll () const;
	bool doCut () const;
	bool doCopy () const;
	bool doPaste () const;
	bool useSelectionForFind () const;
	bool doFind (bool forward = true, size_t oldPos = String::npos) const;
	bool showFindPanel () const;

	String::size_type doFindCaseSensitive (bool forward) const;
	String::size_type doFindIgnoreCase (bool forward) const;

	// undo / redo
	CharT* createUndoRecord (size_t pos, size_t insertLen, size_t deleteLen) const;
	void checkCurrentUndoGroup (bool force) const;
	void doUndo () const;
	void doRedo () const;
	template<bool iterateForward>
	void doUndoRedo () const;

private:
	template<typename Proc>
	bool callSTB (Proc proc) const;

	enum Dirty
	{
		UI = 1 << 0,
		Layout = 1 << 1,

		All = UI | Layout,
	};
	void validateLineStarts (Lines::const_iterator it, const Lines::const_iterator& end) const;
	void invalidateSingleLine (size_t pos, int32_t numChars) const;
	void invalidateSingleLine (Lines::iterator& line, int32_t numChars) const;
	void invalidateLines (size_t pos, int32_t numChars) const;
	void invalidate (Dirty what = Dirty::UI) const;
	void invalidLine (size_t index, bool completeWidth = false) const;
	void invalidLine (Lines::const_iterator it, bool completeWidth = false) const;
	void invalidateRect (CRect r) const;
	void invalidSelectedLines () const;
	CCoord updateLineText (Lines::iterator& line) const;
	CRect calculateLineRect (size_t index) const;
	CRect calculateLineRect (Lines::const_iterator it) const;
	CCoord calculateMaxWidth () const;
	CRect calculateSelectionRect () const;
	void updateLineNumbersView () const;
	void layoutRows () const;
	void onCursorChanged (int oldCursorPos, int newCursorPos) const;
	void onSelectionChanged (Range newSel, bool forceInvalidation = false) const;
	void selectOnDoubleClick (uint32_t clickCount) const;
	template<bool forward>
	void selectPair (size_t startPos, char16_t closingChar) const;
	void updateSelectionOnDoubleClickMove (uint32_t clickCount) const;
	void insertNewLine () const;
	template<typename T>
	T findLine (T begin, T end, size_t pos) const;
	CRect calculateCursorRect (int cursor) const;
	CRect invalidCursorRect () const;
	void toggleCursorVisibility () const;
	void restartBlinkTimer () const;
	void onStyleChanged () const;
	void setFindString (String&& text) const;

	TextEditorView& mutableThis () const { return *const_cast<TextEditorView*> (this); }

public:
	//------------------------------------------------------------------------
	struct UndoRecord
	{
		size_t position {};
		size_t deleted {0};
		String characters;
	};
	using UndoRecords = std::vector<UndoRecord>;

	//------------------------------------------------------------------------
	struct UndoGroup
	{
		uint64_t time {0};
		UndoRecords record;
	};
	using UndoList = std::vector<UndoGroup>;

	//------------------------------------------------------------------------
	struct ModelData
	{
		TextModel model;

		std::shared_ptr<Style> style {std::make_shared<Style> ()};
		ITextEditorController* controller {nullptr};
		const IFontPainter* fontPainer {nullptr};
		FindPanelController* findPanelController {nullptr};

		CScrollView* scrollView {nullptr};
		SharedPointer<LineNumberView> lineNumberView;

		SharedPointer<CVSTGUITimer> blinkTimer;

		CRect cursorRect {};
		CRect lastDrawnCursorRect {};
		Range selectedLines {};
		CPoint lastMouse {MouseOutsidePos};
		CCoord maxHeight {0};
		CCoord maxWidth {200.};
		CCoord horizontalLineMargin {0};
		CCoord lineHeight {style->font->getSize () + style->lineSpacing};
		CCoord fontAscent {0.};
		CCoord fontDescent {0.};

		STB_TexteditState editState {};
		STB_TexteditState editStateOnMouseDown {};
		bool mouseIsDown {false};
		bool isInsertingText {false};
		bool cursorIsVisible {false};
		bool isInUndoRedo {false};
		float cursorAlpha {0.f};

		Lines::const_iterator stbInternalIterator;

		FindOptions findOptions;
		String findString;

		CommandKeyArray commandKeys;
		UndoList undoList {UndoGroup ()};
		UndoList::iterator undoPos;
		UndoGroup currentUndoGroup;
	};

private:
	mutable ModelData md;
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
#define STB_TEXTEDIT_CUSTOM_CREATEUNDO TextEditorView::createUndoRecord
#define STB_TEXTEDIT_CUSTOM_UNDO TextEditorView::undo
#define STB_TEXTEDIT_CUSTOM_REDO TextEditorView::redo
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

//------------------------------------------------------------------------
struct FindPanelController : IControlListener,
							 ViewListenerAdapter,
							 ViewEventListenerAdapter
{
	using RemoveFindPanelFunc = std::function<void ()>;
	static CViewContainer* makeFindPanelView (CRect rect, TextEditorView::ModelData& md,
											  const ITextEditor& editor, RemoveFindPanelFunc&& f);

	void setFindString (StringView text);
	void setFindOptions (ITextEditor::FindOptions opt);

private:
	enum Tag
	{
		Textfield,
		CloseButton,
		WholeWords,
		CaseSensitive,
		FindPrevious,
		FindNext,
	};

	FindPanelController (const ITextEditor& editor);
	~FindPanelController () noexcept;
	void valueChanged (CControl* control) override;
	void viewLostFocus (CView* view) override;
	void viewWillDelete (CView* view) override;
	void viewOnEvent (CView* view, Event& event) override;

	const ITextEditor& editor;
	CTextEdit* editfield {nullptr};
	CControl* closeBox {nullptr};
	CControl* caseSensitiveButton {nullptr};
	CControl* wholeWordButton {nullptr};
	ITextEditor::FindOptions findOptions {};
	CommandKeyArray commandKeys;
	RemoveFindPanelFunc removeFindPanelFunc;
};

static constexpr size_t Index (ITextEditor::Command cmd) { return static_cast<size_t> (cmd); }

//------------------------------------------------------------------------
TextEditorView::TextEditorView (ITextEditorController* controller) : CView ({0, 0, 10, 10})
{
	md.controller = controller;
	md.undoPos = md.undoList.end ();
	setWantsFocus (true);
	onStyleChanged ();
	stb_textedit_initialize_state (&md.editState, false);
	std::fill (md.commandKeys.begin (), md.commandKeys.end (), Key ());
	md.commandKeys[Index (Command::SelectAll)] = {u'a', VirtualKey::None, {ModifierKey::Control}};
	md.commandKeys[Index (Command::Cut)] = {u'x', VirtualKey::None, {ModifierKey::Control}};
	md.commandKeys[Index (Command::Copy)] = {u'c', VirtualKey::None, {ModifierKey::Control}};
	md.commandKeys[Index (Command::Paste)] = {u'v', VirtualKey::None, {ModifierKey::Control}};
	md.commandKeys[Index (Command::Undo)] = {u'z', VirtualKey::None, {ModifierKey::Control}};
	md.commandKeys[Index (Command::Redo)] = {
		u'z', VirtualKey::None, {ModifierKey::Control, ModifierKey::Shift}};
#if MAC
	md.commandKeys[Index (Command::FindNext)] = {u'g', VirtualKey::None, {ModifierKey::Control}};
	md.commandKeys[Index (Command::FindPrevious)] = {
		u'g', VirtualKey::None, {ModifierKey::Control, ModifierKey::Shift}};
	md.commandKeys[Index (Command::UseSelectionForFind)] = {
		u'e', VirtualKey::None, {ModifierKey::Control}};
	md.commandKeys[Index (Command::ShiftLeft)] = {u'[', VirtualKey::None, {ModifierKey::Control}};
	md.commandKeys[Index (Command::ShiftRight)] = {u']', VirtualKey::None, {ModifierKey::Control}};
#else
	md.commandKeys[Index (Command::FindNext)] = {0, VirtualKey::F3, {}};
	md.commandKeys[Index (Command::FindPrevious)] = {0, VirtualKey::F3, {ModifierKey::Shift}};
	md.commandKeys[Index (Command::UseSelectionForFind)] = {
		0, VirtualKey::F3, {ModifierKey::Control}};
	md.commandKeys[Index (Command::ShiftLeft)] = {0, VirtualKey::Tab, {ModifierKey::Shift}};
	md.commandKeys[Index (Command::ShiftRight)] = {0, VirtualKey::Tab, {}};
#endif
	md.commandKeys[Index (Command::ShowFindPanel)] = {
		u'f', VirtualKey::None, {ModifierKey::Control}};
	md.controller->onTextEditorCreated (*this);
}

//------------------------------------------------------------------------
void TextEditorView::beforeDelete ()
{
	if (md.controller)
		md.controller->onTextEditorDestroyed (*this);
	CView::beforeDelete ();
}

//------------------------------------------------------------------------
bool TextEditorView::attached (CView* parent)
{
	if (CView::attached (parent))
	{
		if (auto sv = dynamic_cast<CScrollView*> (parent->getParentView ()))
		{
			md.scrollView = sv;
			layoutRows ();
			md.scrollView->registerViewEventListener (this);
			if (auto sb = md.scrollView->getVerticalScrollbar ())
				sb->registerViewEventListener (this);
			if (md.style->showLineNumbers)
			{
				md.lineNumberView = makeOwned<LineNumberView> (this);
				md.lineNumberView->setStyle (md.style, md.lineHeight);
				updateLineNumbersView ();
				md.scrollView->setEdgeView (CScrollView::Edge::Left, md.lineNumberView);
				md.lineNumberView->remember ();
			}
		}
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool TextEditorView::removed (CView* parent)
{
	if (md.scrollView)
	{
		md.scrollView->unregisterViewEventListener (this);
		if (auto sb = md.scrollView->getVerticalScrollbar ())
			sb->unregisterViewEventListener (this);
	}
	md.scrollView = nullptr;
	md.blinkTimer = nullptr;
	md.lineNumberView = nullptr;
	return CView::removed (parent);
}

//------------------------------------------------------------------------
void TextEditorView::takeFocus ()
{
	if (md.lastMouse != MouseOutsidePos)
		getFrame ()->setCursor (kCursorIBeam);
	restartBlinkTimer ();
}

//------------------------------------------------------------------------
void TextEditorView::looseFocus ()
{
	md.blinkTimer = nullptr;
	md.cursorIsVisible = true;
	toggleCursorVisibility ();
}

//------------------------------------------------------------------------
void TextEditorView::parentSizeChanged ()
{
	if (md.scrollView)
	{
		auto func = [This = shared (this), makeCursorVisible = md.isInsertingText] () mutable {
			if (!This->isAttached () || !This->md.scrollView)
				return;
			auto viewSize = This->getViewSize ();
			viewSize.setHeight (This->md.maxHeight);
			viewSize.setWidth (This->md.maxWidth);
			auto containerSize = This->md.scrollView->calculateOptimalContainerSize ();
			if (containerSize.getWidth () > viewSize.getWidth ())
			{
				if (viewSize.getHeight () > containerSize.getHeight ())
					viewSize.setWidth (containerSize.getWidth () -
									   This->md.scrollView->getScrollbarWidth ());
				else
					viewSize.setWidth (containerSize.getWidth ());
			}
			if (containerSize.getHeight () > viewSize.getHeight ())
			{
				if (viewSize.getWidth () > containerSize.getWidth ())
					viewSize.setHeight (containerSize.getHeight () -
										This->md.scrollView->getScrollbarWidth ());
				else
					viewSize.setHeight (containerSize.getHeight ());
			}
			This->setViewSize (viewSize);
			if (makeCursorVisible)
				This->onCursorChanged (-1, This->md.editState.cursor);
			This->md.editState.row_count_per_page =
				This->getVisibleViewSize ().getHeight () / This->md.lineHeight;
		};
		auto frame = getFrame ();
		if (frame && frame->inEventProcessing ())
		{
			frame->doAfterEventProcessing (func);
			return;
		}
		else
		{
			func ();
		}
	}
	else
	{
		auto viewSize = getViewSize ();
		viewSize.setHeight (md.maxHeight);
		viewSize.setWidth (md.maxWidth);
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
		scrollOffset.y -= std::ceil (wheelEvent.deltaY / 0.1);
	}
	else
	{
		scrollOffset.x += wheelEvent.deltaX * md.lineHeight;
		scrollOffset.y -= wheelEvent.deltaY * md.lineHeight;
	}
	if (wheelEvent.flags & MouseWheelEvent::DirectionInvertedFromDevice)
	{
		scrollOffset.y *= -1.;
	}

	auto viewSize = getVisibleViewSize ();
	viewSize.offset (scrollOffset);

	md.scrollView->makeRectVisible (viewSize);
	event.consumed = true;
}

//------------------------------------------------------------------------
void TextEditorView::onStyleChanged () const
{
	if (auto pf = md.style->font->getPlatformFont ())
	{
		md.fontAscent = pf->getAscent ();
		md.fontDescent = pf->getDescent ();
		md.lineHeight =
			std::round ((md.fontAscent + md.fontDescent + md.style->lineSpacing) * 2.) / 2.;
	}
	else
	{
		md.lineHeight = md.style->font->getSize () + md.style->lineSpacing;
	}

	md.fontPainer = md.style->font->getFontPainter ();

	if (md.lineNumberView)
	{
		if (md.style->showLineNumbers)
		{
			md.lineNumberView->setStyle (md.style, md.lineHeight);
			updateLineNumbersView ();
		}
		else if (md.scrollView)
		{
			md.scrollView->setEdgeView (CScrollView::Edge::Left, nullptr);
			md.lineNumberView = nullptr;
		}
	}
	else if (md.style->showLineNumbers && md.scrollView)
	{
		md.lineNumberView = makeOwned<LineNumberView> (&mutableThis ());
		md.lineNumberView->setStyle (md.style, md.lineHeight);
		updateLineNumbersView ();
		md.scrollView->setEdgeView (CScrollView::Edge::Left, md.lineNumberView);
		md.lineNumberView->remember ();
	}
	md.horizontalLineMargin = md.style->leftMargin * 2.;
	invalidate (Dirty::All);
}

//------------------------------------------------------------------------
void TextEditorView::setStyle (const Style& newStyle) const
{
	if (!md.style->font)
		return;

	md.style = std::make_shared<Style> (newStyle);
	onStyleChanged ();
}

//------------------------------------------------------------------------
bool TextEditorView::setPlainText (std::string_view utf8Text) const
{
	md.model.text = convert (utf8Text.data (), utf8Text.size ());
	invalidate (Dirty::All);
	if (md.lineNumberView)
		updateLineNumbersView ();
	return true;
}

//------------------------------------------------------------------------
std::string TextEditorView::getPlainText () const { return convert (md.model.text); }

//------------------------------------------------------------------------
void TextEditorView::resetController () const { md.controller = nullptr; }

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
			return md.editState.select_start != md.editState.select_end;
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
		case Command::ShiftLeft:
			return true;
		case Command::ShiftRight:
			return true;
		case Command::TakeFocus:
			return true;
		case Command::FindNext:
			[[fallthrough]];
		case Command::FindPrevious:
		{
			return !md.findString.empty ();
		}
		case Command::ShowFindPanel:
			return md.scrollView != nullptr;
	}
	return false;
}

//------------------------------------------------------------------------
bool TextEditorView::handleCommand (Command cmd) const
{
	auto doFinally = finally ([&] () {
		if (getFrame ()->getFocusView () == this)
		{
			restartBlinkTimer ();
		}
	});

	switch (cmd)
	{
		case Command::ShiftLeft:
			return doShifting (false);
		case Command::ShiftRight:
			return doShifting (true);
		case Command::SelectAll:
			selectAll ();
			return true;
		case Command::Cut:
			return doCut ();
		case Command::Copy:
			return doCopy ();
		case Command::Paste:
			return doPaste ();
		case Command::Undo:
			return callSTB ([=] () { stb_text_undo (this, &md.editState); });
		case Command::Redo:
			return callSTB ([=] () { stb_text_redo (this, &md.editState); });
		case Command::FindNext:
			return doFind (true);
		case Command::FindPrevious:
			return doFind (false);
		case Command::UseSelectionForFind:
			return useSelectionForFind ();
		case Command::ShowFindPanel:
			return showFindPanel ();
		case Command::TakeFocus:
		{
			if (auto frame = getFrame ())
			{
				if (frame->getFocusView () != this)
					frame->setFocusView (&mutableThis ());
			}
			return true;
		}
	}
	return false;
}

//------------------------------------------------------------------------
bool TextEditorView::setCommandKeyBinding (Command cmd, char16_t character, VirtualKey virt,
										   Modifiers modifiers) const
{
	md.commandKeys[Index (cmd)] = {character, virt, modifiers};
	return true;
}

//------------------------------------------------------------------------
std::u16string_view TextEditorView::readText (size_t startOffset, size_t length) const
{
	if (startOffset >= md.model.text.length () || md.model.lines.empty ())
		return {};
	if (startOffset + length >= md.model.text.length ())
		length = md.model.text.length () - startOffset;
	return {md.model.text.data () + startOffset, length};
}

//------------------------------------------------------------------------
size_t TextEditorView::getTextLength () const { return md.model.text.length (); }

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
	context->setFillColor (md.style->backColor);
	context->setDrawMode (kAntiAliasing);
	context->drawRect (dirtyRect, kDrawFilled);

	auto styleProvider = dynamic_cast<TextEditorColorization::IStyleProvider*> (md.controller);
	if (styleProvider)
		styleProvider->beginDraw (*this);
	CCoord x = getViewSize ().left + md.style->leftMargin;
	CCoord y = getViewSize ().top - md.style->lineSpacing;
	for (auto index = 0u; index < md.model.lines.size (); ++index)
	{
		const auto& line = md.model.lines[index];
		if (y > dirtyRect.bottom)
			break;
		y += md.lineHeight;
		if (y < dirtyRect.top)
			continue;
		context->setFontColor (md.style->textColor);
		context->setFont (md.style->font);
		auto selRange =
			toLineSelection (line.range, md.editState.select_start, md.editState.select_end);
		if (selRange)
		{
			auto selX = x;
			if (selRange.start != 0)
			{
				auto t = md.model.text.substr (line.range.start, selRange.start);
				auto nonSelectedText = convert (t);
				replaceTabs (nonSelectedText, md.style->tabWidth);
				selX += context->getStringWidth (nonSelectedText.data ());
			}
			CRect r (selX, y - md.fontAscent, selX, y + md.fontDescent);
			if (selRange.start + selRange.length >= line.range.length &&
				(line.range.length > 0 && md.model.text[line.range.end () - 1] == u'\n'))
				r.right = getViewSize ().right;
			else
			{
				auto t = md.model.text.substr (line.range.start + selRange.start, selRange.length);
				auto selectedText = convert (t);
				replaceTabs (selectedText, md.style->tabWidth);
				r.setWidth (context->getStringWidth (selectedText.data ()));
			}
			r.inset (0, -md.style->lineSpacing / 2.);
			context->setFillColor (md.style->selectionBackColor);
			context->drawRect (r, kDrawFilled);
		}
		if (styleProvider)
		{
			auto lineX = x;
			auto styles = styleProvider->getStyles (*this, line.range.start, line.range.length);
			size_t start = line.range.start;
			size_t end = 0u;
			std::string tmpStr;
			for (const auto& style : styles)
			{
				if (style.start != start)
				{
					tmpStr = convert (md.model.text.substr (start, style.start - start));
					replaceTabs (tmpStr, md.style->tabWidth);
					context->setFontColor (md.style->textColor);
					context->drawString (tmpStr.data (), {lineX, y});
					lineX += context->getStringWidth (tmpStr.data ());
				}
				start = style.start < line.range.start ? line.range.start : style.start;
				end = start + style.length;
				if (start == end)
					continue;
				if (end > line.range.start + line.range.length)
					end = line.range.start + line.range.length;
				tmpStr = convert (md.model.text.substr (start, end - start));
				replaceTabs (tmpStr, md.style->tabWidth);
				context->setFontColor (style.color);
				context->drawString (tmpStr.data (), {lineX, y});
				lineX += context->getStringWidth (tmpStr.data ());
				start += style.length;
			}
		}
		else
		{
			context->drawString (line.text.getPlatformString (), {x, y});
		}
	}

	if (styleProvider)
		styleProvider->endDraw (*this);

	// cursor
	if ((md.cursorIsVisible || md.cursorAlpha != 0.f) &&
		md.editState.select_start == md.editState.select_end)
	{
		auto cr = md.cursorRect;
		auto alpha = context->getGlobalAlpha ();
		context->setGlobalAlpha (alpha * md.cursorAlpha);
		cr.offset (getViewSize ().getTopLeft ());
		context->setFillColor (md.style->textColor);
		context->drawRect (cr, kDrawFilled);
		context->setGlobalAlpha (alpha);
		md.lastDrawnCursorRect = cr;
	}
	else
		md.lastDrawnCursorRect = {};
}

//------------------------------------------------------------------------
void TextEditorView::invalidLine (size_t index, bool completeWidth) const
{
	auto rectOfLine = calculateLineRect (index);
	if (completeWidth)
		rectOfLine.right = getViewSize ().right;
	invalidateRect (rectOfLine);
}

//------------------------------------------------------------------------
void TextEditorView::invalidLine (Lines::const_iterator it, bool completeWidth) const
{
	auto rectOfLine = calculateLineRect (it);
	if (completeWidth)
		rectOfLine.right = getViewSize ().right;
	invalidateRect (rectOfLine);
}

//------------------------------------------------------------------------
void TextEditorView::invalidSelectedLines () const { invalidateRect (calculateSelectionRect ()); }

//------------------------------------------------------------------------
CRect TextEditorView::calculateLineRect (size_t index) const
{
	auto width = index < md.model.lines.size () ? md.model.lines[index].width : md.maxWidth;
	CRect r {0., -md.style->lineSpacing, std::ceil (width + md.style->leftMargin),
			 -md.style->lineSpacing};
	r.offset (getViewSize ().left, md.lineHeight * (index + 1) + getViewSize ().top);
	r.top = r.top - md.fontAscent;
	r.bottom = r.bottom + md.fontDescent;
	r.inset (0, -md.style->lineSpacing / 2.);
	return r;
}

//------------------------------------------------------------------------
CRect TextEditorView::calculateLineRect (Lines::const_iterator it) const
{
	auto lineIndex = std::distance (md.model.lines.cbegin (), it);
	return calculateLineRect (lineIndex);
}

//------------------------------------------------------------------------
CRect TextEditorView::calculateSelectionRect () const
{
	CRect result;
	for (auto index = md.selectedLines.start; index < md.selectedLines.end (); ++index)
	{
		auto r = calculateLineRect (index);
		if (result.isEmpty ())
			result = r;
		else
			result.unite (r);
	}
	result.setWidth (getWidth ());
	return result;
}

//------------------------------------------------------------------------
CRect TextEditorView::calculateCursorRect (int cursor) const
{
	auto cursorWidth = md.lineHeight / 10.;
	CRect r {-cursorWidth / 2., -md.style->lineSpacing, cursorWidth / 2., -md.style->lineSpacing};
	for (const auto& line : md.model.lines)
	{
		r.offset (0., md.lineHeight);
		if (static_cast<size_t> (cursor) <= line.range.end ())
		{
			if (static_cast<size_t> (cursor) == line.range.start)
				break;
			if (md.model.text[cursor - 1] != '\n')
			{
				auto lineTextToCursor =
					convert (md.model.text.data () + line.range.start, cursor - line.range.start);
				replaceTabs (lineTextToCursor, md.style->tabWidth);
				auto platformText = getPlatformFactory ().createString (lineTextToCursor.data ());
				auto width = md.fontPainer->getStringWidth (nullptr, platformText);
				r.offset (width, 0);
				break;
			}
		}
	}
	r.offset (md.style->leftMargin, 0.);
	r.top = r.top - md.fontAscent;
	r.bottom = r.bottom + md.fontDescent;
	r.inset (0, -md.style->lineSpacing / 2.);
	return r;
}

//------------------------------------------------------------------------
CRect TextEditorView::invalidCursorRect () const
{
	auto r = md.cursorRect;
	r.offset (getViewSize ().getTopLeft ());
	invalidateRect (r);
	return r;
}

//------------------------------------------------------------------------
void TextEditorView::toggleCursorVisibility () const
{
	using namespace Animation;
	mutableThis ().addAnimation (
		"CursorAlphaBlend",
		new FuncAnimation ([] (CView* view, IdStringPtr name) {},
						   [this] (CView* view, IdStringPtr name, float pos) {
							   md.cursorAlpha = md.cursorIsVisible ? 1.f - pos : pos;
							   invalidCursorRect ();
						   },
						   [this] (CView* view, IdStringPtr name, bool wasCanceled) {
							   if (!wasCanceled)
								   md.cursorIsVisible = !md.cursorIsVisible;
							   md.cursorAlpha = md.cursorIsVisible ? 1.f : 0.f;
							   invalidCursorRect ();
						   }),
		CubicBezierTimingFunction::make (md.cursorIsVisible ? CubicBezierTimingFunction::EasyOut
															: CubicBezierTimingFunction::EasyIn,
										 md.style->cursorBlinkTime / 2));
}

//------------------------------------------------------------------------
void TextEditorView::restartBlinkTimer () const
{
	if (md.style->cursorBlinkTime && md.editState.select_start == md.editState.select_end)
	{
		mutableThis ().removeAnimation ("CursorAlphaBlend");
		md.cursorIsVisible = true;
		md.cursorAlpha = 1.f;
		invalidCursorRect ();
		md.blinkTimer = makeOwned<CVSTGUITimer> ([this] (auto timer) { toggleCursorVisibility (); },
												 md.style->cursorBlinkTime);
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
				restartBlinkTimer ();
			}
		}
	});

	for (auto index = 0u; index < md.commandKeys.size (); ++index)
	{
		const auto& cmd = md.commandKeys[index];
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
	if (callSTB ([&] () { stb_textedit_key (this, &md.editState, key); }))
		event.consumed = true;
}

//------------------------------------------------------------------------
void TextEditorView::onMouseDownEvent (MouseDownEvent& event)
{
	if (!event.buttonState.isLeft ())
		return;

	getFrame ()->setFocusView (this);

	md.mouseIsDown = true;
	md.editStateOnMouseDown = md.editState;

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
		stb_textedit_click (this, &md.editState, static_cast<float> (pos.x),
							static_cast<float> (pos.y));
	});

	if (event.modifiers.is (ModifierKey::Shift))
	{
		md.editState.select_start = md.editStateOnMouseDown.select_start;
		md.editState.select_end = md.editState.cursor;
		onSelectionChanged (makeRange (md.editState));
	}

	event.consumed = true;
}

//------------------------------------------------------------------------
void TextEditorView::onMouseMoveEvent (MouseMoveEvent& event)
{
	md.lastMouse = event.mousePosition;
	if (!md.mouseIsDown)
		return;

	CPoint pos (event.mousePosition);
	pos -= getViewSize ().getTopLeft ();
	auto tm = getGlobalTransform (true);
	pos.x /= tm.m11;
	pos.y /= tm.m22;

	callSTB ([&] () {
		stb_textedit_drag (this, &md.editState, static_cast<float> (pos.x),
						   static_cast<float> (pos.y));
	});
	if (event.clickCount > 1)
		updateSelectionOnDoubleClickMove (event.clickCount);
	event.consumed = true;
}

//------------------------------------------------------------------------
void TextEditorView::onMouseUpEvent (MouseUpEvent& event)
{
	if (!md.mouseIsDown)
		return;
	restartBlinkTimer ();
	event.consumed = true;
	md.mouseIsDown = false;
}

//------------------------------------------------------------------------
void TextEditorView::onMouseCancelEvent (MouseCancelEvent& event)
{
	if (md.mouseIsDown)
		md.mouseIsDown = false;
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
	md.lastMouse = MouseOutsidePos;
}

//------------------------------------------------------------------------
CCoord TextEditorView::calculateMaxWidth () const
{
	auto width = 0.;
	for (const auto& line : md.model.lines)
	{
		if (line.width > width)
			width = line.width;
	}
	return width;
}

//------------------------------------------------------------------------
void TextEditorView::invalidateRect (CRect r) const { mutableThis ().invalidRect (r); }

//------------------------------------------------------------------------
void TextEditorView::invalidate (Dirty what) const
{
	if (what & Dirty::Layout)
	{
		layoutRows ();
	}
	if (what & Dirty::UI)
	{
		onCursorChanged (md.editState.cursor, md.editState.cursor);
		mutableThis ().invalid ();
	}
}

//------------------------------------------------------------------------
CCoord TextEditorView::updateLineText (Lines::iterator& line) const
{
	auto newText = convert (md.model.text.data () + line->range.start, line->range.length);
	replaceTabs (newText, md.style->tabWidth);
	if (newText != line->text)
	{
		line->text = std::move (newText);
		line->width = md.fontPainer->getStringWidth (nullptr, line->text.getPlatformString ());
	}
	return line->width;
}

//------------------------------------------------------------------------
void TextEditorView::updateLineNumbersView () const
{
	if (!md.lineNumberView)
		return;

	UTF8String str;
	auto lineCount = md.model.lines.size ();
	while (lineCount > 0)
	{
		str += "9";
		lineCount /= 10;
	}
	auto lineNumbersWidth = md.style->lineNumbersFont->getFontPainter ()->getStringWidth (
		nullptr, str.getPlatformString ());
	lineNumbersWidth += (md.style->lineNumerLeftMargin + md.style->lineNumerRightMargin);
	lineNumbersWidth = std::ceil (lineNumbersWidth);
	auto vs = md.lineNumberView->getViewSize ();
	if (vs.getWidth () != lineNumbersWidth)
		vs.setWidth (lineNumbersWidth);
	md.lineNumberView->setViewSize (vs);
	md.lineNumberView->setNumLines (md.model.lines.size ());
}

//------------------------------------------------------------------------
void TextEditorView::layoutRows () const
{
	NewLineProcessor::update (md.model.lines, md.model.text, 0);
	CCoord width = 0.;
	for (auto it = md.model.lines.begin (); it != md.model.lines.end (); ++it)
	{
		auto w = updateLineText (it);
		if (w > width)
			width = w;
	}
	md.maxWidth = std::ceil (width + md.horizontalLineMargin);
	md.maxHeight = md.model.lines.size () * md.lineHeight + md.fontDescent;
	mutableThis ().parentSizeChanged ();
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
void TextEditorView::insertNewLine () const
{
	auto cursor = md.editState.cursor;
	auto currentLine = findLine (md.model.lines.begin (), md.model.lines.end (), cursor);
	if (currentLine == md.model.lines.end ())
		return;
	std::u16string insertStr = u"\n";
	auto isWhiteSpace = [] (char16_t character) {
		return character == u'\t' || character == u' ';
	};
	for (auto index = currentLine->range.start; index < cursor; ++index)
	{
		if (isWhiteSpace (md.model.text[index]))
		{
			insertStr += md.model.text[index];
			continue;
		}
		break;
	}
	callSTB ([&] () {
		stb_textedit_paste (this, &md.editState, insertStr.data (),
							static_cast<int32_t> (insertStr.size ()));
	});
}

//------------------------------------------------------------------------
void TextEditorView::validateLineStarts (Lines::const_iterator it,
										 const Lines::const_iterator& end) const
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
void TextEditorView::invalidateSingleLine (size_t pos, int32_t numChars) const
{
	auto line = findLine (md.model.lines.begin (), md.model.lines.end (), pos);
	invalidateSingleLine (line, numChars);
}

//------------------------------------------------------------------------
void TextEditorView::invalidateSingleLine (Lines::iterator& line, int32_t numChars) const
{
	if (line == md.model.lines.end ())
		return;

	auto wasMaxWidth = std::ceil (line->width + md.horizontalLineMargin) == md.maxWidth;
	if (numChars < 0)
	{
		invalidLine (line, true);
	}

	line->range.length += numChars;
	auto width = std::ceil (updateLineText (line) + md.horizontalLineMargin);
	if (width > md.maxWidth)
	{
		md.maxWidth = width;
		mutableThis ().parentSizeChanged ();
	}

	invalidLine (line);
	if (numChars)
	{
		auto it = line;
		++it;
		for (; it != md.model.lines.end (); ++it)
			it->range.start += numChars;
	}

	if (wasMaxWidth && width != md.maxWidth)
	{
		auto newMaxWidth = std::ceil (calculateMaxWidth () + md.horizontalLineMargin);
		if (md.maxWidth != newMaxWidth)
		{
			md.maxWidth = newMaxWidth;
			mutableThis ().parentSizeChanged ();
		}
	}
}

//------------------------------------------------------------------------
void TextEditorView::invalidateLines (size_t pos, int32_t numChars) const
{
	auto line = findLine (md.model.lines.begin (), md.model.lines.end (), pos);
	if (line == md.model.lines.end ())
		return;
	bool numLinesChanged = false;
	if (numChars > 0)
	{
		int32_t counter = 0;
		for (auto index = pos; index < pos + numChars; ++index)
		{
			if (md.model.text[index] == u'\n')
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
				line = md.model.lines.emplace (++line, Line {nextLineRange, "", 0.});
				updateLineText (line);
				auto nextLine = line;
				auto prevLine = line;
				while (++nextLine != md.model.lines.end ())
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
		auto lineIndex = std::distance (md.model.lines.begin (), line);
		while (counter > 0)
		{
			auto numDel = std::min (counter, static_cast<int32_t> (line->range.end ()) -
												 static_cast<int32_t> (pos));
			vstgui_assert (md.model.text.size () > (pos + numDel - 1));
			bool isNewLine = md.model.text[pos + numDel - 1] == u'\n';
			md.model.text.erase (pos, numDel);
			invalidLine (lineIndex++, true);
			if (isNewLine)
			{
				auto nextLine = line;
				++nextLine;
				nextLine->range.length += line->range.length - numDel;
				nextLine->range.start = line->range.start;
				nextLine = md.model.lines.erase (line);
				numLinesChanged = true;
				invalidateSingleLine (nextLine, 0);
				auto prevLine = nextLine;
				while (++nextLine != md.model.lines.end ())
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
			auto width = std::ceil (calculateMaxWidth () + md.horizontalLineMargin);
			if (width != md.maxWidth)
				md.maxWidth = width;
		}
	}
	if (numLinesChanged)
	{
		md.maxHeight = md.model.lines.size () * md.lineHeight + md.fontDescent;
		updateLineNumbersView ();
		if (numChars < 0)
			invalidate ();
		else
		{
			for (auto l = findLine (md.model.lines.begin (), md.model.lines.end (), pos);
				 l != md.model.lines.end (); ++l)
				invalidLine (l, true);
		}

		mutableThis ().parentSizeChanged ();
	}
	validateLineStarts (md.model.lines.begin (), md.model.lines.end ());
}

//------------------------------------------------------------------------
int32_t TextEditorView::deleteChars (size_t pos, size_t num) const
{
	if (pos >= md.model.text.size ())
		return false;

	invalidateLines (pos, -static_cast<int32_t> (num));

	if (md.controller)
		md.controller->onTextEditorTextChanged (*this);
	return true;
}

//------------------------------------------------------------------------
int32_t TextEditorView::insertChars (size_t pos, const CharT* chars, size_t num) const
{
	md.isInsertingText = true;
	if (pos >= md.model.text.size ())
		pos = md.model.text.size ();
	md.model.text.insert (pos, chars, num);
	invalidateLines (pos, static_cast<int32_t> (num));
	md.isInsertingText = false;
	if (md.controller)
		md.controller->onTextEditorTextChanged (*this);
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
	if (static_cast<size_t> (start_i) >= md.model.text.size ())
		return;
	if (start_i == md.stbInternalIterator->range.start)
	{
		row->num_chars = static_cast<int> (md.stbInternalIterator->range.length);
		row->baseline_y_delta = static_cast<float> (md.lineHeight);
		row->ymin = static_cast<float> (-md.fontDescent);
		row->ymax = static_cast<float> (md.lineHeight);
		row->x0 = static_cast<float> (md.style->leftMargin);
		row->x1 = static_cast<float> (getWidth ());
		++md.stbInternalIterator;
		return;
	}
	md.stbInternalIterator = md.model.lines.begin ();
	auto line = findLine (md.model.lines.begin (), md.model.lines.end (), start_i);
	if (line == md.model.lines.end ())
		return;
	row->num_chars = static_cast<int> (line->range.length);
	row->baseline_y_delta = static_cast<float> (md.lineHeight);
	row->ymin = static_cast<float> (-md.fontDescent);
	row->ymax = static_cast<float> (md.lineHeight);
	row->x0 = static_cast<float> (md.style->leftMargin);
	row->x1 = static_cast<float> (getWidth ());
}

//------------------------------------------------------------------------
float TextEditorView::getCharWidth (size_t row, size_t pos) const
{
	if (row + pos >= md.model.text.length ())
		return STB_TEXTEDIT_GETWIDTH_NEWLINE;

	auto platformFont = md.style->font->getPlatformFont ();
	vstgui_assert (platformFont);
	auto fontPainter = platformFont->getPainter ();
	vstgui_assert (fontPainter);

	auto c = md.model.text[row + pos];

	UTF8String str (convert (&c, 1));
	if (str[0] == '\n')
		return STB_TEXTEDIT_GETWIDTH_NEWLINE;
	bool isTab = str[0] == '\t';
	if (isTab && md.style->tabWidth > 0)
	{
		str = " ";
		auto tabWidth = md.style->tabWidth;
		while (--tabWidth)
			str += " ";
	}
	auto width = fontPainter->getStringWidth (nullptr, str.getPlatformString (), true);
	return static_cast<float> (width / getGlobalTransform (true).m11);
}

//------------------------------------------------------------------------
CharT TextEditorView::getChar (int32_t pos) const
{
	if (pos < getLength ())
		return md.model.text[pos];
	return 0;
}

//------------------------------------------------------------------------
int32_t TextEditorView::getLength () const
{
	return static_cast<int32_t> (md.model.text.length ());
}

//------------------------------------------------------------------------
void TextEditorView::onSelectionChanged (Range newSel, bool forceInvalidation) const
{
	Range newSelectedLines = {};
	auto line = findLine (md.model.lines.begin (), md.model.lines.end (), newSel.start);
	if (line != md.model.lines.end ())
	{
		newSelectedLines.start = std::distance (md.model.lines.begin (), line);
		if (newSel.length > 0)
		{
			auto endLine =
				++(findLine (md.model.lines.begin (), md.model.lines.end (), newSel.end ()));
			newSelectedLines.length = std::distance (line, endLine);
		}
	}
	if (forceInvalidation || newSelectedLines != md.selectedLines)
	{
		invalidSelectedLines ();
		md.selectedLines = newSelectedLines;
		invalidSelectedLines ();
	}
	if (md.lineNumberView)
		md.lineNumberView->setSelectedLines (newSelectedLines);
	if (md.selectedLines.length > 0)
	{
		md.blinkTimer = nullptr;
		md.cursorIsVisible = false;
	}
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
template<bool forward>
void TextEditorView::selectPair (size_t startPos, char16_t closingChar) const
{
	auto it = md.model.text.begin ();
	std::advance (it, startPos);
	auto openChar = *it;
	int32_t numOpenChars = 0;
	while (true)
	{
		if (*it == openChar)
			++numOpenChars;
		if (*it == closingChar)
		{
			if (--numOpenChars == 0)
			{
				md.editState.select_start = static_cast<int> (startPos);
				md.editState.select_end =
					static_cast<int> (std::distance (md.model.text.begin (), it)) + 1;
				if constexpr (!forward)
				{
					md.editState.select_start += 1;
					md.editState.select_end -= 1;
				}
				onSelectionChanged (makeRange (md.editState), true);
				return;
			}
		}
		if constexpr (forward)
		{
			++it;
			if (it == md.model.text.end ())
				break;
		}
		else
		{
			if (it == md.model.text.begin ())
				break;
			--it;
		}
	}
}

//------------------------------------------------------------------------
void TextEditorView::selectOnDoubleClick (uint32_t clickCount) const
{
	auto cursor = static_cast<size_t> (md.editState.cursor);
	if (cursor >= md.model.text.size ())
		return;

	if (clickCount > 2)
	{
		auto currentLine =
			findLine (md.model.lines.begin (), md.model.lines.end (), md.editState.cursor);
		if (currentLine != md.model.lines.end ())
		{
			md.editState.select_start = static_cast<int> (currentLine->range.start);
			md.editState.select_end = static_cast<int> (currentLine->range.end ());
			onSelectionChanged (makeRange (md.editState), true);
			md.editStateOnMouseDown = md.editState;
		}
		return;
	}

	auto cursorChar = md.model.text[cursor];
	switch (cursorChar)
	{
		case u'(':
		{
			selectPair<true> (cursor, ')');
			return;
		}
		case u')':
		{
			selectPair<false> (cursor, '(');
			return;
		}
		case u'{':
		{
			selectPair<true> (cursor, '}');
			return;
		}
		case u'}':
		{
			selectPair<false> (cursor, '{');
			return;
		}
		case u'[':
		{
			selectPair<true> (cursor, ']');
			return;
		}
		case u']':
		{
			selectPair<false> (cursor, '[');
			return;
		}
		case u'<':
		{
			selectPair<true> (cursor, '>');
			return;
		}
		case u'>':
		{
			selectPair<false> (cursor, '<');
			return;
		}
	};
	if (isStopChar (cursorChar))
		return;

	if (cursor > 0)
	{
		auto it = md.model.text.begin ();
		std::advance (it, cursor - 1);
		if (findStopChar<false> (it, md.model.text.begin ()))
			++it;
		md.editState.select_start = static_cast<int> (std::distance (md.model.text.begin (), it));
	}
	if (cursor < md.model.text.size ())
	{
		auto it = md.model.text.begin ();
		std::advance (it, cursor + 1);
		findStopChar<true> (it, md.model.text.end ());
		md.editState.select_end = static_cast<int> (std::distance (md.model.text.begin (), it));
	}
	md.editStateOnMouseDown = md.editState;
	onSelectionChanged (makeRange (md.editState), true);
}

//------------------------------------------------------------------------
void TextEditorView::updateSelectionOnDoubleClickMove (uint32_t clickCount) const
{
	auto cursor = static_cast<size_t> (md.editState.cursor);
	if (cursor >= md.model.text.size ())
		return;

	if (clickCount > 2)
	{
		auto currentLine = findLine (md.model.lines.begin (), md.model.lines.end (), cursor);
		if (currentLine != md.model.lines.end ())
		{
			if (currentLine->range.start >=
				static_cast<size_t> (md.editStateOnMouseDown.select_start))
			{
				md.editState.select_start = md.editStateOnMouseDown.select_start;
				md.editState.select_end = static_cast<int> (currentLine->range.end ());
			}
			else
			{
				md.editState.select_start = static_cast<int> (currentLine->range.start);
				md.editState.select_end = md.editStateOnMouseDown.select_end;
			}
			onSelectionChanged (makeRange (md.editState), true);
		}
		return;
	}

	if (cursor < static_cast<size_t> (md.editStateOnMouseDown.select_start))
	{
		auto it = md.model.text.begin ();
		std::advance (it, cursor - 1);
		if (findStopChar<false> (it, md.model.text.begin ()))
			++it;
		auto start = static_cast<int> (std::distance (md.model.text.begin (), it));
		if (start < md.editStateOnMouseDown.select_start)
			md.editState.select_start = start;
		else
			md.editState.select_start = md.editStateOnMouseDown.select_start;
		md.editState.select_end = md.editStateOnMouseDown.select_end;
	}
	else
	{
		auto it = md.model.text.begin ();
		std::advance (it, cursor + 1);
		findStopChar<true> (it, md.model.text.end ());
		auto end = static_cast<int> (std::distance (md.model.text.begin (), it));
		if (end > md.editStateOnMouseDown.select_end)
			md.editState.select_end = end;
		else
			md.editState.select_end = md.editStateOnMouseDown.select_end;
		md.editState.select_start = md.editStateOnMouseDown.select_start;
	}
	onSelectionChanged (makeRange (md.editState), true);
}

//------------------------------------------------------------------------
void TextEditorView::onCursorChanged (int oldCursorPos, int newCursorPos) const
{
	if (!md.lastDrawnCursorRect.isEmpty ())
		invalidateRect (md.lastDrawnCursorRect);
	md.cursorRect = calculateCursorRect (newCursorPos);
	auto r = invalidCursorRect ();
	if (md.scrollView)
	{
		if (md.editState.select_end != md.editState.select_start)
		{
			auto endRect = calculateCursorRect (md.editState.select_end);
			endRect.offset (getViewSize ().getTopLeft ());
			r.unite (endRect);
		}
		r.bottom += md.fontDescent;
		r.inset (-md.style->leftMargin, 0);
		md.scrollView->makeRectVisible (r);
	}
}

//------------------------------------------------------------------------
size_t TextEditorView::moveToWordPrevious (size_t pos) const
{
	if (pos == 0)
		return 0;
	--pos;
	auto it = md.model.text.begin ();
	std::advance (it, pos);
	auto firstIsStopChar = isStopChar (*it);
	for (; it != md.model.text.begin (); --it)
	{
		auto cursorChar = *it;
		if (isStopChar (cursorChar) != firstIsStopChar)
		{
			++it;
			break;
		}
	}
	return std::distance (md.model.text.begin (), it);
}

//------------------------------------------------------------------------
size_t TextEditorView::moveToWordNext (size_t pos) const
{
	if (pos >= md.model.text.size ())
		return pos;
	++pos;
	auto it = md.model.text.begin ();
	std::advance (it, pos);
	auto firstIsStopChar = isStopChar (*it);
	for (; it != md.model.text.end (); ++it)
	{
		auto cursorChar = *it;
		if (isStopChar (cursorChar) != firstIsStopChar)
			break;
	}
	return std::distance (md.model.text.begin (), it);
}

//-----------------------------------------------------------------------------
template<typename Proc>
bool TextEditorView::callSTB (Proc proc) const
{
	md.stbInternalIterator = md.model.lines.begin ();
	auto oldState = md.editState;
	proc ();
	if (memcmp (&oldState, &md.editState, sizeof (STB_TexteditState)) != 0)
	{
		if (oldState.select_start != md.editState.select_start ||
			oldState.select_end != md.editState.select_end)
			onSelectionChanged (makeRange (md.editState), true);
		if (oldState.cursor != md.editState.cursor)
		{
			onCursorChanged (oldState.cursor, md.editState.cursor);
			auto selRange = makeRange (md.editState);
			if (selRange.length == 0)
				onSelectionChanged (makeRange (md.editState.cursor, md.editState.cursor));
		}
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool TextEditorView::doShifting (bool right) const
{
	if (md.editState.select_start > md.editState.select_end)
		std::swap (md.editState.select_start, md.editState.select_end);
	auto hasSelection = md.editState.select_start != md.editState.select_end;
	if (!hasSelection)
		md.editState.select_start = md.editState.select_end = md.editState.cursor;
	auto lineStart =
		findLine (md.model.lines.begin (), md.model.lines.end (), md.editState.select_start);
	if (lineStart == md.model.lines.end ())
		return false;
	auto lineEnd = !hasSelection
					   ? lineStart
					   : findLine (lineStart, md.model.lines.end (), md.editState.select_end - 1);
	if (lineEnd == md.model.lines.end ())
		return false;
	++lineEnd;
	auto originCursor = md.editState.cursor;
	auto originSelectStart = md.editState.select_start;
	auto originSelectEnd = md.editState.select_end;
	md.editState.select_start = md.editState.select_end = originCursor;
	auto numChanges = 0;
	if (right)
	{
		while (lineStart != lineEnd)
		{
			--lineEnd;
			md.editState.cursor = static_cast<int> (lineEnd->range.start);
			md.editState.select_start = md.editState.select_end = md.editState.cursor;
			callSTB ([&] () { stb_textedit_key (this, &md.editState, u'\t'); });
			++numChanges;
		}
		md.editState.select_start = originSelectStart + 1;
		md.editState.select_end = originSelectEnd + numChanges;
		md.editState.cursor = md.editState.select_start;
	}
	else
	{
		while (lineStart != lineEnd)
		{
			--lineEnd;
			md.editState.cursor = static_cast<int> (lineEnd->range.start);
			if (md.model.text[md.editState.cursor] == u'\t')
			{
				md.editState.select_start = md.editState.cursor;
				md.editState.select_end = md.editState.cursor + 1;
				callSTB ([&] () { stb_textedit_key (this, &md.editState, STB_TEXTEDIT_K_DELETE); });
				++numChanges;
			}
		}
		md.editState.select_start = originSelectStart - (numChanges ? 1 : 0);
		md.editState.select_end = originSelectEnd - numChanges;
		md.editState.cursor = md.editState.select_start;
	}
	onCursorChanged (originCursor, md.editState.cursor);
	onSelectionChanged (makeRange (md.editState));
	return true;
}

//------------------------------------------------------------------------
void TextEditorView::selectAll () const
{
	md.editState.select_start = 0;
	md.editState.select_end = getLength ();
	onSelectionChanged (makeRange (md.editState), true);
}

//------------------------------------------------------------------------
bool TextEditorView::doCut () const
{
	if (doCopy ())
	{
		callSTB ([&] () { stb_textedit_cut (this, &md.editState); });
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool TextEditorView::doCopy () const
{
	if (md.editState.select_start == md.editState.select_end)
		return false;
	auto start = md.editState.select_start;
	auto end = md.editState.select_end;
	if (start > end)
		std::swap (start, end);
	auto txt = convert (md.model.text.data () + start, end - start);
	auto dataPackage =
		CDropSource::create (txt.data (), static_cast<uint32_t> (txt.size ()), IDataPackage::kText);
	getFrame ()->setClipboard (dataPackage);
	return true;
}

//------------------------------------------------------------------------
bool TextEditorView::doPaste () const
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
					stb_textedit_paste (this, &md.editState, uText.data (),
										static_cast<int32_t> (uText.size ()));
				});
				return true;
			}
		}
	}
	return false;
}

//------------------------------------------------------------------------
bool TextEditorView::useSelectionForFind () const
{
	if (auto range = makeRange (md.editState))
	{
		setFindString (md.model.text.substr (range.start, range.length));
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool TextEditorView::doFind (bool forward, size_t oldPos) const
{
	if (md.findString.empty ())
		return false;

	std::optional<size_t> firstFound = {};

	auto pos = String::npos;
	while (true)
	{
		if (md.findOptions & FindOption::CaseSensitive)
			pos = doFindCaseSensitive (forward);
		else
			pos = doFindIgnoreCase (forward);
		if (pos == String::npos)
			break;

		auto oldCursor = md.editState.cursor;
		md.editState.cursor = static_cast<int> (pos);
		md.editState.select_start = md.editState.cursor;
		md.editState.select_end = md.editState.cursor + static_cast<int> (md.findString.length ());

		if (md.findOptions & FindOption::WholeWords)
		{
			if ((pos > 0 && !isStopChar (md.model.text[pos - 1])) ||
				(pos + md.findString.length () < md.model.text.size () &&
				 !isStopChar (md.model.text[pos + md.findString.length ()])))
			{
				if (firstFound)
				{
					if (pos == *firstFound)
						break;
				}
				else
				{
					firstFound = {pos};
				}
				continue;
			}
		}

		onCursorChanged (oldCursor, md.editState.cursor);
		onSelectionChanged ({makeRange (md.editState)}, true);

		return true;
	}

	if (md.editState.select_start != md.editState.select_end)
	{
		md.editState.select_end = md.editState.select_start;
		onSelectionChanged ({makeRange (md.editState)}, true);
	}

	return false;
}

//------------------------------------------------------------------------
String::size_type TextEditorView::doFindCaseSensitive (bool forward) const
{
	auto pos = String::npos;
	if (forward)
	{
		auto cursor = static_cast<size_t> (md.editState.select_end);
		if (cursor > md.model.text.length ())
			cursor = 0;
		auto text = StringView (md.model.text.data (), md.model.text.length ());
		pos = text.find (md.findString.data (), cursor, md.findString.length ());
		if (pos == String::npos && cursor != 0 /*wrap around*/)
			pos = text.find (md.findString.data (), 0, md.findString.length ());
	}
	else
	{
		auto cursor = md.editState.select_start - md.findString.length ();
		if (cursor < 0)
			cursor = String::npos;
		auto text = StringView (md.model.text.data (), md.model.text.length ());
		pos = text.rfind (md.findString.data (), cursor);
		if (pos == String::npos /*wrap around*/)
			pos = text.rfind (md.findString.data (), pos);
	}
	return pos;
}

//------------------------------------------------------------------------
String::size_type TextEditorView::doFindIgnoreCase (bool forward) const
{
	auto pos = String::npos;
	if (forward)
	{
		auto cursor = static_cast<size_t> (md.editState.select_end);
		if (cursor > md.model.text.length ())
			cursor = 0;

		auto text = StringView (md.model.text.data () + cursor, md.model.text.length () - cursor);
		auto it = std::search (
			text.begin (), text.end (), md.findString.begin (), md.findString.end (),
			[] (auto ch1, auto ch2) { return std::toupper (ch1) == std::toupper (ch2); });
		if (it != text.end ())
		{
			pos = cursor + std::distance (text.begin (), it);
		}
		else if (cursor != 0) // wrap arround
		{
			text = StringView (md.model.text.data (), cursor);
			it = std::search (
				text.begin (), text.end (), md.findString.begin (), md.findString.end (),
				[] (auto ch1, auto ch2) { return std::toupper (ch1) == std::toupper (ch2); });
			if (it != text.end ())
			{
				pos = std::distance (text.begin (), it);
			}
		}
	}
	else
	{
		auto cursor = md.editState.select_start - md.findString.length ();
		if (cursor < 0)
			cursor = md.model.text.length ();
		auto text = StringView (md.model.text.data (), cursor);
		auto it = std::search (
			text.rbegin (), text.rend (), md.findString.rbegin (), md.findString.rend (),
			[] (auto ch1, auto ch2) { return std::toupper (ch1) == std::toupper (ch2); });
		if (it != text.rend ())
		{
			pos = cursor - (std::distance (text.rbegin (), it) + md.findString.length ());
		}
		else if (cursor < md.model.text.length ()) // wrap arround
		{
			text = StringView (md.model.text.data () + cursor, md.model.text.length () - cursor);
			it = std::search (
				text.rbegin (), text.rend (), md.findString.rbegin (), md.findString.rend (),
				[] (auto ch1, auto ch2) { return std::toupper (ch1) == std::toupper (ch2); });
			if (it != text.rend ())
			{
				pos = (md.model.text.length () - md.findString.length ()) -
					  std::distance (text.rbegin (), it);
			}
		}
	}
	return pos;
}

//------------------------------------------------------------------------
FindPanelController::FindPanelController (const ITextEditor& editor) : editor (editor) {}

//------------------------------------------------------------------------
FindPanelController::~FindPanelController () noexcept
{
	if (caseSensitiveButton)
	{
		caseSensitiveButton->setListener (nullptr);
		caseSensitiveButton->unregisterViewEventListener (this);
		caseSensitiveButton->unregisterViewListener (this);
	}
	if (wholeWordButton)
	{
		wholeWordButton->setListener (nullptr);
		wholeWordButton->unregisterViewEventListener (this);
		wholeWordButton->unregisterViewListener (this);
	}
	if (closeBox)
	{
		closeBox->setListener (nullptr);
		closeBox->unregisterViewListener (this);
	}
	if (editfield)
	{
		editfield->setListener (nullptr);
		editfield->unregisterViewEventListener (this);
		editfield->unregisterViewListener (this);
	}
}

//------------------------------------------------------------------------
void FindPanelController::valueChanged (CControl* control)
{
	switch (control->getTag ())
	{
		case FindPanelController::Textfield:
		{
			editor.setFindString (editfield->getText ().getString ());
			break;
		}
		case FindPanelController::CloseButton:
		{
			removeFindPanelFunc ();
			break;
		}
		case FindPanelController::FindPrevious:
		{
			if (control->getValue () != control->getMax ())
				editor.handleCommand (ITextEditor::Command::FindPrevious);
			break;
		}
		case FindPanelController::FindNext:
		{
			if (control->getValue () != control->getMax ())
				editor.handleCommand (ITextEditor::Command::FindNext);
			break;
		}
		case FindPanelController::CaseSensitive:
		{
			if (control->getValue () == control->getMax ())
				findOptions.add (ITextEditor::FindOption::CaseSensitive);
			else
				findOptions.remove (ITextEditor::FindOption::CaseSensitive);
			editor.setFindOptions (findOptions);
			break;
		}
		case FindPanelController::WholeWords:
		{
			if (control->getValue () == control->getMax ())
				findOptions.add (ITextEditor::FindOption::WholeWords);
			else
				findOptions.remove (ITextEditor::FindOption::WholeWords);
			editor.setFindOptions (findOptions);
			break;
		}
	}
}

//------------------------------------------------------------------------
void FindPanelController::viewLostFocus (CView* view)
{
	if (view == editfield)
	{
		editor.setFindString (editfield->getText ().getString ());
		if (editfield->bWasReturnPressed)
		{
			editor.handleCommand (ITextEditor::Command::TakeFocus);
			editfield->bWasReturnPressed = false;
		}
	}
}

//------------------------------------------------------------------------
void FindPanelController::viewOnEvent (CView* view, Event& event)
{
	if (event.type == EventType::KeyDown)
	{
		auto& keyEvent = castKeyboardEvent (event);
		if (keyEvent.virt == VirtualKey::Escape)
		{
			removeFindPanelFunc ();
			event.consumed = true;
		}
		else if (commandKeys[Index (ITextEditor::Command::FindNext)] == keyEvent)
		{
			editor.handleCommand (ITextEditor::Command::FindNext);
			event.consumed = true;
		}
		else if (commandKeys[Index (ITextEditor::Command::FindPrevious)] == keyEvent)
		{
			editor.handleCommand (ITextEditor::Command::FindPrevious);
			event.consumed = true;
		}
	}
}

//------------------------------------------------------------------------
void FindPanelController::viewWillDelete (CView* view)
{
	view->unregisterViewListener (this);
	if (view == closeBox)
		closeBox = nullptr;
	else if (view == editfield)
		editfield = nullptr;
	else if (view == caseSensitiveButton)
	{
		caseSensitiveButton->unregisterViewEventListener (this);
		caseSensitiveButton = nullptr;
	}
	else if (view == wholeWordButton)
	{
		wholeWordButton->unregisterViewEventListener (this);
		wholeWordButton = nullptr;
	}
	else if (view->asViewContainer ())
		delete this;
}

//------------------------------------------------------------------------
void FindPanelController::setFindString (StringView text)
{
	if (editfield)
		editfield->setText (convert (text.data (), text.size ()).data ());
}

//------------------------------------------------------------------------
void FindPanelController::setFindOptions (ITextEditor::FindOptions opt) { findOptions = opt; }

//------------------------------------------------------------------------
CViewContainer* FindPanelController::makeFindPanelView (CRect vcr, TextEditorView::ModelData& md,
														const ITextEditor& editor,
														RemoveFindPanelFunc&& f)
{
	auto controller = new FindPanelController (editor);
	controller->removeFindPanelFunc = std::move (f);
	controller->commandKeys = md.commandKeys;
	controller->setFindOptions (md.findOptions);

	auto margin = (vcr.getHeight () - md.lineHeight) / 2.;
	vcr.inset (margin, margin);
	auto buttonWidth = vcr.getHeight ();

	CViewContainer* findPanel = new CViewContainer (vcr);
	findPanel->setTransparency (true);
	auto tefr = vcr;
	tefr.left += buttonWidth + margin;
	tefr.right -= 6. * (buttonWidth + margin) + margin;
	CTextEdit* editfield = new CTextEdit (tefr, controller, FindPanelController::Textfield);
	editfield->setPlaceholderString ("Find");
	editfield->setTextInset ({margin, 0});
	editfield->setImmediateTextChange (true);
	editfield->setAutosizeFlags (kAutosizeLeft | kAutosizeRight | kAutosizeTop);
	editfield->setHoriAlign (kLeftText);
	editfield->setFrameColor (md.style->frameColor);
	editfield->setFrameWidth (-1.);
	editfield->setBackColor (md.style->backColor);
	editfield->setFont (md.style->font);
	editfield->setFontColor (md.style->textColor);

	auto styleButton = [&] (CTextButton* button) {
		button->setFont (md.style->font);
		button->setAutosizeFlags (kAutosizeTop | kAutosizeRight);
		button->setFrameWidth (1.);
		button->setRoundRadius (2.);
		auto gradient =
			owned (CGradient::create (0., 1., md.style->backColor, md.style->backColor));
		button->setGradient (gradient);
		auto isOnOffStyle = button->getStyle () == CTextButton::Style::kOnOffStyle;
		if (isOnOffStyle)
			gradient = owned (CGradient::create (0., 1., md.style->textColor, md.style->textColor));
		button->setGradientHighlighted (gradient);
		button->setTextColor (md.style->textColor);
		auto color = md.style->frameColor;
		color.alpha /= 2;
		button->setTextColorHighlighted (isOnOffStyle ? md.style->backColor : md.style->textColor);
		button->setFrameColor (md.style->frameColor);
		button->setFrameColorHighlighted (color);
	};

	auto cbfr = vcr;
	cbfr.right = tefr.left - margin;
	auto closeBox = new CTextButton (cbfr, controller, FindPanelController::CloseButton);
	closeBox->setTitle ("X");
	styleButton (closeBox);
	closeBox->setAutosizeFlags (kAutosizeLeft | kAutosizeTop);

	auto icfr = tefr;
	icfr.left = tefr.right + margin;
	icfr.right = icfr.left + buttonWidth * 2;
	auto caseSensitiveButton =
		new CTextButton (icfr, controller, FindPanelController::CaseSensitive);
	caseSensitiveButton->setTitle ("Aa");
	caseSensitiveButton->setStyle (CTextButton::Style::kOnOffStyle);
	caseSensitiveButton->setValue (md.findOptions & ITextEditor::FindOption::CaseSensitive ? 1.f
																						   : 0.f);
	styleButton (caseSensitiveButton);
	icfr.offset (margin + buttonWidth * 2, 0);
	auto wholeWordButton = new CTextButton (icfr, controller, FindPanelController::WholeWords);
	wholeWordButton->setTitle ("|w|");
	wholeWordButton->setStyle (CTextButton::Style::kOnOffStyle);
	wholeWordButton->setValue (md.findOptions & ITextEditor::FindOption::WholeWords ? 1.f : 0.f);
	styleButton (wholeWordButton);

	auto tbfr = icfr;
	tbfr.left = tbfr.right + margin;
	tbfr.right = tbfr.left + buttonWidth;
	auto prevButton = new CTextButton (tbfr, controller, FindPanelController::FindPrevious);
	prevButton->setTitle ("<");
	styleButton (prevButton);
	tbfr.offset (margin + buttonWidth, 0);
	auto nextButton = new CTextButton (tbfr, controller, FindPanelController::FindNext);
	nextButton->setTitle (">");
	styleButton (nextButton);

	editfield->registerViewListener (controller);
	editfield->registerViewEventListener (controller);
	closeBox->registerViewListener (controller);
	caseSensitiveButton->registerViewListener (controller);
	wholeWordButton->registerViewListener (controller);
	findPanel->registerViewListener (controller);
	wholeWordButton->registerViewEventListener (controller);
	findPanel->registerViewEventListener (controller);

	controller->editfield = editfield;
	controller->closeBox = closeBox;
	controller->caseSensitiveButton = caseSensitiveButton;
	controller->wholeWordButton = wholeWordButton;

	findPanel->addView (closeBox);
	findPanel->addView (editfield);
	findPanel->addView (caseSensitiveButton);
	findPanel->addView (wholeWordButton);
	findPanel->addView (prevButton);
	findPanel->addView (nextButton);
	findPanel->setInitialFocusView (editfield);

	controller->setFindString (md.findString);
	md.findPanelController = controller;

	return findPanel;
}

//------------------------------------------------------------------------
bool TextEditorView::showFindPanel () const
{
	if (!md.scrollView)
		return false;
	if (auto view = md.scrollView->getEdgeView (CScrollView::Edge::Top))
	{
		if (auto frame = getFrame ())
		{
			if (auto container = view->asViewContainer ())
				container->advanceNextFocusView (nullptr);
			else
				frame->setFocusView (view);
		}
		return true;
	}
	auto margin = 3.;

	auto vcr = md.scrollView->getVisibleClientRect ();
	vcr.originize ();
	vcr.setHeight (md.lineHeight + margin * 2);

	using namespace Animation;
	auto findPanel = FindPanelController::makeFindPanelView (vcr, md, *this, [&] () {
		md.findPanelController = nullptr;
		if (auto panel = md.scrollView->getEdgeView (CScrollView::Edge::Top))
		{
			auto size = panel->getViewSize ();
			size.setHeight (1.);
			if (auto frame = panel->getFrame ())
				frame->setFocusView (nullptr);
			panel->addAnimation (
				"ResizeAnimation", new ViewSizeAnimation (size, false),
				CubicBezierTimingFunction::make (CubicBezierTimingFunction::EasyInOut, 120),
				[&] (auto, auto, auto) {
					md.scrollView->setEdgeView (CScrollView::Edge::Top, nullptr);
					handleCommand (Command::TakeFocus);
				});
		}
	});
	auto initRect = vcr;
	initRect.setHeight (1.);
	findPanel->setViewSize (initRect);
	md.scrollView->setEdgeView (CScrollView::Edge::Top, findPanel);

	if (auto frame = getFrame ())
		frame->setFocusView (nullptr);
	findPanel->addAnimation (
		"ResizeAnimation", new ViewSizeAnimation (vcr, false),
		CubicBezierTimingFunction::make (CubicBezierTimingFunction::EasyInOut, 120),
		[panel = shared (findPanel)] (auto, auto, auto) {
			if (panel->isAttached ())
				panel->advanceNextFocusView (nullptr);
		});
	return true;
}

//------------------------------------------------------------------------
void TextEditorView::setFindOptions (FindOptions opt) const
{
	md.findOptions = opt;
	if (md.findPanelController)
	{
		md.findPanelController->setFindOptions (md.findOptions);
		md.editState.select_end = md.editState.select_start;
		onSelectionChanged (makeRange (md.editState));
		doFind ();
	}
}

//------------------------------------------------------------------------
void TextEditorView::setFindString (std::string_view utf8Text) const
{
	setFindString (convert (utf8Text.data (), utf8Text.size ()));
}

//------------------------------------------------------------------------
void TextEditorView::setFindString (String&& text) const
{
	md.findString = std::move (text);
	if (auto frame = getFrame ())
	{
		if (frame->getFocusView () != this)
		{
			md.editState.select_end = md.editState.select_start;
			onSelectionChanged (makeRange (md.editState));
			doFind ();
		}
		else if (md.findPanelController)
		{
			md.findPanelController->setFindString (md.findString);
		}
	}
}

//------------------------------------------------------------------------
void TextEditorView::checkCurrentUndoGroup (bool force) const
{
	auto currentTime = getPlatformFactory ().getTicks ();
	if (force || md.currentUndoGroup.time < currentTime - 500)
	{
		if (md.currentUndoGroup.record.empty ())
			return;
		md.undoList.emplace_back (std::move (md.currentUndoGroup));
		md.undoPos = md.undoList.end ();
		--md.undoPos;
		md.currentUndoGroup = {};
	}
	md.currentUndoGroup.time = currentTime;
}

//------------------------------------------------------------------------
CharT* TextEditorView::createUndoRecord (size_t pos, size_t insertLen, size_t deleteLen) const
{
	if (md.isInUndoRedo)
		return nullptr;
	checkCurrentUndoGroup (false);
	if (md.undoPos != md.undoList.end ())
	{
		md.undoPos++;
		if (md.undoPos != md.undoList.end ())
			md.undoList.erase (md.undoPos, md.undoList.end ());
	}
	md.currentUndoGroup.record.emplace_back (UndoRecord {});
	auto& record = md.currentUndoGroup.record.back ();
	record.position = pos;
	record.deleted = deleteLen;
	if (insertLen)
	{
		record.characters.resize (insertLen);
		return record.characters.data ();
	}
	return nullptr;
}

//------------------------------------------------------------------------
template<bool iterateForward>
void TextEditorView::doUndoRedo () const
{
	auto begin = [this] () {
		if constexpr (iterateForward)
			return md.undoPos->record.begin ();
		else
			return md.undoPos->record.rbegin ();
	};
	auto end = [this] () {
		if constexpr (iterateForward)
			return md.undoPos->record.end ();
		else
			return md.undoPos->record.rend ();
	};

	md.isInUndoRedo = true;
	for (auto it = begin (); it != end (); ++it)
	{
		md.editState.cursor = md.editState.select_start = static_cast<int> (it->position);
		if (it->deleted > 0)
		{
			md.editState.select_end = md.editState.select_start + static_cast<int> (it->deleted);
			it->characters = md.model.text.substr (
				md.editState.select_start, md.editState.select_end - md.editState.select_start);
			it->deleted = 0;
			callSTB ([&] () { stb_textedit_cut (this, &md.editState); });
		}
		else if (it->characters.empty () == false)
		{
			md.editState.select_end =
				md.editState.select_start + static_cast<int> (it->characters.size ());
			callSTB ([&] () {
				stb_textedit_paste (this, &md.editState, it->characters.data (),
									static_cast<int> (it->characters.size ()));
			});
			it->deleted = it->characters.size ();
			it->characters.clear ();
		}
	}
	md.isInUndoRedo = false;
}

//------------------------------------------------------------------------
void TextEditorView::doUndo () const
{
	checkCurrentUndoGroup (true);
	if (md.undoPos == md.undoList.end () || md.undoPos == md.undoList.begin ())
		return;
	doUndoRedo<false> ();
	--md.undoPos;
}

//------------------------------------------------------------------------
void TextEditorView::doRedo () const
{
	checkCurrentUndoGroup (true);
	if (std::distance (md.undoPos, md.undoList.end ()) <= 1)
		return;
	++md.undoPos;
	if (md.undoPos == md.undoList.end ())
		return;
	doUndoRedo<true> ();
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
		if (selectedLines.length == 0)
			selectedLines.length = 1;
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
	if (!controller)
		return nullptr;
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
