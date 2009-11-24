//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2009, Steinberg Media Technologies, All Rights Reserved
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

#include "cbitmap.h"
#include "cdrawcontext.h"

#if MAC
	#include "platform/mac/macglobals.h"
	#include "platform/mac/cgdrawcontext.h"
#endif

namespace VSTGUI {
//-----------------------------------------------------------------------------
// CBitmap Implementation
//-----------------------------------------------------------------------------
/*! @class CBitmap
@section cbitmap_alphablend Alpha Blend and Transparency
With Version 3.0 of VSTGUI it is possible to use alpha blended bitmaps. This comes free on Mac OS X and with Windows you need to include libpng.
Per default PNG images will be rendered alpha blended. If you want to use a transparency color with PNG Bitmaps, you need to call setNoAlpha(true) on the bitmap and set the transparency color.
@section cbitmap_macosx Apple Mac OS X
The Bitmaps can be of type PNG, JPEG, PICT, BMP and are stored in the Resources folder of the plugin bundle. 
With the latest version VSTGUI supports all image types supported by the Image I/O Framework.
@section cbitmap_windows Microsoft Windows
The Bitmaps are .bmp files and must be included in the plug (usually using a .rc file).
It's also possible to use png as of version 3.0 if you define the macro USE_LIBPNG and include the libpng and zlib libraries/sources to your project.
@section new New since 3.0
There is a new way to name the bitmaps in the latest version. Instead of using a number identifier for the bitmaps you can now use real names for it.
The CResourceDescription works with both names and numbers. If you use names, you need to use the real filename with extension. Then it gets automaticly
loaded on Mac OS X out of the Resources folder of the vst bundle. On Windows you also specify the resource in the .rc file with the real filename.
@code
// Old way
1001                    BITMAP  DISCARDABLE     "bmp01001.bmp"
// New way
RealFileName.bmp        BITMAP  DISCARDABLE     "RealFileName.bmp"
@endcode
@code
CBitmap* bitmap1 = new CBitmap (1001);
CBitmap* bitmap2 = new CBitmap ("RealFileName.bmp");
@endcode
*/
//-----------------------------------------------------------------------------
CBitmap::CBitmap ()
: platformBitmap (0)
{
}

//-----------------------------------------------------------------------------
CBitmap::CBitmap (const CResourceDescription& desc)
: platformBitmap (0)
, resourceDesc (desc)
{
	platformBitmap = IPlatformBitmap::create ();
	if (!platformBitmap->load (desc))
	{
		platformBitmap->forget ();
		platformBitmap = 0;
	}
}

//-----------------------------------------------------------------------------
CBitmap::CBitmap (CCoord width, CCoord height)
: platformBitmap (0)
{
	CPoint p (width, height);
	platformBitmap = IPlatformBitmap::create (&p);
}

//-----------------------------------------------------------------------------
CBitmap::CBitmap (IPlatformBitmap* platformBitmap)
: platformBitmap (platformBitmap)
{
	if (platformBitmap)
		platformBitmap->remember ();
}

//-----------------------------------------------------------------------------
CBitmap::~CBitmap ()
{
	if (platformBitmap)
		platformBitmap->forget ();
}

//-----------------------------------------------------------------------------
void CBitmap::draw (CDrawContext* context, const CRect& rect, const CPoint& offset, float alpha)
{
	context->drawBitmap (this, rect, offset, alpha);
}

//-----------------------------------------------------------------------------
CCoord CBitmap::getWidth () const
{
	if (platformBitmap)
		return platformBitmap->getSize ().x;
	return 0;
}

//-----------------------------------------------------------------------------
CCoord CBitmap::getHeight () const
{
	if (platformBitmap)
		return platformBitmap->getSize ().y;
	return 0;
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
CNinePartTiledBitmap::CNinePartTiledBitmap (const CResourceDescription& desc, const PartOffsets& offsets)
: CBitmap (desc)
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
	drawParts (inContext, inDestRect, inAlpha);
}

//-----------------------------------------------------------------------------
void CNinePartTiledBitmap::drawParts (CDrawContext* inContext, const CRect& inDestRect, float inAlpha)
{
	CRect	myBitmapBounds (0, 0, getWidth (), getHeight ());
	CRect	mySourceRect [kPartCount];
	CRect	myDestRect [kPartCount];
	
	calcPartRects (myBitmapBounds, offsets, mySourceRect);
	calcPartRects (inDestRect, offsets, myDestRect);
	
	for (size_t i = 0; i < kPartCount; i++)
		drawPart (inContext, mySourceRect[i], myDestRect[i], inAlpha);
}

//-----------------------------------------------------------------------------
void CNinePartTiledBitmap::calcPartRects(const CRect& inBitmapRect, const PartOffsets& inPartOffset, CRect* outRect)
{
	// Center
	CRect myCenter = outRect[kPartCenter]	(inBitmapRect.left		+ inPartOffset.left,
											 inBitmapRect.top		+ inPartOffset.top,
											 inBitmapRect.right		- inPartOffset.right,
											 inBitmapRect.bottom	- inPartOffset.bottom);
	
	// Edges
	outRect[kPartTop]			(myCenter.left,		inBitmapRect.top,	myCenter.right,		myCenter.top);
	outRect[kPartLeft]			(inBitmapRect.left,	myCenter.top,		myCenter.left,		myCenter.bottom);
	outRect[kPartRight]			(myCenter.right,	myCenter.top,		inBitmapRect.right,	myCenter.bottom);
	outRect[kPartBottom]		(myCenter.left,		myCenter.bottom,	myCenter.right,		inBitmapRect.bottom);
	
	// Corners
	outRect[kPartTopLeft]		(inBitmapRect.left,	inBitmapRect.top,	myCenter.left,		myCenter.top);
	outRect[kPartTopRight]		(myCenter.right,	inBitmapRect.top,	inBitmapRect.right,	myCenter.top);
	outRect[kPartBottomLeft]	(inBitmapRect.left,	myCenter.bottom,	myCenter.left,		inBitmapRect.bottom);
	outRect[kPartBottomRight]	(myCenter.right,	myCenter.bottom,	inBitmapRect.right,	inBitmapRect.bottom);
}

//-----------------------------------------------------------------------------
void CNinePartTiledBitmap::drawPart (CDrawContext* inContext, const CRect& inSourceRect, const CRect& inDestRect, float inAlpha)
{
	if (	(inSourceRect.width()	<= 0)
		||	(inSourceRect.height()	<= 0)
		||	(inDestRect.width()		<= 0)
		||	(inDestRect.height()	<= 0))
		return;
	
	CCoord	myLeft;
	CCoord	myTop;
	CPoint	mySourceOffset (inSourceRect.left, inSourceRect.top);
	CRect	myPartRect;
	
	for (myTop = inDestRect.top; myTop < inDestRect.bottom; myTop += inSourceRect.height())
	{
		myPartRect.top		= myTop;
		myPartRect.bottom	= myTop + inSourceRect.height();
		if (myPartRect.bottom > inDestRect.bottom)
			myPartRect.bottom = inDestRect.bottom;
		// The following if should never be true, I guess
		if (myPartRect.height() > inSourceRect.height())
			myPartRect.setHeight(inSourceRect.height());
		
		for (myLeft = inDestRect.left; myLeft < inDestRect.right; myLeft += inSourceRect.width())
		{
			myPartRect.left		= myLeft;
			myPartRect.right	= myLeft + inSourceRect.width();
			if (myPartRect.right > inDestRect.right)
				myPartRect.right = inDestRect.right;
			// The following if should never be true, I guess
			if (myPartRect.width() > inSourceRect.width())
				myPartRect.setWidth(inSourceRect.width());
			
			CBitmap::draw (inContext, myPartRect, mySourceOffset, inAlpha);
		}
	}
}

} // namespace

