// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "ctextlabel.h"
#include "../platform/iplatformfont.h"
#include "../cdrawmethods.h"
#include "../cdrawcontext.h"
#include <sstream>

namespace VSTGUI {

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
void CTextLabel::registerTextLabelListener (ITextLabelListener* listener)
{
	if (!listeners)
		listeners = std::unique_ptr<TextLabelListenerList> (new TextLabelListenerList ());
	listeners->add (listener);
}

//------------------------------------------------------------------------
void CTextLabel::unregisterTextLabelListener (ITextLabelListener* listener)
{
	if (listeners)
		listeners->remove (listener);
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
		truncatedText = CDrawMethods::createTruncatedText (mode, text, fontID, getWidth () - getTextInset ().x * 2.);
		if (truncatedText == text)
			truncatedText.clear ();
		if (listeners)
		{
			listeners->forEach (
			    [this] (ITextLabelListener* l) { l->onTextLabelTruncatedTextChanged (this); });
		}
	}
	else if (!truncatedText.empty ())
		truncatedText.clear ();
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
		std::string string;
		if (valueToStringFunction (getValue (), string, this))
			setText (UTF8String (std::move (string)));
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
void CMultiLineTextLabel::setValue (float val)
{
	CTextLabel::setValue (val);

	if (valueToStringFunction)
	{
		std::string string;
		if (valueToStringFunction (value, string, this))
			setText (UTF8String (string));
	}
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
void CMultiLineTextLabel::setAutoHeight (bool state)
{
	if (autoHeight == state)
		return;
	autoHeight = state;
	if (autoHeight && isAttached ())
	{
		if (lines.empty ())
			recalculateLines (nullptr);
		recalculateHeight ();
	}
}

//------------------------------------------------------------------------
void CMultiLineTextLabel::setVerticalCentered (bool state)
{
	if (verticalCentered == state)
		return;
	verticalCentered = state;
	lines.clear ();
}

//------------------------------------------------------------------------
CCoord CMultiLineTextLabel::getMaxLineWidth ()
{
	if (lines.empty () && getText ().empty () == false)
		recalculateLines (nullptr);
	CCoord maxWidth {};
	for (const auto& line : lines)
	{
		if (line.r.getWidth () > maxWidth)
			maxWidth = line.r.getWidth ();
	}
	return maxWidth;
}

//------------------------------------------------------------------------
void CMultiLineTextLabel::drawRect (CDrawContext* pContext, const CRect& updateRect)
{
	if (getText ().empty () == false && lines.empty ())
		recalculateLines (pContext);
	drawBack (pContext);
	
	CRect newClip (updateRect);
	newClip.inset (getTextInset ());
	ConcatClip clip (*pContext, newClip);
	newClip = clip.get ();

	pContext->setDrawMode (kAntiAliasing);
	pContext->setFont (getFont ());
	
	newClip.offsetInverse (getViewSize().getTopLeft ());

	CDrawContext::Transform t (*pContext, CGraphicsTransform ().translate (getViewSize ().getTopLeft ()));

	if (style & kShadowText)
	{
		CDrawContext::Transform t2 (*pContext, CGraphicsTransform ().translate (shadowTextOffset));
		pContext->setFontColor (getShadowColor ());
		for (const auto& line : lines)
		{
			if (line.r.rectOverlap (newClip))
				pContext->drawString (line.str.getPlatformString (), line.r, getHoriAlign (), getAntialias ());
		}
	}

	pContext->setFontColor (getFontColor ());
	for (const auto& line : lines)
	{
		if (line.r.rectOverlap (newClip))
			pContext->drawString (line.str.getPlatformString (), line.r, getHoriAlign (), getAntialias ());
		else if (line.r.bottom > newClip.bottom)
			break;
	}

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
	if (autoHeight && isAttached ())
	{
		recalculateLines (nullptr);
		recalculateHeight ();
	}
}

//------------------------------------------------------------------------
void CMultiLineTextLabel::recalculateHeight ()
{
	auto viewSize = getViewSize ();
	if (lines.empty ())
		viewSize.setHeight (0.);
	else
	{
		auto lastLine = lines.back ().r;
		viewSize.setHeight (lastLine.bottom + getTextInset ().y);
	}
	CTextLabel::setViewSize (viewSize);
}

//------------------------------------------------------------------------
void CMultiLineTextLabel::setViewSize (const CRect& rect, bool invalid)
{
	auto viewSize = getViewSize ();
	auto normRect = rect;
	viewSize.originize ();
	normRect.originize ();
	if (viewSize != normRect)
	{
		if (lineLayout != LineLayout::clip ||
		    (lineLayout == LineLayout::clip &&
		     viewSize.getHeight () != normRect.getHeight ()))
		{
			lines.clear ();
		}
	}
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
void CMultiLineTextLabel::calculateWrapLine  (CDrawContext* context,
                                       std::pair<UTF8String, double>& element,
                                       const IFontPainter* const& fontPainter, double lineHeight,
                                       double lineWidth, double maxWidth, const CPoint& textInset,
                                       CCoord& y)
{
	auto start = element.first.begin ();
	auto lastSeparator = start;
	auto pos = start;
	while (pos != element.first.end () && *pos != 0)
	{
		if (isspace (*pos))
			lastSeparator = pos;
		else if (isLineBreakSeparator (*pos))
			lastSeparator = ++pos;
		if (pos == element.first.end ())
			break;
		auto tmpEnd = pos;
		UTF8String tmp ({start.base (), (++tmpEnd).base ()});
		auto width = fontPainter->getStringWidth (context, tmp.getPlatformString ());
		if (width > maxWidth)
		{
			if (lastSeparator == element.first.end ())
				lastSeparator = pos;
			if (start == lastSeparator)
				lastSeparator = pos;
			lines.emplace_back (
			    Line {CRect (textInset.x, y, lineWidth, y + lineHeight + textInset.y),
			          UTF8String ({start.base (), lastSeparator.base ()})});
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
		lines.emplace_back (Line {CRect (textInset.x, y, lineWidth, y + lineHeight + textInset.y),
		                          UTF8String ({start.base (), element.first.end ().base ()})});
		y += lineHeight;
	}
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

	auto lineWidth = getWidth () - textInset.x;
	
	for (auto& element : elements)
	{
		if (lineLayout == LineLayout::clip)
		{
			lines.emplace_back (Line {
			    CRect (textInset.x, y, element.second + textInset.x, y + lineHeight + textInset.y),
			    std::move (element.first)});
		}
		else
		{
			if (element.second > maxWidth)
			{
				if (lineLayout == LineLayout::truncate)
				{
					element.first = CDrawMethods::createTruncatedText (
					    CDrawMethods::kTextTruncateTail, element.first, fontID, maxWidth);
				}
				else // wrap
				{
					calculateWrapLine (context, element, fontPainter, lineHeight, lineWidth,
					                   maxWidth, textInset, y);
					continue;
				}
			}
			lines.emplace_back (
			    Line {CRect (textInset.x, y, lineWidth, y + lineHeight + textInset.y),
			          std::move (element.first)});
		}
		y += lineHeight;
	}
	if (getVerticalCentered () && !lines.empty ())
	{
		auto maxHeight = lines.back ().r.bottom;
		auto offset = ((getHeight () - textInset.y) - maxHeight) / 2.;
		if (offset > 0)
		{
			for (auto& l : lines)
				l.r.offset (0, offset);
		}
	}
}

} // VSTGUI
