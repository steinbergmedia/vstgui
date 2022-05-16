// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "d2dfont.h"

#if WINDOWS

#include "../win32support.h"
#include "../win32resourcestream.h"
#include "../win32factory.h"
#include "../winstring.h"
#include "../comptr.h"
#include "d2ddrawcontext.h"
#include <mutex>
#include <dwrite.h>
#include <d2d1.h>

#ifndef NTDDI_WIN10_RS2
#define NTDDI_WIN10_RS2 0x0A000003 /* ABRACADABRA_WIN10_RS2 */
#endif
#if !defined(VSTGUI_WIN32_CUSTOMFONT_SUPPORT) && WDK_NTDDI_VERSION >= NTDDI_WIN10_RS2
#include <dwrite_3.h>
#define VSTGUI_WIN32_CUSTOMFONT_SUPPORT 1
#else
#define VSTGUI_WIN32_CUSTOMFONT_SUPPORT 0
#pragma message( \
    "Warning: VSTGUI Custom Font support is only available when building with the Windows 10 Creator Update SDK or newer")
#endif

namespace VSTGUI {
namespace D2DFontPrivate {
//-----------------------------------------------------------------------------
struct CustomFonts
{
#if VSTGUI_WIN32_CUSTOMFONT_SUPPORT
	IDWriteFontCollection1* getFontCollection () { return fontCollection.get (); }
	bool contains (const WCHAR* name, DWRITE_FONT_WEIGHT fontWeight,
				   DWRITE_FONT_STRETCH fontStretch, DWRITE_FONT_STYLE fontStyle)
	{
		if (auto fs = fontSet.get ())
		{
			COM::Ptr<IDWriteFontSet> matchingFonts;
			if (SUCCEEDED (fs->GetMatchingFonts (name, fontWeight, fontStretch, fontStyle,
												 matchingFonts.adoptPtr ())))
				return matchingFonts->GetFontCount () > 0;
		}
		return false;
	}

	CustomFonts ()
	{
		auto winFactory = getPlatformFactory ().asWin32Factory ();
		if (!winFactory)
			return;
		auto basePath = winFactory->getResourceBasePath ();
		if (!basePath)
			return;
		*basePath += "Fonts\\*";
		auto files = getDirectoryContents (*basePath);
		if (files.empty ())
			return;
		auto factory = getDWriteFactory ();
		COM::Ptr<IDWriteFactory5> factory5;
		if (!factory || FAILED (factory->QueryInterface<IDWriteFactory5> (factory5.adoptPtr ())))
			return;
		COM::Ptr<IDWriteFontSetBuilder1> fontSetBuilder;
		if (FAILED (factory5->CreateFontSetBuilder (fontSetBuilder.adoptPtr ())))
			return;
		for (const auto& file : files)
		{
			COM::Ptr<IDWriteFontFile> fontFile;
			if (FAILED (factory5->CreateFontFileReference (file.data (), nullptr,
			                                               fontFile.adoptPtr ())))
				continue;
			fontSetBuilder->AddFontFile (fontFile.get ());
		}
		if (FAILED (fontSetBuilder->CreateFontSet (fontSet.adoptPtr ())))
			return;
		factory5->CreateFontCollectionFromFontSet (fontSet.get (), fontCollection.adoptPtr ());
	}

private:
	COM::Ptr<IDWriteFontSet> fontSet;
	COM::Ptr<IDWriteFontCollection1> fontCollection;

