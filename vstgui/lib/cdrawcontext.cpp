//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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

#include "cdrawcontext.h"
#include "cgraphicspath.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
CDrawContext::CDrawContext (const CRect& surfaceRect)
: drawStringHelper (0)
, surfaceRect (surfaceRect)
{
	currentState.font = 0;
	currentState.fontColor = kTransparentCColor;
	currentState.frameWidth = 0;
	currentState.frameColor = kTransparentCColor;
	currentState.fillColor = kTransparentCColor;
	currentState.lineStyle = kLineOnOffDash;
	currentState.drawMode = kAntiAliasing;
	currentState.globalAlpha = 1;
}

//-----------------------------------------------------------------------------
CDrawContext::~CDrawContext ()
{
	#if DEBUG
	if (!globalStatesStack.empty ())
		DebugPrint ("Global state stack not empty. Save and restore global state must be called in sequence !\n");
	#endif
	while (!globalStatesStack.empty ())
	{
		CDrawContextState* state = globalStatesStack.top ();
		globalStatesStack.pop ();
		if (state->font)
			state->font->forget ();
		delete state;
	}
	if (currentState.font)
		currentState.font->forget ();
	if (drawStringHelper)
		drawStringHelper->forget ();
}

//-----------------------------------------------------------------------------
void CDrawContext::init ()
{
	// set the default values
	setFrameColor (kWhiteCColor);
	setLineStyle (kLineSolid);
	setLineWidth (1);
	setFillColor (kBlackCColor);
	setFontColor (kWhiteCColor);
	setFont (kSystemFont);
	setDrawMode (kAliasing);
	setClipRect (surfaceRect);
}

//-----------------------------------------------------------------------------
void CDrawContext::saveGlobalState ()
{
	CDrawContextState* state = new CDrawContextState ();
	*state = currentState;
	globalStatesStack.push (state);
	if (state->font)
		state->font->remember ();
}

//-----------------------------------------------------------------------------
void CDrawContext::restoreGlobalState ()
{
	if (!globalStatesStack.empty ())
	{
		if (currentState.font)
			currentState.font->forget ();
		CDrawContextState* state = globalStatesStack.top ();
		currentState = *state;
		globalStatesStack.pop ();
		delete state;
	}
	else
	{
		#if DEBUG
		DebugPrint ("No saved global state in draw context !!!\n");
		#endif
	}
}

//-----------------------------------------------------------------------------
void CDrawContext::moveTo (const CPoint& point)
{
	currentState.penLoc = point;
}

//-----------------------------------------------------------------------------
void CDrawContext::setLineStyle (const CLineStyle& style)
{
	currentState.lineStyle = style;
}

//-----------------------------------------------------------------------------
void CDrawContext::setLineWidth (CCoord width)
{
	currentState.frameWidth = width;
}

//-----------------------------------------------------------------------------
void CDrawContext::setDrawMode (CDrawMode mode)
{
	currentState.drawMode = mode;
}

//-----------------------------------------------------------------------------
CRect& CDrawContext::getClipRect (CRect &clip) const
{
	clip = currentState.clipRect;
	clip.offset (-currentState.offset.x, -currentState.offset.y);
	return clip;
}

//-----------------------------------------------------------------------------
void CDrawContext::setClipRect (const CRect &clip)
{
	currentState.clipRect = clip;
	currentState.clipRect.offset (currentState.offset.x, currentState.offset.y);
}

//-----------------------------------------------------------------------------
void CDrawContext::resetClipRect ()
{
	currentState.clipRect = surfaceRect;
}

//-----------------------------------------------------------------------------
void CDrawContext::setFillColor (const CColor& color)
{
	currentState.fillColor = color;
}

//-----------------------------------------------------------------------------
void CDrawContext::setFrameColor (const CColor& color)
{
	currentState.frameColor = color;
}

//-----------------------------------------------------------------------------
void CDrawContext::setFontColor (const CColor& color)
{
	currentState.fontColor = color;
}

//-----------------------------------------------------------------------------
void CDrawContext::setFont (const CFontRef newFont, const CCoord& size, const int32_t& style)
{
	if (newFont == 0)
		return;
	if (currentState.font)
		currentState.font->forget ();
	if ((size > 0 && newFont->getSize () != size) || (style != -1 && newFont->getStyle () != style))
	{
		currentState.font = (CFontRef)newFont->newCopy ();
		if (size > 0)
			currentState.font->setSize (size);
		if (style != -1)
			currentState.font->setStyle (style);
	}
	else
	{
		currentState.font = newFont;
		currentState.font->remember ();
	}
}

//-----------------------------------------------------------------------------
void CDrawContext::setGlobalAlpha (float newAlpha)
{
	currentState.globalAlpha = newAlpha;
}

//-----------------------------------------------------------------------------
void CDrawContext::setOffset (const CPoint& offset)
{
	currentState.offset = offset;
}

