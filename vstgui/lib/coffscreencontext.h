// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __coffscreencontext__
#define __coffscreencontext__

#include "vstguifwd.h"
#include "cdrawcontext.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// COffscreenContext Declaration
//! @brief A draw context using a bitmap as it's back buffer
/*! @class COffscreenContext
There are two usage scenarios :
@section offscreen_usage1 Drawing into a bitmap and then push the contents into another draw context

@code
if (auto offscreen = COffscreenContext::create (frame, 100, 100))
{
	offscreen->beginDraw ();
	// ... 
	// draw into offscreen
	// ...
	offscreen->endDraw ();
	offscreen->copyFrom (otherContext, destRect);
}
@endcode

@section offscreen_usage2 Drawing static content into a bitmap and reuse the bitmap for drawing

@code
if (cachedBitmap == 0)
{
	if (auto offscreen = COffscreenContext::create (frame, 100, 100))
	{
		offscreen->beginDraw ();
		// ... 
		// draw into offscreen
		// ...
		offscreen->endDraw ();
		cachedBitmap = offscreen->getBitmap ();
		if (cachedBitmap)
			cachedBitmap->remember ();
	}
}
if (cachedBitmap)
{
	// ...
}

@endcode

 */
//-----------------------------------------------------------------------------
class COffscreenContext : public CDrawContext
{
public:
	static SharedPointer<COffscreenContext> create (CFrame* frame, CCoord width, CCoord height, double scaleFactor = 1.);

	//-----------------------------------------------------------------------------
	/// @name COffscreenContext Methods
	//-----------------------------------------------------------------------------
	//@{
	void copyFrom (CDrawContext *pContext, CRect destRect, CPoint srcOffset = CPoint (0, 0));	///< copy from offscreen to pContext

	CCoord getWidth () const;
	CCoord getHeight () const;
	//@}

	CBitmap* getBitmap () const { return bitmap; }

protected:
	explicit COffscreenContext (CBitmap* bitmap);
	explicit COffscreenContext (const CRect& surfaceRect);

	SharedPointer<CBitmap> bitmap;
};

} // namespace

#endif // __coffscreencontext__
