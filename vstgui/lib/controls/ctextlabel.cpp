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
#include "../cdrawcontext.h"

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
, textTruncateMode (kTruncateNone)
{
	setText (txt);
}

//------------------------------------------------------------------------
CTextLabel::CTextLabel (const CTextLabel& v)
: CParamDisplay (v)
, textTruncateMode (v.textTruncateMode)
{
	setText (v.getText ());
}

//------------------------------------------------------------------------
void CTextLabel::setText (const UTF8String& txt)
{
	if (text == txt)
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
	if (!(textTruncateMode == kTruncateNone || text.empty () || fontID == nullptr || fontID->getPlatformFont () == nullptr || fontID->getPlatformFont ()->getPainter () == nullptr))
	{
		CDrawMethods::TextTruncateMode mode = textTruncateMode == kTruncateHead ? CDrawMethods::kTextTruncateHead : CDrawMethods::kTextTruncateTail;
		truncatedText = CDrawMethods::createTruncatedText (mode, text, fontID, getWidth ());
		if (truncatedText == text)
			truncatedText.clear ();
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
	if (fontID == nullptr || fontID->getPlatformFont () == nullptr || fontID->getPlatformFont ()->getPainter () == nullptr)
		return false;
	CCoord width = fontID->getPlatformFont ()->getPainter ()->getStringWidth (nullptr, text.getPlatformString (), true);
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

//------------------------------------------------------------------------
void CTextLabel::valueChanged ()
{
	if (valueToStringFunction)
	{
		char string[256];
		string[0] = 0;
		if (valueToStringFunction (getValue (), string, this))
			setText (string);
	}
	CParamDisplay::valueChanged ();
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
CMultiLineTextLabel::CMultiLineTextLabel (const CRect& size)
: CTextLabel (size)
{
}

//------------------------------------------------------------------------
void CMultiLineTextLabel::setTextTruncateMode (TextTruncateMode)
{
	// not supported on multi line labels
	CTextLabel::setTextTruncateMode (kTruncateNone);
}

//------------------------------------------------------------------------
void CMultiLineTextLabel::setLineLayout (LineLayout layout)
{
	if (lineLayout == layout)
		return;
	lineLayout = layout;
	lines.clear ();
}

//------------------------------------------------------------------------
void CMultiLineTextLabel::draw (CDrawContext* pContext)
{
	if (getText ().empty () == false && lines.empty ())
		recalculateLines (pContext);
	drawBack (pContext);
	pContext->saveGlobalState ();
	CRect oldClip;
	pContext->getClipRect (oldClip);
	CRect newClip (getViewSize ());
	newClip.inset (getTextInset ());
	newClip.bound (oldClip);
	pContext->setClipRect (newClip);

	pContext->setDrawMode (kAntiAliasing);
	pContext->setFont (getFont ());
	
	CDrawContext::Transform t (*pContext, CGraphicsTransform ().translate (getViewSize ().getTopLeft ()));

	if (style & kShadowText)
	{
		CDrawContext::Transform t (*pContext, CGraphicsTransform ().translate (shadowTextOffset));
		pContext->setFontColor (getShadowColor ());
		for (const auto& line : lines)
			pContext->drawString (line.str.getPlatformString (), line.r, getHoriAlign (), bAntialias);
	}

	pContext->setFontColor (getFontColor ());
	for (const auto& line : lines)
		pContext->drawString (line.str.getPlatformString (), line.r, getHoriAlign (), bAntialias);

	pContext->restoreGlobalState ();
	setDirty (false);
}

//------------------------------------------------------------------------
bool CMultiLineTextLabel::sizeToFit ()
{
	return false;
}

//------------------------------------------------------------------------
void CMultiLineTextLabel::setText (const UTF8String& txt)
{
	if (getText () == txt)
		return;
	CTextLabel::setText (txt);
	lines.clear ();
}

//------------------------------------------------------------------------
void CMultiLineTextLabel::setViewSize (const CRect& rect, bool invalid)
{
	if (getViewSize () != rect)
		lines.clear ();
	CTextLabel::setViewSize (rect, invalid);
}

//------------------------------------------------------------------------
void CMultiLineTextLabel::drawStyleChanged ()
{
	lines.clear ();
	CTextLabel::drawStyleChanged ();
}

//------------------------------------------------------------------------
inline bool isLineBreakSeparator (char32_t c)
{
	switch (c)
	{
		case '-': return true;
		case '_': return true;
		case '/': return true;
		case '\\': return true;
		case '.': return true;
		case ',': return true;
		case ':': return true;
		case ';': return true;
		case '?': return true;
		case '!': return true;
		case '*': return true;
		case '+': return true;
		case '&': return true;
	}
	return false;
}

//------------------------------------------------------------------------
void CMultiLineTextLabel::recalculateLines (CDrawContext* context)
{
	const auto& font = getFont ()->getPlatformFont ();
	const auto& fontPainter = getFont ()->getFontPainter ();
	auto ascent = font->getAscent ();
	auto descent = font->getDescent ();
	auto leading = font->getLeading ();
	auto lineHeight = ascent + descent + leading;

	const auto& textInset = getTextInset ();
	auto maxWidth = getWidth () - (textInset.x * 2);

	std::vector<std::pair<UTF8String, CCoord>> elements;
	std::stringstream stream (getText ().getString ());
	std::string line;
	while (std::getline (stream, line, '\n'))
	{
		UTF8String str (std::move (line));
		auto width = fontPainter->getStringWidth (context, str.getPlatformString ());
		elements.emplace_back (std::move (str), width);
	}

	CCoord y = textInset.y;

	for (auto& element : elements)
	{
		if (element.second > maxWidth && lineLayout != LineLayout::clip)
		{
			if (lineLayout == LineLayout::truncate)
			{
				element.first = CDrawMethods::createTruncatedText (CDrawMethods::kTextTruncateTail, element.first, fontID, maxWidth);
			}
			else // wrap
			{
				auto start = element.first.begin ();
				auto lastSeparator = start;
				auto pos = start;
				while (pos != element.first.end ())
				{
					if (isspace (*pos))
						lastSeparator = pos;
					else if (isLineBreakSeparator (*pos))
						lastSeparator = ++pos;
					UTF8String tmp ({start.base (), ++(pos.base ())});
					auto width = fontPainter->getStringWidth (context, tmp.getPlatformString ());
					if (width > maxWidth)
					{
						if (lastSeparator == element.first.end ())
							lastSeparator = pos;
						if (start == lastSeparator)
							lastSeparator = pos;
						lines.emplace_back (Line {CRect (textInset.x, y, getWidth () - textInset.x, y + lineHeight + textInset.y), UTF8String ({start.base (), lastSeparator.base ()})});
						y += lineHeight;
						pos = lastSeparator;
						start = pos;
						if (isspace (*start))
							++start;
						lastSeparator = element.first.end ();
					}
					++pos;
				}
				if (start != element.first.end ())
				{
					lines.emplace_back (Line {CRect (textInset.x, y, getWidth () - textInset.x, y + lineHeight + textInset.y), UTF8String ({start.base (), element.first.end ().base ()})});
					y += lineHeight;
				}
				continue;
			}
		}
		lines.emplace_back (Line {CRect (textInset.x, y, getWidth () - textInset.x, y + lineHeight + textInset.y), std::move (element.first)});
		y += lineHeight;
	}

}

} // namespace
