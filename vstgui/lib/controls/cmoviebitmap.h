// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "ccontrol.h"
#include "../cbitmap.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CMovieBitmap Declaration
//! @brief a bitmap view that displays different bitmaps according to its current value
///
/// Use a CMultiFrameBitmap for its background bitmap.
///
/// @ingroup views uses_multi_frame_bitmaps
//-----------------------------------------------------------------------------
class CMovieBitmap : public CControl,
					 public MultiFrameBitmapView<CMovieBitmap>
{
public:
	CMovieBitmap (const CRect& size, IControlListener* listener, int32_t tag,
				  const SPtr<CBitmap>& background);

	void draw (CDrawContext&) override;
	bool sizeToFit () override;

protected:
	VSTGUI_SHAREDPTR_FRIEND (CMovieBitmap)
	~CMovieBitmap () noexcept override = default;
};

} // VSTGUI
