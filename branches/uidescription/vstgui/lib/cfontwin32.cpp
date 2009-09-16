//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2009, Steinberg Media Technologies, All Rights Reserved
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

#include "cfontwin32.h"
#include "cdrawcontext.h"
#include "win32support.h"

#if WINDOWS

BEGIN_NAMESPACE_VSTGUI

//-----------------------------------------------------------------------------
CPlatformFont* CPlatformFont::create (const char* name, const CCoord& size, const long& style)
{
	#if GDIPLUS
	GdiPlusFont* font = new GdiPlusFont (name, size, style);
	if (font->getFont ())
		return font;
	font->forget ();
	#else
	GdiFont* font = new GdiFont (name, size, style);
	if (font->getFont ())
		return font;
	font->forget ();
	#endif
	return 0;
}

#if GDIPLUS
//-----------------------------------------------------------------------------
GdiPlusFont::GdiPlusFont (const char* name, const CCoord& size, const long& style)
: font (0)
{
	gdiStyle = Gdiplus::FontStyleRegular;
	if (style & kBoldFace)
		gdiStyle = Gdiplus::FontStyleBold;
	if (style & kItalicFace)
		gdiStyle = Gdiplus::FontStyleItalic;
	if (style & kUnderlineFace)
		gdiStyle = Gdiplus::FontStyleUnderline;

	WCHAR tempName [200];
	mbstowcs (tempName, name, 200);
	font = new Gdiplus::Font (tempName, (Gdiplus::REAL)size, gdiStyle, Gdiplus::UnitPixel);
}

//-----------------------------------------------------------------------------
GdiPlusFont::~GdiPlusFont ()
{
	if (font)
		delete font;
}

//-----------------------------------------------------------------------------
void GdiPlusFont::drawString (CDrawContext* context, const char* utf8String, const CPoint& point, bool antialias)
{
	Gdiplus::Graphics* pGraphics = context->getGraphics ();
	Gdiplus::SolidBrush* pFontBrush = context->getFontBrush ();
	if (pGraphics && font && pFontBrush)
	{
		UTF8StringHelper stringText (utf8String);
		pGraphics->SetTextRenderingHint (antialias ? Gdiplus::TextRenderingHintClearTypeGridFit : Gdiplus::TextRenderingHintSystemDefault);
		Gdiplus::PointF gdiPoint ((Gdiplus::REAL)point.x, (Gdiplus::REAL)point.y + 1.f - font->GetHeight (pGraphics->GetDpiY ()));
		pGraphics->DrawString (stringText, -1, font, gdiPoint, pFontBrush);
	}
}

//-----------------------------------------------------------------------------
CCoord GdiPlusFont::getStringWidth (CDrawContext* context, const char* utf8String, bool antialias)
{
	CCoord result = 0;
	Gdiplus::Graphics* pGraphics = context->getGraphics ();
	if (pGraphics && font)
	{
		UTF8StringHelper stringText (utf8String);
		Gdiplus::PointF gdiPoint (0., 0.);
		Gdiplus::RectF resultRect;
		pGraphics->MeasureString (stringText, -1, font, gdiPoint, &resultRect);
		result = (CCoord)resultRect.Width;
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

#else
//-----------------------------------------------------------------------------
GdiFont::GdiFont (const char* name, const CCoord& size, const long& style)
: font (0)
{
	LOGFONT logfont = {0};
	if (style & kBoldFace)
		logfont.lfWeight = FW_BOLD;
	else
		logfont.lfWeight = FW_NORMAL;
	if (style & kItalicFace)
		logfont.lfItalic = true;
	if (style & kUnderlineFace)
		logfont.lfUnderline = true;
	
	logfont.lfHeight         = -size;
	logfont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
	strcpy (logfont.lfFaceName, name);

	if (!strcmp (name, kSymbolFont->getName ()))
		logfont.lfPitchAndFamily = DEFAULT_PITCH | FF_DECORATIVE;
  
	logfont.lfClipPrecision = CLIP_STROKE_PRECIS;
	logfont.lfOutPrecision  = OUT_STRING_PRECIS;
	logfont.lfQuality 	    = DEFAULT_QUALITY;
	logfont.lfCharSet       = ANSI_CHARSET;

	font = CreateFontIndirect (&logfont);
}

//-----------------------------------------------------------------------------
GdiFont::~GdiFont ()
{
	if (font)
		DeleteObject (font);
}

//-----------------------------------------------------------------------------
void GdiFont::drawString (CDrawContext* context, const char* utf8String, const CPoint& p, bool antialias)
{
	HDC pSystemContext = (HDC)context->getSystemContext ();
	SelectObject (pSystemContext, font);

	// set the visibility mask
	SetBkMode (pSystemContext, opaque ? OPAQUE : TRANSPARENT);

	RECT Rect = {p.x, p.y, p.x, p.y};
	DrawText (pSystemContext, utf8String, (int)strlen (utf8String), &Rect, DT_NOCLIP);

	SetBkMode (pSystemContext, TRANSPARENT);
}

//-----------------------------------------------------------------------------
CCoord GdiFont::getStringWidth (CDrawContext* context, const char* utf8String, bool antialias)
{
	HDC pSystemContext = (HDC)context->getSystemContext ();

	SIZE size;
	GetTextExtentPoint32 ((HDC)pSystemContext, utf8String, (int)strlen (utf8String), &size);
	return (CCoord)size.cx;
}

//-----------------------------------------------------------------------------
double GdiFont::getAscent () const
{
	return -1;
}

//-----------------------------------------------------------------------------
double GdiFont::getDescent () const
{
	return -1;
}

//-----------------------------------------------------------------------------
double GdiFont::getLeading () const
{
	return -1;
}

//-----------------------------------------------------------------------------
double GdiFont::getCapHeight () const
{
	return -1;
}


#endif // GDIPLUS

END_NAMESPACE_VSTGUI

#endif // WINDOWS
