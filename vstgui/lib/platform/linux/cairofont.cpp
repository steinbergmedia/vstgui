// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cairofont.h"
#include "../../../lib/cstring.h"
#include "cairocontext.h"
#include "linuxstring.h"
#include "linuxfactory.h"
#include <pango/pangocairo.h>
#include <pango/pango-features.h>
#include <pango/pangofc-fontmap.h>
#include <fontconfig/fontconfig.h>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Cairo {
namespace {

using PangoFontHandle =
	Handle<PangoFont*, decltype (&g_object_ref), g_object_ref,
		   decltype (&g_object_unref), g_object_unref>;

//------------------------------------------------------------------------
class FontList
{
public:
	static FontList& instance ()
	{
		static FontList gInstance;
		return gInstance;
	}

	FcConfig* getFontConfig ()
	{
		return fcConfig;
	}

	PangoFontMap* getFontMap ()
	{
		return fontMap;
	}

	PangoContext* getFontContext ()
	{
		return fontContext;
	}

	bool queryFont (UTF8StringPtr name, CCoord size, int32_t style, PangoFontHandle& fontHandle)
	{
		PangoFontDescription* desc = pango_font_description_new ();
		pango_font_description_set_family_static (desc, name);
		pango_font_description_set_absolute_size (desc, pango_units_from_double (size));
		if (style & kItalicFace)
			pango_font_description_set_style (desc, PANGO_STYLE_ITALIC);
		if (style & kBoldFace)
			pango_font_description_set_weight (desc, PANGO_WEIGHT_BOLD);
		PangoFont* font = pango_font_map_load_font (fontMap, fontContext, desc);
		pango_font_description_free (desc);
		if (font)
			fontHandle.assign (font);
		return font != nullptr;
	}

	bool getAllFontFamilies(const FontFamilyCallback& callback)
	{
		if (!fontContext)
			return false;
		PangoFontFamily** families = nullptr;
		int numFamilies = 0;
		pango_context_list_families (fontContext, &families, &numFamilies);
		for (int i = 0; i < numFamilies; ++i)
		{
			PangoFontFamily* family = families[i];
			if (!callback (pango_font_family_get_name (family)))
				break;
		}
		g_free (families);
		return true;
	}

private:
	FontList ()
	{
		fontMap = pango_cairo_font_map_new ();
		fontContext = pango_font_map_create_context (fontMap);

		PangoFcFontMap *fcMap = G_TYPE_CHECK_INSTANCE_CAST (fontMap, PANGO_TYPE_FC_FONT_MAP, PangoFcFontMap);
		if (fcMap && FcInit () && (fcConfig = FcInitLoadConfigAndFonts ()))
		{
			if (auto linuxFactory = getPlatformFactory ().asLinuxFactory ())
			{
				const UTF8String& resourcePath = linuxFactory->getResourcePath ();
				if (!resourcePath.empty ())
				{
					auto fontDir = resourcePath + "Fonts/";
					FcConfigAppFontAddDir (fcConfig, reinterpret_cast<const FcChar8*> (fontDir.data ()));
				}
				pango_fc_font_map_set_config (fcMap, fcConfig);
				FcConfigDestroy (fcConfig);
			}
		}
	}

	~FontList ()
	{
		if (fontMap)
			g_object_unref (fontMap);
		if (fontContext)
			g_object_unref (fontContext);
	}

	FontList (const FontList&) = delete;
	FontList& operator= (const FontList&) = delete;

