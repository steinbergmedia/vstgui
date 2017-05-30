// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __csplashscreen__
#define __csplashscreen__

#include "ccontrol.h"
#include "icontrollistener.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CSplashScreen Declaration
//!
/// @ingroup views
//-----------------------------------------------------------------------------
class CSplashScreen : public CControl, public IControlListener
{
public:
	CSplashScreen (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CRect& toDisplay, const CPoint& offset = CPoint (0, 0));
	CSplashScreen (const CRect& size, IControlListener* listener, int32_t tag, CView* splashView);
	CSplashScreen (const CSplashScreen& splashScreen);

	void draw (CDrawContext*) override;
	bool hitTest (const CPoint& where, const CButtonState& buttons = -1) override;

	//-----------------------------------------------------------------------------
	/// @name CSplashScreen Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void unSplash ();

	virtual void setDisplayArea (const CRect& rect)  { toDisplay = rect; }				///< set the area in which the splash will be displayed
	virtual CRect& getDisplayArea (CRect& rect) const { rect = toDisplay; return rect; }	///< get the area in which the splash will be displayed
	//@}

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;

	CLASS_METHODS(CSplashScreen, CControl)
protected:
	~CSplashScreen () noexcept override;
	using CControl::valueChanged;
	void valueChanged (CControl *pControl) override;

	CRect	toDisplay;
	CRect	keepSize;
	CPoint	offset;
	CView* modalView;
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
	CAnimationSplashScreen (const CRect& size, int32_t tag, CBitmap* background, CBitmap* splashBitmap);
	CAnimationSplashScreen (const CAnimationSplashScreen& splashScreen) = default;

	//-----------------------------------------------------------------------------
	/// @name CAnimationSplashScreen Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setSplashBitmap (CBitmap* bitmap);
	CBitmap* getSplashBitmap () const;

	virtual void setSplashRect (const CRect& splashRect);
	const CRect& getSplashRect () const;

	virtual void setAnimationIndex (uint32_t index) { animationIndex = index; }
	uint32_t getAnimationIndex () const { return animationIndex; }

	virtual void setAnimationTime (uint32_t time) { animationTime = time; }
	uint32_t getAnimationTime () const { return animationTime; }

	/** create the animation. subclasses can override this to add special animations */
	virtual bool createAnimation (uint32_t animationIndex, uint32_t animationTime, CView* splashView, bool removeViewAnimation);
	//@}

	void unSplash () override;
	void draw (CDrawContext*) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	bool sizeToFit () override;
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;

protected:
	~CAnimationSplashScreen () noexcept override = default;

	uint32_t animationIndex;
	uint32_t animationTime;
};

} // namespace

#endif
