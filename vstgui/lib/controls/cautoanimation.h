// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __cautoanimation__
#define __cautoanimation__

#include "ccontrol.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CAutoAnimation Declaration
//!
/// @ingroup controls
//-----------------------------------------------------------------------------
class CAutoAnimation : public CControl, public IMultiBitmapControl
{
public:
	CAutoAnimation (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset = CPoint (0, 0));
	CAutoAnimation (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset = CPoint (0, 0));
	CAutoAnimation (const CAutoAnimation& autoAnimation);

	void draw (CDrawContext*) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;

	//-----------------------------------------------------------------------------
	/// @name CAutoAnimation Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void openWindow (void);			///< enabled drawing
	virtual void closeWindow (void);		///< disable drawing

	virtual void nextPixmap (void);			///< the next sub bitmap should be displayed
	virtual void previousPixmap (void);		///< the previous sub bitmap should be displayed

	bool    isWindowOpened () const { return bWindowOpened; }
	//@}

	void setNumSubPixmaps (int32_t numSubPixmaps) override { IMultiBitmapControl::setNumSubPixmaps (numSubPixmaps); invalid (); }

	CLASS_METHODS(CAutoAnimation, CControl)
protected:
	~CAutoAnimation () noexcept override = default;

	CPoint	offset;

	CCoord	totalHeightOfBitmap;

	bool	bWindowOpened;
};

} // namespace

#endif
