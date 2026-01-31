// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cknob.h"
#include "../cbitmap.h"
#include "../cdrawcontext.h"
#include "../cframe.h"
#include "../cgraphicspath.h"
#include "../cvstguitimer.h"
#include "../events.h"
#include <cmath>

namespace VSTGUI {
#if TARGET_OS_IPHONE
static const float kCKnobRangeDefault = 300.f;
#else
static const float kCKnobRangeDefault = 200.f;
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
CKnobBase::CKnobBase (const CRect& size, IControlListener* listener, int32_t tag,
					  const SharedPointer<CBitmap>& background)
: CControl (size, listener, tag, background)
{
	rangeAngle = 1.f;
	setStartAngle ((float)(3.f * Constants::quarter_pi));
	setRangeAngle ((float)(3.f * Constants::half_pi));
	zoomFactor = 1.5f;
	knobRange = kCKnobRangeDefault;
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
	if (!getAttribute (kCKnobMouseStateAttribute, state))
	{
		state = new MouseEditingState;
		setAttribute (kCKnobMouseStateAttribute, state);
	}
	return *state;
}

//------------------------------------------------------------------------
void CKnobBase::clearMouseEditingState ()
{
	MouseEditingState* state = nullptr;
	if (!getAttribute (kCKnobMouseStateAttribute, state))
		return;
	delete state;
	removeAttribute (kCKnobMouseStateAttribute);
}

//------------------------------------------------------------------------
CMouseEventResult CKnobBase::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (!buttons.isLeftButton ())
		return kMouseEventNotHandled;

	invalidMouseWheelEditTimer (this);
	beginEdit ();

	auto& mouseState = getMouseEditingState ();
	mouseState.firstPoint = where;
	mouseState.lastPoint (-1, -1);
	mouseState.startValue = getOldValue ();

	mouseState.modeLinear = false;
	mouseState.entryState = getValue ();
	mouseState.range = knobRange;
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
		if (setValue (mouseState.startValue))
			valueChanged ();
		endEdit ();
		clearMouseEditingState ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CKnobBase::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton () && isEditing ())
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
					mouseState.range = knobRange;
					if (buttons & kZoomModifier)
						mouseState.range *= zoomFactor;

					float coef2 = (getMax () - getMin ()) / mouseState.range;
					mouseState.entryState += (float)(diff * (mouseState.coef - coef2));
					mouseState.coef = coef2;
					mouseState.oldButton = buttons;
				}
				setValue ((float)(mouseState.entryState + diff * mouseState.coef));
				bounceValue ();
			}
			else
			{
				where.offset (-getViewSize ().left, -getViewSize ().top);
				setValue (valueFromPoint (where));
				if (mouseState.startValue - getValue () > middle)
					setValue (getMax ());
				else if (getValue () - mouseState.startValue > middle)
					setValue (getMin ());
				else
					mouseState.startValue = getValue ();
			}
			if (getValue () != getOldValue ())
				valueChanged ();
		}
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
void CKnobBase::onMouseWheelEvent (MouseWheelEvent& event)
{
	onMouseWheelEditing (this);

	float v = getValueNormalized ();
	if (buttonStateFromEventModifiers (event.modifiers) & kZoomModifier)
		v += 0.1f * static_cast<float> (event.deltaY) * getWheelInc ();
	else
		v += static_cast<float> (event.deltaY) * getWheelInc ();
	if (setValueNormalized (v))
		valueChanged ();
	event.consumed = true;
}

