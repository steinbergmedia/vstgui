// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __cmoviebitmap__
#define __cmoviebitmap__

#include "ccontrol.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CMovieBitmap Declaration
//! @brief a bitmap view that displays different bitmaps according to its current value
/// @ingroup views
//-----------------------------------------------------------------------------
class CMovieBitmap : public CControl, public IMultiBitmapControl
{
public:
	CMovieBitmap (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset = CPoint (0, 0));
	CMovieBitmap (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset = CPoint (0, 0));
	CMovieBitmap (const CMovieBitmap& movieBitmap);

	void draw (CDrawContext*) override;
	bool sizeToFit () override;

	void setNumSubPixmaps (int32_t numSubPixmaps) override { IMultiBitmapControl::setNumSubPixmaps (numSubPixmaps); invalid (); }

	CLASS_METHODS(CMovieBitmap, CControl)
protected:
	~CMovieBitmap () noexcept override = default;
	CPoint	offset;
};

} // namespace

#endif