	std::vector<std::wstring> getDirectoryContents (const UTF8String& path) const
	{
		std::vector<std::wstring> result;
		UTF8StringHelper fontsDir (path);
		std::wstring basePath (fontsDir.getWideString ());
		WIN32_FIND_DATA findData {};
		auto find = FindFirstFile (basePath.data (), &findData);
		if (find == INVALID_HANDLE_VALUE)
			return result;
		basePath.erase (basePath.size () - 1);
		do
		{
			if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				result.emplace_back (basePath + findData.cFileName);
			}
		} while (FindNextFile (find, &findData) != 0);
		FindClose (find);
		return result;
	}
#else
	IDWriteFontCollection* getFontCollection () { return nullptr; }
	bool contains (const WCHAR*, DWRITE_FONT_WEIGHT, DWRITE_FONT_STRETCH, DWRITE_FONT_STYLE)
	{
		return false;
	}
#endif
};

//-----------------------------------------------------------------------------
static std::unique_ptr<CustomFonts> customFonts;
static std::once_flag customFontsOnceFlag;

//-----------------------------------------------------------------------------
static CustomFonts* getCustomFonts ()
{
	std::call_once (customFontsOnceFlag, [] () {
		customFonts = std::make_unique<CustomFonts> ();
	});
	return customFonts.get ();
}

//-----------------------------------------------------------------------------
static void gatherFonts (const FontFamilyCallback& callback, IDWriteFontCollection* collection)
{
	UINT32 numFonts = collection->GetFontFamilyCount ();
	for (UINT32 i = 0; i < numFonts; ++i)
	{
		COM::Ptr<IDWriteFontFamily> fontFamily;
		if (!SUCCEEDED (collection->GetFontFamily (i, fontFamily.adoptPtr ())))
			continue;
		COM::Ptr<IDWriteLocalizedStrings> names;
		if (!SUCCEEDED (fontFamily->GetFamilyNames (names.adoptPtr ())))
			continue;
		UINT32 nameLength = 0;
		if (!SUCCEEDED (names->GetStringLength (0, &nameLength)) || nameLength < 1)
			continue;
		nameLength++;
		auto name = std::unique_ptr<WCHAR[]> (new WCHAR[nameLength]);
		if (SUCCEEDED (names->GetString (0, name.get (), nameLength)))
		{
			UTF8StringHelper str (name.get ());
			callback (str.getUTF8String ());
		}
	}
}

//-----------------------------------------------------------------------------
static COM::Ptr<IDWriteFont> getFont (IDWriteTextFormat* format, int32_t style)
{
	DWRITE_FONT_STYLE fontStyle = (style & kItalicFace) ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;
	DWRITE_FONT_WEIGHT fontWeight = (style & kBoldFace) ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL;
	IDWriteFontCollection* fontCollection = nullptr;
	format->GetFontCollection (&fontCollection);
	if (!fontCollection)
		return {};
	auto nameLength = format->GetFontFamilyNameLength () + 1;
	auto familyName = std::unique_ptr<WCHAR[]> (new WCHAR [nameLength]);
	if (FAILED (format->GetFontFamilyName (familyName.get (), nameLength)))
		return {};
	UINT32 index = 0;
	BOOL exists = FALSE;
	if (FAILED (fontCollection->FindFamilyName (familyName.get (), &index, &exists)))
		return {};
	COM::Ptr<IDWriteFontFamily> fontFamily;
	fontCollection->GetFontFamily (index, fontFamily.adoptPtr ());
	if (fontFamily)
	{
		COM::Ptr<IDWriteFont> font;
		fontFamily->GetFirstMatchingFont (fontWeight, DWRITE_FONT_STRETCH_NORMAL, fontStyle, font.adoptPtr ());
		return font;
	}
	return {};
}

//-----------------------------------------------------------------------------
} // D2DFontPrivate

//-----------------------------------------------------------------------------
bool D2DFont::getAllFontFamilies (const FontFamilyCallback& callback)
{
	IDWriteFontCollection* collection = nullptr;
	if (SUCCEEDED (getDWriteFactory ()->GetSystemFontCollection (&collection, true)))
		D2DFontPrivate::gatherFonts (callback, collection);
	if (D2DFontPrivate::customFonts)
	{
		if (auto customFontCollection = D2DFontPrivate::customFonts->getFontCollection ())
			D2DFontPrivate::gatherFonts (callback, customFontCollection);
	}
	return true;
}

//-----------------------------------------------------------------------------
void D2DFont::terminate ()
{
	D2DFontPrivate::customFonts = nullptr;
}