//------------------------------------------------------------------------
void CKnobBase::onKeyboardEvent (KeyboardEvent& event)
{
	if (event.type != EventType::KeyDown)
		return;
	switch (event.virt)
	{
		case VirtualKey::Up :
		case VirtualKey::Right :
		case VirtualKey::Down :
		case VirtualKey::Left :
		{
			float distance = 1.f;
			if (event.virt == VirtualKey::Down || event.virt == VirtualKey::Left)
				distance = -distance;

			float v = getValueNormalized ();
			if (buttonStateFromEventModifiers (event.modifiers) & kZoomModifier)
				v += 0.1f * distance * getWheelInc ();
			else
				v += distance * getWheelInc ();
			if (setValueNormalized (v))
			{
				beginEdit ();
				valueChanged ();
				endEdit ();
			}
			event.consumed = true;
		}
		case VirtualKey::Escape:
		{
			if (isEditing ())
			{
				onMouseCancel ();
				event.consumed = true;
			}
			break;
		}
		default: return;
	}
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
void CKnobBase::compute () { invalid (); }

//------------------------------------------------------------------------
void CKnobBase::valueToPoint (CPoint &point) const
{
	float alpha = (getValue () - getMin ()) / (getMax () - getMin ());
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
	{
		v = float (0.5 + alpha / rangeAngle);
		v = getMin () + (v * getRange ());
	}

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
CKnob::CKnob (const CRect& size, IControlListener* listener, int32_t tag,
			  const SharedPointer<CBitmap>& background, const SharedPointer<CBitmap>& handle,
			  const CPoint& offset, int32_t drawStyle)
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
		inset = (CCoord)((float)pHandle->getWidth () / 2.f + 2.5f);
	}
	else
	{
		inset = 3;
	}

	colorShadowHandle = kGreyCColor;
	colorHandle = kWhiteCColor;
	coronaLineStyle = kLineOnOffDash;
	coronaLineStyle.getDashLengths ()[1] = 2.;

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
, coronaLineStyle (v.coronaLineStyle)
, pHandle (v.pHandle)
{
}

//------------------------------------------------------------------------
CKnob::~CKnob () noexcept {}

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
bool CKnob::getFocusPath (CGraphicsPath& outPath, CCoord focusLineWidth)
{
	if (drawStyle & kCoronaDrawing && wantsFocus ())
	{
		CRect corona (getViewSize ());
		corona.inset (coronaInset, coronaInset);
		corona.inset (handleLineWidth/2., handleLineWidth/2.);
		outPath.addEllipse (corona);
		return true;
	}
	return CKnobBase::getFocusPath (outPath, focusLineWidth);
}

//------------------------------------------------------------------------
void CKnob::draw (CDrawContext& context)
{
	if (getDrawBackground ())
	{
		getDrawBackground ()->draw (context, getViewSize (), offset);
	}
	if (pHandle)
		drawHandle (context);
	else
	{
		if (drawStyle & kCoronaOutline)
			drawCoronaOutline (context);
		if (drawStyle & kCoronaDrawing)
			drawCorona (context);
		if (!(drawStyle & kSkipHandleDrawing))
		{
			if (drawStyle & kHandleCircleDrawing)
				drawHandleAsCircle (context);
			else
				drawHandleAsLine (context);
		}
	}
}

//------------------------------------------------------------------------
void CKnob::addArc (const SharedPointer<CGraphicsPath>& path, const CRect& r, double startAngle,
					double sweepAngle)
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
void CKnob::drawCoronaOutline (CDrawContext& context) const
{
	auto path = context.createGraphicsPath ();
	if (path == nullptr)
		return;
	CRect corona (getViewSize ());
	corona.inset (coronaInset, coronaInset);
	auto start = startAngle;
	auto range = rangeAngle;
	if (coronaOutlineWidthAdd && (drawStyle & kCoronaLineCapButt))
	{
		auto a = static_cast<float> (coronaOutlineWidthAdd / getWidth ());
		start -= a;
		range += a * 2.f;
	}
	addArc (path, corona, start, range);
	context.setFrameColor (colorShadowHandle);
	CLineStyle lineStyle (kLineSolid);
	if (!(drawStyle & kCoronaLineCapButt))
		lineStyle.setLineCap (CLineStyle::kLineCapRound);
	context.setLineStyle (lineStyle);
	context.setLineWidth (handleLineWidth + coronaOutlineWidthAdd);
	context.setDrawMode (kAntiAliasing | kNonIntegralMode);
	context.drawGraphicsPath (path, CDrawContext::kPathStroked);
}

//------------------------------------------------------------------------
void CKnob::drawCorona (CDrawContext& context) const
{
	auto path = context.createGraphicsPath ();
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
	context.setFrameColor (coronaColor);
	if (!(drawStyle & kCoronaLineCapButt))
	{
		CLineStyle lineStyle (kLineSolid);
		lineStyle.setLineCap (CLineStyle::kLineCapRound);
		context.setLineStyle (lineStyle);
	}
	else if (drawStyle & kCoronaLineDashDot)
		context.setLineStyle (coronaLineStyle);
	else
		context.setLineStyle (kLineSolid);

	context.setLineWidth (handleLineWidth);
	context.setDrawMode (kAntiAliasing | kNonIntegralMode);
	context.drawGraphicsPath (path, CDrawContext::kPathStroked);
}

//------------------------------------------------------------------------
void CKnob::drawHandleAsCircle (CDrawContext& context) const
{
	CPoint where;
	valueToPoint (where);

	where.offset (getViewSize ().left, getViewSize ().top);
	CRect r (where.x - 0.5, where.y - 0.5, where.x + 0.5, where.y + 0.5);
	r.extend (handleLineWidth, handleLineWidth);
	context.setDrawMode (kAntiAliasing);
	context.setFrameColor (colorShadowHandle);
	context.setFillColor (colorHandle);
	context.setLineWidth (0.5);
	context.setLineStyle (kLineSolid);
	context.setDrawMode (kAntiAliasing | kNonIntegralMode);
	context.drawEllipse (r, kDrawFilledAndStroked);
}

//------------------------------------------------------------------------
void CKnob::drawHandleAsLine (CDrawContext& context) const
{
	CPoint where;
	valueToPoint (where);

	CPoint origin (getViewSize ().getWidth () / 2, getViewSize ().getHeight () / 2);
	where.offset (getViewSize ().left - 1, getViewSize ().top);
	origin.offset (getViewSize ().left - 1, getViewSize ().top);
	context.setFrameColor (colorShadowHandle);
	context.setLineWidth (handleLineWidth);
	context.setLineStyle (CLineStyle (CLineStyle::kLineCapRound));
	context.setDrawMode (kAntiAliasing | kNonIntegralMode);
	context.drawLine (where, origin);

	where.offset (1, -1);
	origin.offset (1, -1);
	context.setFrameColor (colorHandle);
	context.drawLine (where, origin);
}

//------------------------------------------------------------------------
void CKnob::drawHandle (CDrawContext& context)
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
	pHandle->draw (context, handleSize);
}

