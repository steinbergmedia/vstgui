// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cbitmap.h"
#include "cdrawcontext.h"
#include "ccolor.h"
#include "platform/iplatformbitmap.h"
#include "platform/platformfactory.h"
#include <cassert>

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CBitmap Implementation
//-----------------------------------------------------------------------------
/*! @class CBitmap
@section changes_version_4 Changes in 4.0
In Version 4.0 CBitmap was simplified. Previous versions supported drawing a color transparent of the bitmap. Since CBitmap supports
alpha drawing of bitmaps since some time, it's now the only way of drawing a bitmap with some parts transparent.
@section supported_file_formats Supported file formats
File format support is handled in a platform dependent way. On Windows GDI+ is used to import images. On Mac OS X CoreGraphics is used to import them.
For cross platform compatibility it is recommended to use PNG files.
@section loading Loading Bitmaps
You load a bitmap via a CResourceDescription which can hold a string or a number.
If you use names, you need to use the real filename with extension. Then it gets automaticly
loaded on Mac OS X out of the Resources folder of the vst bundle. On Windows you also specify the resource in the .rc file with the real filename.
@code
// using a number
1001                    PNG  DISCARDABLE     "bmp01001.png"
// using a string
RealFileName.png        PNG  DISCARDABLE     "RealFileName.png"
@endcode
@code
CBitmap* bitmap1 = new CBitmap (1001); // number
CBitmap* bitmap2 = new CBitmap ("RealFileName.png"); // string
@endcode
*/
//-----------------------------------------------------------------------------
CBitmap::CBitmap ()
{
}

//-----------------------------------------------------------------------------
CBitmap::CBitmap (const CResourceDescription& desc)
: resourceDesc (desc)
{
	if (auto platformBitmap = getPlatformFactory ().createBitmap (desc))
		bitmaps.emplace_back (platformBitmap);
}

//-----------------------------------------------------------------------------
CBitmap::CBitmap (CCoord width, CCoord height)
{
	if (auto platformBitmap = getPlatformFactory ().createBitmap ({width, height}))
		bitmaps.emplace_back (platformBitmap);
}

//------------------------------------------------------------------------
CBitmap::CBitmap (CPoint size, double scaleFactor)
{
	size.x *= scaleFactor;
	size.y *= scaleFactor;
	size.makeIntegral ();
	if (auto platformBitmap = getPlatformFactory ().createBitmap (size))
	{
		platformBitmap->setScaleFactor (scaleFactor);
		bitmaps.emplace_back (platformBitmap);
	}
}

//-----------------------------------------------------------------------------
CBitmap::CBitmap (const PlatformBitmapPtr& platformBitmap)
{
	bitmaps.emplace_back (platformBitmap);
}

//-----------------------------------------------------------------------------
void CBitmap::draw (CDrawContext* context, const CRect& rect, const CPoint& offset, float alpha)
{
	drawClipped (context, rect, [&] () {
		context->drawBitmap (this, rect, offset, alpha);
	});
}

//-----------------------------------------------------------------------------
CCoord CBitmap::getWidth () const
{
	if (auto pb = getPlatformBitmap ())
		return pb->getSize ().x / pb->getScaleFactor ();
	return 0;
}

//-----------------------------------------------------------------------------
CCoord CBitmap::getHeight () const
{
	if (auto pb = getPlatformBitmap ())
		return pb->getSize ().y / pb->getScaleFactor ();
	return 0;
}

//------------------------------------------------------------------------
CPoint CBitmap::getSize () const
{
	CPoint p;
	if (auto pb = getPlatformBitmap ())
	{
		auto scaleFactor = pb->getScaleFactor ();
		p = pb->getSize ();
		p.x /= scaleFactor;
		p.y /= scaleFactor;
	}
	return p;
}

//-----------------------------------------------------------------------------
auto CBitmap::getPlatformBitmap () const -> PlatformBitmapPtr
{
	return bitmaps.empty () ? nullptr : bitmaps[0];
}

//-----------------------------------------------------------------------------
void CBitmap::setPlatformBitmap (const PlatformBitmapPtr& bitmap)
{
	if (bitmaps.empty ())
		bitmaps.emplace_back (bitmap);
	else
		bitmaps[0] = bitmap;
}

