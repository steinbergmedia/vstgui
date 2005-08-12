#ifndef __controlsgui__
#define __controlsgui__

#ifndef __vstcontrols__
#include "vstcontrols.h"
#endif

class CLabel;

class ControlsGUI : public CViewContainer, CControlListener
{
public:
	ControlsGUI (const CRect &size, CFrame *pParent, CBitmap *pBackground = 0);

	virtual void valueChanged (CDrawContext *pContext, CControl *pControl);

protected:

	COnOffButton      *cOnOffButton;
	CKickButton       *cKickButton;
	CKnob             *cKnob;
	CMovieButton      *cMovieButton;
	CAnimKnob         *cAnimKnob;
	COptionMenu       *cOptionMenu;

	CRockerSwitch     *cRockerSwitch;
	CHorizontalSwitch *cHorizontalSwitch;
	CVerticalSwitch   *cVerticalSwitch;
	CHorizontalSlider *cHorizontalSlider;
	CHorizontalSlider *cHorizontalSlider2;
	CVerticalSlider   *cVerticalSlider;
	CTextEdit         *cTextEdit;

	CSplashScreen     *cSplashScreen;
	CMovieBitmap      *cMovieBitmap;
	CAutoAnimation    *cAutoAnimation;
	CSpecialDigit     *cSpecialDigit;
	CParamDisplay     *cParamDisplay;
	CVuMeter          *cVuMeter;

	CViewContainer    *cViewContainer;

	// others
	CLabel            *cLabel;

	long              oldTicks;
};

#endif
