// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cknob.h"
#include "../cbitmap.h"
#include "../cdrawcontext.h"
#include "../cframe.h"
#include "../cgraphicspath.h"
#include <cmath>

namespace VSTGUI {
#if TARGET_OS_IPHONE
static const float kCKnobRange = 300.f;
#else
static const float kCKnobRange = 200.f;
#endif

static constexpr CViewAttributeID kCKnobMouseStateAttribute = 'knms';
//------------------------------------------------------------------------
struct CKnobBase::MouseEditingState
{
	CPoint firstPoint;
	CPoint lastPoint;
	float startValue;
	float entryState;
	float range;
	float coef;
	CButtonState oldButton;
	bool modeLinear;
};

//------------------------------------------------------------------------
CKnobBase::CKnobBase (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background)
: CControl (size, listener, tag, background)
{
}

//------------------------------------------------------------------------
CKnobBase::CKnobBase (const CKnobBase& k)
: CControl (k)
, startAngle (k.startAngle)
, rangeAngle (k.rangeAngle)
, zoomFactor (k.zoomFactor)
, inset (k.inset)
{
}

//------------------------------------------------------------------------
void CKnobBase::setViewSize (const CRect &rect, bool invalid)
{
	CControl::setViewSize (rect, invalid);
	compute ();
}

//------------------------------------------------------------------------
bool CKnobBase::sizeToFit ()
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

//------------------------------------------------------------------------
auto CKnobBase::getMouseEditingState () -> MouseEditingState&
{
	MouseEditingState* state = nullptr;
	uint32_t size;
	if (!getAttribute (kCKnobMouseStateAttribute, sizeof (MouseEditingState*), &state, size))
	{
		state = new MouseEditingState;
		setAttribute (kCKnobMouseStateAttribute, sizeof (MouseEditingState*), &state);
	}
	return *state;
}

//------------------------------------------------------------------------
void CKnobBase::clearMouseEditingState ()
{
	MouseEditingState* state = nullptr;
	uint32_t size;
	if (!getAttribute (kCKnobMouseStateAttribute, sizeof (MouseEditingState*), &state, size))
		return;
	delete state;
	removeAttribute (kCKnobMouseStateAttribute);
}

//------------------------------------------------------------------------
CMouseEventResult CKnobBase::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (!buttons.isLeftButton ())
		return kMouseEventNotHandled;

	beginEdit ();

	if (checkDefaultValue (buttons))
	{
		endEdit ();
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}

	auto& mouseState = getMouseEditingState ();
	mouseState.firstPoint = where;
	mouseState.lastPoint (-1, -1);
	mouseState.startValue = getOldValue ();

	mouseState.modeLinear = false;
	mouseState.entryState = value;
	mouseState.range = kCKnobRange;
	mouseState.coef = (getMax () - getMin ()) / mouseState.range;
	mouseState.oldButton = buttons;

	int32_t mode    = kCircularMode;
	int32_t newMode = getFrame ()->getKnobMode ();
	if (kLinearMode == newMode)
	{
		if (!(buttons & kAlt))
			mode = newMode;
	}
	else if (buttons & kAlt)
	{
		mode = kLinearMode;
	}

