//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
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

#include "cvumeter.h"
#include "../coffscreencontext.h"
#include "../cbitmap.h"
#include "../cvstguitimer.h"
#include <list>

namespace VSTGUI {

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
CVuMeter::CVuMeter (const CRect& size, CBitmap* onBitmap, CBitmap* offBitmap, int32_t nbLed, int32_t style)
: CControl (size, 0, 0)
, offBitmap (0)
, nbLed (nbLed)
, style (style)
{
	setDecreaseStepValue (0.1f);

	setOnBitmap (onBitmap);
	setOffBitmap (offBitmap);

	rectOn  (size.left, size.top, size.right, size.bottom);
	rectOff (size.left, size.top, size.right, size.bottom);

	setWantsIdle (true);
}

//------------------------------------------------------------------------
CVuMeter::CVuMeter (const CVuMeter& v)
: CControl (v)
, offBitmap (0)
, nbLed (v.nbLed)
, style (v.style)
, decreaseValue (v.decreaseValue)
, rectOn (v.rectOn)
, rectOff (v.rectOff)
{
	setOffBitmap (v.offBitmap);
	setWantsIdle (true);
}

//------------------------------------------------------------------------
CVuMeter::~CVuMeter ()
{
	setOnBitmap (0);
	setOffBitmap (0);
}

//------------------------------------------------------------------------
void CVuMeter::setViewSize (const CRect& newSize, bool invalid)
{
	CControl::setViewSize (newSize, invalid);
	rectOn  = getViewSize ();
	rectOff = getViewSize ();
}

//------------------------------------------------------------------------
bool CVuMeter::sizeToFit ()
{
	if (getDrawBackground ())
	{
		CRect vs (getViewSize ());
		vs.setWidth (getDrawBackground ()->getWidth ());
		vs.setHeight (getDrawBackground ()->getHeight ());
		setViewSize (vs);
		setMouseableArea (vs);
		return true;
	}
	return false;
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

//------------------------------------------------------------------------
void CVuMeter::setDirty (bool state)
{
	CView::setDirty (state);
}

//------------------------------------------------------------------------
void CVuMeter::onIdle ()
{
	if (getOldValue () != value)
		invalid ();
}

//------------------------------------------------------------------------
void CVuMeter::draw (CDrawContext *_pContext)
{
	if (!getOnBitmap ())
		return;

	CRect _rectOn (rectOn);
	CRect _rectOff (rectOff);
	CPoint pointOn;
	CPoint pointOff;
	CDrawContext *pContext = _pContext;

	bounceValue ();
	
	float newValue = getOldValue () - decreaseValue;
	if (newValue < value)
		newValue = value;
	setOldValue (newValue);

	newValue = (newValue - getMin ()) / getRange (); // normalize
	
	if (style & kHorizontal) 
	{
		CCoord tmp = (CCoord)(((int32_t)(nbLed * newValue + 0.5f) / (float)nbLed) * getOnBitmap ()->getWidth ());
		pointOff (tmp, 0);

		_rectOff.left += tmp;
		_rectOn.right = tmp + rectOn.left;
	}
	else 
	{
		CCoord tmp = (CCoord)(((int32_t)(nbLed * (1.f - newValue) + 0.5f) / (float)nbLed) * getOnBitmap ()->getHeight ());
		pointOn (0, tmp);

		_rectOff.bottom = tmp + rectOff.top;
		_rectOn.top     += tmp;
	}

	if (getOffBitmap ())
	{
		getOffBitmap ()->draw (pContext, _rectOff, pointOff);
	}

	getOnBitmap ()->draw (pContext, _rectOn, pointOn);

	setDirty (false);
}

} // namespace