	FcConfig* fcConfig = nullptr;
	PangoFontMap* fontMap = nullptr;
	PangoContext* fontContext = nullptr;

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
} // anonymous

//------------------------------------------------------------------------
struct Font::Impl
{
	PangoFontHandle font;
	int32_t style;
	CCoord ascent {-1.};
	CCoord descent {-1.};
	CCoord leading {-1.};
	CCoord capHeight {-1.};
};

//------------------------------------------------------------------------
Font::Font (UTF8StringPtr name, const CCoord& size, const int32_t& style)
{
	impl = std::unique_ptr<Impl> (new Impl);

	auto& fontList = FontList::instance ();

	if (fontList.queryFont (name, size, style, impl->font))
	{
		PangoFontMetrics* metrics = pango_font_get_metrics (impl->font, nullptr);
		if (metrics)
		{
			impl->ascent = pango_units_to_double (pango_font_metrics_get_ascent (metrics));
			impl->descent = pango_units_to_double (pango_font_metrics_get_descent (metrics));
			pango_font_metrics_unref (metrics);
		}

		PangoContext* context = fontList.getFontContext();
		if (context)
		{
			PangoLayout* layout = pango_layout_new (context);
			if (layout)
			{
				PangoFontDescription* desc = pango_font_describe (impl->font);
				if (desc)
				{
					pango_layout_set_font_description (layout, desc);
					pango_font_description_free (desc);
				}

				pango_layout_set_text (layout, "M", -1);

				PangoRectangle inkExtents {};
				pango_layout_get_pixel_extents (layout, &inkExtents, nullptr);
				impl->capHeight = inkExtents.height;

				g_object_unref (layout);
			}
		}
	}

	impl->style = style;
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
	return impl->ascent;
}

//------------------------------------------------------------------------
double Font::getDescent () const
{
	return impl->descent;
}

//------------------------------------------------------------------------
double Font::getLeading () const
{
#warning TODO: Implementation
	return impl->leading;
}

//------------------------------------------------------------------------
double Font::getCapHeight () const
{
	return impl->capHeight;
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

				PangoContext* context = FontList::instance ().getFontContext();
				if (context)
				{
					PangoLayout* layout = pango_layout_new (context);
					if (layout)
					{
						if (impl->font)
						{
							PangoFontDescription* desc = pango_font_describe (impl->font);
							if (desc)
							{
								pango_layout_set_font_description (layout, desc);
								pango_font_description_free (desc);
							}
						}

						PangoAttrList* attrs = pango_attr_list_new ();
						if (attrs)
						{
							if (impl->style & kUnderlineFace)
								pango_attr_list_insert (attrs, pango_attr_underline_new (PANGO_UNDERLINE_SINGLE));
							if (impl->style & kStrikethroughFace)
								pango_attr_list_insert (attrs, pango_attr_strikethrough_new (true));
							pango_layout_set_attributes(layout, attrs);
							pango_attr_list_unref (attrs);
						}

						pango_layout_set_text (layout, linuxString->get ().c_str (), -1);

						PangoRectangle extents {};
						pango_layout_get_pixel_extents (layout, nullptr, &extents);

						PangoLayoutIter* iter = pango_layout_get_iter (layout);
						CCoord baseline = 0.0;
						if (iter)
						{
							baseline = pango_units_to_double (pango_layout_iter_get_baseline (iter));
							pango_layout_iter_free (iter);
						}

						cairo_move_to (cr, p.x + extents.x, p.y + extents.y - baseline);
						pango_cairo_show_layout (cr, layout);
						g_object_unref (layout);
					}
				}
			}
		}
	}
}

//------------------------------------------------------------------------
CCoord Font::getStringWidth (CDrawContext* context, IPlatformString* string, bool antialias) const
{
	if (auto linuxString = dynamic_cast<LinuxString*> (string))
	{
		int pangoWidth = 0;
		PangoContext* context = FontList::instance ().getFontContext();
		if (context)
		{
			PangoLayout* layout = pango_layout_new (context);
			if (layout)
			{
				if (impl->font)
				{
					PangoFontDescription* desc = pango_font_describe (impl->font);
					if (desc)
					{
						pango_layout_set_font_description (layout, desc);
						pango_font_description_free (desc);
					}
				}
				pango_layout_set_text (layout, linuxString->get ().c_str (), -1);
				pango_layout_get_pixel_size (layout, &pangoWidth, nullptr);
				g_object_unref (layout);
			}
		}

		return pangoWidth;
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
