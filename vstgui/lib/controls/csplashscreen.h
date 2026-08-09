// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "ccontrol.h"
#include "icontrollistener.h"
#include "../optional.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CSplashScreen Declaration
//!
/// @ingroup views
//-----------------------------------------------------------------------------
class CSplashScreen : public CControl,
					  public ControlListenerAdapter
{
public:
	CSplashScreen (const CRect& size, IControlListener* listener, int32_t tag,
				   const SPtr<CBitmap>& background, const CRect& toDisplay,
				   const CPoint& offset = CPoint (0, 0));
	CSplashScreen (const CRect& size, IControlListener* listener, int32_t tag,
				   const SPtr<CView>& splashView);

	void draw (CDrawContext&) override;
	bool hitTest (const CPoint& where, const Event& event) override;

	//-----------------------------------------------------------------------------
	/// @name CSplashScreen Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void unSplash ();

	/** set the area in which the splash will be displayed */
	virtual void setDisplayArea (const CRect& rect)  { toDisplay = rect; }
	/** get the area in which the splash will be displayed */
	virtual CRect& getDisplayArea (CRect& rect) const { rect = toDisplay; return rect; }
	//@}

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;

protected:
	~CSplashScreen () noexcept override;
	using CControl::valueChanged;
	void valueChanged (CControl& control) override;

	CRect toDisplay;
	CRect keepSize;
	CPoint offset;
	SPtr<CView> modalView;
	Optional<ModalViewSessionID> modalViewSessionID;
};

//-----------------------------------------------------------------------------
// CAnimationSplashScreen Declaration
/// @brief a splash screen which animates the opening and closing of the splash bitmap
/// @ingroup views
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class CAnimationSplashScreen : public CSplashScreen
{
public:
	CAnimationSplashScreen (const CRect& size, int32_t tag, const SPtr<CBitmap>& background,
							const SPtr<CBitmap>& splashBitmap);

	//-----------------------------------------------------------------------------
	/// @name CAnimationSplashScreen Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setSplashBitmap (const SPtr<CBitmap>& bitmap);
	SPtr<CBitmap> getSplashBitmap () const;

	virtual void setSplashRect (const CRect& splashRect);
	const CRect& getSplashRect () const;

	virtual void setAnimationIndex (uint32_t index) { animationIndex = index; }
	uint32_t getAnimationIndex () const { return animationIndex; }

	virtual void setAnimationTime (uint32_t time) { animationTime = time; }
	uint32_t getAnimationTime () const { return animationTime; }

	/** create the animation. subclasses can override this to add special animations */
	virtual bool createAnimation (uint32_t animationIndex, uint32_t animationTime,
								  const SPtr<CView>& splashView, bool removeViewAnimation);
	//@}

	void unSplash () override;
	void draw (CDrawContext&) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	bool sizeToFit () override;

protected:
	VSTGUI_SHAREDPTR_FRIEND (CAnimationSplashScreen)
	~CAnimationSplashScreen () noexcept override = default;

	uint32_t animationIndex{0};
	uint32_t animationTime{500};
};

} // VSTGUI
