// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cdrawcontext.h"
#include "cgraphicspath.h"
#include "cgradient.h"
#include "cbitmap.h"
#include "cstring.h"
#include "platform/iplatformfont.h"
#include "platform/iplatformgraphicspath.h"
#include "platform/iplatformgradient.h"
#include "platform/platformfactory.h"
#include <cassert>
#include <stack>

#define VSTGUI_PLATFORM_DRAWDEVICE 1

#if VSTGUI_PLATFORM_DRAWDEVICE
#include "platform/iplatformgraphicsdevice.h"
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
CDrawContext::CDrawContextState::CDrawContextState (const CDrawContextState& state)
{
	*this = state;
}

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
	penLoc = std::move (state.penLoc);
	clipRect = std::move (state.clipRect);
	lineStyle = std::move (state.lineStyle);
	drawMode = std::move (state.drawMode);
	globalAlpha = std::move (state.globalAlpha);
	return *this;
}

//-----------------------------------------------------------------------------
CDrawContext::Transform::Transform (CDrawContext& context, const CGraphicsTransform& transformation)
: context (context)
, transformation (transformation)
{
	if (transformation.isInvariant () == false)
		context.pushTransform (transformation);
}

//-----------------------------------------------------------------------------
CDrawContext::Transform::~Transform () noexcept
{
	if (transformation.isInvariant () == false)
		context.popTransform ();
}

//-----------------------------------------------------------------------------
struct CDrawContext::Impl
{
	UTF8String* drawStringHelper {nullptr};
	CRect surfaceRect;
	double scaleFactor {1.};

	CDrawContextState currentState;

	std::stack<CDrawContextState> globalStatesStack;
	std::stack<CGraphicsTransform> transformStack;

#if VSTGUI_PLATFORM_DRAWDEVICE
	PlatformGraphicsDeviceContextPtr device;
#endif
};

//-----------------------------------------------------------------------------
CDrawContext::CDrawContext (const CRect& surfaceRect)
{
	impl = std::make_unique<Impl> ();
	impl->surfaceRect = surfaceRect;

	impl->transformStack.push (CGraphicsTransform ());
}

//------------------------------------------------------------------------
CDrawContext::CDrawContext (const PlatformGraphicsDeviceContextPtr device, const CRect& surfaceRect,
							double scaleFactor)
: CDrawContext (surfaceRect)
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	impl->device = device;
	impl->scaleFactor = scaleFactor;
	setClipRect (surfaceRect);
#endif
}

//-----------------------------------------------------------------------------
CDrawContext::~CDrawContext () noexcept
{
	#if DEBUG
	if (!impl->globalStatesStack.empty ())
		DebugPrint ("Global state stack not empty. Save and restore global state must be called in sequence !\n");
	#endif
	if (impl->drawStringHelper)
		delete impl->drawStringHelper;
}

//------------------------------------------------------------------------
const PlatformGraphicsDeviceContextPtr& CDrawContext::getPlatformDeviceContext () const
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	return impl->device;
#else
	return nullptr;
#endif
}

//-----------------------------------------------------------------------------
const CRect& CDrawContext::getSurfaceRect () const { return impl->surfaceRect; }

//------------------------------------------------------------------------
double CDrawContext::getScaleFactor () const { return impl->scaleFactor; }

//-----------------------------------------------------------------------------
auto CDrawContext::getCurrentState () const -> const CDrawContextState&
{
	return impl->currentState;
}

//-----------------------------------------------------------------------------
auto CDrawContext::getCurrentState () -> CDrawContextState& { return impl->currentState; }

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
	setClipRect (impl->surfaceRect);
}

//-----------------------------------------------------------------------------
void CDrawContext::saveGlobalState ()
{
	impl->globalStatesStack.push (getCurrentState ());
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
		impl->device->saveGlobalState ();
#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::restoreGlobalState ()
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
		impl->device->restoreGlobalState ();
#endif
	if (!impl->globalStatesStack.empty ())
	{
		getCurrentState () = std::move (impl->globalStatesStack.top ());
		impl->globalStatesStack.pop ();
	}
	else
	{
		#if DEBUG
		DebugPrint ("No saved global state in draw context !!!\n");
		#endif
	}
}

