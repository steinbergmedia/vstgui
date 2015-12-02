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

#include "cdrawmethods.h"
#include "cbitmap.h"
#include "cstring.h"
#include "cdrawcontext.h"
#include "platform/iplatformfont.h"

namespace VSTGUI {
namespace CDrawMethods {

//------------------------------------------------------------------------
UTF8String createTruncatedText (TextTruncateMode mode, const UTF8String& text, CFontRef font, CCoord maxWidth, const CPoint& textInset, uint32_t flags)
{
	if (mode == kTextTruncateNone)
		return text;
	IFontPainter* painter = font->getPlatformFont ()->getPainter ();
	CCoord width = painter->getStringWidth (0, text.getPlatformString (), true);
	width += textInset.x * 2;
	if (width > maxWidth)
	{
		std::string _truncatedText;
		UTF8String utf8Str;
		if (mode == kTextTruncateTail)
		{
			_truncatedText = text;
			_truncatedText += "..";
			while (width > maxWidth && _truncatedText.size () > 2)
			{
				UTF8CharacterIterator it (_truncatedText);
				it.end ();
				for (int32_t i = 0; i < 3; i++, --it)
				{
					if (it == it.front ())
					{
						break;
					}
				}
				_truncatedText.erase (_truncatedText.size () - (2 + it.getByteLength ()), it.getByteLength ());
				utf8Str.set (_truncatedText.c_str ());
				width = painter->getStringWidth (0, utf8Str.getPlatformString (), true);
				width += textInset.x * 2;
			}
		}
		else if (mode == kTextTruncateHead)
		{
			_truncatedText = "..";
			_truncatedText += text;
			while (width > maxWidth && _truncatedText.size () > 2)
			{
				UTF8CharacterIterator it (_truncatedText);
				for (int32_t i = 0; i < 2; i++, ++it)
				{
					if (it == it.back ())
					{
						break;
					}
				}
				_truncatedText.erase (2, it.getByteLength ());
				utf8Str.set (_truncatedText.c_str ());
				width = painter->getStringWidth (0, utf8Str.getPlatformString (), true);
				width += textInset.x * 2;
			}
		}
		if (width > maxWidth && flags & kReturnEmptyIfTruncationIsPlaceholderOnly)
			utf8Str.set ("");
		return utf8Str;
	}
	return text;
}

//------------------------------------------------------------------------
void drawIconAndText (CDrawContext* context, CBitmap* iconToDraw, IconPosition iconPosition, CHoriTxtAlign textAlignment, CCoord textIconMargin, CRect drawRect, const UTF8String& title, CFontRef font, CColor textColor, TextTruncateMode textTruncateMode)
{
	if (iconToDraw)
	{
		CRect iconRect (0, 0, iconToDraw->getWidth (), iconToDraw->getHeight ());
		iconRect.offset (drawRect.left, drawRect.top);
		switch (iconPosition)
		{
			case kIconLeft:
			{
				iconRect.offset (textIconMargin, drawRect.getHeight () / 2. - iconRect.getHeight () / 2.);
				drawRect.left = iconRect.right;
				drawRect.right -= textIconMargin;
				if (textAlignment == kLeftText)
					drawRect.left += textIconMargin;
				break;
			}
			case kIconRight:
			{
				iconRect.offset (drawRect.getWidth () - (textIconMargin + iconRect.getWidth ()), drawRect.getHeight () / 2. - iconRect.getHeight () / 2.);
				drawRect.right = iconRect.left;
				drawRect.left += textIconMargin;
				if (textAlignment == kRightText)
					drawRect.right -= textIconMargin;
				break;
			}
			case kIconCenterAbove:
			{
				iconRect.offset (drawRect.getWidth () / 2. - iconRect.getWidth () / 2., 0);
				if (title.empty ())
					iconRect.offset (0, drawRect.getHeight () / 2. - iconRect.getHeight () / 2.);
				else
				{
					iconRect.offset (0, drawRect.getHeight () / 2. - (iconRect.getHeight () / 2. + (textIconMargin + font->getSize ()) / 2.));
					drawRect.top = iconRect.bottom + textIconMargin;
					drawRect.setHeight (font->getSize ());
					if (textAlignment == kLeftText)
						drawRect.left += textIconMargin;
					else if (textAlignment == kRightText)
						drawRect.right -= textIconMargin;
				}
				break;
			}
			case kIconCenterBelow:
			{
				iconRect.offset (drawRect.getWidth () / 2. - iconRect.getWidth () / 2., 0);
				if (title.empty ())
					iconRect.offset (0, drawRect.getHeight () / 2. - iconRect.getHeight () / 2.);
				else
				{
					iconRect.offset (0, drawRect.getHeight () / 2. - (iconRect.getHeight () / 2.) + (textIconMargin + font->getSize ()) / 2.);
					drawRect.top = iconRect.top - (textIconMargin + font->getSize ());
					drawRect.setHeight (font->getSize ());
					if (textAlignment == kLeftText)
						drawRect.left += textIconMargin;
					else if (textAlignment == kRightText)
						drawRect.right -= textIconMargin;
				}
				break;
			}
		}
		context->drawBitmap (iconToDraw, iconRect);
	}
	else
	{
		if (textAlignment == kLeftText)
			drawRect.left += textIconMargin;
		else if (textAlignment == kRightText)
			drawRect.right -= textIconMargin;
	}
	if (!title.empty ())
	{
		context->setFont (font);
		context->setFontColor (textColor);
		if (textTruncateMode != kTextTruncateNone)
		{
			UTF8String truncatedText = createTruncatedText (textTruncateMode, title, font, drawRect.getWidth (), CPoint (0, 0), kReturnEmptyIfTruncationIsPlaceholderOnly);
			context->drawString (truncatedText.getPlatformString (), drawRect, textAlignment);
		}
		else
			context->drawString (title.getPlatformString (), drawRect, textAlignment);
	}
}

}} // namespaces
