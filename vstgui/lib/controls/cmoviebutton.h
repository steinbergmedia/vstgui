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
#if VSTGUI_ENABLE_DEPRECATED_METHODS
,
					 public IMultiBitmapControl
#endif
{
public:
	CMovieButton (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background);
	CMovieButton (const CMovieButton& movieButton);

	void draw (CDrawContext*) override;

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;
	void onKeyboardEvent (KeyboardEvent& event) override;
	bool sizeToFit () override;

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	CMovieButton (const CRect& size, IControlListener* listener, int32_t tag,
				  CCoord heightOfOneImage, CBitmap* background,
				  const CPoint& offset = CPoint (0, 0));
	void setNumSubPixmaps (int32_t numSubPixmaps) override { IMultiBitmapControl::setNumSubPixmaps (numSubPixmaps); invalid (); }
#endif

	CLASS_METHODS(CMovieButton, CControl)
protected:
	~CMovieButton () noexcept override = default;
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	CPoint offset {};
#endif
	float    buttonState;

private:
	float    fEntryState;
};

} // VSTGUI
