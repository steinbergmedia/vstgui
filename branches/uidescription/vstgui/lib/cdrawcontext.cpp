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

#include "cdrawcontext.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
CDrawContext::CDrawContext (const CRect& surfaceRect)
: surfaceRect (surfaceRect)
{
	currentState.font = 0;
	currentState.fontColor = kTransparentCColor;
	currentState.frameWidth = 0;
	currentState.frameColor = kTransparentCColor;
	currentState.fillColor = kTransparentCColor;
	currentState.lineStyle = kLineOnOffDash;
	currentState.drawMode = kAntialias;
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
	setDrawMode (kCopyMode);
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
void CDrawContext::setLineStyle (CLineStyle style)
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
void CDrawContext::setFillColor (const CColor color)
{
	currentState.fillColor = color;
}

//-----------------------------------------------------------------------------
void CDrawContext::setFrameColor (const CColor color)
{
	currentState.frameColor = color;
}

//-----------------------------------------------------------------------------
void CDrawContext::setFontColor (const CColor color)
{
	currentState.fontColor = color;
}

//-----------------------------------------------------------------------------
void CDrawContext::setFont (const CFontRef newFont, const long& size, const long& style)
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
CCoord CDrawContext::getStringWidth (const char* string)
{
	CCoord result = -1;
	if (currentState.font == 0 || string == 0)
		return result;

	IFontPainter* painter = currentState.font->getFontPainter ();
	if (painter)
		result = painter->getStringWidth (this, string, true);

	return result;
}

//-----------------------------------------------------------------------------
void CDrawContext::drawString (const char* string, const CPoint& point, bool antialias)
{
	if (string == 0 || currentState.font == 0)
		return;

	IFontPainter* painter = currentState.font->getFontPainter ();
	if (painter)
		painter->drawString (this, string, point, antialias);
}

//-----------------------------------------------------------------------------
void CDrawContext::drawString (const char* string, const CRect& _rect, const CHoriTxtAlign hAlign, bool antialias)
{
	if (!string || currentState.font == 0)
		return;
	
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
		CCoord stringWidth = getStringWidth (string);
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
	drawString (string, CPoint (rect.left, rect.bottom), antialias);
	setClipRect (oldClip);
}

} // namespace
