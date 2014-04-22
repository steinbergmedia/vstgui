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
#include <cassert>

namespace VSTGUI {

//-----------------------------------------------------------------------------
CDrawContext::CDrawContextState::CDrawContextState ()
{
	font = 0;
	fontColor = kTransparentCColor;
	frameWidth = 0;
	frameColor = kTransparentCColor;
	fillColor = kTransparentCColor;
	lineStyle = kLineOnOffDash;
	drawMode = kAntiAliasing;
	globalAlpha = 1;
}

//-----------------------------------------------------------------------------
CDrawContext::CDrawContextState::CDrawContextState (const CDrawContextState& state)
{
	*this = state;
}

//-----------------------------------------------------------------------------
CDrawContext::CDrawContextState& CDrawContext::CDrawContextState::operator= (const CDrawContextState& state)
{
	font = state.font;
	frameColor = state.frameColor;
	fillColor = state.fillColor;
	fontColor = state.fontColor;
	frameWidth = state.frameWidth;
	offset = state.offset;
	penLoc = state.penLoc;
	clipRect = state.clipRect;
	lineStyle = state.lineStyle;
	drawMode = state.drawMode;
	globalAlpha = state.globalAlpha;
	return *this;
}

#if VSTGUI_RVALUE_REF_SUPPORT
//-----------------------------------------------------------------------------
CDrawContext::CDrawContextState::CDrawContextState (CDrawContextState&& state) noexcept
{
	*this = std::move (state);
}

//-----------------------------------------------------------------------------
CDrawContext::CDrawContextState& CDrawContext::CDrawContextState::operator= (CDrawContextState&& state) noexcept
{
	font = std::move (state.font);
	frameColor = std::move (state.frameColor);
	fillColor = std::move (state.fillColor);
	fontColor = std::move (state.fontColor);
	frameWidth = std::move (state.frameWidth);
	offset = std::move (state.offset);
	penLoc = std::move (state.penLoc);
	clipRect = std::move (state.clipRect);
	lineStyle = std::move (state.lineStyle);
	drawMode = std::move (state.drawMode);
	globalAlpha = std::move (state.globalAlpha);
	return *this;
}
#endif

//-----------------------------------------------------------------------------
CDrawContext::Transform::Transform (CDrawContext& context, const CGraphicsTransform& transformation)
: context (context)
{
	context.pushTransform (transformation);
}

//-----------------------------------------------------------------------------
CDrawContext::Transform::~Transform ()
{
	context.popTransform ();
}

//-----------------------------------------------------------------------------
CDrawContext::CDrawContext (const CRect& surfaceRect)
: drawStringHelper (0)
, surfaceRect (surfaceRect)
{
	transformStack.push (CGraphicsTransform ());
}

//-----------------------------------------------------------------------------
CDrawContext::~CDrawContext ()
{
	#if DEBUG
	if (!globalStatesStack.empty ())
		DebugPrint ("Global state stack not empty. Save and restore global state must be called in sequence !\n");
	#endif
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
	globalStatesStack.push (currentState);
}

//-----------------------------------------------------------------------------
void CDrawContext::restoreGlobalState ()
{
	if (!globalStatesStack.empty ())
	{
	#if VSTGUI_RVALUE_REF_SUPPORT
		currentState = std::move (globalStatesStack.top ());
	#else
		currentState = globalStatesStack.top ();
	#endif
		globalStatesStack.pop ();
	}
	else
	{
		#if DEBUG
		DebugPrint ("No saved global state in draw context !!!\n");
		#endif
	}
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
void CDrawContext::moveTo (const CPoint& point)
{
	currentState.penLoc = point;
}

//-----------------------------------------------------------------------------
void CDrawContext::lineTo (const CPoint &point)
{
	drawLine (std::make_pair (currentState.penLoc, point));
	currentState.penLoc = point;
}

//-----------------------------------------------------------------------------
void CDrawContext::drawLines (const CPoint* points, const int32_t& numberOfLines)
{
	LineList list (numberOfLines);
	for (int32_t i = 0; i < numberOfLines * 2; i += 2)
	{
		list.push_back (std::make_pair (points[i], points[i+1]));
	}
	drawLines (list);
}

//-----------------------------------------------------------------------------
void CDrawContext::drawPolygon (const CPoint* pPoints, int32_t numberOfPoints, const CDrawStyle drawStyle)
{
	PointList list (numberOfPoints);
	for (int32_t i = 0; i < numberOfPoints; i++)
	{
		list.push_back (pPoints[i]);
	}
	drawPolygon (list, drawStyle);
}

#endif

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
	if ((size > 0 && newFont->getSize () != size) || (style != -1 && newFont->getStyle () != style))
	{
		OwningPointer<CFontDesc> font = (CFontRef)newFont->newCopy ();
		currentState.font = font;
		if (size > 0)
			currentState.font->setSize (size);
		if (style != -1)
			currentState.font->setStyle (style);
	}
	else
	{
		currentState.font = newFont;
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
		rect.bottom -= (rect.getHeight ()/2 - capHeight / 2);
	else
		rect.bottom -= (rect.getHeight ()/2 - currentState.font->getSize () / 2) + 1;
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
void CDrawContext::pushTransform (const CGraphicsTransform& transformation)
{
	assert (transformStack.size () > 0);
	const CGraphicsTransform& currentTransform = transformStack.top ();
	CGraphicsTransform newTransform = currentTransform * transformation;
	transformStack.push (newTransform);
}

//-----------------------------------------------------------------------------
void CDrawContext::popTransform ()
{
	assert (transformStack.size () > 1);
	transformStack.pop ();
}

//-----------------------------------------------------------------------------
const CGraphicsTransform& CDrawContext::getCurrentTransform () const
{
	return transformStack.top ();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CLineStyle::CLineStyle (LineCap _cap, LineJoin _join, CCoord _dashPhase, uint32_t _dashCount, const CCoord* _dashLengths)
: cap (_cap)
, join (_join)
, dashPhase (_dashPhase)
{
	if (_dashCount && _dashLengths)
	{
		for (uint32_t i = 0; i < _dashCount; i++)
			dashLengths.push_back (_dashLengths[i]);
	}
}

//-----------------------------------------------------------------------------
CLineStyle::CLineStyle (LineCap _cap, LineJoin _join, CCoord _dashPhase, const CoordVector& _dashLengths)
: cap (_cap)
, join (_join)
, dashPhase (_dashPhase)
, dashLengths (_dashLengths)
{
}

//-----------------------------------------------------------------------------
CLineStyle::CLineStyle (const CLineStyle& lineStyle)
{
	*this = lineStyle;
}

//-----------------------------------------------------------------------------
CLineStyle::~CLineStyle ()
{
}

#if VSTGUI_RVALUE_REF_SUPPORT
//-----------------------------------------------------------------------------
CLineStyle::CLineStyle (LineCap _cap, LineJoin _join, CCoord _dashPhase, CoordVector&& _dashLengths) noexcept
: cap (_cap)
, join (_join)
, dashPhase (_dashPhase)
, dashLengths (std::move (_dashLengths))
{
}

//-----------------------------------------------------------------------------
CLineStyle::CLineStyle (CLineStyle&& cls) noexcept
{
	*this = std::move (cls);
}

//-----------------------------------------------------------------------------
CLineStyle& CLineStyle::operator= (CLineStyle&& cls) noexcept
{
	dashLengths.clear ();
	cap = cls.cap;
	join = cls.join;
	dashPhase = cls.dashPhase;
	dashLengths = std::move (cls.dashLengths);
	return *this;
}
#endif

//-----------------------------------------------------------------------------
bool CLineStyle::operator== (const CLineStyle& cls) const
{
	if (cap == cls.cap && join == cls.join && dashPhase == cls.dashPhase && dashLengths == cls.dashLengths)
	{
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
CLineStyle& CLineStyle::operator= (const CLineStyle& cls)
{
	dashLengths.clear ();
	cap = cls.cap;
	join = cls.join;
	dashPhase = cls.dashPhase;
	dashLengths = cls.dashLengths;
	return *this;
}

//-----------------------------------------------------------------------------
static const CCoord kDefaultOnOffDashLength[] = {1, 1};
const CLineStyle kLineSolid;
const CLineStyle kLineOnOffDash (CLineStyle::kLineCapButt, CLineStyle::kLineJoinMiter, 0, 2, kDefaultOnOffDashLength);

} // namespace
