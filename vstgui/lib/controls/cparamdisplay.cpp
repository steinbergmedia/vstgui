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
CParamDisplay::CParamDisplay (const CRect& size, CBitmap* background, const int32_t style)
: CControl (size, nullptr, -1, background)
, horiTxtAlign (kCenterText)
, style (style)
, valuePrecision (2)
, roundRectRadius (6.)
, frameWidth (1.)
, textRotation (0.)
, bAntialias (true)
{
	backOffset (0, 0);

	fontID      = kNormalFont; fontID->remember ();
	fontColor   = kWhiteCColor;
	backColor   = kBlackCColor;
	frameColor  = kBlackCColor;
	shadowColor = kRedCColor;
	if (style & kNoDrawStyle)
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
, roundRectRadius (v.roundRectRadius)
, frameWidth (v.frameWidth)
, textRotation (v.textRotation)
, bAntialias (v.bAntialias)
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
	if (style != val)
	{
		style = val;
		drawStyleChanged ();
	}
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
		CCoord focusWidth = getFrame ()->getFocusWidth ();
		CRect r (getViewSize ());
		if (style & kRoundRectStyle)
		{
			r.inset (getFrameWidth () / 2., getFrameWidth () / 2.);
			outPath.addRoundRect (r, roundRectRadius);
			outPath.closeSubpath ();
			r.extend (focusWidth, focusWidth);
			outPath.addRoundRect (r, roundRectRadius);
		}
		else
		{
			r.inset (getFrameWidth () / 2., getFrameWidth () / 2.);
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
	if (style & kNoDrawStyle)
		return;

	std::string string;

	bool converted = false;
	if (valueToStringFunction)
		converted = valueToStringFunction (value, string, this);
	if (!converted)
	{
		char tmp[255];
		char precisionStr[10];
		sprintf (precisionStr, "%%.%hhuf", valuePrecision);
		sprintf (tmp, precisionStr, value);
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
			bool strokePath = !(style & (k3DIn|k3DOut|kNoFrame));
			pContext->setFillColor (backColor);
			if (style & kRoundRectStyle)
			{
				CRect pathRect = getViewSize ();
				pathRect.inset (frameWidth/2., frameWidth/2.);
				SharedPointer<CGraphicsPath> path = owned (pContext->createRoundRectGraphicsPath (pathRect, roundRectRadius));
				if (path)
				{
					pContext->setDrawMode (kAntiAliasing);
					pContext->drawGraphicsPath (path, CDrawContext::kPathFilled);
					if (strokePath)
					{
						pContext->setLineStyle (kLineSolid);
						pContext->setLineWidth (frameWidth);
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
						frameRect.inset (frameWidth/2., frameWidth/2.);
					path->addRect (frameRect);
					pContext->drawGraphicsPath (path, CDrawContext::kPathFilled);
					if (strokePath)
					{
						pContext->setLineStyle (kLineSolid);
						pContext->setLineWidth (frameWidth);
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
						frameRect.inset (frameWidth/2., frameWidth/2.);

						pContext->setLineStyle (kLineSolid);
						pContext->setLineWidth (frameWidth);
						pContext->setFrameColor (frameColor);
						pContext->drawRect (frameRect);
					}
				}
			}
		}
	}
	// draw the frame for the 3D effect
	if (style & (k3DIn|k3DOut)) 
	{
		CRect r (getViewSize ());
		r.inset (frameWidth/2., frameWidth/2.);
		pContext->setDrawMode (kAliasing);
		pContext->setLineWidth (frameWidth);
		pContext->setLineStyle (kLineSolid);
		if (style & k3DIn)
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

		if (style & k3DIn)
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
	if (!(style & kNoTextStyle))
	{
		pContext->saveGlobalState ();
		CRect textRect (size);
		textRect.inset (textInset.x, textInset.y);
		
		CRect oldClip;
		pContext->getClipRect (oldClip);
		CRect newClip (textRect);
		newClip.bound (oldClip);
		pContext->setClipRect (newClip);
		
		CPoint center (textRect.getCenter ());
		CGraphicsTransform transform;
		transform.rotate (textRotation, center);
		CDrawContext::Transform ctxTransform (*pContext, transform);
		
		pContext->setDrawMode (kAntiAliasing);
		pContext->setFont (fontID);
		
		// draw darker text (as shadow)
		if (style & kShadowText)
		{
			CRect newSize (textRect);
			newSize.offset (shadowTextOffset);
			pContext->setFontColor (shadowColor);
			pContext->drawString (string, newSize, horiTxtAlign, bAntialias);
		}
		pContext->setFontColor (fontColor);
		pContext->drawString (string, textRect, horiTxtAlign, bAntialias);
		pContext->restoreGlobalState ();
	}
}

//------------------------------------------------------------------------
void CParamDisplay::setFont (CFontRef fontID)
{
	if (this->fontID)
		this->fontID->forget ();
	this->fontID = fontID;
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

} // namespace