	if (mode == kLinearMode)
	{
		if (buttons & kZoomModifier)
			mouseState.range *= zoomFactor;
		mouseState.lastPoint = where;
		mouseState.modeLinear = true;
		mouseState.coef = (getMax () - getMin ()) / mouseState.range;
	}
	else
	{
		CPoint where2 (where);
		where2.offset (-getViewSize ().left, -getViewSize ().top);
		mouseState.startValue = valueFromPoint (where2);
		mouseState.lastPoint = where;
	}

	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CKnobBase::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	if (isEditing ())
	{
		endEdit ();
		clearMouseEditingState ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CKnobBase::onMouseCancel ()
{
	if (isEditing ())
	{
		auto& mouseState = getMouseEditingState ();
		value = mouseState.startValue;
		if (isDirty ())
		{
			valueChanged ();
			invalid ();
		}
		endEdit ();
		clearMouseEditingState ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CKnobBase::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (isEditing ())
	{
		auto& mouseState = getMouseEditingState ();

		float middle = (getMax () - getMin ()) * 0.5f;

		if (where != mouseState.lastPoint)
		{
			mouseState.lastPoint = where;
			if (mouseState.modeLinear)
			{
				CCoord diff = (mouseState.firstPoint.y - where.y) + (where.x - mouseState.firstPoint.x);
				if (buttons != mouseState.oldButton)
				{
					mouseState.range = kCKnobRange;
					if (buttons & kZoomModifier)
						mouseState.range *= zoomFactor;

					float coef2 = (getMax () - getMin ()) / mouseState.range;
					mouseState.entryState += (float)(diff * (mouseState.coef - coef2));
					mouseState.coef = coef2;
					mouseState.oldButton = buttons;
				}
				value = (float)(mouseState.entryState + diff * mouseState.coef);
				bounceValue ();
			}
			else
			{
				where.offset (-getViewSize ().left, -getViewSize ().top);
				value = valueFromPoint (where);
				if (mouseState.startValue - value > middle)
					value = getMax ();
				else if (value - mouseState.startValue > middle)
					value = getMin ();
				else
					mouseState.startValue = value;
			}
			if (value != getOldValue ())
				valueChanged ();
			if (isDirty ())
				invalid ();
		}
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
bool CKnobBase::onWheel (const CPoint& where, const float &distance, const CButtonState &buttons)
{
	if (!getMouseEnabled ())
		return false;

	float v = getValueNormalized ();
	if (buttons & kZoomModifier)
		v += 0.1f * distance * wheelInc;
	else
		v += distance * wheelInc;
	setValueNormalized (v);

	if (isDirty ())
	{
		invalid ();

		// begin of edit parameter
		beginEdit ();
	
		valueChanged ();
	
		// end of edit parameter
		endEdit ();
	}
	return true;
}

//------------------------------------------------------------------------
int32_t CKnobBase::onKeyDown (VstKeyCode& keyCode)
{
	switch (keyCode.virt)
	{
		case VKEY_UP :
		case VKEY_RIGHT :
		case VKEY_DOWN :
		case VKEY_LEFT :
		{
			float distance = 1.f;
			if (keyCode.virt == VKEY_DOWN || keyCode.virt == VKEY_LEFT)
				distance = -distance;

			float v = getValueNormalized ();
			if (mapVstKeyModifier (keyCode.modifier) & kZoomModifier)
				v += 0.1f * distance * wheelInc;
			else
				v += distance * wheelInc;
			setValueNormalized (v);

			if (isDirty ())
			{
				invalid ();

				// begin of edit parameter
				beginEdit ();
				
				valueChanged ();
			
				// end of edit parameter
				endEdit ();
			}
		} return 1;
	}
	return -1;
}

//------------------------------------------------------------------------
void CKnobBase::setStartAngle (float val)
{
	startAngle = val;
	compute ();
}

//------------------------------------------------------------------------
void CKnobBase::setRangeAngle (float val)
{
	rangeAngle = val;
	compute ();
}

//------------------------------------------------------------------------
void CKnobBase::compute ()
{
	setDirty ();
}

//------------------------------------------------------------------------
void CKnobBase::valueToPoint (CPoint &point) const
{
	float alpha = (value - getMin()) / (getMax() - getMin());
	alpha = startAngle + alpha*rangeAngle;

	CPoint c (getViewSize ().getWidth () / 2., getViewSize ().getHeight () / 2.);
	double xradius = c.x - inset;
	double yradius = c.y - inset;

	point.x = (CCoord)(c.x + cosf (alpha) * xradius + 0.5f);
	point.y = (CCoord)(c.y + sinf (alpha) * yradius + 0.5f);
}

//------------------------------------------------------------------------
float CKnobBase::valueFromPoint (CPoint &point) const
{
	float v;
	double d = rangeAngle * 0.5;
	double a = startAngle + d;

	CPoint c (getViewSize ().getWidth () / 2., getViewSize ().getHeight () / 2.);
	double xradius = c.x - inset;
	double yradius = c.y - inset;

	double dx = (point.x - c.x) / xradius;
	double dy = (point.y - c.y) / yradius;

	double alpha = atan2 (dy, dx) - a;
	while (alpha >= Constants::pi)
		alpha -= Constants::double_pi;
	while (alpha < -Constants::pi)
		alpha += Constants::double_pi;

	if (d < 0.0)
		alpha = -alpha;

	if (alpha > d)
		v = getMax ();
	else if (alpha < -d)
		v = getMin ();
	else
		v = float (0.5 + alpha / rangeAngle);

	v *= (getMax () - getMin ());

	return v;
}

//------------------------------------------------------------------------
void CKnobBase::setMin (float val)
{
	CControl::setMin (val);
	if (getValue () < val)
		setValue (val);
	compute ();
}

//------------------------------------------------------------------------
void CKnobBase::setMax (float val)
{
	CControl::setMax (val);
	if (getValue () > val)
		setValue (val);
	compute ();
}

//------------------------------------------------------------------------
// CKnob
//------------------------------------------------------------------------
/*! @class CKnob
Define a knob with a given background and foreground handle.
The handle describes a circle over the background (between -45deg and +225deg).
By clicking alt modifier and left mouse button the default value is used.
By clicking alt modifier and left mouse button the value changes with a vertical move (version 2.1)
*/
//------------------------------------------------------------------------
/**
 * CKnob constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background background bitmap
 * @param handle handle bitmap
 * @param offset offset of background bitmap
 * @param drawStyle draw style
 */
//------------------------------------------------------------------------
CKnob::CKnob (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, CBitmap* handle, const CPoint& offset, int32_t drawStyle)
: CKnobBase (size, listener, tag, background)
, offset (offset)
, drawStyle (drawStyle)
, handleLineWidth (1.)
, coronaInset (0)
, coronaOutlineWidthAdd (2.)
, pHandle (handle)
{
	if (pHandle)
	{
		pHandle->remember ();
		inset = (CCoord)((float)pHandle->getWidth () / 2.f + 2.5f);
	}
	else
	{
		inset = 3;
	}
	
	colorShadowHandle = kGreyCColor;
	colorHandle = kWhiteCColor;

	rangeAngle = 1.f;
	setStartAngle ((float)(3.f * Constants::quarter_pi));
	setRangeAngle ((float)(3.f * Constants::half_pi));
	zoomFactor = 1.5f;

	setWantsFocus (true);
}

//------------------------------------------------------------------------
CKnob::CKnob (const CKnob& v)
: CKnobBase (v)
, offset (v.offset)
, drawStyle (v.drawStyle)
, colorHandle (v.colorHandle)
, colorShadowHandle (v.colorShadowHandle)
, handleLineWidth (v.handleLineWidth)
, coronaInset (v.coronaInset)
, coronaOutlineWidthAdd (v.coronaInset)
, pHandle (v.pHandle)
{
	if (pHandle)
		pHandle->remember ();
}

//------------------------------------------------------------------------
CKnob::~CKnob () noexcept
{
	if (pHandle)
		pHandle->forget ();
}

//------------------------------------------------------------------------
bool CKnob::drawFocusOnTop ()
{
	if (drawStyle & kCoronaDrawing && wantsFocus ())
	{
		return false;
	}
	return CKnobBase::drawFocusOnTop ();
}

//------------------------------------------------------------------------
bool CKnob::getFocusPath (CGraphicsPath &outPath)
{
	if (drawStyle & kCoronaDrawing && wantsFocus ())
	{
		CRect corona (getViewSize ());
		corona.inset (coronaInset, coronaInset);
		corona.inset (handleLineWidth/2., handleLineWidth/2.);
		outPath.addEllipse (corona);
		return true;
	}
	return CKnobBase::getFocusPath (outPath);
}

//------------------------------------------------------------------------
void CKnob::draw (CDrawContext *pContext)
{
	if (getDrawBackground ())
	{
		getDrawBackground ()->draw (pContext, getViewSize (), offset);
	}
	if (pHandle)
		drawHandle (pContext);
	else
	{
		if (drawStyle & kCoronaOutline)
			drawCoronaOutline (pContext);
		if (drawStyle & kCoronaDrawing)
			drawCorona (pContext);
		if (!(drawStyle & kSkipHandleDrawing))
		{
			if (drawStyle & kHandleCircleDrawing)
				drawHandleAsCircle (pContext);
			else
				drawHandleAsLine (pContext);
		}
	}
	setDirty (false);
}

//------------------------------------------------------------------------
void CKnob::addArc (CGraphicsPath* path, const CRect& r, double startAngle, double sweepAngle) const
{
	CCoord w = r.getWidth ();
	CCoord h = r.getHeight ();
	double endAngle = startAngle + sweepAngle;
	if (w != h)
	{
		startAngle = atan2 (sin (startAngle) * h, cos (startAngle) * w);
		endAngle = atan2 (sin (endAngle) * h, cos (endAngle) * w);
	}
	path->addArc (r, startAngle / Constants::pi * 180, endAngle / Constants::pi * 180, sweepAngle >= 0);
}

//------------------------------------------------------------------------
void CKnob::drawCoronaOutline (CDrawContext* pContext) const
{
	auto path = owned (pContext->createGraphicsPath ());
	if (path == nullptr)
		return;
	CRect corona (getViewSize ());
	corona.inset (coronaInset, coronaInset);
	addArc (path, corona, startAngle, rangeAngle);
	pContext->setFrameColor (colorShadowHandle);
	CLineStyle lineStyle (kLineSolid);
	if (!(drawStyle & kCoronaLineCapButt))
		lineStyle.setLineCap (CLineStyle::kLineCapRound);
	pContext->setLineStyle (lineStyle);
	pContext->setLineWidth (handleLineWidth+coronaOutlineWidthAdd);
	pContext->setDrawMode (kAntiAliasing | kNonIntegralMode);
	pContext->drawGraphicsPath (path, CDrawContext::kPathStroked);
}

//------------------------------------------------------------------------
void CKnob::drawCorona (CDrawContext* pContext) const
{
	auto path = owned (pContext->createGraphicsPath ());
	if (path == nullptr)
		return;
	float coronaValue = getValueNormalized ();
	if (drawStyle & kCoronaInverted)
		coronaValue = 1.f - coronaValue;
	CRect corona (getViewSize ());
	corona.inset (coronaInset, coronaInset);
	if (drawStyle & kCoronaFromCenter)
		addArc (path, corona, 1.5 * Constants::pi, rangeAngle * (coronaValue - 0.5));
	else
	{
		if (drawStyle & kCoronaInverted)
			addArc (path, corona, startAngle + rangeAngle, -rangeAngle * coronaValue);
		else
			addArc (path, corona, startAngle, rangeAngle * coronaValue);
	}
	pContext->setFrameColor (coronaColor);
	CLineStyle lineStyle ((drawStyle & kCoronaLineDashDot) ? kLineOnOffDash : kLineSolid);
	if (!(drawStyle & kCoronaLineCapButt))
		lineStyle.setLineCap (CLineStyle::kLineCapRound);
	if (drawStyle & kCoronaLineDashDot)
		lineStyle.getDashLengths ()[1] = 2;
	pContext->setLineStyle (lineStyle);
	pContext->setLineWidth (handleLineWidth);
	pContext->setDrawMode (kAntiAliasing | kNonIntegralMode);
	pContext->drawGraphicsPath (path, CDrawContext::kPathStroked);
}

//------------------------------------------------------------------------
void CKnob::drawHandleAsCircle (CDrawContext* pContext) const
{
	CPoint where;
	valueToPoint (where);

	where.offset (getViewSize ().left, getViewSize ().top);
	CRect r (where.x - 0.5, where.y - 0.5, where.x + 0.5, where.y + 0.5);
	r.extend (handleLineWidth, handleLineWidth);
	pContext->setDrawMode (kAntiAliasing);
	pContext->setFrameColor (colorShadowHandle);
	pContext->setFillColor (colorHandle);
	pContext->setLineWidth (0.5);
	pContext->setLineStyle (kLineSolid);
	pContext->setDrawMode (kAntiAliasing | kNonIntegralMode);
	pContext->drawEllipse (r, kDrawFilledAndStroked);
}

//------------------------------------------------------------------------
void CKnob::drawHandleAsLine (CDrawContext* pContext) const
{
	CPoint where;
	valueToPoint (where);

	CPoint origin (getViewSize ().getWidth () / 2, getViewSize ().getHeight () / 2);
	where.offset (getViewSize ().left - 1, getViewSize ().top);
	origin.offset (getViewSize ().left - 1, getViewSize ().top);
	pContext->setFrameColor (colorShadowHandle);
	pContext->setLineWidth (handleLineWidth);
	pContext->setLineStyle (CLineStyle (CLineStyle::kLineCapRound));
	pContext->setDrawMode (kAntiAliasing | kNonIntegralMode);
	pContext->drawLine (where, origin);
	
	where.offset (1, -1);
	origin.offset (1, -1);
	pContext->setFrameColor (colorHandle);
	pContext->drawLine (where, origin);
}

//------------------------------------------------------------------------
void CKnob::drawHandle (CDrawContext *pContext)
{
	CPoint where;
	valueToPoint (where);

	CCoord width  = pHandle->getWidth ();
	CCoord height = pHandle->getHeight ();
	where.offset (getViewSize ().left - width / 2, getViewSize ().top - height / 2);

	where.x = floor (where.x);
	where.y = floor (where.y);

	CRect handleSize (0, 0, width, height);
	handleSize.offset (where.x, where.y);
	pHandle->draw (pContext, handleSize);
}

//------------------------------------------------------------------------
void CKnob::setCoronaInset (CCoord inset)
{
	if (inset != coronaInset)
	{
		coronaInset = inset;
		setDirty ();
	}
}

//------------------------------------------------------------------------
void CKnob::setCoronaColor (CColor color)
{
	if (color != coronaColor)
	{
		coronaColor = color;
		setDirty ();
	}
}

//------------------------------------------------------------------------
void CKnob::setColorShadowHandle (CColor color)
{
	if (color != colorShadowHandle)
	{
		colorShadowHandle = color;
		setDirty ();
	}
}

//------------------------------------------------------------------------
void CKnob::setColorHandle (CColor color)
{
	if (color != colorHandle)
	{
		colorHandle = color;
		setDirty ();
	}
}

//------------------------------------------------------------------------
void CKnob::setHandleLineWidth (CCoord width)
{
	if (width != handleLineWidth)
	{
		handleLineWidth = width;
		setDirty ();
	}
}

//------------------------------------------------------------------------
void CKnob::setCoronaOutlineWidthAdd (CCoord width)
{
	if (width != coronaOutlineWidthAdd)
	{
		coronaOutlineWidthAdd = width;
		setDirty ();
	}
}

//------------------------------------------------------------------------
void CKnob::setDrawStyle (int32_t style)
{
	if (style != drawStyle)
	{
		drawStyle = style;
		setDirty ();
	}
}

//------------------------------------------------------------------------
void CKnob::setHandleBitmap (CBitmap* bitmap)
{
	if (pHandle)
	{
		pHandle->forget ();
		pHandle = nullptr;
	}

	if (bitmap)
	{
		pHandle = bitmap;
		pHandle->remember ();
		inset = (CCoord)((float)pHandle->getWidth () / 2.f + 2.5f);
	}
	setDirty ();
}

//------------------------------------------------------------------------
// CAnimKnob
//------------------------------------------------------------------------
/*! @class CAnimKnob
Such as a CKnob control object, but there is a unique bitmap which contains different views (subbitmaps) of this knob.
According to the value, a specific subbitmap is displayed. The different subbitmaps are stacked in the bitmap object.
*/
//------------------------------------------------------------------------
/**
 * CAnimKnob constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background the background bitmap
 * @param offset unused
 */
//------------------------------------------------------------------------
CAnimKnob::CAnimKnob (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint &offset)
: CKnobBase (size, listener, tag, background)
, bInverseBitmap (false)
{
	heightOfOneImage = size.getHeight ();
	setNumSubPixmaps (background ? (int32_t)(background->getHeight () / heightOfOneImage) : 0);
	inset = 0;
}

//------------------------------------------------------------------------
/**
 * CAnimKnob constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param subPixmaps number of sub bitmaps in background
 * @param heightOfOneImage the height of one sub bitmap
 * @param background the background bitmap
 * @param offset unused
 */
//------------------------------------------------------------------------
CAnimKnob::CAnimKnob (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint &offset)
: CKnobBase (size, listener, tag, background)
, bInverseBitmap (false)
{
	setNumSubPixmaps (subPixmaps);
	setHeightOfOneImage (heightOfOneImage);
	inset = 0;
}

//------------------------------------------------------------------------
CAnimKnob::CAnimKnob (const CAnimKnob& v)
: CKnobBase (v)
, bInverseBitmap (v.bInverseBitmap)
{
	setNumSubPixmaps (v.subPixmaps);
	setHeightOfOneImage (v.heightOfOneImage);
}

//-----------------------------------------------------------------------------------------------
bool CAnimKnob::sizeToFit ()
{
	if (getDrawBackground ())
	{
		CRect vs (getViewSize ());
		vs.setWidth (getDrawBackground ()->getWidth ());
		vs.setHeight (getHeightOfOneImage ());
		setViewSize (vs);
		setMouseableArea (vs);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------------------
void CAnimKnob::setHeightOfOneImage (const CCoord& height)
{
	IMultiBitmapControl::setHeightOfOneImage (height);
	if (getDrawBackground () && heightOfOneImage > 0)
		setNumSubPixmaps ((int32_t)(getDrawBackground ()->getHeight () / heightOfOneImage));
}

//-----------------------------------------------------------------------------------------------
void CAnimKnob::setBackground (CBitmap *background)
{
	CKnobBase::setBackground (background);
	if (heightOfOneImage == 0)
		heightOfOneImage = getViewSize ().getHeight ();
	if (background && heightOfOneImage > 0)
		setNumSubPixmaps ((int32_t)(background->getHeight () / heightOfOneImage));
}

//------------------------------------------------------------------------
void CAnimKnob::draw (CDrawContext *pContext)
{
	if (getDrawBackground ())
	{
		CPoint where (0, 0);
		float val = getValueNormalized ();
		if (val >= 0.f && heightOfOneImage > 0.)
		{
			CCoord tmp = heightOfOneImage * (getNumSubPixmaps () - 1);
			if (bInverseBitmap)
				where.y = floor ((1. - val) * tmp);
			else
				where.y = floor (val * tmp);
			where.y -= (int32_t)where.y % (int32_t)heightOfOneImage;
		}

		getDrawBackground ()->draw (pContext, getViewSize (), where);
	}
	setDirty (false);
}

} // VSTGUI
