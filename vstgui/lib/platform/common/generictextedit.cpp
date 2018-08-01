// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "generictextedit.h"
#include "../iplatformfont.h"
#include "../iplatformframe.h"
#include "../../controls/ctextlabel.h"
#include "../../cframe.h"
#include "../../cvstguitimer.h"
#include <numeric>

//-----------------------------------------------------------------------------
namespace VSTGUI {

#define STB_TEXTEDIT_CHARTYPE char
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

	int32_t onKeyDown (const VstKeyCode& code, CFrame* frame) override;
	int32_t onKeyUp (const VstKeyCode& code, CFrame* frame) override;

	void onMouseEntered (CView* view, CFrame* frame) override;
	void onMouseExited (CView* view, CFrame* frame) override;
	CMouseEventResult onMouseMoved (CFrame* frame,
									const CPoint& where,
									const CButtonState& buttons) override;
	CMouseEventResult onMouseDown (CFrame* frame,
								   const CPoint& where,
								   const CButtonState& buttons) override;

	bool attached (CView* parent) override;
	bool removed (CView* parent) override;
	void drawStyleChanged () override;

	void selectAll ();

	static int deleteChars (STBTextEditView* self, size_t pos, size_t num);
	static int insertChars (STBTextEditView* self, size_t pos, const char* text, size_t num);
	static void layout (StbTexteditRow* row, STBTextEditView* self, int start_i);
	static float getCharWidth (STBTextEditView* self, int n, int i);

private:
	using CTextLabel::onKeyDown;
	using CTextLabel::onKeyUp;
	using CTextLabel::onMouseEntered;
	using CTextLabel::onMouseExited;
	using CTextLabel::onMouseMoved;
	using CTextLabel::onMouseDown;

	template<typename Proc>
	bool callSTB (Proc proc);
	void onStateChanged ();
	void fillCharWidthCache ();
	void calcCursorSizes ();
	CCoord getCharWidth (char c, char pc) const;

	static constexpr auto BitRecursiveKeyGuard = 1 << 0;
	static constexpr auto BitBlinkToggle = 1 << 1;
	static constexpr auto BitCursorIsSet = 1 << 2;
	static constexpr auto BitCursorSizesValid = 1 << 3;

	bool isRecursiveKeyEventGuard () const { return hasBit (flags, BitRecursiveKeyGuard); }
	bool isBlinkToggle () const { return hasBit (flags, BitBlinkToggle); }
	bool isCursorSet () const { return hasBit (flags, BitCursorIsSet); }
	bool cursorSizesValid () const { return hasBit (flags, BitCursorSizesValid); }

	void setRecursiveKeyEventGuard (bool state) { setBit (flags, BitRecursiveKeyGuard, state); }
	void setBlinkToggle (bool state) { setBit (flags, BitBlinkToggle, state); }
	void setCursorIsSet (bool state) { setBit (flags, BitCursorIsSet, state); }
	void setCursorSizesValid (bool state) { setBit (flags, BitCursorSizesValid, state); }

	SharedPointer<CVSTGUITimer> blinkTimer;
	IPlatformTextEditCallback* callback;
	STB_TexteditState editState;
	std::vector<CCoord> charWidthCache;
	CColor selectionColor{kBlueCColor};
	CCoord cursorOffset {0.};
	CCoord cursorHeight {0.};
	uint32_t flags{0};
};

//-----------------------------------------------------------------------------
#define VIRTUAL_KEY_BIT 0x80000000
#define STB_TEXTEDIT_K_SHIFT 0x40000000
#define STB_TEXTEDIT_K_CONTROL 0x20000000

#define STB_TEXTEDIT_STRINGLEN(tc) (static_cast<int> (tc->getText ().length ()))
#define STB_TEXTEDIT_LAYOUTROW STBTextEditView::layout
#define STB_TEXTEDIT_GETWIDTH(tc, n, i) STBTextEditView::getCharWidth (tc, n, i)
#define STB_TEXTEDIT_KEYTOTEXT(key) ((key & VIRTUAL_KEY_BIT) ? 0 : ((key & STB_TEXTEDIT_K_CONTROL) ? 0 : (key & (~0xF0000000))));
#define STB_TEXTEDIT_GETCHAR(tc, i) ((tc)->getText ().getString ()[i])
#define STB_TEXTEDIT_NEWLINE '\n'
#define STB_TEXTEDIT_IS_SPACE(ch) isSpace (ch)
#define STB_TEXTEDIT_DELETECHARS STBTextEditView::deleteChars
#define STB_TEXTEDIT_INSERTCHARS STBTextEditView::insertChars