//-----------------------------------------------------------------------------
const CString& CDrawContext::getDrawString (UTF8StringPtr string)
{
	if (drawStringHelper == 0)
		drawStringHelper = new CString (string);
	else
		drawStringHelper->setUTF8String (string);
	return *drawStringHelper;
}

//-----------------------------------------------------------------------------
void CDrawContext::clearDrawString ()
{
	if (drawStringHelper)
		drawStringHelper->setUTF8String (0);
}

//-----------------------------------------------------------------------------
CCoord CDrawContext::getStringWidth (UTF8StringPtr string)
{
	CCoord result = -1;
	if (currentState.font == 0 || string == 0)
		return result;

	IFontPainter* painter = currentState.font->getFontPainter ();
	if (painter)
	{
		result = painter->getStringWidth (this, getDrawString (string), true);
		clearDrawString ();
	}

	return result;
}

//-----------------------------------------------------------------------------
void CDrawContext::drawString (UTF8StringPtr string, const CPoint& point, bool antialias)
{
	if (string == 0 || currentState.font == 0)
		return;

	IFontPainter* painter = currentState.font->getFontPainter ();
	if (painter)
	{
		painter->drawString (this, getDrawString (string), point, antialias);
		clearDrawString ();
	}
}

//-----------------------------------------------------------------------------
void CDrawContext::drawString (UTF8StringPtr _string, const CRect& _rect, const CHoriTxtAlign hAlign, bool antialias)
{
	if (!_string || currentState.font == 0)
		return;
	IFontPainter* painter = currentState.font->getFontPainter ();
	if (painter == 0)
		return;
	
	const CString& string = getDrawString (_string);
	CRect rect (_rect);

	double capHeight = -1;
	IPlatformFont* platformFont = currentState.font->getPlatformFont ();
	if (platformFont)
		capHeight = platformFont->getCapHeight ();
	
	if (capHeight > 0.)
		rect.bottom -= (rect.height ()/2 - capHeight / 2);
	else
		rect.bottom -= (rect.height ()/2 - currentState.font->getSize () / 2) + 1;
	if (hAlign != kLeftText)
	{
		CCoord stringWidth = painter->getStringWidth (this, string, antialias);
		if (hAlign == kRightText)
			rect.left = rect.right - stringWidth;
		else
			rect.left = (CCoord)(rect.left + (rect.getWidth () / 2.f) - (stringWidth / 2.f));
	}
	CRect oldClip;
	getClipRect (oldClip);
	CRect newClip (_rect);
	newClip.bound (oldClip);
	setClipRect (newClip);
	painter->drawString (this, string, CPoint (rect.left, rect.bottom), antialias);
	setClipRect (oldClip);
	clearDrawString ();
}

//-----------------------------------------------------------------------------
CGraphicsPath* CDrawContext::createRoundRectGraphicsPath (const CRect& size, CCoord radius)
{
	CGraphicsPath* path = createGraphicsPath ();
	if (path)
	{
		path->addRoundRect (size, radius);
	}
	return path;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CLineStyle::CLineStyle (LineCap _cap, LineJoin _join, CCoord _dashPhase, int32_t _dashCount, const CCoord* _dashLengths)
: cap (_cap)
, join (_join)
, dashPhase (_dashPhase)
, dashCount (0)
, dashLengths (0)
{
	if (_dashCount && _dashLengths)
	{
		dashCount = _dashCount;
		dashLengths = new CCoord[dashCount];
		for (int32_t i = 0; i < dashCount; i++)
			dashLengths[i] = _dashLengths[i];
	}
}

//-----------------------------------------------------------------------------
CLineStyle::~CLineStyle ()
{
	if (dashLengths)
		delete [] dashLengths;
}

//-----------------------------------------------------------------------------
bool CLineStyle::operator== (const CLineStyle& cls) const
{
	if (cap == cls.cap && join == cls.join && dashPhase == cls.dashPhase && dashCount == cls.dashCount)
	{
		if (dashCount)
		{
			for (int32_t i = 0; i < dashCount; i++)
			{
				if (dashLengths[i] != cls.dashLengths[i])
					return false;
			}
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
CLineStyle& CLineStyle::operator= (const CLineStyle& cls)
{
	if (dashLengths)
	{
		delete [] dashLengths;
		dashLengths = 0;
	}
	cap = cls.cap;
	join = cls.join;
	dashPhase = cls.dashPhase;
	dashCount = 0;
	if (cls.dashCount && cls.dashLengths)
	{
		dashCount = cls.dashCount;
		dashLengths = new CCoord[dashCount];
		for (int32_t i = 0; i < dashCount; i++)
			dashLengths[i] = cls.dashLengths[i];
	}
	return *this;
}

//-----------------------------------------------------------------------------
static const CCoord kDefaultOnOffDashLength[] = {1, 1};
const CLineStyle kLineSolid;
const CLineStyle kLineOnOffDash (CLineStyle::kLineCapButt, CLineStyle::kLineJoinMiter, 0, 2, kDefaultOnOffDashLength);

} // namespace
