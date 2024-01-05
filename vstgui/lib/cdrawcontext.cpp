// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cdrawcontext.h"
#include "cgraphicspath.h"
#include "cgradient.h"
#include "cbitmap.h"
#include "cstring.h"
#include "platform/iplatformgraphicsdevice.h"
#include "platform/iplatformfont.h"
#include "platform/iplatformgraphicspath.h"
#include "platform/iplatformgradient.h"
#include "platform/platformfactory.h"
#include <cassert>
#include <stack>

namespace VSTGUI {

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
	//-----------------------------------------------------------------------------
	struct State
	{
		SharedPointer<CFontDesc> font;
		CColor frameColor {kTransparentCColor};
		CColor fillColor {kTransparentCColor};
		CColor fontColor {kTransparentCColor};
		CCoord frameWidth {0.};
		CPoint penLoc {};
		CRect clipRect {};
		CLineStyle lineStyle {kLineOnOffDash};
		CDrawMode drawMode {kAntiAliasing};
		float globalAlpha {1.f};
		BitmapInterpolationQuality bitmapQuality {BitmapInterpolationQuality::kDefault};

		State () = default;
		State (const State& state);
		State& operator= (const State& state) = default;
		State (State&& state) noexcept;
		State& operator= (State&& state) noexcept;
	};

	UTF8String* drawStringHelper {nullptr};
	CRect surfaceRect;
	double scaleFactor {1.};

	State currentState;

	std::stack<State> globalStatesStack;
	std::stack<CGraphicsTransform> transformStack;

	PlatformGraphicsDeviceContextPtr device;
};

//-----------------------------------------------------------------------------
CDrawContext::Impl::State::State (const State& state) { *this = state; }

//-----------------------------------------------------------------------------
CDrawContext::Impl::State::State (State&& state) noexcept { *this = std::move (state); }

//-----------------------------------------------------------------------------
CDrawContext::Impl::State&
	CDrawContext::Impl::State::operator= (CDrawContext::Impl::State&& state) noexcept
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
	impl->device = device;
	impl->scaleFactor = scaleFactor;
	setClipRect (surfaceRect);
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
	return impl->device;
}

//-----------------------------------------------------------------------------
const CRect& CDrawContext::getSurfaceRect () const { return impl->surfaceRect; }

//------------------------------------------------------------------------
double CDrawContext::getScaleFactor () const { return impl->scaleFactor; }

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
	impl->globalStatesStack.push (impl->currentState);
	if (impl->device)
		impl->device->saveGlobalState ();
}

