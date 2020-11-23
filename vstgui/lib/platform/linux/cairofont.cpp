// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cairofont.h"
#include "../../../lib/cstring.h"
#include "cairocontext.h"
#include "linuxstring.h"
#include "linuxfactory.h"
#include <cairo/cairo-ft.h>
#include <fontconfig/fontconfig.h>
#include <freetype2/ft2build.h>
#include <map>
#include <set>
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

	FreeTypeFontFace createFromPath (const std::string& path, int index);

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
	CairoFontFace (const std::string& path, int index) : path (path), index (index) {}

	CairoFontFace (const CairoFontFace& o) { assert (false); }
	CairoFontFace& operator= (const CairoFontFace& o) = delete;

	CairoFontFace (CairoFontFace&& o) noexcept { *this = std::move (o); }
	CairoFontFace& operator= (CairoFontFace&& o) noexcept
	{
		std::swap (ftFace, o.ftFace);
		std::swap (face, o.face);
		std::swap (path, o.path);
		std::swap (index, o.index);
		return *this;
	}

	operator cairo_font_face_t* () const
	{
		if (!face && !path.empty ())
		{
			ftFace = FreeType::instance ().createFromPath (path, index);
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
	int index = 0;
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

	FcConfig* getFontConfig () { return fcConfig; }

	using Fonts = std::map<std::pair<std::string, int>, CairoFontFace>;

	const Fonts& getFonts () const { return fonts; }

	CairoFontFace& createFont (const std::string& file, int index)
	{
		auto key = std::make_pair (file, index);
		auto it = fonts.find (key);
		if (it == fonts.end ())
		{
			auto value = std::make_pair (key, CairoFontFace (file, index));
			it = fonts.insert (std::move (value)).first;
		}
		return it->second;
	}

	void clear () { fonts.clear (); }

	bool queryFont (UTF8StringPtr name, CCoord size, int32_t style, std::string* fileStr,
					int* indexInt)
	{
		bool found = false;
		if (!fcConfig)
			return false;
		FcPattern* pattern = nullptr;
		if ((pattern = FcPatternCreate ()) &&
			FcPatternAddString (pattern, FC_FAMILY, reinterpret_cast<const FcChar8*> (name)) &&
			FcPatternAddInteger (pattern, FC_SLANT, slantFromStyle (style)) &&
			FcPatternAddInteger (pattern, FC_WEIGHT, weightFromStyle (style)) &&
			FcConfigSubstitute (fcConfig, pattern, FcMatchPattern))
		{
			FcDefaultSubstitute (pattern);
			FcResult result;
			FcPattern* font = FcFontMatch (fcConfig, pattern, &result);
			if (result == FcResultMatch)
			{
				FcChar8* file;
				int index;
				if (FcPatternGetString (font, FC_FILE, 0, &file) == FcResultMatch &&
					FcPatternGetInteger (font, FC_INDEX, 0, &index) == FcResultMatch)
				{
					if (fileStr)
						fileStr->assign (reinterpret_cast<char*> (file));
					if (indexInt)
						*indexInt = index;
					found = true;
				}
			}
			if (font)
				FcPatternDestroy (font);
		}
		if (pattern)
			FcPatternDestroy (pattern);
		return found;
	}

	bool getAllFontFamilies (const FontFamilyCallback& callback)
	{
		if (!fcConfig)
			return false;
		std::set<std::string> fontFamilyKnown;
		FcPattern* pattern = nullptr;
		FcObjectSet* objectSet = nullptr;
		FcFontSet* fontList = nullptr;
		if ((pattern = FcPatternCreate ()) &&
			(objectSet = FcObjectSetBuild (FC_FAMILY, FC_FILE, FC_STYLE, nullptr)) &&
			(fontList = FcFontList (fcConfig, pattern, objectSet)))
		{
			for (int i = 0; i < fontList->nfont; ++i)
			{
				auto* font = fontList->fonts[i];
				FcChar8* family;
				if (FcPatternGetString (font, FC_FAMILY, 0, &family) == FcResultMatch)
				{
					std::string familyStr (reinterpret_cast<const char*> (family));
					if (fontFamilyKnown.insert (familyStr).second)
					{
						if (!callback (familyStr))
							break;
					}
				}
			}
		}
		if (fontList)
			FcFontSetDestroy (fontList);
		if (objectSet)
			FcObjectSetDestroy (objectSet);
		if (pattern)
			FcPatternDestroy (pattern);
		return true;
	}

private:
	FontList ()
	{
		if (!FcInit ())
			return;

		fcConfig = FcInitLoadConfigAndFonts ();
		if (!fcConfig)
			return;

		if (auto linuxFactory = getPlatformFactory ().asLinuxFactory ())
		{
			auto resPath = linuxFactory->getResourcePath ();
			if (!resPath.empty ())
			{
				auto fontDir = resPath + "Fonts/";
				FcConfigAppFontAddDir (fcConfig,
									   reinterpret_cast<const FcChar8*> (fontDir.data ()));
			}
		}
	}

	~FontList ()
	{
		if (fcConfig)
			FcConfigDestroy (fcConfig);
	}

	FontList (const FontList&) = delete;
	FontList& operator= (const FontList&) = delete;

	FcConfig* fcConfig = nullptr;
	Fonts fonts;

	static int slantFromStyle (int32_t style)
	{
		return (style & kItalicFace) ? FC_SLANT_ITALIC : FC_SLANT_ROMAN;
	}

	static int weightFromStyle (int32_t style)
	{
		return (style & kBoldFace) ? FC_WEIGHT_BOLD : FC_WEIGHT_REGULAR;
	}
};

//------------------------------------------------------------------------
FreeType& FreeType::instance ()
{
	static FreeType gInstance;
	return gInstance;
}

//------------------------------------------------------------------------
FreeTypeFontFace FreeType::createFromPath (const std::string& path, int index)
{
	FT_Face face {nullptr};
	FT_New_Face (library, path.data (), index, &face);
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

	auto& fontList = FontList::instance ();

	bool found = false;
	std::string file;
	int index {};

	static constexpr auto defaultNames = {"Liberation Sans", "Noto Sans", "Ubuntu", "FreeSans"};

	for (int numTry = 0; !found && numTry < 2; ++numTry)
	{
		int32_t tryStyle = (numTry == 0) ? style : 0;
		if (name[0] != '\0')
			found = fontList.queryFont (name, size, tryStyle, &file, &index);
		if (!found)
		{
			for (auto& defName : defaultNames)
			{
				found = fontList.queryFont (defName, size, tryStyle, &file, &index);
				if (found)
					break;
			}
		}
	}

	if (found)
	{
		CairoFontFace& face = fontList.createFont (file, index);
		cairo_matrix_t matrix, ctm;
		cairo_matrix_init_scale (&matrix, size, size);
		cairo_matrix_init_identity (&ctm);
		auto options = cairo_font_options_create ();
		cairo_font_options_set_hint_style (options, CAIRO_HINT_STYLE_NONE);
		cairo_font_options_set_hint_metrics (options, CAIRO_HINT_METRICS_ON);
		impl->font = ScaledFontHandle (cairo_scaled_font_create (face, &matrix, &ctm, options));
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
Font::~Font () {}

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
bool Font::getAllFamilies (const FontFamilyCallback& callback)
{
	return Cairo::FontList::instance ().getAllFontFamilies (callback);
}

//------------------------------------------------------------------------
} // Cairo
} // VSTGUI