//-----------------------------------------------------------------------------
D2DFont::D2DFont (const UTF8String& name, const CCoord& size, const int32_t& style)
: textFormat (nullptr)
, ascent (-1)
, descent (-1)
, leading (-1)
, capHeight (-1)
, style (style)
{
	DWRITE_FONT_STYLE fontStyle = (style & kItalicFace) ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;
	DWRITE_FONT_WEIGHT fontWeight = (style & kBoldFace) ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL;
	UTF8StringHelper nameStr (name.data ());

	IDWriteFontCollection* fontCollection = nullptr;
	auto customFonts = D2DFontPrivate::getCustomFonts ();
	if (customFonts && customFonts->contains (nameStr.getWideString (), fontWeight,
											  DWRITE_FONT_STRETCH_NORMAL, fontStyle))
		fontCollection = customFonts->getFontCollection ();

	getDWriteFactory ()->CreateTextFormat (nameStr, fontCollection, fontWeight, fontStyle,
	                                       DWRITE_FONT_STRETCH_NORMAL, (FLOAT)size, L"en-us",
	                                       &textFormat);
	if (textFormat)
	{
		if (auto font = D2DFontPrivate::getFont (textFormat, style))
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

//-----------------------------------------------------------------------------
D2DFont::~D2DFont ()
{
	if (textFormat)
		textFormat->Release ();
}

//-----------------------------------------------------------------------------
bool D2DFont::asLogFont (LOGFONTW& logfont) const
{
	if (!textFormat)
		return false;
	COM::Ptr<IDWriteGdiInterop> interOp;
	if (FAILED (getDWriteFactory ()->GetGdiInterop (interOp.adoptPtr ())))
		return false;
	if (auto font = D2DFontPrivate::getFont (textFormat, style))
	{
		BOOL isSystemFont;
		if (SUCCEEDED (interOp->ConvertFontToLOGFONT (font.get (), &logfont, &isSystemFont)))
		{
			logfont.lfHeight = -static_cast<LONG> (std::round (textFormat->GetFontSize ()));
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
IDWriteTextLayout* D2DFont::createTextLayout (IPlatformString* string) const
{
	const auto* winString = dynamic_cast<const WinString*> (string);
	IDWriteTextLayout* textLayout = nullptr;
	if (winString)
		getDWriteFactory ()->CreateTextLayout (winString->getWideString (), (UINT32)wcslen (winString->getWideString ()), textFormat, 10000, 1000, &textLayout);
	return textLayout;
}

//-----------------------------------------------------------------------------
void D2DFont::drawString (CDrawContext* context, IPlatformString* string, const CPoint& p, bool antialias) const
{
	auto* d2dContext = dynamic_cast<D2DDrawContext*> (context);
	if (d2dContext && textFormat)
	{
		D2DDrawContext::D2DApplyClip ac (d2dContext);
		if (ac.isEmpty ())
			return;
		ID2D1RenderTarget* renderTarget = d2dContext->getRenderTarget ();
		if (renderTarget)
		{
			IDWriteTextLayout* textLayout = createTextLayout (string);
			if (textLayout)
			{
				if (style & kUnderlineFace)
				{
					DWRITE_TEXT_RANGE range = { 0, UINT_MAX };
					textLayout->SetUnderline (true, range);
				}
				if (style & kStrikethroughFace)
				{
					DWRITE_TEXT_RANGE range = { 0, UINT_MAX };
					textLayout->SetStrikethrough (true, range);
				}
				renderTarget->SetTextAntialiasMode (antialias ? D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE : D2D1_TEXT_ANTIALIAS_MODE_ALIASED);

				CPoint pos (p);
				DWRITE_LINE_METRICS lm;
				UINT lineCount;
				if (SUCCEEDED (textLayout->GetLineMetrics (&lm, 1, &lineCount)))
					pos.y -= lm.baseline;
				else
					pos.y -= textFormat->GetFontSize ();

				if (context->getDrawMode ().integralMode ())
					pos.makeIntegral ();
				pos.y += 0.5;

				D2D1_POINT_2F origin = {(FLOAT)(p.x), (FLOAT)(pos.y)};
				d2dContext->getRenderTarget ()->DrawTextLayout (origin, textLayout, d2dContext->getFontBrush ());
				textLayout->Release ();
			}
		}
	}
}

//-----------------------------------------------------------------------------
CCoord D2DFont::getStringWidth (CDrawContext* context, IPlatformString* string, bool antialias) const
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

} // VSTGUI

#endif // WINDOWS