//-----------------------------------------------------------------------------
void CDrawContext::setBitmapInterpolationQuality (BitmapInterpolationQuality quality)
{
	getCurrentState ().bitmapQuality = quality;
}

//-----------------------------------------------------------------------------
void CDrawContext::setLineStyle (const CLineStyle& style)
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
		impl->device->setLineStyle (style);
#endif
	getCurrentState ().lineStyle = style;
}

//-----------------------------------------------------------------------------
void CDrawContext::setLineWidth (CCoord width)
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
		impl->device->setLineWidth (width);
#endif
	getCurrentState ().frameWidth = width;
}

//-----------------------------------------------------------------------------
void CDrawContext::setDrawMode (CDrawMode mode)
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
		impl->device->setDrawMode (mode);
#endif
	getCurrentState ().drawMode = mode;
}

//-----------------------------------------------------------------------------
CRect& CDrawContext::getClipRect (CRect &clip) const
{
	clip = getCurrentState ().clipRect;
	getCurrentTransform ().inverse ().transform (clip);
	clip.normalize ();
	return clip;
}

//-----------------------------------------------------------------------------
void CDrawContext::setClipRect (const CRect &clip)
{
	getCurrentState ().clipRect = clip;
	getCurrentTransform ().transform (getCurrentState ().clipRect);
	getCurrentState ().clipRect.normalize ();
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
		impl->device->setClipRect (getCurrentState ().clipRect);
#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::resetClipRect ()
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
		impl->device->setClipRect (getSurfaceRect ());
#endif
	getCurrentState ().clipRect = getSurfaceRect ();
}

//-----------------------------------------------------------------------------
void CDrawContext::setFillColor (const CColor& color)
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
		impl->device->setFillColor (color);
#endif
	getCurrentState ().fillColor = color;
}

//-----------------------------------------------------------------------------
void CDrawContext::setFrameColor (const CColor& color)
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
		impl->device->setFrameColor (color);
#endif
	getCurrentState ().frameColor = color;
}

//-----------------------------------------------------------------------------
void CDrawContext::setFontColor (const CColor& color) { getCurrentState ().fontColor = color; }

//-----------------------------------------------------------------------------
void CDrawContext::setFont (const CFontRef newFont, const CCoord& size, const int32_t& style)
{
	if (newFont == nullptr)
		return;
	if ((size > 0 && newFont->getSize () != size) || (style != -1 && newFont->getStyle () != style))
	{
		getCurrentState ().font = makeOwned<CFontDesc> (*newFont);
		if (size > 0)
			getCurrentState ().font->setSize (size);
		if (style != -1)
			getCurrentState ().font->setStyle (style);
	}
	else
	{
		getCurrentState ().font = newFont;
	}
}

//-----------------------------------------------------------------------------
void CDrawContext::setGlobalAlpha (float newAlpha)
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
		impl->device->setGlobalAlpha (newAlpha);
#endif
	getCurrentState ().globalAlpha = newAlpha;
}

//-----------------------------------------------------------------------------
const UTF8String& CDrawContext::getDrawString (UTF8StringPtr string)
{
	if (impl->drawStringHelper == nullptr)
		impl->drawStringHelper = new UTF8String (string);
	else
		impl->drawStringHelper->assign (string);
	return *impl->drawStringHelper;
}

//-----------------------------------------------------------------------------
void CDrawContext::clearDrawString ()
{
	if (impl->drawStringHelper)
		impl->drawStringHelper->clear ();
}

//------------------------------------------------------------------------
CCoord CDrawContext::getStringWidth (IPlatformString* string)
{
	CCoord result = -1;
	if (getCurrentState ().font == nullptr || string == nullptr)
		return result;

	if (auto painter = getCurrentState ().font->getFontPainter ())
		result = painter->getStringWidth (this, string, true);
	
	return result;
}

