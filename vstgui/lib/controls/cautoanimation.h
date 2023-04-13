// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "ccontrol.h"
#include "../cbitmap.h"
#include "../cvstguitimer.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CAutoAnimation Declaration
//!
/// @ingroup controls uses_multi_frame_bitmaps
//-----------------------------------------------------------------------------
class CAutoAnimation : public CControl,
					   public MultiFrameBitmapView<CAutoAnimation>
#if VSTGUI_ENABLE_DEPRECATED_METHODS
,
					   public IMultiBitmapControl
#endif
{
public:
	CAutoAnimation (const CRect& size, IControlListener* listener, int32_t tag,
					CBitmap* background);
	CAutoAnimation (const CAutoAnimation& autoAnimation);

	void draw (CDrawContext*) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	bool attached (CView* parent) override;
	bool removed (CView* parent) override;

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

	bool isWindowOpened () const;

	void setAnimationTime (uint32_t animationTime);
	uint32_t getAnimationTime () const;

	//@}

	void setBackground (CBitmap* background) override;

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	CAutoAnimation (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background,
					const CPoint& offset);
	CAutoAnimation (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps,
					CCoord heightOfOneImage, CBitmap* background,
					const CPoint& offset = CPoint (0, 0));
	void setNumSubPixmaps (int32_t numSubPixmaps) override
	{
		IMultiBitmapControl::setNumSubPixmaps (numSubPixmaps);
		invalid ();
	}

	void setBitmapOffset (const CPoint& off);
	CPoint getBitmapOffset () const;
#endif
	CLASS_METHODS(CAutoAnimation, CControl)
protected:
	~CAutoAnimation () noexcept override = default;

	void updateMinMaxFromBackground ();
	void startTimer ();

	uint32_t animationFrameTime {0u};
	SharedPointer<CVSTGUITimer> timer;
	bool bWindowOpened {false};

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	CPoint offset {};
	CCoord totalHeightOfBitmap {0};
#endif
};

} // VSTGUI
