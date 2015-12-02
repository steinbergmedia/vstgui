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

#include "cbitmap.h"
#include "cdrawcontext.h"
#include "ccolor.h"
#include "platform/iplatformbitmap.h"
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
	SharedPointer<IPlatformBitmap> platformBitmap = owned (IPlatformBitmap::create ());
	if (platformBitmap && platformBitmap->load (desc))
	{
		bitmaps.push_back (platformBitmap);
	}
}

//-----------------------------------------------------------------------------
CBitmap::CBitmap (CCoord width, CCoord height)
{
	CPoint p (width, height);
	bitmaps.push_back (owned (IPlatformBitmap::create (&p)));
}

//-----------------------------------------------------------------------------
CBitmap::CBitmap (IPlatformBitmap* platformBitmap)
{
	bitmaps.push_back (platformBitmap);
}

//-----------------------------------------------------------------------------
CBitmap::~CBitmap ()
{
}

//-----------------------------------------------------------------------------
void CBitmap::draw (CDrawContext* context, const CRect& rect, const CPoint& offset, float alpha)
{
	CRect clipRect;
	context->getClipRect (clipRect);
	clipRect.bound (rect);
	if (!clipRect.isEmpty ())
		context->drawBitmap (this, rect, offset, alpha);
}

//-----------------------------------------------------------------------------
CCoord CBitmap::getWidth () const
{
	if (getPlatformBitmap ())
		return getPlatformBitmap ()->getSize ().x / getPlatformBitmap ()->getScaleFactor ();
	return 0;
}

//-----------------------------------------------------------------------------
CCoord CBitmap::getHeight () const
{
	if (getPlatformBitmap ())
		return getPlatformBitmap ()->getSize ().y / getPlatformBitmap ()->getScaleFactor ();
	return 0;
}

//-----------------------------------------------------------------------------
IPlatformBitmap* CBitmap::getPlatformBitmap () const
{
	return bitmaps.empty () ? 0 : bitmaps[0];
}

//-----------------------------------------------------------------------------
void CBitmap::setPlatformBitmap (IPlatformBitmap* bitmap)
{
	if (bitmaps.empty ())
		bitmaps.push_back (bitmap);
	else
		bitmaps[0] = bitmap;
}

//-----------------------------------------------------------------------------
bool CBitmap::addBitmap (IPlatformBitmap* platformBitmap)
{
	double scaleFactor = platformBitmap->getScaleFactor ();
	CPoint size (getWidth (), getHeight ());
	CPoint bitmapSize = platformBitmap->getSize ();
	bitmapSize.x /= scaleFactor;
	bitmapSize.y /= scaleFactor;
	if (size != bitmapSize)
	{
		vstgui_assert (size == bitmapSize, "wrong bitmap size");
		return false;
	}
	VSTGUI_RANGE_BASED_FOR_LOOP (BitmapVector, bitmaps, BitmapPointer, bitmap)
		if (bitmap->getScaleFactor () == scaleFactor || bitmap == platformBitmap)
		{
			vstgui_assert (bitmap->getScaleFactor () != scaleFactor && bitmap != platformBitmap);
			return false;
		}
	VSTGUI_RANGE_BASED_FOR_LOOP_END
	bitmaps.push_back (platformBitmap);
	return true;
}

//-----------------------------------------------------------------------------
IPlatformBitmap* CBitmap::getBestPlatformBitmapForScaleFactor (double scaleFactor) const
{
	if (bitmaps.empty ())
		return 0;
	IPlatformBitmap* bestBitmap = bitmaps[0];
	double bestDiff = std::abs (scaleFactor - bestBitmap->getScaleFactor ());
	VSTGUI_RANGE_BASED_FOR_LOOP (BitmapVector, bitmaps, BitmapPointer, bitmap)
		if (bitmap->getScaleFactor () == scaleFactor)
			return bitmap;
		else if (std::abs (scaleFactor - bitmap->getScaleFactor ()) <= bestDiff && bitmap->getScaleFactor () > bestBitmap->getScaleFactor ())
		{
			bestBitmap = bitmap;
			bestDiff = std::abs (scaleFactor - bitmap->getScaleFactor ());
		}
	VSTGUI_RANGE_BASED_FOR_LOOP_END

	return bestBitmap;
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
CNinePartTiledBitmap::CNinePartTiledBitmap (IPlatformBitmap* platformBitmap, const CNinePartTiledDescription& offsets)
: CBitmap (platformBitmap)
, offsets (offsets)
{
}

//-----------------------------------------------------------------------------
CNinePartTiledBitmap::~CNinePartTiledBitmap ()
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
: bitmap (0)
, pixelAccess (0)
, currentPos (0)
, address (0)
, bytesPerRow (0)
, maxX (0)
, maxY (0)
, x (0)
, y (0)
{
}

//------------------------------------------------------------------------
CBitmapPixelAccess::~CBitmapPixelAccess ()
{
	if (pixelAccess)
		pixelAccess->forget ();
}

//------------------------------------------------------------------------
void CBitmapPixelAccess::init (CBitmap* _bitmap, IPlatformBitmapPixelAccess* _pixelAccess)
{
	bitmap = _bitmap;
	pixelAccess = _pixelAccess;
	address = currentPos = pixelAccess->getAddress ();
	bytesPerRow = pixelAccess->getBytesPerRow ();
	maxX = (uint32_t)(bitmap->getPlatformBitmap ()->getSize ().x)-1;
	maxY = (uint32_t)(bitmap->getPlatformBitmap ()->getSize ().y)-1;
}

/// @cond ignore
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
template <int32_t redPosition, int32_t greenPosition, int32_t bluePosition, int32_t alphaPosition>
class CBitmapPixelAccessOrder : public CBitmapPixelAccess
{
public:
	void getColor (CColor& c) const VSTGUI_OVERRIDE_VMETHOD
	{
		c.red = currentPos[redPosition];
		c.green = currentPos[greenPosition];
		c.blue = currentPos[bluePosition];
		c.alpha = currentPos[alphaPosition];
	}
	void setColor (const CColor& c) VSTGUI_OVERRIDE_VMETHOD
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
	if (bitmap == 0 || bitmap->getPlatformBitmap () == 0)
		return 0;
	IPlatformBitmapPixelAccess* pixelAccess = bitmap->getPlatformBitmap ()->lockPixels (alphaPremultiplied);
	if (pixelAccess == 0)
		return 0;
	CBitmapPixelAccess* result = 0;
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

} // namespace VSTGUI

