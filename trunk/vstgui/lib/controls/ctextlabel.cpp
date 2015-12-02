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

#include "ctextlabel.h"
#include "../platform/iplatformfont.h"
#include "../cdrawmethods.h"

namespace VSTGUI {

IdStringPtr CTextLabel::kMsgTruncatedTextChanged = "CTextLabel::kMsgTruncatedTextChanged";

//------------------------------------------------------------------------
// CTextLabel
//------------------------------------------------------------------------
/*! @class CTextLabel
*/
//------------------------------------------------------------------------
/**
 * CTextLabel constructor.
 * @param size the size of this view
 * @param txt the initial text as c string (UTF-8 encoded)
 * @param background the background bitmap
 * @param style the display style (see CParamDisplay for styles)
 */
//------------------------------------------------------------------------
CTextLabel::CTextLabel (const CRect& size, UTF8StringPtr txt, CBitmap* background, const int32_t style)
: CParamDisplay (size, background, style)
, text (0)
, textTruncateMode (kTruncateNone)
{
	setText (txt);
}

//------------------------------------------------------------------------
CTextLabel::CTextLabel (const CTextLabel& v)
: CParamDisplay (v)
, text (0)
, textTruncateMode (v.textTruncateMode)
{
	setText (v.getText ());
}

//------------------------------------------------------------------------
CTextLabel::~CTextLabel ()
{
}

//------------------------------------------------------------------------
void CTextLabel::setText (UTF8StringPtr txt)
{
	if (txt && UTF8StringView (txt) == text)
		return;
	text = txt;
	if (textTruncateMode != kTruncateNone)
		calculateTruncatedText ();
	setDirty (true);
}

//------------------------------------------------------------------------
void CTextLabel::setTextTruncateMode (TextTruncateMode mode)
{
	if (textTruncateMode != mode)
	{
		textTruncateMode = mode;
		calculateTruncatedText ();
	}
}

//------------------------------------------------------------------------
void CTextLabel::calculateTruncatedText ()
{
	if (textRotation != 0.) // currently truncation is only supported when not rotated
	{
		truncatedText = "";
		return;
	}
	if (!(textTruncateMode == kTruncateNone || text.getByteCount () == 0 || fontID == 0 || fontID->getPlatformFont () == 0 || fontID->getPlatformFont ()->getPainter () == 0))
	{
		CDrawMethods::TextTruncateMode mode = textTruncateMode == kTruncateHead ? CDrawMethods::kTextTruncateHead : CDrawMethods::kTextTruncateTail;
		truncatedText = CDrawMethods::createTruncatedText (mode, text, fontID, getWidth ());
		if (truncatedText == text)
			truncatedText.set (0);
		changed (kMsgTruncatedTextChanged);
	}
}

//------------------------------------------------------------------------
const UTF8String& CTextLabel::getText () const
{
	return text;
}

//------------------------------------------------------------------------
void CTextLabel::draw (CDrawContext *pContext)
{
	drawBack (pContext);
	drawPlatformText (pContext, truncatedText.empty () ? text.getPlatformString () : truncatedText.getPlatformString ());
	setDirty (false);
}

//------------------------------------------------------------------------
bool CTextLabel::sizeToFit ()
{
	if (fontID == 0 || fontID->getPlatformFont () == 0 || fontID->getPlatformFont ()->getPainter () == 0)
		return false;
	CCoord width = fontID->getPlatformFont ()->getPainter ()->getStringWidth (0, text.getPlatformString (), true);
	if (width > 0)
	{
		width += (getTextInset ().x * 2.);
		CRect newSize = getViewSize ();
		newSize.setWidth (width);
		setViewSize (newSize);
		setMouseableArea (newSize);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
void CTextLabel::setViewSize (const CRect& rect, bool invalid)
{
	CRect current (getViewSize ());
	CParamDisplay::setViewSize (rect, invalid);
	if (textTruncateMode != kTruncateNone && current.getWidth () != getWidth ())
	{
		calculateTruncatedText ();
	}
}

//------------------------------------------------------------------------
void CTextLabel::drawStyleChanged ()
{
	if (textTruncateMode != kTruncateNone)
	{
		calculateTruncatedText ();
	}
	CParamDisplay::drawStyleChanged ();
}

} // namespace
