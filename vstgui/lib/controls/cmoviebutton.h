// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "ccontrol.h"
#include "../cbitmap.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CMovieButton Declaration
//! @brief a bi-states button with 2 subbitmaps
///
/// Use a CMultiFrameBitmap for its background bitmap.
///
/// @ingroup controls uses_multi_frame_bitmaps
//-----------------------------------------------------------------------------
class CMovieButton : public CControl,
					 public MultiFrameBitmapView<CMovieButton>
{
public:
	CMovieButton (const CRect& size, IControlListener* listener, int32_t tag,
				  const SharedPointer<CBitmap>& background);
	CMovieButton (const CMovieButton& movieButton);

	void draw (CDrawContext&) override;

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;
	void onKeyboardEvent (KeyboardEvent& event) override;
	bool sizeToFit () override;

	CLASS_METHODS(CMovieButton, CControl)
protected:
	~CMovieButton () noexcept override = default;
	float    buttonState;

private:
	float    fEntryState;
};

} // VSTGUI