//------------------------------------------------------------------------
void CDrawContext::drawString (IPlatformString* string, const CRect& _rect, const CHoriTxtAlign hAlign, bool antialias)
{
	if (!string || getCurrentState ().font == nullptr)
		return;
	auto painter = getCurrentState ().font->getFontPainter ();
	if (painter == nullptr)
		return;
	
	CRect rect (_rect);
	
	double capHeight = -1;
	auto platformFont = getCurrentState ().font->getPlatformFont ();
	if (platformFont)
		capHeight = platformFont->getCapHeight ();
	
	if (capHeight > 0.)
		rect.bottom -= (rect.getHeight () / 2. - capHeight / 2.);
	else
		rect.bottom -= (rect.getHeight () / 2. - getCurrentState ().font->getSize () / 2.) + 1.;
	if (hAlign != kLeftText)
	{
		CCoord stringWidth = painter->getStringWidth (this, string, antialias);
		if (hAlign == kRightText)
			rect.left = rect.right - stringWidth;
		else
			rect.left = rect.left + (rect.getWidth () / 2.) - (stringWidth / 2.);
	}

	painter->drawString (this, string, CPoint (rect.left, rect.bottom), antialias);
}

//------------------------------------------------------------------------
void CDrawContext::drawString (IPlatformString* string, const CPoint& point, bool antialias)
{
	if (string == nullptr || getCurrentState ().font == nullptr)
		return;

	if (auto painter = getCurrentState ().font->getFontPainter ())
		painter->drawString (this, string, point, antialias);
}

//-----------------------------------------------------------------------------
CCoord CDrawContext::getStringWidth (UTF8StringPtr string)
{
	return getStringWidth (getDrawString (string).getPlatformString ());
}

//-----------------------------------------------------------------------------
void CDrawContext::drawString (UTF8StringPtr string, const CPoint& point, bool antialias)
{
	drawString (getDrawString (string).getPlatformString (), point, antialias);
	clearDrawString ();
}

//-----------------------------------------------------------------------------
void CDrawContext::drawString (UTF8StringPtr string, const CRect& rect, const CHoriTxtAlign hAlign, bool antialias)
{
	drawString (getDrawString (string).getPlatformString (), rect, hAlign, antialias);
	clearDrawString ();
}

//-----------------------------------------------------------------------------
void CDrawContext::fillRectWithBitmap (CBitmap* bitmap, const CRect& srcRect, const CRect& dstRect, float alpha)
{
	if (srcRect.isEmpty () || dstRect.isEmpty ())
		return;

#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
	{
		if (auto deviceBitmapExt = impl->device->asBitmapExt ())
		{
			double transformedScaleFactor = getScaleFactor ();
			CGraphicsTransform t = getCurrentTransform ();
			if (t.m11 == t.m22 && t.m12 == 0 && t.m21 == 0)
				transformedScaleFactor *= t.m11;

			if (auto pb = bitmap->getBestPlatformBitmapForScaleFactor (transformedScaleFactor))
			{
				if (deviceBitmapExt->fillRectWithBitmap (*pb, srcRect, dstRect, alpha,
														 getBitmapInterpolationQuality ()))
				{
					return;
				}
			}
		}
	}
#endif

	CRect bitmapPartRect;
	CPoint sourceOffset (srcRect.left, srcRect.top);

	for (auto top = dstRect.top; top < dstRect.bottom; top += srcRect.getHeight ())
	{
		bitmapPartRect.top = top;
		bitmapPartRect.bottom = top + srcRect.getHeight ();
		if (bitmapPartRect.bottom > dstRect.bottom)
			bitmapPartRect.bottom = dstRect.bottom;
		// The following should never be true, I guess
		if (bitmapPartRect.getHeight () > srcRect.getHeight ())
			bitmapPartRect.setHeight (srcRect.getHeight ());
		
		for (auto left = dstRect.left; left < dstRect.right; left += srcRect.getWidth ())
		{
			bitmapPartRect.left = left;
			bitmapPartRect.right = left + srcRect.getWidth ();
			if (bitmapPartRect.right > dstRect.right)
				bitmapPartRect.right = dstRect.right;
			// The following should never be true, I guess
			if (bitmapPartRect.getWidth () > srcRect.getWidth ())
				bitmapPartRect.setWidth (srcRect.getWidth ());
			
			drawBitmap (bitmap, bitmapPartRect, sourceOffset, alpha);
		}
	}
}

