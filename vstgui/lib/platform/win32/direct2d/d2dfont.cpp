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

#include "d2dfont.h"

#if WINDOWS && VSTGUI_DIRECT2D_SUPPORT

#include "../win32support.h"
#include "../winstring.h"
#include "d2ddrawcontext.h"
#include <dwrite.h>
#include <d2d1.h>

namespace VSTGUI {

//-----------------------------------------------------------------------------
D2DFont::D2DFont (const char* name, const CCoord& size, const int32_t& style)
: textFormat (0)
, ascent (-1)
, descent (-1)
, leading (-1)
, capHeight (-1)
, style (style)
{
	DWRITE_FONT_STYLE fontStyle = (style & kItalicFace) ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;
	DWRITE_FONT_WEIGHT fontWeight = (style & kBoldFace) ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL;
	UTF8StringHelper nameStr (name);
	getDWriteFactory ()->CreateTextFormat (nameStr, NULL, fontWeight, fontStyle, DWRITE_FONT_STRETCH_NORMAL, (FLOAT)size, L"en-us", &textFormat);
	if (textFormat)
	{
		IDWriteFontCollection* fontCollection = 0;
		textFormat->GetFontCollection (&fontCollection);
		if (fontCollection)
		{
			IDWriteFontFamily* fontFamily = 0;
			fontCollection->GetFontFamily (0, &fontFamily);
			if (fontFamily)
			{
				IDWriteFont* font;
				fontFamily->GetFont (0, &font);
				if (font)
				{
					DWRITE_FONT_METRICS fontMetrics;
					font->GetMetrics (&fontMetrics);
					ascent = fontMetrics.ascent * (size / fontMetrics.designUnitsPerEm);
					descent = fontMetrics.descent * (size / fontMetrics.designUnitsPerEm);
					leading = fontMetrics.lineGap * (size / fontMetrics.designUnitsPerEm);
					capHeight = fontMetrics.capHeight * (size / fontMetrics.designUnitsPerEm);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
D2DFont::~D2DFont ()
{
	if (textFormat)
		textFormat->Release ();
}

//-----------------------------------------------------------------------------
IDWriteTextLayout* D2DFont::createTextLayout (const CString& string)
{
	const WinString* winString = dynamic_cast<const WinString*> (string.getPlatformString ());
	IDWriteTextLayout* textLayout = 0;
	if (winString)
		getDWriteFactory ()->CreateTextLayout (winString->getWideString (), (UINT32)wcslen (winString->getWideString ()), textFormat, 10000, 1000, &textLayout);
	return textLayout;
}

//-----------------------------------------------------------------------------
void D2DFont::drawString (CDrawContext* context, const CString& string, const CPoint& p, bool antialias)
{
	D2DDrawContext* d2dContext = dynamic_cast<D2DDrawContext*> (context);
	if (d2dContext && textFormat)
	{
		ID2D1RenderTarget* renderTarget = d2dContext->getRenderTarget ();
		if (renderTarget)
		{
			IDWriteTextLayout* textLayout = createTextLayout (string);
			if (textLayout)
			{
				if (style & kUnderlineFace)
				{
					DWRITE_TEXT_RANGE range = { 0, -1 };
					textLayout->SetUnderline (true, range);
				}
				if (style & kStrikethroughFace) 
				{
					DWRITE_TEXT_RANGE range = { 0, -1 };
					textLayout->SetStrikethrough (true, range);
				}
				renderTarget->SetTextAntialiasMode (antialias ? D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE : D2D1_TEXT_ANTIALIAS_MODE_ALIASED);
				CRect clipRect;
				D2DDrawContext::D2DApplyClip ac (d2dContext);
				D2D1_POINT_2F origin = {(FLOAT)(p.x + context->getOffset ().x), (FLOAT)(p.y + context->getOffset ().y + 1.) - textFormat->GetFontSize ()};
				d2dContext->getRenderTarget ()->DrawTextLayout (origin, textLayout, d2dContext->getFontBrush ());
				textLayout->Release ();
			}
		}
	}
}

//-----------------------------------------------------------------------------
CCoord D2DFont::getStringWidth (CDrawContext* context, const CString& string, bool antialias)
{
	CCoord result = 0;
	if (textFormat)
	{
		IDWriteTextLayout* textLayout = createTextLayout (string);
		if (textLayout)
		{
			DWRITE_TEXT_METRICS textMetrics;
			if (SUCCEEDED (textLayout->GetMetrics (&textMetrics)))
				result = (CCoord)textMetrics.widthIncludingTrailingWhitespace;
			textLayout->Release ();
		}
	}
	return result;
}

} // namespace

#endif // WINDOWS && VSTGUI_DIRECT2D_SUPPORT