//-----------------------------------------------------------------------------
void CDrawContext::restoreGlobalState ()
{
	if (impl->device)
		impl->device->restoreGlobalState ();
	if (!impl->globalStatesStack.empty ())
	{
		impl->currentState = std::move (impl->globalStatesStack.top ());
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
	impl->currentState.bitmapQuality = quality;
}

//-----------------------------------------------------------------------------
BitmapInterpolationQuality CDrawContext::getBitmapInterpolationQuality () const
{
	return impl->currentState.bitmapQuality;
}

//-----------------------------------------------------------------------------
void CDrawContext::setLineStyle (const CLineStyle& style)
{
	if (impl->device)
		impl->device->setLineStyle (style);
	impl->currentState.lineStyle = style;
}

//-----------------------------------------------------------------------------
const CLineStyle& CDrawContext::getLineStyle () const { return impl->currentState.lineStyle; }

//-----------------------------------------------------------------------------
void CDrawContext::setLineWidth (CCoord width)
{
	if (impl->device)
		impl->device->setLineWidth (width);
	impl->currentState.frameWidth = width;
}

//-----------------------------------------------------------------------------
CCoord CDrawContext::getLineWidth () const { return impl->currentState.frameWidth; }

//-----------------------------------------------------------------------------
void CDrawContext::setDrawMode (CDrawMode mode)
{
	if (impl->device)
		impl->device->setDrawMode (mode);
	impl->currentState.drawMode = mode;
}

//-----------------------------------------------------------------------------
CDrawMode CDrawContext::getDrawMode () const { return impl->currentState.drawMode; }

//-----------------------------------------------------------------------------
CRect& CDrawContext::getClipRect (CRect &clip) const
{
	clip = impl->currentState.clipRect;
	getCurrentTransform ().inverse ().transform (clip);
	clip.normalize ();
	return clip;
}

//-----------------------------------------------------------------------------
const CRect& CDrawContext::getAbsoluteClipRect () const { return impl->currentState.clipRect; }

//-----------------------------------------------------------------------------
void CDrawContext::setClipRect (const CRect &clip)
{
	impl->currentState.clipRect = clip;
	getCurrentTransform ().transform (impl->currentState.clipRect);
	impl->currentState.clipRect.normalize ();
	if (impl->device)
		impl->device->setClipRect (impl->currentState.clipRect);
}

//-----------------------------------------------------------------------------
void CDrawContext::resetClipRect ()
{
	if (impl->device)
		impl->device->setClipRect (getSurfaceRect ());
	impl->currentState.clipRect = getSurfaceRect ();
}

//-----------------------------------------------------------------------------
void CDrawContext::setFillColor (const CColor& color)
{
	if (impl->device)
		impl->device->setFillColor (color);
	impl->currentState.fillColor = color;
}

//-----------------------------------------------------------------------------
CColor CDrawContext::getFillColor () const { return impl->currentState.fillColor; }

//-----------------------------------------------------------------------------
void CDrawContext::setFrameColor (const CColor& color)
{
	if (impl->device)
		impl->device->setFrameColor (color);
	impl->currentState.frameColor = color;
}

//-----------------------------------------------------------------------------
CColor CDrawContext::getFrameColor () const { return impl->currentState.frameColor; }

//-----------------------------------------------------------------------------
void CDrawContext::setFontColor (const CColor& color) { impl->currentState.fontColor = color; }

//-----------------------------------------------------------------------------
CColor CDrawContext::getFontColor () const { return impl->currentState.fontColor; }

//-----------------------------------------------------------------------------
void CDrawContext::setFont (const CFontRef newFont, const CCoord& size, const int32_t& style)
{
	if (newFont == nullptr)
		return;
	if ((size > 0 && newFont->getSize () != size) || (style != -1 && newFont->getStyle () != style))
	{
		impl->currentState.font = makeOwned<CFontDesc> (*newFont);
		if (size > 0)
			impl->currentState.font->setSize (size);
		if (style != -1)
			impl->currentState.font->setStyle (style);
	}
	else
	{
		impl->currentState.font = newFont;
	}
}

//-----------------------------------------------------------------------------
const CFontRef CDrawContext::getFont () const { return impl->currentState.font; }

//-----------------------------------------------------------------------------
void CDrawContext::setGlobalAlpha (float newAlpha)
{
	if (impl->device)
		impl->device->setGlobalAlpha (newAlpha);
	impl->currentState.globalAlpha = newAlpha;
}

//-----------------------------------------------------------------------------
float CDrawContext::getGlobalAlpha () const { return impl->currentState.globalAlpha; }

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
	if (impl->currentState.font == nullptr || string == nullptr)
		return result;

	if (auto painter = impl->currentState.font->getFontPainter ())
		result = painter->getStringWidth (impl->device, string, true);

	return result;
}

//------------------------------------------------------------------------
void CDrawContext::drawString (IPlatformString* string, const CRect& _rect, const CHoriTxtAlign hAlign, bool antialias)
{
	if (!string || impl->currentState.font == nullptr)
		return;
	auto painter = impl->currentState.font->getFontPainter ();
	if (painter == nullptr)
		return;
	
	CRect rect (_rect);
	
	double capHeight = -1;
	auto platformFont = impl->currentState.font->getPlatformFont ();
	if (platformFont)
		capHeight = platformFont->getCapHeight ();
	
	if (capHeight > 0.)
		rect.bottom -= (rect.getHeight () / 2. - capHeight / 2.);
	else
		rect.bottom -= (rect.getHeight () / 2. - impl->currentState.font->getSize () / 2.) + 1.;
	if (hAlign != kLeftText)
	{
		CCoord stringWidth = painter->getStringWidth (impl->device, string, antialias);
		if (hAlign == kRightText)
			rect.left = rect.right - stringWidth;
		else
			rect.left = rect.left + (rect.getWidth () / 2.) - (stringWidth / 2.);
	}

	painter->drawString (impl->device, string, CPoint (rect.left, rect.bottom),
						 impl->currentState.fontColor, antialias);
}

//------------------------------------------------------------------------
void CDrawContext::drawString (IPlatformString* string, const CPoint& point, bool antialias)
{
	if (string == nullptr || impl->currentState.font == nullptr)
		return;

	if (auto painter = impl->currentState.font->getFontPainter ())
		painter->drawString (impl->device, string, point, impl->currentState.fontColor, antialias);
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
	if (srcRect.getWidth () == dstRect.getWidth () && srcRect.getHeight () == dstRect.getHeight ())
	{
		drawBitmap (bitmap, dstRect, srcRect.getTopLeft (), alpha);
		return;
	}

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
	if (impl->device)
		impl->device->setTransformMatrix (newTransform);
}

