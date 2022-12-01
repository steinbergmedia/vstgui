// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "ccontrol.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CMovieBitmap Declaration
//! @brief a bitmap view that displays different bitmaps according to its current value
///
/// Use a CMultiFrameBitmap for its background bitmap.
///
/// @ingroup views uses_multi_frame_bitmaps
//-----------------------------------------------------------------------------
class CMovieBitmap : public CControl
#if VSTGUI_ENABLE_DEPRECATED_METHODS
,
					 public IMultiBitmapControl
#endif
{
public:
	CMovieBitmap (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background,
				  const CPoint& offset = CPoint (0, 0));
	CMovieBitmap (const CMovieBitmap& movieBitmap);

	void draw (CDrawContext*) override;
	bool sizeToFit () override;

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	CMovieBitmap (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps,
				  CCoord heightOfOneImage, CBitmap* background,
				  const CPoint& offset = CPoint (0, 0));
	void setNumSubPixmaps (int32_t numSubPixmaps) override { IMultiBitmapControl::setNumSubPixmaps (numSubPixmaps); invalid (); }
#endif

	static bool useLegacyFrameCalculation;
	CLASS_METHODS(CMovieBitmap, CControl)
protected:
	~CMovieBitmap () noexcept override = default;
	CPoint	offset;
};

} // VSTGUI