//-----------------------------------------------------------------------------
void CDrawContext::drawBitmapNinePartTiled (CBitmap* bitmap, const CRect& dest, const CNinePartTiledDescription& desc, float alpha)
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
	{
		if (auto deviceBitmapExt = impl->device->asBitmapExt ())
		{
			double transformedScaleFactor = getScaleFactor ();
			CGraphicsTransform t = getCurrentTransform ();
			if (t.m11 == t.m22 && t.m12 == 0 && t.m21 == 0)
				transformedScaleFactor *= t.m11;

			if (auto pb = bitmap->getBestPlatformBitmapForScaleFactor (transformedScaleFactor))
			{
				if (deviceBitmapExt->drawBitmapNinePartTiled (*pb, dest, desc, alpha,
															  getBitmapInterpolationQuality ()))
				{
					return;
				}
			}
		}
	}
#endif

	CRect myBitmapBounds (0, 0, bitmap->getWidth (), bitmap->getHeight ());
	CRect mySourceRect [CNinePartTiledDescription::kPartCount];
	CRect myDestRect [CNinePartTiledDescription::kPartCount];
	
	desc.calcRects (myBitmapBounds, mySourceRect);
	desc.calcRects (dest, myDestRect);
	
	for (size_t i = 0; i < CNinePartTiledDescription::kPartCount; i++)
		fillRectWithBitmap (bitmap, mySourceRect[i], myDestRect[i], alpha);
}

//-----------------------------------------------------------------------------
CGraphicsPath* CDrawContext::createRoundRectGraphicsPath (const CRect& size, CCoord radius)
{
	if (auto path = createGraphicsPath ())
	{
		path->addRoundRect (size, radius);
		return path;
	}
	return {};
}

//-----------------------------------------------------------------------------
void CDrawContext::pushTransform (const CGraphicsTransform& transformation)
{
	vstgui_assert (!impl->transformStack.empty ());
	const CGraphicsTransform& currentTransform = impl->transformStack.top ();
	CGraphicsTransform newTransform = currentTransform * transformation;
	impl->transformStack.push (newTransform);
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
		impl->device->setTransformMatrix (newTransform);
#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::popTransform ()
{
	vstgui_assert (impl->transformStack.size () > 1);
	impl->transformStack.pop ();
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
		impl->device->setTransformMatrix (impl->transformStack.top ());
#endif
}

//-----------------------------------------------------------------------------
const CGraphicsTransform& CDrawContext::getCurrentTransform () const
{
	return impl->transformStack.top ();
}

//------------------------------------------------------------------------
CCoord CDrawContext::getHairlineSize () const
{
	return 1. / (getScaleFactor () * getCurrentTransform ().m11);
}

//------------------------------------------------------------------------
static PlatformGraphicsDrawStyle convert (CDrawStyle s)
{
	switch (s)
	{
		case CDrawStyle::kDrawFilled:
			return PlatformGraphicsDrawStyle::Filled;
		case CDrawStyle::kDrawStroked:
			return PlatformGraphicsDrawStyle::Stroked;
		case CDrawStyle::kDrawFilledAndStroked:
			return PlatformGraphicsDrawStyle::FilledAndStroked;
		default:
			assert (false);
	}
	return {};
}

//------------------------------------------------------------------------
void CDrawContext::drawLine (const LinePair& line)
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
		impl->device->drawLine (line);
#endif
}

//------------------------------------------------------------------------
void CDrawContext::drawLines (const LineList& lines)
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
		impl->device->drawLines (lines);
#endif
}

//------------------------------------------------------------------------
void CDrawContext::drawPolygon (const PointList& polygonPointList, const CDrawStyle drawStyle)
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
		impl->device->drawPolygon (polygonPointList, convert (drawStyle));
#endif
}

//------------------------------------------------------------------------
void CDrawContext::drawRect (const CRect& rect, const CDrawStyle drawStyle)
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
		impl->device->drawRect (rect, convert (drawStyle));
#endif
}

//------------------------------------------------------------------------
void CDrawContext::drawArc (const CRect& rect, const float startAngle1, const float endAngle2,
							const CDrawStyle drawStyle)
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
		impl->device->drawArc (rect, startAngle1, endAngle2, convert (drawStyle));
#endif
}

//------------------------------------------------------------------------
void CDrawContext::drawEllipse (const CRect& rect, const CDrawStyle drawStyle)
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
		impl->device->drawEllipse (rect, convert (drawStyle));
#endif
}

//------------------------------------------------------------------------
void CDrawContext::drawPoint (const CPoint& point, const CColor& color)
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device && impl->device->drawPoint (point, color))
		return;