//-----------------------------------------------------------------------------
void CDrawContext::popTransform ()
{
	vstgui_assert (impl->transformStack.size () > 1);
	impl->transformStack.pop ();
	if (impl->device)
		impl->device->setTransformMatrix (impl->transformStack.top ());
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
	if (impl->device)
		impl->device->drawLine (line);
}

//------------------------------------------------------------------------
void CDrawContext::drawLines (const LineList& lines)
{
	if (impl->device)
		impl->device->drawLines (lines);
}

//------------------------------------------------------------------------
void CDrawContext::drawPolygon (const PointList& polygonPointList, const CDrawStyle drawStyle)
{
	if (impl->device)
		impl->device->drawPolygon (polygonPointList, convert (drawStyle));
}

//------------------------------------------------------------------------
void CDrawContext::drawRect (const CRect& rect, const CDrawStyle drawStyle)
{
	if (impl->device)
		impl->device->drawRect (rect, convert (drawStyle));
}

//------------------------------------------------------------------------
void CDrawContext::drawArc (const CRect& rect, const float startAngle1, const float endAngle2,
							const CDrawStyle drawStyle)
{
	if (impl->device)
		impl->device->drawArc (rect, startAngle1, endAngle2, convert (drawStyle));
}

//------------------------------------------------------------------------
void CDrawContext::drawEllipse (const CRect& rect, const CDrawStyle drawStyle)
{
	if (impl->device)
		impl->device->drawEllipse (rect, convert (drawStyle));
}

//------------------------------------------------------------------------
void CDrawContext::drawPoint (const CPoint& point, const CColor& color)
{
	if (impl->device && impl->device->drawPoint (point, color))
		return;

	// if the platform does not support drawing points, emulate it somehow

	saveGlobalState ();

	CRect r (point.x, point.y, point.x, point.y);
	r.inset (-0.5, -0.5);
	setDrawMode (kAliasing);
	setFillColor (color);
	drawRect (r, kDrawFilled);

	restoreGlobalState ();
}

//------------------------------------------------------------------------
void CDrawContext::drawBitmap (CBitmap* bitmap, const CRect& dest, const CPoint& offset,
							   float alpha)
{
	if (impl->device)
	{
		double transformedScaleFactor = getScaleFactor ();
		CGraphicsTransform t = getCurrentTransform ();
		if (t.m11 == t.m22 && t.m12 == 0 && t.m21 == 0)
			transformedScaleFactor *= t.m11;
		if (auto pb = bitmap->getBestPlatformBitmapForScaleFactor (transformedScaleFactor))
			impl->device->drawBitmap (*pb, dest, offset, alpha, getBitmapInterpolationQuality ());
	}
}

//------------------------------------------------------------------------
void CDrawContext::clearRect (const CRect& rect)
{
	if (impl->device)
		impl->device->clearRect (rect);
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
	if (impl->device)
	{
		if (auto& pp = path->getPlatformPath (mode == kPathFilledEvenOdd
												  ? PlatformGraphicsPathFillMode::Alternate
												  : PlatformGraphicsPathFillMode::Winding))
			impl->device->drawGraphicsPath (*pp.get (), convert (mode), transformation);
	}
}

//------------------------------------------------------------------------
void CDrawContext::fillLinearGradient (CGraphicsPath* path, const CGradient& gradient,
									   const CPoint& startPoint, const CPoint& endPoint,
									   bool evenOdd, CGraphicsTransform* transformation)
{
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
}

//------------------------------------------------------------------------
void CDrawContext::fillRadialGradient (CGraphicsPath* path, const CGradient& gradient,
									   const CPoint& center, CCoord radius,
									   const CPoint& originOffset, bool evenOdd,
									   CGraphicsTransform* transformation)
{
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
}

//------------------------------------------------------------------------
CGraphicsPath* CDrawContext::createGraphicsPath ()
{
	if (impl->device)
		return new CGraphicsPath (impl->device->getGraphicsPathFactory (), nullptr);
	return nullptr;
}

//------------------------------------------------------------------------
CGraphicsPath* CDrawContext::createTextPath (const CFontRef font, UTF8StringPtr text)
{
	if (impl->device)
	{
		auto platformFont = font->getPlatformFont ();
		auto pathFactory = impl->device->getGraphicsPathFactory ();
		if (platformFont && pathFactory)
		{
			if (auto textPath = pathFactory->createTextPath (platformFont, text))
				return new CGraphicsPath (pathFactory, std::move (textPath));
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------
void CDrawContext::beginDraw ()
{
	if (impl->device)
		impl->device->beginDraw ();
}

//------------------------------------------------------------------------
void CDrawContext::endDraw ()
{
	if (impl->device)
		impl->device->endDraw ();
}

} // VSTGUI
