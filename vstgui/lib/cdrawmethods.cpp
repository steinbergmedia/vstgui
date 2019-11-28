// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cdrawmethods.h"
#include "cbitmap.h"
#include "cstring.h"
#include "cdrawcontext.h"
#include "platform/iplatformfont.h"

namespace VSTGUI {

namespace CDrawMethods {

//------------------------------------------------------------------------
UTF8String createTruncatedText (TextTruncateMode mode, const UTF8String& text, CFontRef font,
                                CCoord maxWidth, const CPoint& textInset, uint32_t flags)
{
	if (mode == kTextTruncateNone)
		return text;
	auto painter = font->getPlatformFont () ? font->getPlatformFont ()->getPainter () : nullptr;
	if (!painter)
		return text;
	CCoord width = painter->getStringWidth (nullptr, text.getPlatformString (), true);
	width += textInset.x * 2;
	if (width > maxWidth)
	{
		std::string truncatedText;
		UTF8String result;
		auto left = text.begin ();
		auto right = text.end ();
		while (width > maxWidth && left != right)
		{
			if (mode == kTextTruncateHead)
			{
				++left;
				truncatedText = "..";
			}
			else if (mode == kTextTruncateTail)
			{
				--right;
				truncatedText = "";
			}

			truncatedText += {left.base (), right.base ()};

			if (mode == kTextTruncateTail)
				truncatedText += "..";

			result = truncatedText;
			width = painter->getStringWidth (nullptr, result.getPlatformString (), true);
			width += textInset.x * 2;
		}
		if (left == right && flags & kReturnEmptyIfTruncationIsPlaceholderOnly)
			result = "";
		return result;
	}
	return text;
}

//------------------------------------------------------------------------
void drawIconAndText (CDrawContext* context, CBitmap* iconToDraw, IconPosition iconPosition,
                      CHoriTxtAlign textAlignment, CCoord textIconMargin, CRect drawRect,
                      const UTF8String& title, CFontRef font, CColor textColor,
                      TextTruncateMode textTruncateMode)
{
	if (iconToDraw)
	{
		CRect iconRect (0, 0, iconToDraw->getWidth (), iconToDraw->getHeight ());
		iconRect.offset (drawRect.left, drawRect.top);
		switch (iconPosition)
		{
			case kIconLeft:
			{
				iconRect.offset (textIconMargin,
				                 drawRect.getHeight () / 2. - iconRect.getHeight () / 2.);
				drawRect.left = iconRect.right;
				drawRect.right -= textIconMargin;
				if (textAlignment == kLeftText)
					drawRect.left += textIconMargin;
				break;
			}
			case kIconRight:
			{
				iconRect.offset (drawRect.getWidth () - (textIconMargin + iconRect.getWidth ()),
				                 drawRect.getHeight () / 2. - iconRect.getHeight () / 2.);
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
					iconRect.offset (0, drawRect.getHeight () / 2. -
					                        (iconRect.getHeight () / 2. +
					                         (textIconMargin + font->getSize ()) / 2.));
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
					iconRect.offset (0, drawRect.getHeight () / 2. - (iconRect.getHeight () / 2.) +
					                        (textIconMargin + font->getSize ()) / 2.);
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
			UTF8String truncatedText =
			    createTruncatedText (textTruncateMode, title, font, drawRect.getWidth (),
			                         CPoint (0, 0), kReturnEmptyIfTruncationIsPlaceholderOnly);
			context->drawString (truncatedText.getPlatformString (), drawRect, textAlignment);
		}
		else
			context->drawString (title.getPlatformString (), drawRect, textAlignment);
	}
}
}
} // namespaces
