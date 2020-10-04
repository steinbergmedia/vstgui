// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cstringlist.h"
#include "../cdrawcontext.h"
#include "../ccolor.h"
#include "../cfont.h"
#include "../platform/platformfactory.h"
#include "../platform/iplatformstring.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
struct StringListControlDrawer::Impl
{
	Func func;

	SharedPointer<CFontDesc> font {kNormalFont};
	CColor fontColor {kBlackCColor};
	CColor fontColorSelected {kWhiteCColor};
	CColor backColor {kWhiteCColor};
	CColor backColorSelected {kBlueCColor};
	CColor hoverColor {MakeCColor (0, 0, 0, 100)};
	CColor lineColor {kBlackCColor};
	CCoord lineWidth {1.};
	CCoord textInset {5.};
	CHoriTxtAlign textAlign {kLeftText};

};

//------------------------------------------------------------------------
StringListControlDrawer::StringListControlDrawer ()
{
	impl = std::unique_ptr<Impl> (new Impl);
}

//------------------------------------------------------------------------
StringListControlDrawer::~StringListControlDrawer () noexcept = default;

//------------------------------------------------------------------------
void StringListControlDrawer::setStringProvider (Func&& getStringFunc)
{
	impl->func = std::move (getStringFunc);
}

//------------------------------------------------------------------------
void StringListControlDrawer::setStringProvider (const Func& getStringFunc)
{
	impl->func = getStringFunc;
}

//------------------------------------------------------------------------
void StringListControlDrawer::setFont (CFontRef f)
{
	impl->font = f;
}

//------------------------------------------------------------------------
void StringListControlDrawer::setFontColor (CColor color)
{
	impl->fontColor = color;
}

//------------------------------------------------------------------------
void StringListControlDrawer::setSelectedFontColor (CColor color)
{
	impl->fontColorSelected = color;
}

//------------------------------------------------------------------------
void StringListControlDrawer::setBackColor (CColor color)
{
	impl->backColor = color;
}

//------------------------------------------------------------------------
void StringListControlDrawer::setSelectedBackColor (CColor color)
{
	impl->backColorSelected = color;
}

//------------------------------------------------------------------------
void StringListControlDrawer::setHoverColor (CColor color)
{
	impl->hoverColor = color;
}

//------------------------------------------------------------------------
void StringListControlDrawer::setLineColor (CColor color)
{
	impl->lineColor = color;
}

//------------------------------------------------------------------------
void StringListControlDrawer::setLineWidth (CCoord width)
{
	impl->lineWidth = width;
}

//------------------------------------------------------------------------
void StringListControlDrawer::setTextInset (CCoord inset)
{
	impl->textInset = inset;
}

//------------------------------------------------------------------------
void StringListControlDrawer::setTextAlign (CHoriTxtAlign align)
{
	impl->textAlign = align;
}

//------------------------------------------------------------------------
CFontRef StringListControlDrawer::getFont () const
{
	return impl->font;
}

//------------------------------------------------------------------------
CColor StringListControlDrawer::getFontColor () const
{
	return impl->fontColor;
}

//------------------------------------------------------------------------
CColor StringListControlDrawer::getSelectedFontColor () const
{
	return impl->fontColorSelected;
}

//------------------------------------------------------------------------
CColor StringListControlDrawer::getBackColor () const
{
	return impl->backColor;
}

//------------------------------------------------------------------------
CColor StringListControlDrawer::getSelectedBackColor () const
{
	return impl->backColorSelected;
}

//------------------------------------------------------------------------
CColor StringListControlDrawer::getHoverColor () const
{
	return impl->hoverColor;
}

//------------------------------------------------------------------------
CColor StringListControlDrawer::getLineColor () const
{
	return impl->lineColor;
}

//------------------------------------------------------------------------
CCoord StringListControlDrawer::getLineWidth () const
{
	return impl->lineWidth;
}

//------------------------------------------------------------------------
CCoord StringListControlDrawer::getTextInset () const
{
	return impl->textInset;
}

//------------------------------------------------------------------------
CHoriTxtAlign StringListControlDrawer::getTextAlign () const
{
	return impl->textAlign;
}

//------------------------------------------------------------------------
void StringListControlDrawer::drawBackground (CDrawContext* context, CRect size)
{
	context->setFillColor (impl->backColor);
	context->drawRect (size, kDrawFilled);
}

//------------------------------------------------------------------------
void StringListControlDrawer::drawRow (CDrawContext* context, CRect size, Row row)
{
	context->setDrawMode (kAntiAliasing);
	if (row.isHovered ())
	{
		context->setFillColor (impl->hoverColor);
		context->drawRect (size, kDrawFilled);
	}
	if (row.isSelected ())
	{
		context->setFillColor (impl->backColorSelected);
		context->drawRect (size, kDrawFilled);
	}

	auto lw = impl->lineWidth < 0. ? context->getHairlineSize () : impl->lineWidth;
	size.bottom -= lw * 0.5;

	if (!(row.isLastRow ()) && lw != 0.)
	{
		context->setDrawMode (kAntiAliasing | kNonIntegralMode);
		context->setFrameColor (impl->lineColor);
		context->setLineWidth (lw);
		context->drawLine (size.getBottomLeft (), size.getBottomRight ());
	}
	if (auto string = getString (row))
	{
		size.inset (impl->textInset, 0);
		context->setFontColor (row.isSelected () ? impl->fontColorSelected : impl->fontColor);
		context->setFont (impl->font);
		context->drawString (string, size, impl->textAlign);
	}
}

//------------------------------------------------------------------------
PlatformStringPtr StringListControlDrawer::getString (int32_t row) const
{
	return impl->func ? impl->func (row) : getPlatformFactory ().createString (toString (row));
}

//------------------------------------------------------------------------
} // VSTGUI