//------------------------------------------------------------------------
void CKnob::setCoronaInset (CCoord inset)
{
	if (inset != coronaInset)
	{
		coronaInset = inset;
		invalid ();
	}
}

//------------------------------------------------------------------------
void CKnob::setCoronaColor (CColor color)
{
	if (color != coronaColor)
	{
		coronaColor = color;
		invalid ();
	}
}

//------------------------------------------------------------------------
void CKnob::setColorShadowHandle (CColor color)
{
	if (color != colorShadowHandle)
	{
		colorShadowHandle = color;
		invalid ();
	}
}

//------------------------------------------------------------------------
void CKnob::setColorHandle (CColor color)
{
	if (color != colorHandle)
	{
		colorHandle = color;
		invalid ();
	}
}

//------------------------------------------------------------------------
void CKnob::setHandleLineWidth (CCoord width)
{
	if (width != handleLineWidth)
	{
		handleLineWidth = width;
		invalid ();
	}
}

//------------------------------------------------------------------------
void CKnob::setCoronaOutlineWidthAdd (CCoord width)
{
	if (width != coronaOutlineWidthAdd)
	{
		coronaOutlineWidthAdd = width;
		invalid ();
	}
}

//------------------------------------------------------------------------
const CLineStyle::CoordVector& CKnob::getCoronaDashDotLengths () const
{
	return coronaLineStyle.getDashLengths ();
}

//------------------------------------------------------------------------
void CKnob::setCoronaDashDotLengths (const CLineStyle::CoordVector& lengths)
{
	if (coronaLineStyle.getDashLengths () != lengths)
	{
		coronaLineStyle.getDashLengths () = lengths;
		invalid ();
	}
}

//------------------------------------------------------------------------
void CKnob::setDrawStyle (int32_t style)
{
	if (style != drawStyle)
	{
		drawStyle = style;
		invalid ();
	}
}

//------------------------------------------------------------------------
void CKnob::setHandleBitmap (const SharedPointer<CBitmap>& bitmap)
{
	pHandle.reset ();
	if (bitmap)
	{
		pHandle = bitmap;
		inset = (CCoord)((float)pHandle->getWidth () / 2.f + 2.5f);
	}
	invalid ();
}

//------------------------------------------------------------------------
// CAnimKnob
//------------------------------------------------------------------------
/*! @class CAnimKnob
Such as a CKnob control object, but there is a unique bitmap which contains different views
(subbitmaps) of this knob. According to the value, a specific subbitmap is displayed. Use a
CMultiFrameBitmap for its background bitmap.
*/
//------------------------------------------------------------------------
/**
 * CAnimKnob constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background the background bitmap
 */
//------------------------------------------------------------------------
CAnimKnob::CAnimKnob (const CRect& size, IControlListener* listener, int32_t tag,
					  const SharedPointer<CBitmap>& background)
: CKnobBase (size, listener, tag, background), bInverseBitmap (false)
{
	inset = 0;
}

//------------------------------------------------------------------------
CAnimKnob::CAnimKnob (const CAnimKnob& v) : CKnobBase (v), bInverseBitmap (v.bInverseBitmap) {}

//-----------------------------------------------------------------------------------------------
bool CAnimKnob::sizeToFit ()
{
	if (auto bitmap = getDrawBackground ())
	{
		CRect vs (getViewSize ());
		if (auto mfb = bitmap.cast<CMultiFrameBitmap> ())
		{
			vs.setSize (mfb->getFrameSize ());
		}
		else
		{
			vs.setWidth (bitmap->getWidth ());
			vs.setHeight (bitmap->getHeight ());
		}
		setViewSize (vs);
		setMouseableArea (vs);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------------------
void CAnimKnob::setBackground (const SharedPointer<CBitmap>& background)
{
	CKnobBase::setBackground (background);
}

//------------------------------------------------------------------------
void CAnimKnob::draw (CDrawContext& context)
{
	if (auto bitmap = getDrawBackground ())
	{
		if (auto mfb = bitmap.cast<CMultiFrameBitmap> ())
		{
			auto frameIndex = getMultiFrameBitmapIndex (*mfb.get (), getValueNormalized ());
			if (bInverseBitmap)
				frameIndex = getInverseIndex (*mfb.get (), frameIndex);
			mfb->drawFrame (context, frameIndex, getViewSize ().getTopLeft ());
		}
		else
		{
			CView::draw (context);
		}
	}
}

} // VSTGUI
