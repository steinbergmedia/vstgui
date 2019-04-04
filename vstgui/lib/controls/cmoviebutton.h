// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "ccontrol.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CMovieButton Declaration
//! @brief a bi-states button with 2 subbitmaps
/// @ingroup controls
//-----------------------------------------------------------------------------
class CMovieButton : public CControl, public IMultiBitmapControl
{
public:
	CMovieButton (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset = CPoint (0, 0));
	CMovieButton (const CRect& size, IControlListener* listener, int32_t tag, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset = CPoint (0, 0));
	CMovieButton (const CMovieButton& movieButton);

	void draw (CDrawContext*) override;

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;
	int32_t onKeyDown (VstKeyCode& keyCode) override;
	bool sizeToFit () override;

	void setNumSubPixmaps (int32_t numSubPixmaps) override { IMultiBitmapControl::setNumSubPixmaps (numSubPixmaps); invalid (); }

	CLASS_METHODS(CMovieButton, CControl)
protected:
	~CMovieButton () noexcept override = default;
	CPoint   offset;
	float    buttonState;

private:
	float    fEntryState;
};

} // VSTGUI
