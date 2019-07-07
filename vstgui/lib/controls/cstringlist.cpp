// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cstringlist.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
void StringListControlDrawer::setGetStringFunc (Func&& getStringFunc)
{
	func = std::move (getStringFunc);
}

//------------------------------------------------------------------------
void StringListControlDrawer::setGetStringFunc (const Func& getStringFunc)
{
	func = getStringFunc;
}

//------------------------------------------------------------------------
void StringListControlDrawer::setFont (CFontRef f)
{
	font = f;
}

//------------------------------------------------------------------------
void StringListControlDrawer::setFontColor (CColor color)
{
	fontColor = color;
}

//------------------------------------------------------------------------
void StringListControlDrawer::setSelectedFontColor (CColor color)
{
	fontColorSelected = color;
}

//------------------------------------------------------------------------
void StringListControlDrawer::setBackColor (CColor color)
{
	backColor = color;
}

//------------------------------------------------------------------------
void StringListControlDrawer::setSelectedBackColor (CColor color)
{
	backColorSelected = color;
}

//------------------------------------------------------------------------
void StringListControlDrawer::setHoverColor (CColor color)
{
	hoverColor = color;
}

//------------------------------------------------------------------------
void StringListControlDrawer::setLineColor (CColor color)
{
	lineColor = color;
}

//------------------------------------------------------------------------
void StringListControlDrawer::setLineWidth (CCoord width)
{
	lineWidth = width;
}

//------------------------------------------------------------------------
void StringListControlDrawer::setTextInset (CCoord inset)
{
	textInset = inset;
}

//------------------------------------------------------------------------
void StringListControlDrawer::setTextAlign (CHoriTxtAlign align)
{
	textAlign = align;
}

//------------------------------------------------------------------------
CFontRef StringListControlDrawer::getFont () const
{
	return font;
}

//------------------------------------------------------------------------
CColor StringListControlDrawer::getFontColor () const
{
	return fontColor;
}

//------------------------------------------------------------------------
CColor StringListControlDrawer::getSelectedFontColor () const
{
	return fontColorSelected;
}

//------------------------------------------------------------------------
CColor StringListControlDrawer::getBackColor () const
{
	return backColor;
}

//------------------------------------------------------------------------
CColor StringListControlDrawer::getSelectedBackColor () const
{
	return backColorSelected;
}

//------------------------------------------------------------------------
CColor StringListControlDrawer::getHoverColor () const
{
	return hoverColor;
}

//------------------------------------------------------------------------
CColor StringListControlDrawer::getLineColor () const
{
	return lineColor;
}

//------------------------------------------------------------------------
CCoord StringListControlDrawer::getLineWidth () const
{
	return lineWidth;
}

//------------------------------------------------------------------------
CCoord StringListControlDrawer::getTextInset () const
{
	return textInset;
}

//------------------------------------------------------------------------
CHoriTxtAlign StringListControlDrawer::getTextAlign () const
{
	return textAlign;
}

//------------------------------------------------------------------------
void StringListControlDrawer::drawBackground (CDrawContext* context, CRect size)
{
	context->setFillColor (backColor);
	context->drawRect (size, kDrawFilled);
}

//------------------------------------------------------------------------
void StringListControlDrawer::drawRow (CDrawContext* context, CRect size, int32_t row,
                                       int32_t flags)
{
	context->setDrawMode (kAntiAliasing);
	if (flags & Hovered)
	{
		context->setFillColor (hoverColor);
		context->drawRect (size, kDrawFilled);
	}
	if (flags & Selected)
	{
		context->setFillColor (backColorSelected);
		context->drawRect (size, kDrawFilled);
	}

	auto lw = lineWidth < 0. ? context->getHairlineSize () : lineWidth;
	size.bottom -= lw * 0.5;

	if (!(flags & LastRow) && lw != 0.)
	{
		context->setFrameColor (lineColor);
		context->setLineWidth (lw);
		context->drawLine (size.getBottomLeft (), size.getBottomRight ());
	}
	SharedPointer<IPlatformString> string;
	if (func)
		string = func (row);
	else
		string = IPlatformString::createWithUTF8String (toString (row));
	if (string)
	{
		size.inset (textInset, 0);
		context->setFontColor (flags & Selected ? fontColorSelected : fontColor);
		context->setFont (font);
		context->drawString (string, size, textAlign);
	}
}

//------------------------------------------------------------------------
} // VSTGUI
