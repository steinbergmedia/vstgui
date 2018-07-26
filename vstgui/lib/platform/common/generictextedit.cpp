// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "generictextedit.h"
#include "../iplatformfont.h"
#include "../../controls/ctextlabel.h"
#include "../../cframe.h"
#include <numeric>

//-----------------------------------------------------------------------------
namespace VSTGUI {

#define STB_TEXTEDIT_CHARTYPE char
#define STB_TEXTEDIT_POSITIONTYPE size_t
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

	void selectAll ();

	static int deleteChars (STBTextEditView* self, size_t pos, size_t num);
	static int insertChars (STBTextEditView* self, size_t pos, const char* text, size_t num);
	static void layout (StbTexteditRow* row, STBTextEditView* self, int start_i);
	static float getCharWidth (STBTextEditView* self, int n, int i);

private:
	void onSelectionChanged ();
	void fillCharWidthCache ();
	CCoord getCharWidth (char c, char pc) const;

	IPlatformTextEditCallback* callback;
	STB_TexteditState editState;
	CColor selectionColor{kBlueCColor};
	bool recursiveKeyEventGuard{false};
	std::vector<CCoord> charWidthCache;
};

//-----------------------------------------------------------------------------
#define KEYDOWN_BIT 0x80000000
#define VIRTUAL_KEY_BIT 0x10000000

#define STB_TEXTEDIT_STRINGLEN(tc) ((tc)->getText ().length ())
#define STB_TEXTEDIT_LAYOUTROW STBTextEditView::layout
#define STB_TEXTEDIT_GETWIDTH(tc, n, i) STBTextEditView::getCharWidth (tc, n, i)
#define STB_TEXTEDIT_KEYTOTEXT(key) (((key)&KEYDOWN_BIT) ? 0 : (key))
#define STB_TEXTEDIT_GETCHAR(tc, i) ((tc)->getText ().getString ()[i])
#define STB_TEXTEDIT_NEWLINE '\n'
#define STB_TEXTEDIT_IS_SPACE(ch) isSpace (ch)
#define STB_TEXTEDIT_DELETECHARS STBTextEditView::deleteChars
#define STB_TEXTEDIT_INSERTCHARS STBTextEditView::insertChars

#define STB_TEXTEDIT_K_SHIFT 0x40000000
#define STB_TEXTEDIT_K_CONTROL 0x20000000
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
#define STB_TEXTEDIT_K_UNDO (KEYDOWN_BIT | STB_TEXTEDIT_K_CONTROL | 'z')
#define STB_TEXTEDIT_K_REDO (KEYDOWN_BIT | STB_TEXTEDIT_K_CONTROL | 'y')
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
	impl->view->setFont (callback->platformGetFont ());
	impl->view->setFontColor (callback->platformGetFontColor ());
	impl->view->setTextInset (callback->platformGetTextInset ());
	impl->view->setText (callback->platformGetText ());
	impl->view->selectAll ();
	updateSize ();

	auto view = dynamic_cast<CView*> (callback);
	assert (view);
	view->getFrame ()->addView (impl->view);
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
	impl->view->setViewSize (textEdit->platformGetSize ());
	impl->view->setMouseableArea (textEdit->platformGetSize ());
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
int32_t STBTextEditView::onKeyDown (const VstKeyCode& code, CFrame* frame)
{
	if (recursiveKeyEventGuard)
		return -1;
	recursiveKeyEventGuard = true;
	if (callback->platformOnKeyDown (code))
	{
		recursiveKeyEventGuard = false;
		return 1;
	}
	recursiveKeyEventGuard = false;

	if (code.character == 0 && code.virt == 0)
		return -1;

	auto key = code.character;
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
	auto oldState = editState;
	stb_textedit_key (this, &editState, key);
	if (memcmp (&oldState, &editState, sizeof (STB_TexteditState)) != 0)
		invalid ();
	return 1;
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
		auto oldEditState = editState;
		stb_textedit_click (this, &editState, where2.x, where2.y);
		if (oldEditState.select_start != editState.select_start ||
			oldEditState.select_end != editState.select_end)
		{
			onSelectionChanged ();
		}
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
		auto oldEditState = editState;
		stb_textedit_drag (this, &editState, where2.x, where2.y);
		if (oldEditState.select_start != editState.select_start ||
			oldEditState.select_end != editState.select_end)
		{
			onSelectionChanged ();
		}
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
void STBTextEditView::onMouseEntered (CView* view, CFrame* frame)
{
	if (view == this)
		getFrame ()->setCursor (kCursorIBeam);
}

//-----------------------------------------------------------------------------
void STBTextEditView::onMouseExited (CView* view, CFrame* frame)
{
	if (view == this)
		getFrame ()->setCursor (kCursorDefault);
}

//-----------------------------------------------------------------------------
bool STBTextEditView::attached (CView* parent)
{
	if (auto frame = parent->getFrame ())
	{
		frame->registerMouseObserver (this);
		frame->registerKeyboardHook (this);
	}
	return CTextLabel::attached (parent);
}

//-----------------------------------------------------------------------------
bool STBTextEditView::removed (CView* parent)
{
	if (auto frame = parent->getFrame ())
	{
		frame->unregisterMouseObserver (this);
		frame->unregisterKeyboardHook (this);
	}
	return CTextLabel::removed (parent);
}

//-----------------------------------------------------------------------------
void STBTextEditView::selectAll ()
{
	editState.select_start = 0;
	editState.select_end = getText ().length ();
	invalid ();
}

//-----------------------------------------------------------------------------
void STBTextEditView::onSelectionChanged ()
{
	invalid ();
}

//-----------------------------------------------------------------------------
void STBTextEditView::setText (const UTF8String& txt)
{
	charWidthCache.clear ();
	CTextLabel::setText (txt);
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
	for (auto i = 0; i < num; ++i)
	{
		charWidthCache[i] = getCharWidth (str[i], i == 0 ? 0 : str[i - 1]);
	}
}

//-----------------------------------------------------------------------------
void STBTextEditView::draw (CDrawContext* context)
{
	fillCharWidthCache ();

	drawBack (context, nullptr);
	drawPlatformText (context, getText ().getPlatformString ());

	if (editState.select_start != editState.select_end)
		return;

	// draw cursor
	StbTexteditRow row{};
	layout (&row, this, 0);

	context->setFillColor (getFontColor ());
	context->setDrawMode (kAntiAliasing);
	CRect r = getViewSize ();
	r.offset (row.x0, 0);
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
		selection.offset (row.x0, 0);
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
	CCoord textWidth =
		std::accumulate (self->charWidthCache.begin (), self->charWidthCache.end (), 0.);

	int remaining_chars = self->getText ().length ();
	row->num_chars = remaining_chars;
	row->baseline_y_delta = 1.25;
	row->ymin = 0.f;
	row->ymax = self->getFont ()->getSize ();
	switch (self->getHoriAlign ())
	{
		case kLeftText:
		{
			row->x0 = self->getTextInset ().x;
			row->x1 = row->x0 + textWidth;
			break;
		}
		case kCenterText:
		{
			row->x0 = (self->getViewSize ().getWidth () / 2.) - (textWidth / 2.);
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
	return self->charWidthCache[i];
}

//-----------------------------------------------------------------------------
} // VSTGUI