#endif

	// if the platform does not support drawing points, emulate it somehow

	saveGlobalState ();

	setLineWidth (1);
	setFrameColor (color);
	CPoint point2 (point);
	point2.x++;
	drawLine (point, point2);

	restoreGlobalState ();
}

//------------------------------------------------------------------------
void CDrawContext::drawBitmap (CBitmap* bitmap, const CRect& dest, const CPoint& offset,
							   float alpha)
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
	{
		double transformedScaleFactor = getScaleFactor ();
		CGraphicsTransform t = getCurrentTransform ();
		if (t.m11 == t.m22 && t.m12 == 0 && t.m21 == 0)
			transformedScaleFactor *= t.m11;
		if (auto pb = bitmap->getBestPlatformBitmapForScaleFactor (transformedScaleFactor))
			impl->device->drawBitmap (*pb, dest, offset, alpha, getBitmapInterpolationQuality ());
	}
#endif
}

//------------------------------------------------------------------------
void CDrawContext::clearRect (const CRect& rect)
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
		impl->device->clearRect (rect);
#endif
}

//------------------------------------------------------------------------
static PlatformGraphicsPathDrawMode convert (CDrawContext::PathDrawMode mode)
{
	switch (mode)
	{
		case CDrawContext::PathDrawMode::kPathFilled:
			return PlatformGraphicsPathDrawMode::Filled;
		case CDrawContext::PathDrawMode::kPathStroked:
			return PlatformGraphicsPathDrawMode::Stroked;
		case CDrawContext::PathDrawMode::kPathFilledEvenOdd:
			return PlatformGraphicsPathDrawMode::FilledEvenOdd;
		default:
			assert (false);
	}
	return {};
}

//------------------------------------------------------------------------
void CDrawContext::drawGraphicsPath (CGraphicsPath* path, PathDrawMode mode,
									 CGraphicsTransform* transformation)
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
	{
		if (auto& pp = path->getPlatformPath (PlatformGraphicsPathFillMode::Ignored))
			impl->device->drawGraphicsPath (*pp.get (), convert (mode), transformation);
	}
#endif
}

//------------------------------------------------------------------------
void CDrawContext::fillLinearGradient (CGraphicsPath* path, const CGradient& gradient,
									   const CPoint& startPoint, const CPoint& endPoint,
									   bool evenOdd, CGraphicsTransform* transformation)
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
	{
		if (auto& platformGradient = gradient.getPlatformGradient ())
		{
			if (auto& pp = path->getPlatformPath (evenOdd ? PlatformGraphicsPathFillMode::Alternate
														  : PlatformGraphicsPathFillMode::Winding))
			{
				impl->device->fillLinearGradient (*pp.get (), *platformGradient, startPoint,
												  endPoint, evenOdd, transformation);
			}
		}
	}
#endif
}

//------------------------------------------------------------------------
void CDrawContext::fillRadialGradient (CGraphicsPath* path, const CGradient& gradient,
									   const CPoint& center, CCoord radius,
									   const CPoint& originOffset, bool evenOdd,
									   CGraphicsTransform* transformation)
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
	{
		if (auto& platformGradient = gradient.getPlatformGradient ())
		{
			if (auto& pp = path->getPlatformPath (evenOdd ? PlatformGraphicsPathFillMode::Alternate
														  : PlatformGraphicsPathFillMode::Winding))
			{
				impl->device->fillRadialGradient (*pp.get (), *platformGradient, center, radius,
												  originOffset, evenOdd, transformation);
			}
		}
	}
#endif
}

//------------------------------------------------------------------------
CGraphicsPath* CDrawContext::createGraphicsPath ()
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
		return new CGraphicsPath (impl->device->getDevice ().getGraphicsPathFactory ());
#endif
	return nullptr;
}

//------------------------------------------------------------------------
CGraphicsPath* CDrawContext::createTextPath (const CFontRef font, UTF8StringPtr text)
{
#if VSTGUI_PLATFORM_DRAWDEVICE
#endif
	return nullptr;
}

//------------------------------------------------------------------------
void CDrawContext::beginDraw ()
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
		impl->device->beginDraw ();
#endif
}

//------------------------------------------------------------------------
void CDrawContext::endDraw ()
{
#if VSTGUI_PLATFORM_DRAWDEVICE
	if (impl->device)
		impl->device->endDraw ();
#endif
}

} // VSTGUI
