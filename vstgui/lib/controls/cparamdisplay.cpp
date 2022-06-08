// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cparamdisplay.h"
#include "../cbitmap.h"
#include "../cframe.h"
#include "../cstring.h"
#include "../cgraphicspath.h"
#include "../cdrawcontext.h"
#include <string>

namespace VSTGUI {

//------------------------------------------------------------------------
// CParamDisplay
//------------------------------------------------------------------------
/*! @class CParamDisplay
Define a rectangle view where a text-value can be displayed with a given font and color.
The user can specify its convert function (from float to char) by default the string format is "%2.2f".
The text-value is centered in the given rect.
*/
CParamDisplay::CParamDisplay (const CRect& size, CBitmap* background, int32_t inStyle)
: CControl (size, nullptr, -1, background)
, horiTxtAlign (kCenterText)
, style (inStyle)
, valuePrecision (2)
, roundRectRadius (6.)
, frameWidth (1.)
, textRotation (0.)
{
	setBit (style, kAntialias, true);
	backOffset (0, 0);

	fontID      = kNormalFont; fontID->remember ();
	fontColor   = kWhiteCColor;
	backColor   = kBlackCColor;
	frameColor  = kBlackCColor;
	shadowColor = kRedCColor;
	if (hasBit (style, kNoDrawStyle))
		setDirty (false);
}

//------------------------------------------------------------------------
CParamDisplay::CParamDisplay (const CParamDisplay& v)
: CControl (v)
, valueToStringFunction (v.valueToStringFunction)
, horiTxtAlign (v.horiTxtAlign)
, style (v.style)
, valuePrecision (v.valuePrecision)
, fontID (v.fontID)
, fontColor (v.fontColor)
, backColor (v.backColor)
, frameColor (v.frameColor)
, shadowColor (v.shadowColor)
, textInset (v.textInset)
, backOffset (v.backOffset)
, roundRectRadius (v.roundRectRadius)
, frameWidth (v.frameWidth)
, textRotation (v.textRotation)
{
	fontID->remember ();
}

//------------------------------------------------------------------------
CParamDisplay::~CParamDisplay () noexcept
{
	if (fontID)
		fontID->forget ();
}

//------------------------------------------------------------------------
bool CParamDisplay::removed (CView* parent)
{
	return CControl::removed (parent);
}

//------------------------------------------------------------------------
void CParamDisplay::setStyle (int32_t val)
{
	setBit (val, kAntialias, hasBit (style, kAntialias));
	if (style != val)
	{
		style = val;
		drawStyleChanged ();
	}
}

//------------------------------------------------------------------------
int32_t CParamDisplay::getStyle () const
{
	auto tmp = style;
	setBit (tmp, kAntialias, false);
	return tmp;
}

//------------------------------------------------------------------------
void CParamDisplay::setPrecision (uint8_t precision)
{
	if (valuePrecision != precision)
	{
		valuePrecision = precision;
		drawStyleChanged ();
	}
}

//------------------------------------------------------------------------
void CParamDisplay::setValueToStringFunction2 (const ValueToStringFunction2& valueToStringFunc)
{
	valueToStringFunction = valueToStringFunc;
}

//------------------------------------------------------------------------
void CParamDisplay::setValueToStringFunction2 (ValueToStringFunction2&& valueToStringFunc)
{
	valueToStringFunction = std::move (valueToStringFunc);
}

//------------------------------------------------------------------------
void CParamDisplay::setValueToStringFunction (const ValueToStringFunction& func)
{
	if (!func)
	{
		setValueToStringFunction2 (nullptr);
		return;
	}
	setValueToStringFunction2 ([=] (float value, std::string& str, CParamDisplay* display) {
		char string[256];
		string[0] = 0;
		if (func (value, string, display))
		{
			str = string;
			return true;
		}
		return false;
	});
}

//------------------------------------------------------------------------
void CParamDisplay::setValueToStringFunction (ValueToStringFunction&& func)
{
	setValueToStringFunction (func);
}

//------------------------------------------------------------------------
bool CParamDisplay::getFocusPath (CGraphicsPath& outPath)
{
	if (wantsFocus ())
	{
        auto lineWidth = getFrameWidth ();
        if (lineWidth < 0.)
            lineWidth = 1.;
		CCoord focusWidth = getFrame ()->getFocusWidth ();
		CRect r (getViewSize ());
		if (hasBit (style, kRoundRectStyle))
		{
			r.inset (lineWidth / 2., lineWidth / 2.);
			outPath.addRoundRect (r, roundRectRadius);
			outPath.closeSubpath ();
			r.extend (focusWidth, focusWidth);
			outPath.addRoundRect (r, roundRectRadius);
		}
		else
		{
			r.inset (lineWidth / 2., lineWidth / 2.);
			outPath.addRect (r);
			r.extend (focusWidth, focusWidth);
			outPath.addRect (r);
		}
	}
	return true;
}

//------------------------------------------------------------------------
void CParamDisplay::draw (CDrawContext *pContext)
{
	if (hasBit (style, kNoDrawStyle))
		return;

	std::string string;

	bool converted = false;
	if (valueToStringFunction)
		converted = valueToStringFunction (value, string, this);
	if (!converted)
	{
		char tmp[255];
		char precisionStr[10];
		snprintf (precisionStr, 10, "%%.%hhuf", valuePrecision);
		snprintf (tmp, 255, precisionStr, value);
		string = tmp;
	}

	drawBack (pContext);
	drawPlatformText (pContext, UTF8String (string).getPlatformString ());
	setDirty (false);
}

//------------------------------------------------------------------------
void CParamDisplay::drawBack (CDrawContext* pContext, CBitmap* newBack)
{
	pContext->setDrawMode (kAliasing);
	auto lineWidth = getFrameWidth ();
	if (lineWidth < 0.)
		lineWidth = pContext->getHairlineSize ();
	if (newBack)
	{
		newBack->draw (pContext, getViewSize (), backOffset);
	}
	else if (getDrawBackground ())
	{
		getDrawBackground ()->draw (pContext, getViewSize (), backOffset);
	}
	else
	{
		if (!getTransparency ())
		{
			bool strokePath = !(hasBit (style, (k3DIn|k3DOut|kNoFrame)));
			pContext->setFillColor (backColor);
			if (hasBit (style, kRoundRectStyle))
			{
				CRect pathRect = getViewSize ();
				pathRect.inset (lineWidth/2., lineWidth/2.);
				SharedPointer<CGraphicsPath> path = owned (pContext->createRoundRectGraphicsPath (pathRect, roundRectRadius));
				if (path)
				{
					pContext->setDrawMode (kAntiAliasing);
					pContext->drawGraphicsPath (path, CDrawContext::kPathFilled);
					if (strokePath)
					{
						pContext->setLineStyle (kLineSolid);
						pContext->setLineWidth (lineWidth);
						pContext->setFrameColor (frameColor);
						pContext->drawGraphicsPath (path, CDrawContext::kPathStroked);
					}
				}
			}
			else
			{
				pContext->setDrawMode (kAntiAliasing);
				SharedPointer<CGraphicsPath> path = owned (pContext->createGraphicsPath ());
				if (path)
				{
					CRect frameRect = getViewSize ();
					if (strokePath)
						frameRect.inset (lineWidth/2., lineWidth/2.);
					path->addRect (frameRect);
					pContext->drawGraphicsPath (path, CDrawContext::kPathFilled);
					if (strokePath)
					{
						pContext->setLineStyle (kLineSolid);
						pContext->setLineWidth (lineWidth);
						pContext->setFrameColor (frameColor);
						pContext->drawGraphicsPath (path, CDrawContext::kPathStroked);
					}
				}
				else
				{
					pContext->drawRect (getViewSize (), kDrawFilled);
			
					if (strokePath)
					{
						CRect frameRect = getViewSize ();
						frameRect.inset (lineWidth/2., lineWidth/2.);

						pContext->setLineStyle (kLineSolid);
						pContext->setLineWidth (lineWidth);
						pContext->setFrameColor (frameColor);
						pContext->drawRect (frameRect);
					}
				}
			}
		}
	}
	// draw the frame for the 3D effect
	if (hasBit (style, (k3DIn|k3DOut)))
	{
		CRect r (getViewSize ());
		r.inset (lineWidth/2., lineWidth/2.);
		pContext->setDrawMode (kAliasing);
		pContext->setLineWidth (lineWidth);
		pContext->setLineStyle (kLineSolid);
		if (hasBit (style, k3DIn))
			pContext->setFrameColor (backColor);
		else
			pContext->setFrameColor (frameColor);

		CPoint p;
		SharedPointer<CGraphicsPath> path = owned (pContext->createGraphicsPath ());
		if (path)
		{
			path->beginSubpath (p (r.left, r.bottom));
			path->addLine (p (r.left, r.top));
			path->addLine (p (r.right, r.top));
			pContext->drawGraphicsPath (path, CDrawContext::kPathStroked);
		}
		else
		{
			pContext->drawLine (CPoint (r.left, r.bottom), CPoint (r.left, r.top));
			pContext->drawLine (CPoint (r.left, r.top), CPoint (r.right, r.top));
		}

		if (hasBit (style, k3DIn))
			pContext->setFrameColor (frameColor);
		else
			pContext->setFrameColor (backColor);

		path = owned (pContext->createGraphicsPath ());
		if (path)
		{
			path->beginSubpath (p (r.right, r.top));
			path->addLine (p (r.right, r.bottom));
			path->addLine (p (r.left, r.bottom));
			pContext->drawGraphicsPath (path, CDrawContext::kPathStroked);
		}
		else
		{
			pContext->drawLine (CPoint (r.right, r.top), CPoint (r.right, r.bottom));
			pContext->drawLine (CPoint (r.right, r.bottom), CPoint (r.left, r.bottom));
		}
	}
}

//------------------------------------------------------------------------
void CParamDisplay::drawPlatformText (CDrawContext* pContext, IPlatformString* string)
{
	drawPlatformText (pContext, string, getViewSize ());
}

//------------------------------------------------------------------------
void CParamDisplay::drawPlatformText (CDrawContext* pContext, IPlatformString* string, const CRect& size)
{
	if (!hasBit (style, kNoTextStyle))
	{
		pContext->saveGlobalState ();
		CRect textRect (size);
		textRect.inset (textInset.x, textInset.y);

		drawClipped (pContext, textRect, [&] () {
			CPoint center (textRect.getCenter ());
			CGraphicsTransform transform;
			transform.rotate (textRotation, center);
			CDrawContext::Transform ctxTransform (*pContext, transform);

			pContext->setDrawMode (kAntiAliasing);
			pContext->setFont (fontID);

			// draw darker text (as shadow)
			if (hasBit (style, kShadowText))
			{
				CRect newSize (textRect);
				newSize.offset (shadowTextOffset);
				pContext->setFontColor (shadowColor);
				pContext->drawString (string, newSize, horiTxtAlign, hasBit (style, kAntialias));
			}
			pContext->setFontColor (fontColor);
			pContext->drawString (string, textRect, horiTxtAlign, hasBit (style, kAntialias));
		});
		pContext->restoreGlobalState ();
	}
}

//------------------------------------------------------------------------
void CParamDisplay::setFont (CFontRef inFontID)
{
	if (fontID)
		fontID->forget ();
	fontID = inFontID;
	if (fontID)
		fontID->remember ();
	drawStyleChanged ();
}

//------------------------------------------------------------------------
void CParamDisplay::setFontColor (CColor color)
{
	// to force the redraw
	if (fontColor != color)
	{
		fontColor = color;
		drawStyleChanged ();
	}
}

//------------------------------------------------------------------------
void CParamDisplay::setBackColor (CColor color)
{
	// to force the redraw
	if (backColor != color)
	{
		backColor = color;
		drawStyleChanged ();
	}
}

//------------------------------------------------------------------------
void CParamDisplay::setFrameColor (CColor color)
{
	// to force the redraw
	if (frameColor != color)
	{
		frameColor = color;
		drawStyleChanged ();
	}
}

//------------------------------------------------------------------------
void CParamDisplay::setShadowColor (CColor color)
{
	// to force the redraw
	if (shadowColor != color)
	{
		shadowColor = color;
		drawStyleChanged ();
	}
}

//------------------------------------------------------------------------
void CParamDisplay::setShadowTextOffset (const CPoint& offset)
{
	if (shadowTextOffset != offset)
	{
		shadowTextOffset = offset;
		drawStyleChanged ();
	}
}

//------------------------------------------------------------------------
void CParamDisplay::setHoriAlign (CHoriTxtAlign hAlign)
{
	// to force the redraw
	if (horiTxtAlign != hAlign)
	{
		horiTxtAlign = hAlign;
		drawStyleChanged ();
	}
}

//------------------------------------------------------------------------
void CParamDisplay::setTextInset (const CPoint& p)
{
	if (textInset != p)
	{
		textInset = p;
		drawStyleChanged ();
	}
}

//------------------------------------------------------------------------
void CParamDisplay::setTextRotation (double angle)
{
	while (angle < 0.)
		angle += 360.;
	while (angle > 360.)
		angle -= 360.;
	if (textRotation != angle)
	{
		textRotation = angle;
		drawStyleChanged ();
	}
}

//------------------------------------------------------------------------
void CParamDisplay::setRoundRectRadius (const CCoord& radius)
{
	if (roundRectRadius != radius)
	{
		roundRectRadius = radius;
		drawStyleChanged ();
	}
}

//------------------------------------------------------------------------
void CParamDisplay::setFrameWidth (const CCoord& width)
{
	if (frameWidth != width)
	{
		frameWidth = width;
		drawStyleChanged ();
	}
}

//------------------------------------------------------------------------
void CParamDisplay::drawStyleChanged ()
{
	setDirty ();
}

//------------------------------------------------------------------------
void CParamDisplay::setBackOffset (const CPoint &offset)
{
	backOffset = offset;
}

//-----------------------------------------------------------------------------
void CParamDisplay::copyBackOffset ()
{
	backOffset (getViewSize ().left, getViewSize ().top);
}

} // VSTGUI
