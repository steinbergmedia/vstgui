// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cairofont.h"
#include "../../../lib/cstring.h"
#include "cairocontext.h"
#include "linuxstring.h"
#include "x11frame.h"
#include <cairo/cairo-ft.h>
#include <fontconfig/fontconfig.h>
#include <freetype2/ft2build.h>
#include <unordered_map>
#include <cassert>

#include FT_FREETYPE_H

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Cairo {
namespace {

struct FreeTypeFontFace;
//------------------------------------------------------------------------
class FreeType
{
public:
	static FreeType& instance ();

	FreeTypeFontFace createFromPath (const std::string& path);

private:
	FreeType ();
	~FreeType ();

	FT_Library library {nullptr};
};

//------------------------------------------------------------------------
struct FreeTypeFontFace
{
	FreeTypeFontFace (FT_Face face = nullptr) : face (face) {}
	~FreeTypeFontFace () { destroy (); }

	FreeTypeFontFace (const FreeTypeFontFace&) = delete;
	FreeTypeFontFace& operator= (const FreeTypeFontFace& o) = delete;

	FreeTypeFontFace (FreeTypeFontFace&& o) { *this = std::move (o); }

	FreeTypeFontFace& operator= (FreeTypeFontFace&& o)
	{
		destroy ();
		std::swap (face, o.face);
		return *this;
	}

	bool valid () const { return face != nullptr; }
	operator FT_Face () const { return face; }
private:
	void destroy ()
	{
		if (face)
			FT_Done_Face (face);
		face = nullptr;
	}

	FT_Face face {nullptr};
};

using CairoFontFaceHandle =
	Handle<cairo_font_face_t*, decltype (&cairo_font_face_reference), cairo_font_face_reference,
		   decltype (&cairo_font_face_destroy), cairo_font_face_destroy>;

//------------------------------------------------------------------------
struct CairoFontFace
{
	CairoFontFace () noexcept {}
	CairoFontFace (const std::string& path) : path (path) {}

	CairoFontFace (const CairoFontFace& o) { assert (false); }
	CairoFontFace& operator= (const CairoFontFace& o) = delete;

	CairoFontFace (CairoFontFace&& o) noexcept { *this = std::move (o); }
	CairoFontFace& operator= (CairoFontFace&& o) noexcept
	{
		std::swap (ftFace, o.ftFace);
		std::swap (face, o.face);
		std::swap (path, o.path);
		return *this;
	}

	operator cairo_font_face_t* () const
	{
		if (!face && !path.empty ())
		{
			ftFace = FreeType::instance ().createFromPath (path);
			if (ftFace.valid ())
			{
				face = CairoFontFaceHandle (cairo_ft_font_face_create_for_ft_face (ftFace, 0));
			}
		}
		return face;
	}

private:
	mutable FreeTypeFontFace ftFace;
	mutable CairoFontFaceHandle face;
	std::string path;
};

//------------------------------------------------------------------------
class FontList
{
public:
	static FontList& instance ()
	{
		static FontList gInstance;
		return gInstance;
	}

	struct FontFamily
	{
		std::unordered_map<std::string, CairoFontFace> styles;
	};

	using Fonts = std::unordered_map<std::string, FontFamily>;

	std::vector<std::string> fontFamilyNames ()
	{
		std::vector<std::string> result;
		for (auto e : fonts)
			result.push_back (e.first);
		return result;
	}

	const Fonts& getFonts () const { return fonts; }

	void clear () { fonts.clear (); }

private:
	FontList ()
	{
		FcInit ();
		auto config = FcInitLoadConfigAndFonts ();
		if (!X11::Frame::resourcePath.empty ())
		{
			auto fontDir = X11::Frame::resourcePath + "Fonts/";
			FcConfigAppFontAddDir (config, reinterpret_cast<const FcChar8*> (fontDir.data ()));
		}

		auto pattern = FcPatternCreate ();
		auto objectSet = FcObjectSetBuild (FC_FAMILY, FC_FILE, FC_STYLE, nullptr);
		auto fontList = FcFontList (config, pattern, objectSet);
		for (auto i = 0; i < fontList->nfont; ++i)
		{
			auto font = fontList->fonts[i];
			FcChar8* family;
			FcChar8* file;
			FcChar8* style;
			if (FcPatternGetString (font, FC_FAMILY, 0, &family) == FcResultMatch &&
				FcPatternGetString (font, FC_FILE, 0, &file) == FcResultMatch &&
				FcPatternGetString (font, FC_STYLE, 0, &style) == FcResultMatch)
			{
				std::string familyStr (reinterpret_cast<const char*> (family));
				std::string fileStr (reinterpret_cast<const char*> (file));
				std::string styleStr (reinterpret_cast<const char*> (style));
				auto it = fonts.find (familyStr);
				if (it == fonts.end ())
				{
					FontFamily fam;
					fam.styles.emplace (styleStr, CairoFontFace {fileStr});
					fonts.emplace (familyStr, std::move (fam));
				}
				else
					it->second.styles.emplace (styleStr, CairoFontFace {fileStr});
			}
		}
		FcFontSetDestroy (fontList);
		FcObjectSetDestroy (objectSet);
		FcPatternDestroy (pattern);
		FcConfigDestroy (config);
	}

	~FontList () {}

	Fonts fonts;
};

//------------------------------------------------------------------------
FreeType& FreeType::instance ()
{
	static FreeType gInstance;
	return gInstance;
}

//------------------------------------------------------------------------
FreeTypeFontFace FreeType::createFromPath (const std::string& path)
{
	FT_Face face {nullptr};
	FT_New_Face (library, path.data (), 0, &face);
	return FreeTypeFontFace (face);
}

//------------------------------------------------------------------------
FreeType::FreeType ()
{
	if (FT_Init_FreeType (&library) != 0)
	{
		vstgui_assert (false, "Could not initialize FreeType");
	}
}

//------------------------------------------------------------------------
FreeType::~FreeType ()
{
	FontList::instance ().clear ();
	if (library)
		FT_Done_FreeType (library);
}

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
struct Font::Impl
{
	ScaledFontHandle font;
	cairo_font_extents_t extents {};
};

//------------------------------------------------------------------------
Font::Font (UTF8StringPtr name, const CCoord& size, const int32_t& style)
{
	impl = std::unique_ptr<Impl> (new Impl);
	auto& map = FontList::instance ().getFonts ();
	auto it = map.find (name);
	if (it == map.end ())
	{
		static constexpr auto defaults = {"Liberation Sans", "Noto Sans", "Ubuntu", "FreeSans"};
		for (auto& defName : defaults)
		{
			it = map.find (defName); // default font
			if (it != map.end ())
				break;
		}
	}
	if (it != map.end ())
	{
		cairo_matrix_t matrix, ctm;
		cairo_matrix_init_scale (&matrix, size, size);
		cairo_matrix_init_identity (&ctm);
		auto options = cairo_font_options_create ();
		cairo_font_options_set_hint_style (options, CAIRO_HINT_STYLE_NONE);
		cairo_font_options_set_hint_metrics (options, CAIRO_HINT_METRICS_ON);

		auto styleIt = it->second.styles.find ("Regular");
		if (style & kBoldFace)
		{
			if (style & kItalicFace)
				styleIt = it->second.styles.find ("Bold Italic");
			else
				styleIt = it->second.styles.find ("Bold");
		}
		else if (style & kItalicFace)
		{
			styleIt = it->second.styles.find ("Italic");
		}
		if (styleIt == it->second.styles.end ())
			styleIt = it->second.styles.find ("Regular");
		if (styleIt == it->second.styles.end ())
			styleIt = it->second.styles.begin ();
		if (styleIt != it->second.styles.end ())
		{
			impl->font = ScaledFontHandle (
				cairo_scaled_font_create (styleIt->second, &matrix, &ctm, options));
		}
		cairo_font_options_destroy (options);
		auto status = cairo_scaled_font_status (impl->font);
		if (status != CAIRO_STATUS_SUCCESS)
		{
			impl->font.reset ();
		}
		else if (impl->font)
		{
			cairo_scaled_font_extents (impl->font, &impl->extents);
		}
	}
}

//------------------------------------------------------------------------
Font::~Font ()
{
}

//------------------------------------------------------------------------
bool Font::valid () const
{
	return impl->font;
}

//------------------------------------------------------------------------
double Font::getAscent () const
{
	return impl->extents.ascent;
}

//------------------------------------------------------------------------
double Font::getDescent () const
{
	return impl->extents.descent;
}

//------------------------------------------------------------------------
double Font::getLeading () const
{
#warning TODO: Implementation
	return -1.;
}

//------------------------------------------------------------------------
double Font::getCapHeight () const
{
#warning TODO: Implementation
	return -1.;
}

//------------------------------------------------------------------------
const IFontPainter* Font::getPainter () const
{
	return this;
}

//------------------------------------------------------------------------
void Font::drawString (CDrawContext* context, IPlatformString* string, const CPoint& p,
					   bool antialias) const
{
	if (auto cairoContext = dynamic_cast<Context*> (context))
	{
		if (auto cd = DrawBlock::begin (*cairoContext))
		{
			if (auto linuxString = dynamic_cast<LinuxString*> (string))
			{
				auto color = cairoContext->getFontColor ();
				const auto& cr = cairoContext->getCairo ();
				auto alpha = color.normAlpha<double> () * cairoContext->getGlobalAlpha ();
				cairo_set_source_rgba (cr, color.normRed<double> (), color.normGreen<double> (),
				                       color.normBlue<double> (), alpha);
				cairo_move_to (cr, p.x, p.y);
				cairo_set_scaled_font (cr, impl->font);
				cairo_show_text (cr, linuxString->get ().data ());
			}
		}
	}
}

//------------------------------------------------------------------------
CCoord Font::getStringWidth (CDrawContext* context, IPlatformString* string, bool antialias) const
{
	if (auto linuxString = dynamic_cast<LinuxString*> (string))
	{
		cairo_text_extents_t e;
		cairo_scaled_font_text_extents (impl->font, linuxString->get ().data (), &e);
		return e.x_advance;
	}
	return 0;
}

//------------------------------------------------------------------------
} // Cairo

//------------------------------------------------------------------------
SharedPointer<IPlatformFont> IPlatformFont::create (const UTF8String& name, const CCoord& size, const int32_t& style)
{
	auto font = owned (new Cairo::Font (name, size, style));
	if (!font->valid ())
		font = nullptr;
	return font;
}

//------------------------------------------------------------------------
bool IPlatformFont::getAllPlatformFontFamilies (std::list<std::string>& fontFamilyNames)
{
	auto& map = Cairo::FontList::instance ().getFonts ();
	for (auto& e : map)
		fontFamilyNames.push_back (e.first);
	return true;
}

//------------------------------------------------------------------------
} // VSTGUI
