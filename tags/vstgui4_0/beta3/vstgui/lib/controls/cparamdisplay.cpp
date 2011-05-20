//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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

#include "cparamdisplay.h"
#include "../cbitmap.h"

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
: CControl (size, 0, -1, background)
, valueToString (0)
, valueToStringUserData (0)
, horiTxtAlign (kCenterText)
, style (style)
, bTextTransparencyEnabled (true)
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
, valueToString (v.valueToString)
, valueToStringUserData (v.valueToStringUserData)
, horiTxtAlign (v.horiTxtAlign)
, style (v.style)
, fontID (v.fontID)
, fontColor (v.fontColor)
, backColor (v.backColor)
, frameColor (v.frameColor)
, shadowColor (v.shadowColor)
, textInset (v.textInset)
, bTextTransparencyEnabled (v.bTextTransparencyEnabled)
, bAntialias (v.bAntialias)
{
	fontID->remember ();
}

//------------------------------------------------------------------------
CParamDisplay::~CParamDisplay ()
{
	if (fontID)
		fontID->forget ();
}

//------------------------------------------------------------------------
void CParamDisplay::setStyle (int32_t val)
{
	if (style != val)
	{
		style = val;
		setDirty ();
	}
}

//------------------------------------------------------------------------
void CParamDisplay::setValueToStringProc (CParamDisplayValueToStringProc proc, void* userData)
{
	valueToString = proc;
	valueToStringUserData = userData;
}

//------------------------------------------------------------------------
void CParamDisplay::draw (CDrawContext *pContext)
{
	if (style & kNoDrawStyle)
		return;

	char string[256];
	string[0] = 0;

	bool converted = false;
	if (valueToString)
		converted = valueToString (value, string, valueToStringUserData);
	if (!converted)
		sprintf (string, "%2.2f", value);

	drawBack (pContext);
	drawText (pContext, string);
	setDirty (false);
}

//------------------------------------------------------------------------
void CParamDisplay::drawBack (CDrawContext* pContext, CBitmap* newBack)
{
	pContext->setDrawMode (kAliasing);
	if (newBack)
	{
		newBack->draw (pContext, size, backOffset);
	}
	else if (pBackground)
	{
		pBackground->draw (pContext, size, backOffset);
	}
	else
	{
		if (!getTransparency ())
		{
			pContext->setFillColor (backColor);
			pContext->drawRect (size, kDrawFilled);
	
			if (!(style & (k3DIn|k3DOut|kNoFrame))) 
			{
				pContext->setLineStyle (kLineSolid);
				pContext->setLineWidth (1);
				pContext->setFrameColor (frameColor);
				pContext->drawRect (size);
			}
		}
	}
	// draw the frame for the 3D effect
	if (style & (k3DIn|k3DOut)) 
	{
		CRect r (size);
		r.right--; r.top++;
		pContext->setLineWidth (1);
		pContext->setLineStyle (kLineSolid);
		if (style & k3DIn)
			pContext->setFrameColor (backColor);
		else
			pContext->setFrameColor (frameColor);
		CPoint p;
		pContext->moveTo (p (r.left, r.bottom));
		pContext->lineTo (p (r.left, r.top));
		pContext->lineTo (p (r.right, r.top));

		if (style & k3DIn)
			pContext->setFrameColor (frameColor);
		else
			pContext->setFrameColor (backColor);
		pContext->moveTo (p (r.right, r.top));
		pContext->lineTo (p (r.right, r.bottom));
		pContext->lineTo (p (r.left, r.bottom));
	}
}

//------------------------------------------------------------------------
void CParamDisplay::drawText (CDrawContext *pContext, UTF8StringPtr string)
{
	if (!(style & kNoTextStyle) && string && strlen (string))
	{
		CRect textRect (size);
		textRect.inset (textInset.x, textInset.y);
		CRect oldClip;
		pContext->getClipRect (oldClip);
		CRect newClip (textRect);
		newClip.bound (oldClip);
		pContext->setClipRect (newClip);
		pContext->setFont (fontID);
	
		// draw darker text (as shadow)
		if (style & kShadowText) 
		{
			CRect newSize (textRect);
			newSize.offset (1, 1);
			pContext->setFontColor (shadowColor);
			pContext->drawString (string, newSize, horiTxtAlign, bAntialias);
		}
		pContext->setFontColor (fontColor);
		pContext->drawString (string, textRect, horiTxtAlign, bAntialias);
		pContext->setClipRect (oldClip);
	}
}

//------------------------------------------------------------------------
void CParamDisplay::setFont (CFontRef fontID)
{
	setDirty ();
	if (this->fontID)
		this->fontID->forget ();
	this->fontID = fontID;
	fontID->remember ();
}

//------------------------------------------------------------------------
void CParamDisplay::setFontColor (CColor color)
{
	// to force the redraw
	if (fontColor != color)
		setDirty ();
	fontColor = color;
}

//------------------------------------------------------------------------
void CParamDisplay::setBackColor (CColor color)
{
	// to force the redraw
	if (backColor != color)
		setDirty ();
	backColor = color;
}

//------------------------------------------------------------------------
void CParamDisplay::setFrameColor (CColor color)
{
	// to force the redraw
	if (frameColor != color)
		setDirty ();
	frameColor = color;
}

//------------------------------------------------------------------------
void CParamDisplay::setShadowColor (CColor color)
{
	// to force the redraw
	if (shadowColor != color)
		setDirty ();
	shadowColor = color;
}

//------------------------------------------------------------------------
void CParamDisplay::setHoriAlign (CHoriTxtAlign hAlign)
{
	// to force the redraw
	if (horiTxtAlign != hAlign)
		setDirty ();
	horiTxtAlign = hAlign;
}

} // namespace
