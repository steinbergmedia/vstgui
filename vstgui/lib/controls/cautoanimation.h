// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "ccontrol.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CAutoAnimation Declaration
//!
/// @ingroup controls
//-----------------------------------------------------------------------------
class CAutoAnimation : public CControl
#if VSTGUI_ENABLE_DEPRECATED_METHODS
,
					   public IMultiBitmapControl
#endif
{
public:
	CAutoAnimation (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background,
					const CPoint& offset = CPoint (0, 0));
	CAutoAnimation (const CAutoAnimation& autoAnimation);

	void draw (CDrawContext*) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;

	//-----------------------------------------------------------------------------
	/// @name CAutoAnimation Methods
	//-----------------------------------------------------------------------------
	//@{
	/** enabled drawing */
	virtual void openWindow ();
	/** disable drawing */
	virtual void closeWindow ();

	/** the next sub bitmap should be displayed */
	virtual void nextPixmap ();
	/** the previous sub bitmap should be displayed */
	virtual void previousPixmap ();

	bool    isWindowOpened () const { return bWindowOpened; }
	//@}

	void setBackground (CBitmap* background) override;

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	CAutoAnimation (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps,
					CCoord heightOfOneImage, CBitmap* background,
					const CPoint& offset = CPoint (0, 0));
	void setNumSubPixmaps (int32_t numSubPixmaps) override
	{
		IMultiBitmapControl::setNumSubPixmaps (numSubPixmaps);
		invalid ();
	}
#endif
	CLASS_METHODS(CAutoAnimation, CControl)
protected:
	~CAutoAnimation () noexcept override = default;

	void updateMinMaxFromBackground ();

	CPoint	offset;

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	CCoord	totalHeightOfBitmap;
#endif

	bool	bWindowOpened;
};

} // VSTGUI