#define STB_TEXTEDIT_K_LEFT (VIRTUAL_KEY_BIT | VKEY_LEFT)
#define STB_TEXTEDIT_K_RIGHT (VIRTUAL_KEY_BIT | VKEY_RIGHT)
#define STB_TEXTEDIT_K_UP (VIRTUAL_KEY_BIT | VKEY_UP)
#define STB_TEXTEDIT_K_DOWN (VIRTUAL_KEY_BIT | VKEY_DOWN)
#define STB_TEXTEDIT_K_LINESTART (VIRTUAL_KEY_BIT | VKEY_HOME)
#define STB_TEXTEDIT_K_LINEEND (VIRTUAL_KEY_BIT | VKEY_END)
#define STB_TEXTEDIT_K_TEXTSTART (STB_TEXTEDIT_K_LINESTART | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_TEXTEND (STB_TEXTEDIT_K_LINEEND | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_DELETE (VIRTUAL_KEY_BIT | VKEY_DELETE)
#define STB_TEXTEDIT_K_BACKSPACE (VIRTUAL_KEY_BIT | VKEY_BACK)
#define STB_TEXTEDIT_K_UNDO (STB_TEXTEDIT_K_CONTROL | 'z')
#define STB_TEXTEDIT_K_REDO (STB_TEXTEDIT_K_CONTROL | 'y')
#define STB_TEXTEDIT_K_INSERT (VIRTUAL_KEY_BIT | VKEY_INSERT)
#define STB_TEXTEDIT_K_WORDLEFT (STB_TEXTEDIT_K_LEFT | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_WORDRIGHT (STB_TEXTEDIT_K_RIGHT | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_PGUP (VIRTUAL_KEY_BIT | VKEY_PAGEUP)
#define STB_TEXTEDIT_K_PGDOWN (VIRTUAL_KEY_BIT | VKEY_PAGEDOWN)

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
	assert (view);
	view->getFrame ()->addView (impl->view);

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
	auto r = textEdit->platformGetSize ();
	r = impl->view->getParentView ()->translateToLocal (r);
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
int32_t STBTextEditView::onKeyDown (const VstKeyCode& code, CFrame* frame)
{
	if (isRecursiveKeyEventGuard ())
		return -1;
	auto selfGuard = SharedPointer<CBaseObject> (this);
	BitScopeToggleT<uint32_t, uint32_t> br (flags, BitRecursiveKeyGuard);
	if (callback->platformOnKeyDown (code))
		return 1;

	if (code.character == 0 && code.virt == 0)
		return -1;

	if (code.character == 'a' && code.modifier == MODIFIER_CONTROL)
	{
		selectAll ();
		return 1;
	}

	auto key = code.character;
	if (key)
	{
		if (auto text = getFrame ()->getPlatformFrame ()->convertCurrentKeyEventToText ())
		{
			if (text->length () != 1) // we can only handle one-byte UTF-8 characters currently
				return -1;
			key = text->getString ()[0];
		}
	}
	if (code.virt)
	{
		switch (code.virt)
		{
			case VKEY_SPACE:
			{
				key = 0x20;
				break;
			}
			case VKEY_TAB:
			{
				return -1;
			}
			default:
			{
				key = code.virt | VIRTUAL_KEY_BIT;
				break;
			}
		}
	}
	if (code.modifier & MODIFIER_CONTROL)
		key |= STB_TEXTEDIT_K_CONTROL;
	if (code.modifier & MODIFIER_SHIFT)
		key |= STB_TEXTEDIT_K_SHIFT;
	return callSTB ([&]() { stb_textedit_key (this, &editState, key); }) ? 1 : -1;
}

//-----------------------------------------------------------------------------
int32_t STBTextEditView::onKeyUp (const VstKeyCode& code, CFrame* frame)
{
	return -1;
}

//-----------------------------------------------------------------------------
CMouseEventResult STBTextEditView::onMouseDown (CFrame* frame,
												const CPoint& where,
												const CButtonState& buttons)
{
	if (buttons.isLeftButton () && hitTest (where, buttons))
	{
		CPoint where2 (where);
		where2.x -= getViewSize ().left;
		where2.y -= getViewSize ().top;
		callSTB ([&] () {
			stb_textedit_click (this, &editState, static_cast<float> (where2.x),
			                    static_cast<float> (where2.y));
		});
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult STBTextEditView::onMouseMoved (CFrame* frame,
												 const CPoint& where,
												 const CButtonState& buttons)
{
	if (buttons.isLeftButton () && hitTest (where, buttons))
	{
		CPoint where2 (where);
		where2.x -= getViewSize ().left;
		where2.y -= getViewSize ().top;
		callSTB ([&] () {
			stb_textedit_drag (this, &editState, static_cast<float> (where2.x),
			                   static_cast<float> (where2.y));
		});
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

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
	if (auto frame = parent->getFrame ())
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
}

//-----------------------------------------------------------------------------
CCoord STBTextEditView::getCharWidth (char c, char pc) const
{
	auto platformFont = getFont ()->getPlatformFont ();
	assert (platformFont);
	auto fontPainter = platformFont->getPainter ();
	assert (fontPainter);

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
}

//-----------------------------------------------------------------------------
void STBTextEditView::fillCharWidthCache ()
{
	if (!charWidthCache.empty ())
		return;
	auto num = getText ().length ();
	charWidthCache.resize (num);
	const auto& str = getText ().getString ();
	for (auto i = 0u; i < num; ++i)
	{
		charWidthCache[i] = getCharWidth (str[i], i == 0 ? 0 : str[i - 1]);
	}
}

//-----------------------------------------------------------------------------
void STBTextEditView::calcCursorSizes ()
{
	if (cursorSizesValid ())
		return;
	
	auto platformFont = getFont ()->getPlatformFont ();
	assert (platformFont);
	auto fontPainter = platformFont->getPainter ();
	assert (fontPainter);

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
int STBTextEditView::deleteChars (STBTextEditView* self, size_t pos, size_t num)
{
	auto str = self->text.getString ();
	str.erase (pos, num);
	self->setText (str.data ());
	return true; // success
}

//-----------------------------------------------------------------------------
int STBTextEditView::insertChars (STBTextEditView* self, size_t pos, const char* text, size_t num)
{
	auto str = self->text.getString ();
	str.insert (pos, text, num);
	self->setText (str.data ());
	return true; // success
}

//-----------------------------------------------------------------------------
void STBTextEditView::layout (StbTexteditRow* row, STBTextEditView* self, int start_i)
{
	assert (start_i == 0);

	self->fillCharWidthCache ();
	auto textWidth =
		static_cast<float> (std::accumulate (self->charWidthCache.begin (), self->charWidthCache.end (), 0.));

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
			row->x0 = static_cast<float> ((self->getViewSize ().getWidth () / 2.) - (textWidth / 2.));
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
