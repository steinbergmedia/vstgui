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
{
public:
	CAutoAnimation (const CRect& size, IControlListener* listener, int32_t tag,
					const SharedPointer<CBitmap>& background);
	CAutoAnimation (const CAutoAnimation& autoAnimation);

	void draw (CDrawContext&) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	bool attached (CViewContainer& parent) override;
	bool removed (CViewContainer& parent) override;

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

	void setBackground (const SharedPointer<CBitmap>& background) override;

	CLASS_METHODS(CAutoAnimation, CControl)
protected:
	VSTGUI_SHAREDPTR_FRIEND (CAutoAnimation)
	~CAutoAnimation () noexcept override = default;

	void updateMinMaxFromBackground ();
	void startTimer ();

	uint32_t animationFrameTime {0u};
	SharedPointer<CVSTGUITimer> timer;
	bool bWindowOpened {false};
};

} // VSTGUI
