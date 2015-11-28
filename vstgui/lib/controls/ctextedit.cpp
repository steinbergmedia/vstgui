//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "ctextedit.h"
#include "../cframe.h"
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
CTextEdit::CTextEdit (const CRect& size, IControlListener* listener, int32_t tag, UTF8StringPtr txt, CBitmap* background, const int32_t style)
: CTextLabel (size, txt, background, style)
, platformControl (0)
#if !VSTGUI_HAS_FUNCTIONAL
, textToValue (0)
, textToValueUserData (0)
#endif
, immediateTextChange (false)
{
	this->listener = listener;
	this->tag = tag;

	setWantsFocus (true);
}

//------------------------------------------------------------------------
CTextEdit::CTextEdit (const CTextEdit& v)
: CTextLabel (v)
, platformControl (0)
#if VSTGUI_HAS_FUNCTIONAL
, stringToValueFunction (v.stringToValueFunction)
#else
, textToValue (v.textToValue)
, textToValueUserData (v.textToValueUserData)
#endif
, immediateTextChange (v.immediateTextChange)
{
	setWantsFocus (true);
}

//------------------------------------------------------------------------
CTextEdit::~CTextEdit ()
{
	listener = 0;
	vstgui_assert (platformControl == 0);
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS || !VSTGUI_HAS_FUNCTIONAL
//------------------------------------------------------------------------
void CTextEdit::setStringToValueProc (CTextEditStringToValueProc proc, void* userData)
{
#if VSTGUI_HAS_FUNCTIONAL
	setStringToValueFunction ([proc, userData] (UTF8StringPtr txt, float& result, CTextEdit* textEdit) {
		return proc (txt, result, userData);
	});
#else
	textToValue = proc;
	textToValueUserData = userData;
#endif
}
#endif

#if VSTGUI_HAS_FUNCTIONAL
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
#endif

//------------------------------------------------------------------------
void CTextEdit::setImmediateTextChange (bool state)
{
	immediateTextChange = state;
}

//------------------------------------------------------------------------
void CTextEdit::setValue (float val)
{
	CTextLabel::setValue (val);
	bool converted = false;
	char string[256] = {0};
#if VSTGUI_HAS_FUNCTIONAL
	if (valueToStringFunction)
		converted = valueToStringFunction (getValue (), string, this);
#else
	if (valueToString)
		converted = valueToString (getValue (), string, valueToStringUserData);
#endif
	if (!converted)
	{
		char precisionStr[10];
		sprintf (precisionStr, "%%.%hhuf", valuePrecision);
		sprintf (string, precisionStr, getValue ());
	}

	setText (string);
}

//------------------------------------------------------------------------
void CTextEdit::setText (UTF8StringPtr txt)
{
#if VSTGUI_HAS_FUNCTIONAL
	if (stringToValueFunction)
	{
		float val = getValue ();
		if (stringToValueFunction (txt, val, this))
		{
			CTextLabel::setValue (val);
			if (valueToStringFunction)
			{
				char string[256] = {0};
				valueToStringFunction (getValue (), string, this);
				CTextLabel::setText (string);
				if (platformControl)
					platformControl->setText (getText ());
				return;
			}
		}
	}
#else
	if (textToValue)
	{
		float val = getValue ();
		if (textToValue (txt, val, textToValueUserData))
		{
			CTextLabel::setValue (val);
			if (valueToString)
			{
				char string[256] = {0};
				valueToString (getValue (), string, valueToStringUserData);
				CTextLabel::setText (string);
				if (platformControl)
					platformControl->setText (getText ());
				return;
			}
		}
	}
#endif
	CTextLabel::setText (txt);
	if (platformControl)
		platformControl->setText (getText ());
}

//------------------------------------------------------------------------
void CTextEdit::draw (CDrawContext *pContext)
{
	if (platformControl)
	{
		drawBack (pContext);
		setDirty (false);
		return;
	}

	CTextLabel::draw (pContext);
}

//------------------------------------------------------------------------
CMouseEventResult CTextEdit::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (buttons & kLButton)
	{
		if (getFrame ()->getFocusView () != this)
		{
			if (style & kDoubleClickStyle)
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
			getFrame ()->setFocusView (0);
			looseFocus ();
			return 1;
		}
		else if (keyCode.virt == VKEY_RETURN)
		{
			bWasReturnPressed = true;
			getFrame ()->setFocusView (0);
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
	platformFont = owned (new CFontDesc (*font));
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
	getFrame ()->setFocusView (0);
	forget ();
}

//------------------------------------------------------------------------
bool CTextEdit::platformOnKeyDown (const VstKeyCode& key)
{
	return getFrame ()->onKeyDown (const_cast<VstKeyCode&> (key)) == 1;
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
	platformControl = getFrame ()->getPlatformFrame ()->createPlatformTextEdit (this);
}

//------------------------------------------------------------------------
bool CTextEdit::wantsFocus () const
{
	if (getStyle () & kDoubleClickStyle && !platformControl)
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
	return;
}

//------------------------------------------------------------------------
void CTextEdit::looseFocus ()
{
	if (platformControl == 0)
		return;

	CBaseObjectGuard guard (this);

	IPlatformTextEdit* _platformControl = platformControl;
	platformControl = 0;
	
	updateText (_platformControl);
	
	_platformControl->forget ();

	// if you want to destroy the text edit do it with the loose focus message
	CView* receiver = pParentView ? pParentView : pParentFrame;
	while (receiver)
	{
		if (receiver->notify (this, kMsgLooseFocus) == kMessageNotified)
			break;
		receiver = receiver->getParentView ();
	}
	CTextLabel::looseFocus ();
}

//------------------------------------------------------------------------
void CTextEdit::updateText (IPlatformTextEdit* pte)
{
	UTF8StringView newText (pte->getText ());
	if (newText != getText ())
	{
		beginEdit ();

		setText (newText);

		valueChanged ();

		endEdit ();
	}
}

} // namespace

