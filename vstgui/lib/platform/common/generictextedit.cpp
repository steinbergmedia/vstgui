// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#if defined(_WIN32)
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#endif

#include "generictextedit.h"
#include "../iplatformfont.h"
#include "../iplatformframe.h"
#include "../../controls/ctextlabel.h"
#include "../../cframe.h"
#include "../../cvstguitimer.h"
#include "../../cdropsource.h"
#include "../../events.h"

#include <numeric>
#include <string>
#include <codecvt>
#include <locale>

//-----------------------------------------------------------------------------
namespace VSTGUI {

#define VSTGUI_STB_TEXTEDIT_USE_UNICODE 1

#if VSTGUI_STB_TEXTEDIT_USE_UNICODE
#if defined(_MSC_VER) && _MSC_VER >= 1900 && _MSC_VER < 1920
using STB_CharT = wchar_t;
#else
using STB_CharT = char16_t;
#endif
using StringConvert = std::wstring_convert<std::codecvt_utf8_utf16<STB_CharT>, STB_CharT>;
#else
using STB_CharT = char;
#endif
#define STB_TEXTEDIT_CHARTYPE STB_CharT
#define STB_TEXTEDIT_POSITIONTYPE int
#define STB_TEXTEDIT_STRING STBTextEditView
#define STB_TEXTEDIT_KEYTYPE uint32_t

#include "stb_textedit.h"

//-----------------------------------------------------------------------------
class STBTextEditView
	: public CTextLabel
	, public IKeyboardHook
	, public IMouseObserver
{
public:
	STBTextEditView (IPlatformTextEditCallback* callback);

	void draw (CDrawContext* pContext) override;
	void drawBack (CDrawContext* pContext, CBitmap* newBack = nullptr) override;
	void setText (const UTF8String& txt) override;

	void onKeyboardEvent (KeyboardEvent& event, CFrame* frame) override;

	void onMouseEntered (CView* view, CFrame* frame) override;
	void onMouseExited (CView* view, CFrame* frame) override;
	void onMouseEvent (MouseEvent& event, CFrame* frame) override;

	bool attached (CView* parent) override;
	bool removed (CView* parent) override;
	void drawStyleChanged () override;

	void selectAll ();
	bool doCut ();
	bool doCopy ();
	bool doPaste ();

	static int deleteChars (STBTextEditView* self, size_t pos, size_t num);
	static int insertChars (STBTextEditView* self, size_t pos, const STB_CharT* text, size_t num);
	static void layout (StbTexteditRow* row, STBTextEditView* self, int start_i);
	static float getCharWidth (STBTextEditView* self, int n, int i);
	static STB_CharT getChar (STBTextEditView* self, int pos);
	static int getLength (STBTextEditView* self);

private:
	using CTextLabel::onKeyboardEvent;
	using CTextLabel::onMouseEntered;
	using CTextLabel::onMouseExited;
	using CTextLabel::onMouseMoved;
	using CTextLabel::onMouseDown;

	template<typename Proc>
	bool callSTB (Proc proc);
	void onStateChanged ();
	void onTextChange ();
	void fillCharWidthCache ();
	void calcCursorSizes ();
	CCoord getCharWidth (STB_CharT c, STB_CharT pc) const;

	static constexpr auto BitRecursiveKeyGuard = 1 << 0;
	static constexpr auto BitBlinkToggle = 1 << 1;
	static constexpr auto BitCursorIsSet = 1 << 2;
	static constexpr auto BitCursorSizesValid = 1 << 3;
	static constexpr auto BitNotifyTextChange = 1 << 4;
	static constexpr auto BitMouseDownHandling = 1 << 5;

	bool isRecursiveKeyEventGuard () const { return hasBit (flags, BitRecursiveKeyGuard); }
	bool isBlinkToggle () const { return hasBit (flags, BitBlinkToggle); }
	bool isCursorSet () const { return hasBit (flags, BitCursorIsSet); }
	bool cursorSizesValid () const { return hasBit (flags, BitCursorSizesValid); }
	bool notifyTextChange () const { return hasBit (flags, BitNotifyTextChange); }
	bool mouseDownHandling () const { return hasBit (flags, BitMouseDownHandling); }

	void setRecursiveKeyEventGuard (bool state) { setBit (flags, BitRecursiveKeyGuard, state); }
	void setBlinkToggle (bool state) { setBit (flags, BitBlinkToggle, state); }
	void setCursorIsSet (bool state) { setBit (flags, BitCursorIsSet, state); }
	void setCursorSizesValid (bool state) { setBit (flags, BitCursorSizesValid, state); }
	void setNotifyTextChange (bool state) { setBit (flags, BitNotifyTextChange, state); }
	void setMouseDownHandling (bool state) { setBit (flags, BitMouseDownHandling, state); }

	SharedPointer<CVSTGUITimer> blinkTimer;
	IPlatformTextEditCallback* callback;
	STB_TexteditState editState;
	std::vector<CCoord> charWidthCache;
	CColor selectionColor{kBlueCColor};
	CCoord cursorOffset{0.};
	CCoord cursorHeight{0.};
	uint32_t flags{0};
#if VSTGUI_STB_TEXTEDIT_USE_UNICODE
	std::u16string uString;
#endif
};

//-----------------------------------------------------------------------------
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
#	define STB_TEXTEDIT_K_LINESTART (STB_TEXTEDIT_K_CONTROL | STB_TEXTEDIT_K_LEFT)
#	define STB_TEXTEDIT_K_LINEEND (STB_TEXTEDIT_K_CONTROL | STB_TEXTEDIT_K_RIGHT)
#	define STB_TEXTEDIT_K_WORDLEFT (STB_TEXTEDIT_K_ALT | STB_TEXTEDIT_K_LEFT)
#	define STB_TEXTEDIT_K_WORDRIGHT (STB_TEXTEDIT_K_ALT | STB_TEXTEDIT_K_RIGHT)
#	define STB_TEXTEDIT_K_TEXTSTART (STB_TEXTEDIT_K_CONTROL | STB_TEXTEDIT_K_UP)
#	define STB_TEXTEDIT_K_TEXTEND (STB_TEXTEDIT_K_CONTROL | STB_TEXTEDIT_K_DOWN)
#else
#	define STB_TEXTEDIT_K_LINESTART (VIRTUAL_KEY_BIT | VKEY_HOME)
#	define STB_TEXTEDIT_K_LINEEND (VIRTUAL_KEY_BIT | VKEY_END)
#	define STB_TEXTEDIT_K_WORDLEFT (STB_TEXTEDIT_K_LEFT | STB_TEXTEDIT_K_CONTROL)
#	define STB_TEXTEDIT_K_WORDRIGHT (STB_TEXTEDIT_K_RIGHT | STB_TEXTEDIT_K_CONTROL)
#	define STB_TEXTEDIT_K_TEXTSTART (STB_TEXTEDIT_K_LINESTART | STB_TEXTEDIT_K_CONTROL)
#	define STB_TEXTEDIT_K_TEXTEND (STB_TEXTEDIT_K_LINEEND | STB_TEXTEDIT_K_CONTROL)
#endif
#define STB_TEXTEDIT_K_DELETE (VIRTUAL_KEY_BIT | VKEY_DELETE)
#define STB_TEXTEDIT_K_BACKSPACE (VIRTUAL_KEY_BIT | VKEY_BACK)
#define STB_TEXTEDIT_K_UNDO (STB_TEXTEDIT_K_CONTROL | 'z')
#define STB_TEXTEDIT_K_REDO (STB_TEXTEDIT_K_CONTROL | STB_TEXTEDIT_K_SHIFT | 'z')
#define STB_TEXTEDIT_K_INSERT (VIRTUAL_KEY_BIT | VKEY_INSERT)
#define STB_TEXTEDIT_K_PGUP (VIRTUAL_KEY_BIT | VKEY_PAGEUP)
#define STB_TEXTEDIT_K_PGDOWN (VIRTUAL_KEY_BIT | VKEY_PAGEDOWN)
// functions
#define STB_TEXTEDIT_STRINGLEN(tc) STBTextEditView::getLength (tc)
#define STB_TEXTEDIT_LAYOUTROW STBTextEditView::layout
#define STB_TEXTEDIT_GETWIDTH(tc, n, i) STBTextEditView::getCharWidth (tc, n, i)
#define STB_TEXTEDIT_KEYTOTEXT(key)                                                                \
	((key & VIRTUAL_KEY_BIT) ? 0 : ((key & STB_TEXTEDIT_K_CONTROL) ? 0 : (key & (~0xF0000000))));
#define STB_TEXTEDIT_GETCHAR(tc, i) STBTextEditView::getChar (tc, i)
#define STB_TEXTEDIT_NEWLINE '\n'
#define STB_TEXTEDIT_IS_SPACE(ch) isSpace (ch)
#define STB_TEXTEDIT_DELETECHARS STBTextEditView::deleteChars
#define STB_TEXTEDIT_INSERTCHARS STBTextEditView::insertChars

#define STB_TEXTEDIT_IMPLEMENTATION
#include "stb_textedit.h"

//-----------------------------------------------------------------------------
struct GenericTextEdit::Impl
{
	STBTextEditView* view;
};

//-----------------------------------------------------------------------------
GenericTextEdit::GenericTextEdit (IPlatformTextEditCallback* callback)
	: IPlatformTextEdit (callback)
{
	impl = std::unique_ptr<Impl> (new Impl);
	impl->view = new STBTextEditView (callback);
	auto view = dynamic_cast<CView*> (callback);
	vstgui_assert (view);
	view->getParentView ()->asViewContainer ()->addView (impl->view);

	auto font = shared (callback->platformGetFont ());
	auto fontSize = font->getSize () / impl->view->getGlobalTransform ().m11;
	if (fontSize != font->getSize ())
	{
		font = makeOwned<CFontDesc> (*font);
		font->setSize (fontSize);
	}
	impl->view->setFont (font);
	impl->view->setFontColor (callback->platformGetFontColor ());
	impl->view->setTextInset (callback->platformGetTextInset ());
	impl->view->setHoriAlign (callback->platformGetHoriTxtAlign ());
	impl->view->setText (callback->platformGetText ());
	impl->view->selectAll ();

	updateSize ();
}

//-----------------------------------------------------------------------------
GenericTextEdit::~GenericTextEdit () noexcept
{
	if (impl->view->isAttached ())
		impl->view->getParentView ()->asViewContainer ()->removeView (impl->view);
	else
		impl->view->forget ();
}

//-----------------------------------------------------------------------------
UTF8String GenericTextEdit::getText ()
{
	return impl->view->getText ();
}

//-----------------------------------------------------------------------------
bool GenericTextEdit::setText (const UTF8String& text)
{
	impl->view->setText (text);
	return true;
}

//-----------------------------------------------------------------------------
bool GenericTextEdit::updateSize ()
{
	auto r = textEdit->platformGetVisibleSize ();
	r = impl->view->translateToLocal (r);
	impl->view->setViewSize (r);
	impl->view->setMouseableArea (r);
	return true;
}

//-----------------------------------------------------------------------------
STBTextEditView::STBTextEditView (IPlatformTextEditCallback* callback)
	: CTextLabel ({}), callback (callback)
{
	stb_textedit_initialize_state (&editState, true);
	setTransparency (true);
}

//-----------------------------------------------------------------------------
template<typename Proc>
bool STBTextEditView::callSTB (Proc proc)
{
	auto oldState = editState;
	proc ();
	if (memcmp (&oldState, &editState, sizeof (STB_TexteditState)) != 0)
	{
		onStateChanged ();
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void STBTextEditView::onKeyboardEvent (KeyboardEvent& event, CFrame* frame)
{
	if (event.type == EventType::KeyUp)
		return;

	if (isRecursiveKeyEventGuard ())
		return;
	auto selfGuard = SharedPointer<CBaseObject> (this);
	BitScopeToggleT<uint32_t, uint32_t> br (flags, BitRecursiveKeyGuard);
	callback->platformOnKeyboardEvent (event);
	if (event.consumed)
		return;

	if (event.character == 0 && event.virt == VirtualKey::None)
		return;

	if (event.modifiers.is (ModifierKey::Control))
	{
		switch (event.character)
		{
			case 'a':
			{
				selectAll ();
				event.consumed = true;
				return;
			}
			case 'x':
			{
				if (doCut ())
					event.consumed = true;
				return;
			}
			case 'c':
			{
				if (doCopy ())
					event.consumed = true;
				return;
			}
			case 'v':
			{
				if (doPaste ())
					event.consumed = true;
				return;
			}
		}
	}

	auto key = event.character;
	if (key)
	{
		if (auto text = getFrame ()->getPlatformFrame ()->convertCurrentKeyEventToText ())
		{
#if VSTGUI_STB_TEXTEDIT_USE_UNICODE
			auto tmp = StringConvert{}.from_bytes (text->getString ());
			key = tmp[0];
#else
			if (text->length () != 1)
				return;
			key = text->getString ()[0];
#endif
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
	if (callSTB ([&]() { stb_textedit_key (this, &editState, key); }))
		event.consumed = true;
}

//-----------------------------------------------------------------------------
void STBTextEditView::onMouseEvent (MouseEvent& event, CFrame* frame)
{
	if (event.buttonState.isLeft () == false)
		return;

	if (auto parent = getParentView ())
	{
		auto where = event.mousePosition;
		where = translateToLocal (where, true);
		if (mouseDownHandling () || hitTest (where, event))
		{
			where.x -= getViewSize ().left;
			where.y -= getViewSize ().top;
			switch (event.type)
			{
				case EventType::MouseDown:
				{
					setMouseDownHandling (true);
					callSTB ([&] () {
						stb_textedit_click (this, &editState, static_cast<float> (where.x),
											static_cast<float> (where.y));
					});
					event.consumed = true;
					break;
				}
				case EventType::MouseMove:
				{
					if (mouseDownHandling ())
					{
						callSTB ([&] () {
							stb_textedit_drag (this, &editState, static_cast<float> (where.x),
											   static_cast<float> (where.y));
						});
						event.consumed = true;
					}
					break;
				}
				case EventType::MouseUp:
				{
					if (mouseDownHandling ())
					{
						event.consumed = true;
						setMouseDownHandling (false);
					}
					break;
				}
				default: break;
			}
		}
	}
}

#if 0
//-----------------------------------------------------------------------------
CMouseEventResult STBTextEditView::onMouseDown (CFrame* frame,
												const CPoint& _where,
												const CButtonState& buttons)
{
	auto where = _where;
	if (auto parent = getParentView ())
	{
		where = translateToLocal (where, true);
		if (buttons.isLeftButton () && hitTest (where, noEvent ()))
		{
			where.x -= getViewSize ().left;
			where.y -= getViewSize ().top;
			callSTB ([&]() {
				stb_textedit_click (this, &editState, static_cast<float> (where.x),
									static_cast<float> (where.y));
			});
			return kMouseEventHandled;
		}
	}
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult STBTextEditView::onMouseMoved (CFrame* frame,
												 const CPoint& _where,
												 const CButtonState& buttons)
{
	auto where = _where;
	if (auto parent = getParentView ())
	{
		where = translateToLocal (where, true);
		if (buttons.isLeftButton () && hitTest (where, noEvent ()))
		{
			where.x -= getViewSize ().left;
			where.y -= getViewSize ().top;
			callSTB ([&]() {
				stb_textedit_drag (this, &editState, static_cast<float> (where.x),
								   static_cast<float> (where.y));
			});
			return kMouseEventHandled;
		}
	}
	return kMouseEventNotHandled;
}
#endif

//-----------------------------------------------------------------------------
void STBTextEditView::onMouseEntered (CView* view, CFrame* frame)
{
	if (view == this)
	{
		setCursorIsSet (true);
		getFrame ()->setCursor (kCursorIBeam);
	}
}

//-----------------------------------------------------------------------------
void STBTextEditView::onMouseExited (CView* view, CFrame* frame)
{
	if (view == this)
	{
		setCursorIsSet (false);
		getFrame ()->setCursor (kCursorDefault);
	}
}

//-----------------------------------------------------------------------------
bool STBTextEditView::attached (CView* parent)
{
	if (auto frame = parent->getFrame ())
	{
		frame->registerMouseObserver (this);
		frame->registerKeyboardHook (this);
		selectionColor = frame->getFocusColor ();
		drawStyleChanged ();
	}
	return CTextLabel::attached (parent);
}

//-----------------------------------------------------------------------------
bool STBTextEditView::removed (CView* parent)
{
	if (auto frame = getFrame ())
	{
		blinkTimer = nullptr;
		frame->unregisterMouseObserver (this);
		frame->unregisterKeyboardHook (this);
		if (isCursorSet ())
			frame->setCursor (kCursorDefault);
	}
	return CTextLabel::removed (parent);
}

//-----------------------------------------------------------------------------
void STBTextEditView::drawStyleChanged ()
{
	setCursorSizesValid (false);
	charWidthCache.clear ();
	CTextLabel::drawStyleChanged ();
}

//-----------------------------------------------------------------------------
void STBTextEditView::selectAll ()
{
	editState.select_start = 0;
	editState.select_end = static_cast<int> (getText ().length ());
	onStateChanged ();
}

//-----------------------------------------------------------------------------
bool STBTextEditView::doCut ()
{
	if (doCopy ())
	{
		callSTB ([&]() { stb_textedit_cut (this, &editState); });
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool STBTextEditView::doCopy ()
{
	if (editState.select_start == editState.select_end)
		return false;
#if VSTGUI_STB_TEXTEDIT_USE_UNICODE
	auto txt = StringConvert{}.to_bytes (reinterpret_cast<const STB_CharT*> (uString.data () + editState.select_start),
										 reinterpret_cast<const STB_CharT*> (uString.data () + editState.select_end));
	auto dataPackage =
		CDropSource::create (txt.data (), static_cast<uint32_t> (txt.size ()), IDataPackage::kText);
#else
	auto dataPackage =
		CDropSource::create (getText ().data (), getText ().length (), IDataPackage::kText);
#endif
	getFrame ()->setClipboard (dataPackage);
	return true;
}

//-----------------------------------------------------------------------------
bool STBTextEditView::doPaste ()
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
				auto text = reinterpret_cast<const char*> (buffer);
#if VSTGUI_STB_TEXTEDIT_USE_UNICODE
				auto uText = StringConvert {}.from_bytes (text, text + size);
				callSTB ([&] () {
					stb_textedit_paste (this, &editState, uText.data (),
										static_cast<int> (uText.size ()));
				});
#else
				callSTB ([&]() { stb_textedit_paste (this, &editState, text, size); });
#endif
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
void STBTextEditView::onStateChanged ()
{
	setBlinkToggle (true);
	if (isAttached ())
	{
		blinkTimer = makeOwned<CVSTGUITimer> (
			[&](CVSTGUITimer* timer) {
				setBlinkToggle (!isBlinkToggle ());
				if (editState.select_start == editState.select_end)
					invalid ();
			},
			500);
	}
	invalid ();
}

//-----------------------------------------------------------------------------
void STBTextEditView::setText (const UTF8String& txt)
{
	charWidthCache.clear ();
	CTextLabel::setText (txt);
	if (editState.select_start != editState.select_end)
		selectAll ();
#if VSTGUI_STB_TEXTEDIT_USE_UNICODE
	auto tmpStr = StringConvert{}.from_bytes (CTextLabel::getText ().getString ());
	uString = {tmpStr.data (), tmpStr.data () + tmpStr.size ()};
#endif
}

//-----------------------------------------------------------------------------
CCoord STBTextEditView::getCharWidth (STB_CharT c, STB_CharT pc) const
{
	auto platformFont = getFont ()->getPlatformFont ();
	vstgui_assert (platformFont);
	auto fontPainter = platformFont->getPainter ();
	vstgui_assert (fontPainter);

#if VSTGUI_STB_TEXTEDIT_USE_UNICODE
	if (pc)
	{
		UTF8String str (StringConvert{}.to_bytes (pc));
		auto pcWidth = fontPainter->getStringWidth (nullptr, str.getPlatformString (), true);
		str += StringConvert{}.to_bytes (c);
		auto tcWidth = fontPainter->getStringWidth (nullptr, str.getPlatformString (), true);
		return tcWidth - pcWidth;
	}
	UTF8String str (StringConvert{}.to_bytes (c));
	auto width = fontPainter->getStringWidth (nullptr, str.getPlatformString (), true);
	return width / getGlobalTransform ().m11; 
#else
	if (pc)
	{
		UTF8String str (std::string (1, pc));
		auto pcWidth = fontPainter->getStringWidth (nullptr, str.getPlatformString (), true);
		str += std::string (1, c);
		auto tcWidth = fontPainter->getStringWidth (nullptr, str.getPlatformString (), true);
		return tcWidth - pcWidth;
	}

	UTF8String str (std::string (1, c));
	return fontPainter->getStringWidth (nullptr, str.getPlatformString (), true);
#endif
}

//-----------------------------------------------------------------------------
void STBTextEditView::fillCharWidthCache ()
{
	if (!charWidthCache.empty ())
		return;
#if VSTGUI_STB_TEXTEDIT_USE_UNICODE
	auto num = uString.size ();
	charWidthCache.resize (num);
	for (auto i = 0u; i < num; ++i)
		charWidthCache[i] = getCharWidth (uString[i], i == 0 ? 0 : uString[i - 1]);
#else
	auto num = getText ().length ();
	charWidthCache.resize (num);
	const auto& str = getText ().getString ();
	for (auto i = 0u; i < num; ++i)
		charWidthCache[i] = getCharWidth (str[i], i == 0 ? 0 : str[i - 1]);
#endif
}

//-----------------------------------------------------------------------------
void STBTextEditView::calcCursorSizes ()
{
	if (cursorSizesValid ())
		return;

	auto platformFont = getFont ()->getPlatformFont ();
	vstgui_assert (platformFont);

	cursorHeight = platformFont->getAscent () + platformFont->getDescent ();
	auto viewHeight = getViewSize ().getHeight ();
	cursorOffset = (viewHeight / 2. - cursorHeight / 2.);
	setCursorSizesValid (true);
}

//-----------------------------------------------------------------------------
void STBTextEditView::draw (CDrawContext* context)
{
	fillCharWidthCache ();
	calcCursorSizes ();

	drawBack (context, nullptr);
	drawPlatformText (context, getText ().getPlatformString ());

	if (!isBlinkToggle () || editState.select_start != editState.select_end)
		return;

	// draw cursor
	StbTexteditRow row{};
	layout (&row, this, 0);

	context->setFillColor (getFontColor ());
	context->setDrawMode (kAntiAliasing);
	CRect r = getViewSize ();
	r.setHeight (cursorHeight);
	r.offset (row.x0, cursorOffset);
	r.setWidth (1);
	for (auto i = 0; i < editState.cursor; ++i)
		r.offset (charWidthCache[i], 0);
	r.offset (-0.5, 0);
	context->drawRect (r, kDrawFilled);
}

//-----------------------------------------------------------------------------
void STBTextEditView::drawBack (CDrawContext* context, CBitmap* newBack)
{
	CTextLabel::drawBack (context, newBack);

	auto selStart = editState.select_start;
	auto selEnd = editState.select_end;
	if (selStart > selEnd)
		std::swap (selStart, selEnd);

	if (selStart != selEnd)
	{
		StbTexteditRow row{};
		layout (&row, this, 0);

		// draw selection
		CRect selection = getViewSize ();
		selection.setHeight (cursorHeight);
		selection.offset (row.x0, cursorOffset);
		selection.setWidth (0);
		auto index = 0;
		for (; index < selStart; ++index)
			selection.offset (charWidthCache[index], 0);
		for (; index < selEnd; ++index)
			selection.right += charWidthCache[index];
		context->setFillColor (selectionColor);
		context->drawRect (selection, kDrawFilled);
	}
}

//-----------------------------------------------------------------------------
void STBTextEditView::onTextChange ()
{
	if (notifyTextChange ())
		return;
	if (auto frame = getFrame ())
	{
		if (frame->inEventProcessing ())
		{
			setNotifyTextChange (true);
			auto self = shared (this);
			frame->doAfterEventProcessing ([self]() {
				self->setNotifyTextChange (false);
				self->callback->platformTextDidChange ();
			});
		}
	}
}

//-----------------------------------------------------------------------------
int STBTextEditView::deleteChars (STBTextEditView* self, size_t pos, size_t num)
{
#if VSTGUI_STB_TEXTEDIT_USE_UNICODE
	self->uString.erase (pos, num);
	self->setText (StringConvert{}.to_bytes (reinterpret_cast<const STB_CharT*> (self->uString.data ()), reinterpret_cast<const STB_CharT*> (self->uString.data () + self->uString.size ())));
	self->onTextChange ();
	return true;
#else
	auto str = self->text.getString ();
	str.erase (pos, num);
	self->setText (str.data ());
	self->onTextChange ();
	return true; // success
#endif
}

//-----------------------------------------------------------------------------
int STBTextEditView::insertChars (STBTextEditView* self,
								  size_t pos,
								  const STB_CharT* text,
								  size_t num)
{
#if VSTGUI_STB_TEXTEDIT_USE_UNICODE
	self->uString.insert (pos, reinterpret_cast<const char16_t*> (text), num);
	self->setText (StringConvert{}.to_bytes (reinterpret_cast<const STB_CharT*> (self->uString.data ()), reinterpret_cast<const STB_CharT*> (self->uString.data () + self->uString.size ())));
	self->onTextChange ();
	return true;
#else
	auto str = self->text.getString ();
	str.insert (pos, text, num);
	self->setText (str.data ());
	self->onTextChange ();
	return true; // success
#endif
}

//-----------------------------------------------------------------------------
STB_CharT STBTextEditView::getChar (STBTextEditView* self, int pos)
{
#if VSTGUI_STB_TEXTEDIT_USE_UNICODE
	return self->uString[pos];
#else
	return self->getText ().getString ()[pos];
#endif
}

//-----------------------------------------------------------------------------
int STBTextEditView::getLength (STBTextEditView* self)
{
#if VSTGUI_STB_TEXTEDIT_USE_UNICODE
	return static_cast<int> (self->uString.size ());
#else
	return static_cast<int> (self->getText ().length ());
#endif
}

//-----------------------------------------------------------------------------
void STBTextEditView::layout (StbTexteditRow* row, STBTextEditView* self, int start_i)
{
	vstgui_assert (start_i == 0);

	self->fillCharWidthCache ();
	auto textWidth = static_cast<float> (
		std::accumulate (self->charWidthCache.begin (), self->charWidthCache.end (), 0.));

	row->num_chars = static_cast<int> (self->getText ().length ());
	row->baseline_y_delta = 1.25;
	row->ymin = 0.f;
	row->ymax = static_cast<float> (self->getFont ()->getSize ());
	switch (self->getHoriAlign ())
	{
		case kLeftText:
		{
			row->x0 = static_cast<float> (self->getTextInset ().x);
			row->x1 = row->x0 + textWidth;
			break;
		}
		case kCenterText:
		{
			row->x0 =
				static_cast<float> ((self->getViewSize ().getWidth () / 2.) - (textWidth / 2.));
			row->x1 = row->x0 + textWidth;
			break;
		}
		default:
		{
			vstgui_assert (false, "Not Implemented !");
			break;
		}
	}
}

//-----------------------------------------------------------------------------
float STBTextEditView::getCharWidth (STBTextEditView* self, int n, int i)
{
	self->fillCharWidthCache ();
	return static_cast<float> (self->charWidthCache[i]);
}

//-----------------------------------------------------------------------------
} // VSTGUI
