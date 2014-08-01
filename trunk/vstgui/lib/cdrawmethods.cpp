//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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

namespace VSTGUI {
namespace CDrawMethods {

//------------------------------------------------------------------------
void drawIconAndText (CDrawContext* context, CBitmap* iconToDraw, IconPosition iconPosition, CHoriTxtAlign textAlignment, CCoord textMargin, CRect drawRect, const std::string& title, CFontRef font, CColor textColor)
{
	if (iconToDraw)
	{
		CRect iconRect (0, 0, iconToDraw->getWidth (), iconToDraw->getHeight ());
		iconRect.offset (drawRect.left, drawRect.top);
		switch (iconPosition)
		{
			case kIconLeft:
			{
				iconRect.offset (textMargin, drawRect.getHeight () / 2. - iconRect.getHeight () / 2.);
				drawRect.left = iconRect.right;
				drawRect.right -= textMargin;
				if (textAlignment == kLeftText)
					drawRect.left += textMargin;
				break;
			}
			case kIconRight:
			{
				iconRect.offset (drawRect.getWidth () - (textMargin + iconRect.getWidth ()), drawRect.getHeight () / 2. - iconRect.getHeight () / 2.);
				drawRect.right = iconRect.left;
				drawRect.left += textMargin;
				if (textAlignment == kRightText)
					drawRect.right -= textMargin;
				break;
			}
			case kIconCenterAbove:
			{
				iconRect.offset (drawRect.getWidth () / 2. - iconRect.getWidth () / 2., 0);
				if (title.empty ())
					iconRect.offset (0, drawRect.getHeight () / 2. - iconRect.getHeight () / 2.);
				else
				{
					iconRect.offset (0, drawRect.getHeight () / 2. - (iconRect.getHeight () / 2. + (textMargin + font->getSize ()) / 2.));
					drawRect.top = iconRect.bottom + textMargin;
					drawRect.setHeight (font->getSize ());
					if (textAlignment == kLeftText)
						drawRect.left += textMargin;
					else if (textAlignment == kRightText)
						drawRect.right -= textMargin;
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
					iconRect.offset (0, drawRect.getHeight () / 2. - (iconRect.getHeight () / 2.) + (textMargin + font->getSize ()) / 2.);
					drawRect.top = iconRect.top - (textMargin + font->getSize ());
					drawRect.setHeight (font->getSize ());
					if (textAlignment == kLeftText)
						drawRect.left += textMargin;
					else if (textAlignment == kRightText)
						drawRect.right -= textMargin;
				}
				break;
			}
		}
		context->drawBitmap (iconToDraw, iconRect);
	}
	else
	{
		if (textAlignment == kLeftText)
			drawRect.left += textMargin;
		else if (textAlignment == kRightText)
			drawRect.right -= textMargin;
	}
	if (!title.empty ())
	{
		context->setFont (font);
		context->setFontColor (textColor);
		context->drawString (title.c_str (), drawRect, textAlignment);
	}
}

}} // namespaces
