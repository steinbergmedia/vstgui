// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "ctextedit.h"
#include "itexteditlistener.h"
#include "../cframe.h"
#include "../cdrawcontext.h"
#include "../events.h"
#include "../platform/iplatformframe.h"
#include "../platform/iplatformtextedit.h"
#include <cassert>

namespace VSTGUI {

//------------------------------------------------------------------------
// CTextEdit::PlatformCallbackImpl
//------------------------------------------------------------------------
class CTextEdit::PlatformCallbackImpl final : public IPlatformTextEditCallback
{
public:
	explicit PlatformCallbackImpl (CTextEdit& textEdit) : textEdit (textEdit) {}

	CColor platformGetBackColor () const override { return textEdit.getBackColor (); }
	CColor platformGetFontColor () const override { return textEdit.getFontColor (); }
	SPtr<CFontDesc> platformGetFont () const override
	{
		auto font = textEdit.getFont ();
		CCoord fontSize = font->getSize ();
		fontSize *= textEdit.getGlobalTransform ().m11;
		if (fontSize == font->getSize ())
			return font;
		auto platformFont = makeShared<CFontDesc> (*font.get ());
		platformFont->setSize (fontSize);
		return platformFont;
	}
	CHoriTxtAlign platformGetHoriTxtAlign () const override { return textEdit.getHoriAlign (); }
	const UTF8String& platformGetText () const override { return textEdit.text; }
	const UTF8String& platformGetPlaceholderText () const override
	{
		return textEdit.placeholderString;
	}
	CRect platformGetSize () const override { return textEdit.platformGetSize (); }
	CRect platformGetVisibleSize () const override { return textEdit.platformGetVisibleSize (); }
	CPoint platformGetTextInset () const override { return textEdit.getTextInset (); }
	void platformLooseFocus (bool returnPressed) override
	{
		textEdit.platformLooseFocus (returnPressed);
	}
	void platformOnKeyboardEvent (KeyboardEvent& event) override
	{
		textEdit.platformOnKeyboardEvent (event);
	}
	void platformTextDidChange () override { textEdit.platformTextDidChange (); }
	bool platformIsSecureTextEdit () override { return textEdit.getSecureStyle (); }

private:
	CTextEdit& textEdit;
};

CTextEdit::PlatformCallbackImpl* CTextEdit::getPlatformTextEditCallback ()
{
	if (!platformCallback)
		platformCallback = std::make_unique<PlatformCallbackImpl> (*this);
	return platformCallback.get ();
}

//------------------------------------------------------------------------
// CTextEdit
//------------------------------------------------------------------------
/*! @class CTextEdit
Define a rectangle view where a text-value can be displayed and edited with a given font and color.
The user can specify its convert function (from char to char). The text-value is centered in the given rect.
A bitmap can be used as background.
*/
//------------------------------------------------------------------------
/**
 * CTextEdit constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param txt the initial text as c string (UTF-8 encoded)
 * @param background the background bitmap
 * @param style the display style (see CParamDisplay for styles)
 */
//------------------------------------------------------------------------
CTextEdit::CTextEdit (const CRect& size, IControlListener* listener, int32_t tag, UTF8StringPtr txt,
					  const SPtr<CBitmap>& background, const int32_t style)
: CTextLabel (size, txt, background, style)
{
	setListener (listener);
	setTag (tag);

	setWantsFocus (true);
}

//------------------------------------------------------------------------
CTextEdit::~CTextEdit () noexcept { vstgui_assert (platformControl == nullptr); }

//------------------------------------------------------------------------
void CTextEdit::setStringToValueFunction (const StringToValueFunction& stringToValueFunc)
{
	stringToValueFunction = stringToValueFunc;
}

//------------------------------------------------------------------------
void CTextEdit::setStringToValueFunction (StringToValueFunction&& stringToValueFunc)
{
	stringToValueFunction = std::move (stringToValueFunc);
}

//------------------------------------------------------------------------
void CTextEdit::setImmediateTextChange (bool state)
{
	immediateTextChange = state;
}

//------------------------------------------------------------------------
void CTextEdit::setSecureStyle (bool state)
{
	if (secureStyle != state)
	{
		secureStyle = state;
		if (platformControl)
		{
			
		}
	}
}

//------------------------------------------------------------------------
bool CTextEdit::getSecureStyle () const
{
	return secureStyle;
}

//------------------------------------------------------------------------
void CTextEdit::registerTextEditListener (const SPtr<ITextEditListener>& listener)
{
	textEditListeners.add (listener);
}

//------------------------------------------------------------------------
void CTextEdit::unregisterTextEditListener (const SPtr<ITextEditListener>& listener)
{
	textEditListeners.remove (listener);
}

//------------------------------------------------------------------------
bool CTextEdit::setValue (float val)
{
	auto result = CTextLabel::setValue (val);
	bool converted = false;
	std::string string;
	if (valueToStringFunction)
		converted = valueToStringFunction (getValue (), string, *this);
	if (!converted)
	{
		char tmp[255];
		char precisionStr[10];
		snprintf (precisionStr, 10, "%%.%hhuf", valuePrecision);
		snprintf (tmp, 255, precisionStr, getValue ());
		string = tmp;
	}

	if (converted)
	{
		CTextLabel::setText (UTF8String (std::move (string)));
		if (platformControl)
			platformControl->setText (getText ());
	}
	else
		setText (UTF8String (std::move (string)));
	return result;
}

//------------------------------------------------------------------------
void CTextEdit::setText (const UTF8String& txt)
{
	if (stringToValueFunction)
	{
		float val = getValue ();
		if (stringToValueFunction (txt, val, *this))
		{
			CTextLabel::setValue (val);
			if (valueToStringFunction)
			{
				std::string string;
				valueToStringFunction (getValue (), string, *this);
				CTextLabel::setText (UTF8String (std::move (string)));
				if (platformControl)
					platformControl->setText (getText ());
				return;
			}
		}
	}
	CTextLabel::setText (txt);
	if (platformControl)
		platformControl->setText (getText ());
}

//------------------------------------------------------------------------
void CTextEdit::valueChanged ()
{
	if (stringToValueFunction)
		CTextLabel::valueChanged ();
	
	CParamDisplay::valueChanged ();
}

//------------------------------------------------------------------------
void CTextEdit::setPlaceholderString (const UTF8String& str)
{
	placeholderString = str;
}

//------------------------------------------------------------------------
void CTextEdit::draw (CDrawContext& context)
{
	if (platformControl)
	{
		drawBack (context);
		if (!platformControl->drawsPlaceholder () && !placeholderString.empty () &&
		    platformControl->getText ().empty ())
		{
			context.saveGlobalState ();
			context.setGlobalAlpha (context.getGlobalAlpha () * 0.5f);
			drawPlatformText (context, placeholderString);
			context.restoreGlobalState ();
		}
		return;
	}
	drawBack (context);
	if (text.empty ())
	{
		if (!placeholderString.empty ())
		{
			context.saveGlobalState ();
			context.setGlobalAlpha (context.getGlobalAlpha () * 0.5f);
			drawPlatformText (context, placeholderString);
			context.restoreGlobalState ();
		}
	}
	else if (getSecureStyle ())
	{
		constexpr auto bulletCharacter = "\xE2\x80\xA2";
		UTF8String str;
		for (auto i = 0u; i < text.length (); ++i)
			str += bulletCharacter;
		drawPlatformText (context, str);
	}
	else
		CTextLabel::draw (context);
}

//------------------------------------------------------------------------
CMouseEventResult CTextEdit::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (buttons & kLButton)
	{
		if (auto frame = getFrame ())
		{
			if (frame->getFocusView ().get () != this)
			{
				if (isDoubleClickStyle ())
				{
					if (!(buttons & kDoubleClick))
						return kMouseEventNotHandled;
				}

				takeFocus ();
				return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
			}
		}
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
void CTextEdit::onKeyboardEvent (KeyboardEvent& event)
{
	if (!platformControl || event.type != EventType::KeyDown)
		return;

	auto lifeGuard = makeLifeGuard (this);

	if (event.virt == VirtualKey::Escape)
	{
		bWasReturnPressed = false;
		platformControl->setText (text);
		if (auto frame = getFrame ())
			frame->setFocusView (nullptr);
		looseFocus ();
		event.consumed = true;
	}
	else if (event.virt == VirtualKey::Return)
	{
		bWasReturnPressed = true;
		if (auto frame = getFrame ())
			frame->setFocusView (nullptr);
		looseFocus ();
		event.consumed = true;
	}
}

//------------------------------------------------------------------------
CRect CTextEdit::platformGetSize () const
{
	return translateToGlobal (getViewSize ());
}

//------------------------------------------------------------------------
CRect CTextEdit::platformGetVisibleSize () const
{
	return translateToGlobal (getVisibleViewSize ());
}

//------------------------------------------------------------------------
void CTextEdit::platformLooseFocus (bool returnPressed)
{
	auto lifeGuard = makeLifeGuard (this);
	bWasReturnPressed = returnPressed;
	if (auto frame = getFrame ())
	{
		if (frame->getFocusView ().get () == this)
			frame->setFocusView (nullptr);
	}
}

//------------------------------------------------------------------------
void CTextEdit::platformOnKeyboardEvent (KeyboardEvent& event)
{
	if (auto frame = getFrame ())
		static_cast<IPlatformFrameCallback*> (frame)->platformOnEvent (event);
	if (event.consumed)
		return;
	if (event.virt == VirtualKey::Return)
	{
		platformLooseFocus (true);
		event.consumed = true;
	}
	else if (event.virt == VirtualKey::Escape)
	{
		platformLooseFocus (false);
		event.consumed = true;
	}
}

//------------------------------------------------------------------------
void CTextEdit::platformTextDidChange ()
{
	if (platformControl && immediateTextChange)
		updateText (platformControl);
}

//------------------------------------------------------------------------
void CTextEdit::parentSizeChanged ()
{
	if (platformControl)
		platformControl->updateSize ();
}

//------------------------------------------------------------------------
void CTextEdit::setViewSize (const CRect& newSize, bool invalid)
{
	CTextLabel::setViewSize (newSize, invalid);
	if (platformControl)
		platformControl->updateSize ();
}

//------------------------------------------------------------------------
void CTextEdit::createPlatformTextEdit ()
{
	if (platformControl)
		return;
	
	bWasReturnPressed = false;
	if (auto frame = getFrame ())
	{
		platformControl =
			frame->getPlatformFrame ()->createPlatformTextEdit (getPlatformTextEditCallback ());
		textEditListeners.forEach (
			[this] (auto& l) { l->onTextEditPlatformControlTookFocus (*this); });
		if (frame->getFocusView ().get () != this)
			frame->setFocusView (shared (this));
	}
}

//------------------------------------------------------------------------
bool CTextEdit::wantsFocus () const
{
	if (isDoubleClickStyle () && !platformControl)
		return false;
	return CTextLabel::wantsFocus ();
}

//------------------------------------------------------------------------
void CTextEdit::takeFocus ()
{
	if (!getFrame ())
		return;
	createPlatformTextEdit ();
	CTextLabel::takeFocus ();
	invalid ();
}

//------------------------------------------------------------------------
void CTextEdit::looseFocus ()
{
	if (platformControl == nullptr)
		return;

	auto guard = makeLifeGuard (this);

	auto _platformControl = std::move (platformControl);
	updateText (_platformControl);
	
	_platformControl = nullptr;

	textEditListeners.forEach ([this] (auto& l) { l->onTextEditPlatformControlLostFocus (*this); });

	// if you want to destroy the text edit do it with the loose focus message
	auto receiver = getParentView ();
	while (receiver)
	{
		if (receiver->notify (this, kMsgLooseFocus) == kMessageNotified)
			break;
		receiver = receiver->getParentView ();
	}
	CTextLabel::looseFocus ();
	invalid ();
}

//------------------------------------------------------------------------
void CTextEdit::updateText (const PlatformTextEditPtr& pte)
{
	auto newText = pte->getText ();
	if (newText != getText ())
	{
		beginEdit ();

		setText (newText);

		CParamDisplay::valueChanged ();

		endEdit ();
	}
}

} // VSTGUI