//-----------------------------------------------------------------------------
bool CBitmap::addBitmap (const PlatformBitmapPtr& platformBitmap)
{
	double scaleFactor = platformBitmap->getScaleFactor ();
	CPoint size = getSize ();
	CPoint bitmapSize = platformBitmap->getSize ();
	bitmapSize.x /= scaleFactor;
	bitmapSize.y /= scaleFactor;
	if (size != bitmapSize)
	{
		vstgui_assert (size == bitmapSize, "wrong bitmap size");
		return false;
	}
	for (const auto& bitmap : bitmaps)
	{
		if (bitmap->getScaleFactor () == scaleFactor || bitmap == platformBitmap)
		{
			vstgui_assert (bitmap->getScaleFactor () != scaleFactor && bitmap != platformBitmap);
			return false;
		}
	}
	bitmaps.emplace_back (platformBitmap);
	return true;
}

//-----------------------------------------------------------------------------
auto CBitmap::getBestPlatformBitmapForScaleFactor (double scaleFactor) const -> PlatformBitmapPtr
{
	if (bitmaps.empty ())
		return nullptr;
	auto bestBitmap = bitmaps[0];
	double bestDiff = std::abs (scaleFactor - bestBitmap->getScaleFactor ());
	for (const auto& bitmap : bitmaps)
	{
		if (bitmap->getScaleFactor () == scaleFactor)
			return bitmap;
		else if (std::abs (scaleFactor - bitmap->getScaleFactor ()) <= bestDiff && bitmap->getScaleFactor () > bestBitmap->getScaleFactor ())
		{
			bestBitmap = bitmap;
			bestDiff = std::abs (scaleFactor - bitmap->getScaleFactor ());
		}
	}

	return bestBitmap;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CMultiFrameBitmap::CMultiFrameBitmap (const CResourceDescription& desc,
									  CMultiFrameBitmapDescription multiFrameDesc)
: CBitmap (desc), description (multiFrameDesc)
{
}

//-----------------------------------------------------------------------------
bool CMultiFrameBitmap::setMultiFrameDesc (CMultiFrameBitmapDescription desc)
{
	if (desc.frameSize.x * desc.framesPerRow > getSize ().x)
		return false;
	if (desc.frameSize.y * (desc.numFrames / desc.framesPerRow) > getSize ().y)
		return false;
	description = desc;
	return true;
}

//-----------------------------------------------------------------------------
CMultiFrameBitmapDescription CMultiFrameBitmap::getMultiFrameDesc () const { return description; }

//-----------------------------------------------------------------------------
CPoint CMultiFrameBitmap::getFrameSize () const { return description.frameSize; }

//-----------------------------------------------------------------------------
uint16_t CMultiFrameBitmap::getNumFrames () const { return description.numFrames; }

//-----------------------------------------------------------------------------
uint16_t CMultiFrameBitmap::getNumFramesPerRow () const { return description.framesPerRow; }

//-----------------------------------------------------------------------------
CRect CMultiFrameBitmap::calcFrameRect (uint32_t frameIndex) const
{
	if (getNumFrames () == 0)
		return CRect ({}, getSize ());
	if (frameIndex >= getNumFrames ())
		frameIndex = getNumFrames () - 1;
	auto rowIndex = frameIndex / getNumFramesPerRow ();
	auto colIndex = frameIndex - (rowIndex * getNumFramesPerRow ());
	CRect r;
	r.left = getFrameSize ().x * colIndex;
	r.right = r.left + getFrameSize ().x;
	r.top = getFrameSize ().y * rowIndex;
	r.bottom = r.top + getFrameSize ().y;
	return r;
}

//-----------------------------------------------------------------------------
void CMultiFrameBitmap::drawFrame (CDrawContext* context, uint16_t frameIndex, CPoint pos)
{
	auto fr = calcFrameRect (frameIndex);
	auto r = CRect (pos, getFrameSize ());
	draw (context, r, fr.getTopLeft ());
}

//-----------------------------------------------------------------------------
// CNinePartTiledBitmap Implementation
//-----------------------------------------------------------------------------
/*! @class CNinePartTiledBitmap
A nine-part tiled bitmap is tiled in nine parts which are drawing according to its part offsets:
- top left corner
- top right corner
- bottom left corner
- bottom right corner
- top edge, repeated as often as necessary and clipped appropriately
- left edge, dto.
- right edge, dto.
- bottom edge, dto.
- center, repeated horizontally and vertically as often as necessary

@verbatim
|------------------------------------------------------------------------------------------------|
| Top-Left Corner    |         <----        Top Edge        ---->          |    Top-Right Corner |
|--------------------|-----------------------------------------------------|---------------------|
|         ^          |                         ^                           |          ^          |
|         |          |                         |                           |          |          |
|     Left Edge      |         <----         Center         ---->          |      Right Edge     |
|         |          |                         |                           |          |          |
|         v          |                         v                           |          v          |
|--------------------|-----------------------------------------------------|---------------------|
| Bottom-Left Corner |         <----       Bottom Edge      ---->          | Bottom-Right Corner |
|------------------------------------------------------------------------------------------------|
@endverbatim

*/
//-----------------------------------------------------------------------------
CNinePartTiledBitmap::CNinePartTiledBitmap (const CResourceDescription& desc, const CNinePartTiledDescription& offsets)
: CBitmap (desc)
, offsets (offsets)
{
}

//-----------------------------------------------------------------------------
CNinePartTiledBitmap::CNinePartTiledBitmap (const PlatformBitmapPtr& platformBitmap, const CNinePartTiledDescription& offsets)
: CBitmap (platformBitmap)
, offsets (offsets)
{
}

//-----------------------------------------------------------------------------
void CNinePartTiledBitmap::draw (CDrawContext* inContext, const CRect& inDestRect, const CPoint& offset, float inAlpha)
{
	inContext->drawBitmapNinePartTiled (this, inDestRect, offsets, inAlpha);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
CBitmapPixelAccess::CBitmapPixelAccess ()
: bitmap (nullptr)
, pixelAccess (nullptr)
, currentPos (nullptr)
, address (nullptr)
, bytesPerRow (0)
, maxX (0)
, maxY (0)
, x (0)
, y (0)
{
}

//------------------------------------------------------------------------
void CBitmapPixelAccess::init (CBitmap* _bitmap, IPlatformBitmapPixelAccess* _pixelAccess)
{
	bitmap = _bitmap;
	pixelAccess = _pixelAccess;
	address = currentPos = pixelAccess->getAddress ();
	bytesPerRow = pixelAccess->getBytesPerRow ();
	auto size = bitmap->getPlatformBitmap ()->getSize ();
	maxX = static_cast<uint32_t> (size.x) - 1;
	maxY = static_cast<uint32_t> (size.y) - 1;
}

/// @cond ignore
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
template <int32_t redPosition, int32_t greenPosition, int32_t bluePosition, int32_t alphaPosition>
class CBitmapPixelAccessOrder : public CBitmapPixelAccess
{
public:
	void getColor (CColor& c) const override
	{
		c.red = currentPos[redPosition];
		c.green = currentPos[greenPosition];
		c.blue = currentPos[bluePosition];
		c.alpha = currentPos[alphaPosition];
	}
	void setColor (const CColor& c) override
	{
		currentPos[redPosition] = c.red;
		currentPos[greenPosition] = c.green;
		currentPos[bluePosition] = c.blue;
		currentPos[alphaPosition] = c.alpha;
	}
};
/// @endcond

//------------------------------------------------------------------------
CBitmapPixelAccess* CBitmapPixelAccess::create (CBitmap* bitmap, bool alphaPremultiplied)
{
	if (bitmap == nullptr || bitmap->getPlatformBitmap () == nullptr)
		return nullptr;
	auto pixelAccess = bitmap->getPlatformBitmap ()->lockPixels (alphaPremultiplied);
	if (pixelAccess == nullptr)
		return nullptr;
	CBitmapPixelAccess* result = nullptr;
	switch (pixelAccess->getPixelFormat ())
	{
		case IPlatformBitmapPixelAccess::kARGB: result = new CBitmapPixelAccessOrder<1,2,3,0> (); break;
		case IPlatformBitmapPixelAccess::kRGBA: result = new CBitmapPixelAccessOrder<0,1,2,3> (); break;
		case IPlatformBitmapPixelAccess::kABGR: result = new CBitmapPixelAccessOrder<3,2,1,0> (); break;
		case IPlatformBitmapPixelAccess::kBGRA: result = new CBitmapPixelAccessOrder<2,1,0,3> (); break;
	}
	if (result)
		result->init (bitmap, pixelAccess);
	return result;
}

} // VSTGUI
