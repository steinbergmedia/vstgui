// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "ctextedit.h"
#include "itexteditlistener.h"
#include "../cframe.h"
#include "../platform/iplatformframe.h"
#include <cassert>

namespace VSTGUI {

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
                      CBitmap* background, const int32_t style)
: CTextLabel (size, txt, background, style)
{
	this->listener = listener;
	this->tag = tag;

	setWantsFocus (true);
}

//------------------------------------------------------------------------
CTextEdit::CTextEdit (const CTextEdit& v)
: CTextLabel (v)
, bWasReturnPressed (false)
, stringToValueFunction (v.stringToValueFunction)
, immediateTextChange (v.immediateTextChange)
, secureStyle (v.secureStyle)
, platformFont (v.platformFont)
, placeholderString (v.placeholderString)
{
	setWantsFocus (true);
}

//------------------------------------------------------------------------
CTextEdit::~CTextEdit () noexcept
{
	listener = nullptr;
	vstgui_assert (platformControl == nullptr);
}

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
void CTextEdit::registerTextEditListener (ITextEditListener* listener)
{
	textEditListeners.add (listener);
}

//------------------------------------------------------------------------
void CTextEdit::unregisterTextEditListener (ITextEditListener* listener)
{
	textEditListeners.remove (listener);
}

//------------------------------------------------------------------------
void CTextEdit::setValue (float val)
{
	CTextLabel::setValue (val);
	bool converted = false;
	std::string string;
	if (valueToStringFunction)
		converted = valueToStringFunction (getValue (), string, this);
	if (!converted)
	{
		char tmp[255];
		char precisionStr[10];
		sprintf (precisionStr, "%%.%hhuf", valuePrecision);
		sprintf (tmp, precisionStr, getValue ());
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
}

//------------------------------------------------------------------------
void CTextEdit::setText (const UTF8String& txt)
{
	if (stringToValueFunction)
	{
		float val = getValue ();
		if (stringToValueFunction (txt, val, this))
		{
			CTextLabel::setValue (val);
			if (valueToStringFunction)
			{
				std::string string;
				valueToStringFunction (getValue (), string, this);
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
void CTextEdit::draw (CDrawContext *pContext)
{
	if (platformControl)
	{
		drawBack (pContext);
		if (!platformControl->drawsPlaceholder () && !placeholderString.empty () &&
		    platformControl->getText ().empty ())
		{
			pContext->saveGlobalState ();
			pContext->setGlobalAlpha (pContext->getGlobalAlpha () * 0.5f);
			drawPlatformText (pContext, placeholderString.getPlatformString ());
			pContext->restoreGlobalState ();
		}
		setDirty (false);
		return;
	}
	drawBack (pContext);
	if (text.empty ())
	{
		if (!placeholderString.empty ())
		{
			pContext->saveGlobalState ();
			pContext->setGlobalAlpha (pContext->getGlobalAlpha () * 0.5f);
			drawPlatformText (pContext, placeholderString.getPlatformString ());
			pContext->restoreGlobalState ();
		}
	}
	else if (getSecureStyle ())
	{
		constexpr auto bulletCharacter = "\xE2\x80\xA2";
		UTF8String str;
		for (auto i = 0u; i < text.length (); ++i)
			str += bulletCharacter;
		drawPlatformText (pContext, str.getPlatformString ());
	}
	else
		CTextLabel::draw (pContext);
	setDirty (false);
}

//------------------------------------------------------------------------
CMouseEventResult CTextEdit::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (buttons & kLButton)
	{
		if (getFrame ()->getFocusView () != this)
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
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
int32_t CTextEdit::onKeyDown (VstKeyCode& keyCode)
{
	if (platformControl)
	{
		if (keyCode.virt == VKEY_ESCAPE)
		{
			bWasReturnPressed = false;
			platformControl->setText (text);
			getFrame ()->setFocusView (nullptr);
			looseFocus ();
			return 1;
		}
		else if (keyCode.virt == VKEY_RETURN)
		{
			bWasReturnPressed = true;
			getFrame ()->setFocusView (nullptr);
			looseFocus ();
			return 1;
		}
	}
	return -1;
}

//------------------------------------------------------------------------
CFontRef CTextEdit::platformGetFont () const
{
	CFontRef font = getFont ();
	CCoord fontSize = font->getSize ();
	fontSize *= getGlobalTransform ().m11;
	if (fontSize == font->getSize ())
		return font;
	platformFont = makeOwned<CFontDesc> (*font);
	platformFont->setSize (fontSize);
	return platformFont;
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
	remember ();
	bWasReturnPressed = returnPressed;
	if (getFrame ()->getFocusView () == this)
		getFrame ()->setFocusView (nullptr);
	forget ();
}

//------------------------------------------------------------------------
bool CTextEdit::platformOnKeyDown (const VstKeyCode& key)
{
	if (dynamic_cast<IPlatformFrameCallback*> (getFrame ())->platformOnKeyDown (const_cast<VstKeyCode&> (key)) == 1)
		return true;
	if (key.virt == VKEY_RETURN)
	{
		platformLooseFocus (true);
		return true;
	}
	else if (key.virt == VKEY_ESCAPE)
	{
		platformLooseFocus (false);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
void CTextEdit::platformTextDidChange ()
{
	if (platformControl && immediateTextChange)
		updateText (platformControl);
}

//------------------------------------------------------------------------
bool CTextEdit::platformIsSecureTextEdit ()
{
	return getSecureStyle ();
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
	platformControl = getFrame ()->getPlatformFrame ()->createPlatformTextEdit (this);
	textEditListeners.forEach (
	    [this] (ITextEditListener* l) { l->onTextEditPlatformControlTookFocus (this); });
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
	if (getFrame()->getFocusView () != this)
		getFrame()->setFocusView (this);
	CTextLabel::takeFocus ();
	invalid ();
}

//------------------------------------------------------------------------
void CTextEdit::looseFocus ()
{
	if (platformControl == nullptr)
		return;

	CBaseObjectGuard guard (this);

	auto _platformControl = platformControl;
	platformControl = nullptr;
	updateText (_platformControl);
	
	_platformControl = nullptr;

	textEditListeners.forEach (
	    [this] (ITextEditListener* l) { l->onTextEditPlatformControlLostFocus (this); });

	// if you want to destroy the text edit do it with the loose focus message
	CView* receiver = getParentView () ? getParentView () : getFrame ();
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
void CTextEdit::updateText (IPlatformTextEdit* pte)
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
