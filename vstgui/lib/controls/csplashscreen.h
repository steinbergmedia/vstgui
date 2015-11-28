//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins :
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

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

	virtual void draw (CDrawContext*) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool hitTest (const CPoint& where, const CButtonState& buttons = -1) VSTGUI_OVERRIDE_VMETHOD;

	//-----------------------------------------------------------------------------
	/// @name CSplashScreen Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void unSplash ();

	virtual void setDisplayArea (const CRect& rect)  { toDisplay = rect; }				///< set the area in which the splash will be displayed
	virtual CRect& getDisplayArea (CRect& rect) const { rect = toDisplay; return rect; }	///< get the area in which the splash will be displayed
	//@}

	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;

	CLASS_METHODS(CSplashScreen, CControl)
protected:
	~CSplashScreen ();
	using CControl::valueChanged;
	void valueChanged (CControl *pControl) VSTGUI_OVERRIDE_VMETHOD;

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
	CAnimationSplashScreen (const CAnimationSplashScreen& splashScreen);

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

	virtual void unSplash () VSTGUI_OVERRIDE_VMETHOD;
	virtual void draw (CDrawContext*) VSTGUI_OVERRIDE_VMETHOD;
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool sizeToFit () VSTGUI_OVERRIDE_VMETHOD;
	virtual CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD;

protected:
	~CAnimationSplashScreen ();

	uint32_t animationIndex;
	uint32_t animationTime;
};

} // namespace

#endif
