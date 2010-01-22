//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
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
CTextEdit::CTextEdit (const CRect& size, CControlListener* listener, long tag, const char *txt, CBitmap* background, const long style)
: CParamDisplay (size, background, style)
, platformControl (0)
, editConvert (0)
, editConvert2 (0)
{
	this->listener = listener;
	this->tag = tag;

	if (txt)
		strcpy (text, txt);
	else
		strcpy (text, "");
	setWantsFocus (true);
}

//------------------------------------------------------------------------
CTextEdit::CTextEdit (const CTextEdit& v)
: CParamDisplay (v)
, platformControl (0)
, editConvert (v.editConvert)
, editConvert2 (v.editConvert2)
{
	setText ((char*)v.text);
	setWantsFocus (true);
}

//------------------------------------------------------------------------
CTextEdit::~CTextEdit ()
{
	listener = 0;
	if (platformControl)
		looseFocus ();
}

//------------------------------------------------------------------------
void CTextEdit::setValue (float val, bool updateSubListeners)
{
	CParamDisplay::setValue (val, updateSubListeners);
	if (platformControl)
	{
		char string[256];
		string[0] = 0;

		if (editConvert2)
			editConvert2 (text, string, userData);
		else if (editConvert)
			editConvert (text, string);
		else if (stringConvert2)
		{
			string[0] = 0;
			stringConvert2 (value, string, userData);
		}
		else if (stringConvert)
		{
			string[0] = 0;
			stringConvert (value, string);
		}
		else
			sprintf (string, "%s", text);
		platformControl->setText (string);
	}
}

//------------------------------------------------------------------------
void CTextEdit::setText (const char *txt)
{
	if (txt)
	{
		if (strcmp (text, txt))
		{
			strncpy (text, txt, 255);
			text[255] = 0;	// make sure we end with a 0 byte

			// to force the redraw
			setDirty ();
		}
	}
	else
	{
		if (strcmp (text, ""))
		{
			strcpy (text, "");

			// to force the redraw
			setDirty ();
		}
	}
	if (platformControl)
		platformControl->setText (text);
}

//------------------------------------------------------------------------
void CTextEdit::getText (char *txt) const
{
	if (txt)
		strcpy (txt, text);
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

	char string[256];
	string[0] = 0;

	if (editConvert2)
		editConvert2 (text, string, userData);
	else if (editConvert)
		editConvert (text, string);
	// Allow to display strings through the stringConvert
	// callbacks inherited from CParamDisplay
	else if (stringConvert2)
	{
		string[0] = 0;
		stringConvert2 (value, string, userData);
		strcpy(text, string);
	}
	else if (stringConvert)
	{
		string[0] = 0;
		stringConvert (value, string);
		strcpy(text, string);
	}
	else
		sprintf (string, "%s", text);

	drawBack (pContext);
	drawText (pContext, string);
	setDirty (false);
}

//------------------------------------------------------------------------
CMouseEventResult CTextEdit::onMouseDown (CPoint& where, const long& buttons)
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
long CTextEdit::onKeyDown (VstKeyCode& keyCode)
{
	if (platformControl)
	{
		if (keyCode.virt == VKEY_ESCAPE)
		{
			bWasReturnPressed = false;
			platformControl->setText (text);
			looseFocus ();
			return 1;
		}
		else if (keyCode.virt == VKEY_RETURN)
		{
			bWasReturnPressed = true;
			looseFocus ();
			return 1;
		}
	}
	return -1;
}

//------------------------------------------------------------------------
CRect CTextEdit::platformGetSize () const
{
	CRect rect = getViewSize ();
	CPoint p (0, 0);
	localToFrame (p);
	rect.offset (p.x, p.y);
	return rect;
}

//------------------------------------------------------------------------
CRect CTextEdit::platformGetVisibleSize () const
{
	CRect rect = getVisibleSize ();
	CPoint p (0, 0);
	localToFrame (p);
	rect.offset (p.x, p.y);
	return rect;
}

//------------------------------------------------------------------------
void CTextEdit::platformLooseFocus (bool returnPressed)
{
	remember ();
	bWasReturnPressed = returnPressed;
	looseFocus ();
	forget ();
}

//------------------------------------------------------------------------
bool CTextEdit::platformOnKeyDown (const VstKeyCode& key)
{
	return getFrame ()->onKeyDown (const_cast<VstKeyCode&> (key)) == 1;
}

//------------------------------------------------------------------------
void CTextEdit::parentSizeChanged ()
{
	if (platformControl)
		platformControl->updateSize ();
}

//------------------------------------------------------------------------
void CTextEdit::setViewSize (CRect& newSize, bool invalid)
{
	CView::setViewSize (newSize, invalid);
	if (platformControl)
		platformControl->updateSize ();
}

//------------------------------------------------------------------------
void CTextEdit::takeFocus ()
{
	if (platformControl)
		return;

	bWasReturnPressed = false;

	// calculate offset for CViewContainers
	CRect rect (size);
	CPoint p (0, 0);
	localToFrame (p);
	rect.offset (p.x, p.y);

	platformControl = getFrame ()->getPlatformFrame ()->createPlatformTextEdit (this);
	if (platformControl)
		beginEdit ();
}

//------------------------------------------------------------------------
void CTextEdit::looseFocus ()
{
	// Call this yet to avoid recursive call
	if (getFrame () && getFrame ()->getFocusView () == this)
		getFrame ()->setFocusView (0);

	if (platformControl == 0)
		return;

	endEdit();

	char oldText[256];
	strcpy (oldText, text);

	platformControl->getText (text, 255);
	platformControl->forget ();
	platformControl = 0;

	// update dependency
	bool change = false;
	if (strcmp (oldText, text))
	{
		change = true;
		if (string2FloatConvert)
			string2FloatConvert (text, value);
		valueChanged ();
	}

	// if you want to destroy the text edit do it with the loose focus message
	CView* receiver = pParentView ? pParentView : pParentFrame;
	while (receiver)
	{
		if (receiver->notify (this, kMsgLooseFocus) == kMessageNotified)
			break;
		receiver = receiver->getParentView ();
	}
}

//------------------------------------------------------------------------
void CTextEdit::setTextEditConvert (void (*convert) (char *input, char *string))
{
	editConvert = convert;
}

//------------------------------------------------------------------------
void CTextEdit::setTextEditConvert (void (*convert) (char *input, char *string,
									  void *userDta), void *userData)
{
	editConvert2 = convert;
	this->userData = userData;
}

} // namespace

