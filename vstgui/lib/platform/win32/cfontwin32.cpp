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

#include "cfontwin32.h"
#include "../../cdrawcontext.h"
#include "win32support.h"

#if WINDOWS

#include "winstring.h"
#include "gdiplusdrawcontext.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
bool GdiPlusFont::getAllPlatformFontFamilies (std::list<std::string>& fontFamilyNames)
{
	GDIPlusGlobals::enter ();
	Gdiplus::InstalledFontCollection fonts;
	INT numFonts = fonts.GetFamilyCount ();
	if (numFonts > 0)
	{
		Gdiplus::FontFamily* families = ::new Gdiplus::FontFamily[numFonts];
		if (fonts.GetFamilies (numFonts, families, &numFonts) == Gdiplus::Ok)
		{
			WCHAR familyName[LF_FACESIZE];
			for (INT i = 0; i < numFonts; i++)
			{
				families[i].GetFamilyName (familyName);
				UTF8StringHelper str (familyName);
				fontFamilyNames.push_back (std::string (str));
			}
		}
		::delete [] families;
		GDIPlusGlobals::exit ();
		return true;
	}
	GDIPlusGlobals::exit ();
	return false;
}

//-----------------------------------------------------------------------------
static Gdiplus::Graphics* getGraphics (CDrawContext* context)
{
	GdiplusDrawContext* gpdc = dynamic_cast<GdiplusDrawContext*> (context);
	return gpdc ? gpdc->getGraphics () : 0;
}

//-----------------------------------------------------------------------------
static Gdiplus::Brush* getFontBrush (CDrawContext* context)
{
	GdiplusDrawContext* gpdc = dynamic_cast<GdiplusDrawContext*> (context);
	return gpdc ? gpdc->getFontBrush () : 0;
}
//-----------------------------------------------------------------------------
GdiPlusFont::GdiPlusFont (const char* name, const CCoord& size, const int32_t& style)
: font (0)
{
	gdiStyle = Gdiplus::FontStyleRegular;
	if (style & kBoldFace)
		gdiStyle |= Gdiplus::FontStyleBold;
	if (style & kItalicFace)
		gdiStyle |= Gdiplus::FontStyleItalic;
	if (style & kUnderlineFace)
		gdiStyle |= Gdiplus::FontStyleUnderline;
	if (style & kStrikethroughFace)
		gdiStyle |= Gdiplus::FontStyleStrikeout;

	WCHAR tempName [200];
	mbstowcs (tempName, name, 200);
	font = ::new Gdiplus::Font (tempName, (Gdiplus::REAL)size, gdiStyle, Gdiplus::UnitPixel);
}
//-----------------------------------------------------------------------------
GdiPlusFont::~GdiPlusFont ()
{
	if (font)
		::delete font;
}

//-----------------------------------------------------------------------------
void GdiPlusFont::drawString (CDrawContext* context, IPlatformString* string, const CPoint& point, bool antialias)
{
	Gdiplus::Graphics* pGraphics = getGraphics (context);
	Gdiplus::Brush* pFontBrush = getFontBrush (context);
	const WinString* winString = dynamic_cast<const WinString*> (string);
	if (pGraphics && font && pFontBrush && winString)
	{
		GdiplusDrawScope drawScope (pGraphics, context->getAbsoluteClipRect (), context->getCurrentTransform ());
		pGraphics->SetTextRenderingHint (antialias ? Gdiplus::TextRenderingHintClearTypeGridFit : Gdiplus::TextRenderingHintSystemDefault);
		Gdiplus::PointF gdiPoint ((Gdiplus::REAL)point.x, (Gdiplus::REAL)point.y + 1.f - font->GetHeight (pGraphics->GetDpiY ()));
		pGraphics->DrawString (winString->getWideString (), -1, font, gdiPoint, pFontBrush);
	}
}

//-----------------------------------------------------------------------------
CCoord GdiPlusFont::getStringWidth (CDrawContext* context, IPlatformString* string, bool antialias)
{
	CCoord result = 0;
	const WinString* winString = dynamic_cast<const WinString*> (string);
	if (winString)
	{
		Gdiplus::Graphics* pGraphics = context ? getGraphics (context) : 0;
		HDC hdc = 0;
		if (context == 0)
		{
			hdc = CreateCompatibleDC (0);
			pGraphics = ::new Gdiplus::Graphics (hdc);
		}
		if (pGraphics && font)
		{
			Gdiplus::PointF gdiPoint (0., 0.);
			Gdiplus::RectF resultRect;
			pGraphics->MeasureString (winString->getWideString (), -1, font, gdiPoint, &resultRect);
			result = (CCoord)resultRect.Width;
		}
		if (hdc)
		{
			::delete pGraphics;
			DeleteDC (hdc);
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
double GdiPlusFont::getAscent () const
{
	Gdiplus::FontFamily fontFamily;
	if (font->GetFamily (&fontFamily) == Gdiplus::Ok)
	{
		UINT16 cellAscent = fontFamily.GetCellAscent (gdiStyle);
		return (double)font->GetSize () * (double)cellAscent / (double)fontFamily.GetEmHeight (gdiStyle);
	}
	return -1;
}

//-----------------------------------------------------------------------------
double GdiPlusFont::getDescent () const
{
	Gdiplus::FontFamily fontFamily;
	if (font->GetFamily (&fontFamily) == Gdiplus::Ok)
	{
		UINT16 cellDescent = fontFamily.GetCellDescent (gdiStyle);
		return (double)font->GetSize () * (double)cellDescent / (double)fontFamily.GetEmHeight (gdiStyle);
	}
	return -1;
}

//-----------------------------------------------------------------------------
double GdiPlusFont::getLeading () const
{
	Gdiplus::FontFamily fontFamily;
	if (font->GetFamily (&fontFamily) == Gdiplus::Ok)
	{
		UINT16 leading = fontFamily.GetLineSpacing (gdiStyle);
		return (double)font->GetSize () * (double)leading / (double)fontFamily.GetEmHeight (gdiStyle);
	}
	return -1;
}

//-----------------------------------------------------------------------------
double GdiPlusFont::getCapHeight () const
{
	return -1;
}

} // namespace

#endif // WINDOWS
