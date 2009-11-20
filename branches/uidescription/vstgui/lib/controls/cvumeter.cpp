//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2008, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "cvumeter.h"
#include "../coffscreencontext.h"
#include "../cbitmap.h"

BEGIN_NAMESPACE_VSTGUI

//------------------------------------------------------------------------
// CVuMeter
//------------------------------------------------------------------------
/**
 * CVuMeter constructor.
 * @param size the size of this view
 * @param onBitmap TODO
 * @param offBitmap TODO
 * @param nbLed TODO
 * @param style kHorizontal or kVertical
 */
//------------------------------------------------------------------------
CVuMeter::CVuMeter (const CRect& size, CBitmap* onBitmap, CBitmap* offBitmap, long nbLed, long style)
: CControl (size, 0, 0)
, offBitmap (0)
, pOScreen (0)
, nbLed (nbLed)
, style (style)
, bUseOffscreen (false)
{
	setDecreaseStepValue (0.1f);

	setOnBitmap (onBitmap);
	setOffBitmap (offBitmap);

	rectOn  (size.left, size.top, size.right, size.bottom);
	rectOff (size.left, size.top, size.right, size.bottom);
}

//------------------------------------------------------------------------
CVuMeter::CVuMeter (const CVuMeter& v)
: CControl (v)
, offBitmap (0)
, pOScreen (0)
, nbLed (v.nbLed)
, style (v.style)
, decreaseValue (v.decreaseValue)
, bUseOffscreen (v.bUseOffscreen)
, rectOn (v.rectOn)
, rectOff (v.rectOff)
{
	setOffBitmap (v.offBitmap);
}

//------------------------------------------------------------------------
CVuMeter::~CVuMeter ()
{
	setOnBitmap (0);
	setOffBitmap (0);
}

//------------------------------------------------------------------------
void CVuMeter::setDirty (const bool val)
{
	CView::setDirty (val);
}

//-----------------------------------------------------------------------------
bool CVuMeter::attached (CView *parent)
{
	if (pOScreen)
		pOScreen->forget ();

	if (bUseOffscreen)
	{
		pOScreen = COffscreenContext::create (getFrame (), size.width (), size.height ());
		rectOn  (0, 0, size.width (), size.height ());
		rectOff (0, 0, size.width (), size.height ());
	}
	else
	{
		rectOn  (size.left, size.top, size.right, size.bottom);
		rectOff (size.left, size.top, size.right, size.bottom);
	}

	return CControl::attached (parent);
}

//------------------------------------------------------------------------
void CVuMeter::setUseOffscreen (bool val)
{
	bUseOffscreen = val;
}

//-----------------------------------------------------------------------------
void CVuMeter::setOffBitmap (CBitmap* bitmap)
{
	if (offBitmap)
		offBitmap->forget ();
	offBitmap = bitmap;
	if (offBitmap)
		offBitmap->remember ();
}

//-----------------------------------------------------------------------------
bool CVuMeter::removed (CView *parent)
{
	if (pOScreen)
	{
		pOScreen->forget ();
		pOScreen = 0;
	}
	return CControl::removed (parent);
}

//------------------------------------------------------------------------
void CVuMeter::draw (CDrawContext *_pContext)
{
	if (!getOnBitmap ())
		return;

	CPoint pointOn;
	CPoint pointOff;
	CDrawContext *pContext = _pContext;

	bounceValue ();
	
	float newValue = oldValue - decreaseValue;
	if (newValue < value)
		newValue = value;
	oldValue = newValue;

	if (bUseOffscreen)
	{
		if (!pOScreen)
		{
			pOScreen = COffscreenContext::create (getFrame (), size.width (), size.height ());
			rectOn  (0, 0, size.width (), size.height ());
			rectOff (0, 0, size.width (), size.height ());
		}
		if (pOScreen)
		{
			pContext = pOScreen;
		}
		else
		{
			#if DEBUG
			DebugPrint ("Could not create Offscreen for VUMeter\n");
			#endif
			setDirty (false);
			return;
		}
	}

	if (style & kHorizontal) 
	{
		CCoord tmp = (CCoord)(((long)(nbLed * newValue + 0.5f) / (float)nbLed) * getOnBitmap ()->getWidth ());
		pointOff (tmp, 0);
		if (!bUseOffscreen)
		tmp += size.left;

		rectOff.left = tmp;
		rectOn.right = tmp;
	}
	else 
	{
		CCoord tmp = (CCoord)(((long)(nbLed * (getMax () - newValue) + 0.5f) / (float)nbLed) * getOnBitmap ()->getHeight ());
		pointOn (0, tmp);
		if (!bUseOffscreen)
		tmp += size.top;

		rectOff.bottom = tmp;
		rectOn.top     = tmp;
	}

	if (getOffBitmap ())
	{
		if (bTransparencyEnabled)
			getOffBitmap ()->drawTransparent (pContext, rectOff, pointOff);
		else
			getOffBitmap ()->draw (pContext, rectOff, pointOff);
	}

	if (bTransparencyEnabled)
		getOnBitmap ()->drawTransparent (pContext, rectOn, pointOn);
	else
		getOnBitmap ()->draw (pContext, rectOn, pointOn);

	if (pOScreen)
		pOScreen->copyFrom (_pContext, size);
	setDirty (false);
}

END_NAMESPACE_VSTGUI
