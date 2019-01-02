// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
: CControl (size, nullptr, 0)
, offBitmap (nullptr)
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
, offBitmap (nullptr)
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
CVuMeter::~CVuMeter () noexcept
{
	setOnBitmap (nullptr);
	setOffBitmap (nullptr);
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
		auto tmp = (CCoord)(((int32_t)(nbLed * newValue + 0.5f) / (float)nbLed) * getOnBitmap ()->getWidth ());
		pointOff (tmp, 0);

		_rectOff.left += tmp;
		_rectOn.right = tmp + rectOn.left;
	}
	else 
	{
		auto tmp = (CCoord)(((int32_t)(nbLed * (1.f - newValue) + 0.5f) / (float)nbLed) * getOnBitmap ()->getHeight ());
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

} // VSTGUI
