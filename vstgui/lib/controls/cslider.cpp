// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cslider.h"
#include "../cdrawcontext.h"
#include "../cbitmap.h"
#include "../cgraphicspath.h"
#include "../cvstguitimer.h"
#include <cmath>

namespace VSTGUI {

bool CSlider::kAlwaysUseZoomFactor = false;
CSliderMode CSlider::globalMode = CSliderMode::FreeClick;

//------------------------------------------------------------------------
struct CSlider::Impl
{
	CPoint	offset;
	CPoint	offsetHandle;

	SharedPointer<CBitmap> pHandle;
	SharedPointer<CVSTGUITimer> rampTimer;

	int32_t	style;
	CSliderMode mode {CSliderMode::UseGlobal};

	CCoord	widthOfSlider{1};
	CCoord	heightOfSlider{1};
	CCoord	rangeHandle;
	CCoord	minTmp;
	CCoord	maxTmp;
	CCoord	minPos{0.};
	CCoord	widthControl;
	CCoord	heightControl;
	CCoord	frameWidth {1.};
	float	zoomFactor;

	int32_t	drawStyle {0};
	CColor  frameColor {kGreyCColor};
	CColor  backColor {kBlackCColor};
	CColor  valueColor {kWhiteCColor};


	CCoord	delta;
	float	oldVal;
	float	startVal;
	CButtonState oldButton;
	CPoint mouseStartPoint;
	CPoint rampMouseMovePos;

	bool styleHorizontal () const { return style & kHorizontal; }
	bool styleRight () const { return style & kRight; }
	bool styleBottom () const { return style & kBottom; }
	bool styleIsInverseStyle () const
	{
		if ((style & kVertical) && (style & kTop))
			return true;
		if ((style & kHorizontal) && (style & kRight))
			return true;
		return false;
	}

};

//------------------------------------------------------------------------
// CSlider
//------------------------------------------------------------------------
/*! @class CSlider
Define a slider with a given background and handle.
The range of variation of the handle should be defined.
By default the handler is drawn with transparency.
By clicking Alt+Left Mouse the default value is used.
*/
//------------------------------------------------------------------------
/**
 * CSlider constructor.
 * @param rect the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param iMinPos min position in pixel
 * @param iMaxPos max position in pixel
 * @param handle bitmap of the slider
 * @param background bitmap of the background
 * @param offset offset of the background
 * @param style style (kBottom,kRight,kTop,kLeft,kHorizontal,kVertical)
 */
//------------------------------------------------------------------------
CSlider::CSlider (const CRect &rect, IControlListener* listener, int32_t tag, int32_t iMinPos, int32_t iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset, const int32_t style)
: CControl (rect, listener, tag, background)
{
	impl = std::unique_ptr<Impl> (new Impl);
	impl->offset = offset;
	impl->style = style;
	impl->minPos = iMinPos;
	setHandle (handle);

	impl->widthControl  = getViewSize ().getWidth ();
	impl->heightControl = getViewSize ().getHeight ();

	if (style & kHorizontal)
	{
		impl->minPos = iMinPos - getViewSize ().left;
		impl->rangeHandle = (CCoord)iMaxPos - iMinPos;
	}
	else
	{
		impl->minPos = iMinPos - getViewSize ().top;
		impl->rangeHandle = (CCoord)iMaxPos - iMinPos;
	}

	CPoint p (0, 0);
	setOffsetHandle (p);

	impl->zoomFactor = 10.f;

	setWantsFocus (true);
}

//------------------------------------------------------------------------
/**
 * CSlider constructor.
 * @param rect the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param offsetHandle handle offset
 * @param _rangeHandle size of handle range
 * @param handle bitmap of the slider
 * @param background bitmap of the background
 * @param offset offset of the background
 * @param style style (kBottom,kRight,kTop,kLeft,kHorizontal,kVertical)
 */
//------------------------------------------------------------------------
CSlider::CSlider (const CRect &rect, IControlListener* listener, int32_t tag, const CPoint& offsetHandle, int32_t _rangeHandle, CBitmap* handle, CBitmap* background, const CPoint& offset, const int32_t style)
: CControl (rect, listener, tag, background)
{
	impl = std::unique_ptr<Impl> (new Impl);

	impl->offset = offset;
	impl->style = style;
	setHandle (handle);

	impl->widthControl  = getViewSize ().getWidth ();
	impl->heightControl = getViewSize ().getHeight ();
	if (impl->style & kHorizontal)
		impl->rangeHandle = _rangeHandle - impl->widthOfSlider;
	else
		impl->rangeHandle = _rangeHandle - impl->heightOfSlider;

	setOffsetHandle (offsetHandle);
	
	impl->zoomFactor = 10.f;

	setWantsFocus (true);
}

//------------------------------------------------------------------------
CSlider::CSlider (const CSlider& v)
: CControl (v)
{
	impl = std::unique_ptr<Impl> (new Impl (*v.impl.get ()));
}

//------------------------------------------------------------------------
CSlider::~CSlider () noexcept
{
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
void CSlider::setMode (Mode newMode)
{
	switch (newMode)
	{
		case kTouchMode: setSliderMode (CSliderMode::Touch); break;
		case kRelativeTouchMode: setSliderMode (CSliderMode::RelativeTouch); break;
		case kFreeClickMode: setSliderMode (CSliderMode::FreeClick); break;
		case kRampMode: setSliderMode (CSliderMode::Ramp); break;
	}
}

//------------------------------------------------------------------------
auto CSlider::getMode () const -> Mode
{
	switch (impl->mode)
	{
		case CSliderMode::Touch: return kTouchMode;
		case CSliderMode::RelativeTouch: return kRelativeTouchMode;
		case CSliderMode::FreeClick: return kFreeClickMode;
		case CSliderMode::Ramp: return kRampMode;
		case CSliderMode::UseGlobal:
		{
			switch (globalMode)
			{
				case CSliderMode::Touch: return kTouchMode;
				case CSliderMode::RelativeTouch: return kRelativeTouchMode;
				case CSliderMode::FreeClick: return kFreeClickMode;
				case CSliderMode::Ramp: return kRampMode;
				case CSliderMode::UseGlobal: vstgui_assert(false, ""); break;
			}
		}
	}
	return kTouchMode;
}
#endif

//------------------------------------------------------------------------
void CSlider::setSliderMode (CSliderMode newMode)
{
	impl->mode = newMode;
}

//------------------------------------------------------------------------
CSliderMode CSlider::getSliderMode () const
{
	return impl->mode;
}

//------------------------------------------------------------------------
CSliderMode CSlider::getEffectiveSliderMode () const
{
	if (impl->mode == CSliderMode::UseGlobal)
		return globalMode;
	return impl->mode;
}

//------------------------------------------------------------------------
void CSlider::setGlobalMode (CSliderMode mode)
{
	vstgui_assert (mode != CSliderMode::UseGlobal, "do not set the global mode to use global");
	globalMode = mode;
}

//------------------------------------------------------------------------
CSliderMode CSlider::getGlobalMode ()
{
	return globalMode;
}

//------------------------------------------------------------------------
void CSlider::setStyle (int32_t _style)
{
	impl->style =_style;
}

//------------------------------------------------------------------------
int32_t CSlider::getStyle () const
{
	return impl->style;
}

//------------------------------------------------------------------------
void CSlider::setViewSize (const CRect& rect, bool invalid)
{
	CControl::setViewSize (rect, invalid);
	if (impl->styleHorizontal ())
	{
		impl->minPos = rect.left - getViewSize ().left;
		impl->rangeHandle = rect.getWidth () - (impl->widthOfSlider + impl->offsetHandle.x * 2);
	}
	else
	{
		impl->minPos = rect.top - getViewSize ().top;
		impl->rangeHandle = rect.getHeight () - (impl->heightOfSlider + impl->offsetHandle.y * 2);
	}
	
	impl->widthControl  = rect.getWidth ();
	impl->heightControl = rect.getHeight ();

	setOffsetHandle (impl->offsetHandle);
}

//------------------------------------------------------------------------
bool CSlider::sizeToFit ()
{
	if (getDrawBackground ())
	{
		CRect vs (getViewSize ());
		vs.setWidth (getDrawBackground ()->getWidth ());
		vs.setHeight (getDrawBackground ()->getHeight ());
		setViewSize (vs, true);
		setMouseableArea (vs);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
void CSlider::setOffsetHandle (const CPoint &val)
{
	impl->offsetHandle = val;

	if (impl->styleHorizontal ())
	{
		impl->minTmp = impl->offsetHandle.x + impl->minPos;
		impl->maxTmp = impl->minTmp + impl->rangeHandle + impl->widthOfSlider;
	}
	else
	{
		impl->minTmp = impl->offsetHandle.y + impl->minPos;
		impl->maxTmp = impl->minTmp + impl->rangeHandle + impl->heightOfSlider;
	}
}

//------------------------------------------------------------------------
CPoint CSlider::getOffsetHandle () const
{
	return impl->offsetHandle;
}

//------------------------------------------------------------------------
void CSlider::setOffset (const CPoint& val)
{
	impl->offset = val;
}

//------------------------------------------------------------------------
CPoint CSlider::getOffset () const
{
	return impl->offset;
}

//------------------------------------------------------------------------
void CSlider::draw (CDrawContext *pContext)
{
	CDrawContext* drawContext = pContext;

	// draw background
	if (getDrawBackground ())
	{
		CRect rect (0, 0, impl->widthControl, impl->heightControl);
		rect.offset (getViewSize ().left, getViewSize ().top);
		getDrawBackground ()->draw (drawContext, rect, impl->offset);
	}
	
	if (impl->drawStyle != 0)
	{
		auto lineWidth = getFrameWidth ();
		if (lineWidth < 0.)
			lineWidth = pContext->getHairlineSize ();
		CRect r (getViewSize ());
		pContext->setDrawMode (kAntiAliasing);
		pContext->setLineStyle (kLineSolid);
		pContext->setLineWidth (lineWidth);
		if (impl->drawStyle & kDrawFrame || impl->drawStyle & kDrawBack)
		{
			pContext->setFrameColor (impl->frameColor);
			pContext->setFillColor (impl->backColor);
			if (auto path = owned (pContext->createGraphicsPath ()))
			{
				if (impl->drawStyle & kDrawFrame)
					r.inset (lineWidth / 2., lineWidth / 2.);
				path->addRect (r);
				if (impl->drawStyle & kDrawBack)
					pContext->drawGraphicsPath (path, CDrawContext::kPathFilled);
				if (impl->drawStyle & kDrawFrame)
					pContext->drawGraphicsPath (path, CDrawContext::kPathStroked);
			}
			else
			{
				CDrawStyle d = kDrawFilled;
				if (impl->drawStyle & kDrawFrame && impl->drawStyle & kDrawBack)
					d = kDrawFilledAndStroked;
				else if (impl->drawStyle & kDrawFrame)
					d = kDrawStroked;
				pContext->drawRect (r, d);
			}
		}
		if (impl->drawStyle & kDrawValue)
		{
			pContext->setDrawMode (kAliasing);
			if (impl->drawStyle & kDrawFrame)
				r.inset (lineWidth / 2., lineWidth / 2.);
			float drawValue = getValueNormalized ();
			if (impl->drawStyle & kDrawValueFromCenter)
			{
				if (impl->drawStyle & kDrawInverted)
					drawValue = 1.f - drawValue;
				if (getStyle () & kHorizontal)
				{
					CCoord width = r.getWidth ();
					r.right = r.left + r.getWidth () * drawValue;
					r.left += width / 2.;
					r.normalize ();
				}
				else
				{
					CCoord height = r.getHeight ();
					r.bottom = r.top + r.getHeight () * drawValue;
					r.top += height / 2.;
					r.normalize ();
				}
			}
			else
			{
				if (getStyle () & kHorizontal)
				{
					if (impl->drawStyle & kDrawInverted)
						r.left = r.right - r.getWidth () * drawValue;
					else
						r.right = r.left + r.getWidth () * drawValue;
				}
				else
				{
					if (impl->drawStyle & kDrawInverted)
						r.bottom = r.top + r.getHeight () * drawValue;
					else
						r.top = r.bottom - r.getHeight () * drawValue;
				}
			}
			r.normalize ();
			if (r.getWidth () >= 0.5 && r.getHeight () >= 0.5)
			{
				pContext->setFillColor (impl->valueColor);
				if (auto path = owned (pContext->createGraphicsPath ()))
				{
					path->addRect (r);
					pContext->drawGraphicsPath (path, CDrawContext::kPathFilled);
				}
				else
					pContext->drawRect (r, kDrawFilled);
			}
		}
	}
	
	if (impl->pHandle)
	{
		// calc new coords of slider
		CRect rectNew = calculateHandleRect (getValueNormalized ());

		// draw slider at new position
		impl->pHandle->draw (drawContext, rectNew);
	}

	setDirty (false);
}

//------------------------------------------------------------------------
CRect CSlider::calculateHandleRect (float normValue) const
{
	if (impl->styleRight () || impl->styleBottom ())
		normValue = 1.f - normValue;

	CRect r;
	if (impl->styleHorizontal ())
	{
		r.top = impl->offsetHandle.y;
		r.bottom = r.top + impl->heightOfSlider;

		r.left = impl->offsetHandle.x + floor (normValue * impl->rangeHandle);
		r.left = (r.left < impl->minTmp) ? impl->minTmp : r.left;

		r.right = r.left + impl->widthOfSlider;
		r.right = (r.right > impl->maxTmp) ? impl->maxTmp : r.right;
	}
	else
	{
		r.left = impl->offsetHandle.x;
		r.right = r.left + impl->widthOfSlider;

		r.top = impl->offsetHandle.y + floor (normValue * impl->rangeHandle);
		r.top = (r.top < impl->minTmp) ? impl->minTmp : r.top;

		r.bottom = r.top + impl->heightOfSlider;
		r.bottom = (r.bottom > impl->maxTmp) ? impl->maxTmp : r.bottom;
	}
	r.offset (getViewSize ().left, getViewSize ().top);
	return r;
}

//------------------------------------------------------------------------
float CSlider::calculateDelta (const CPoint& where, CRect* handleRect) const
{
	CCoord result;
	if (impl->styleHorizontal ())
		result = getViewSize ().left + impl->offsetHandle.x;
	else
		result = getViewSize ().top + impl->offsetHandle.y;
	if (getEffectiveSliderMode () != CSliderMode::FreeClick)
	{
		float normValue = getValueNormalized ();
		if (impl->styleRight () || impl->styleBottom ())
			normValue = 1.f - normValue;
		CCoord actualPos;
		
		actualPos = result + (int32_t)(normValue * impl->rangeHandle);

		if (impl->styleHorizontal ())
		{
			if (handleRect)
			{
				handleRect->left   = actualPos;
				handleRect->top    = getViewSize ().top  + impl->offsetHandle.y;
				handleRect->right  = handleRect->left + impl->widthOfSlider;
				handleRect->bottom = handleRect->top  + impl->heightOfSlider;
			}
			result += where.x - actualPos;
		}
		else
		{
			if (handleRect)
			{
				handleRect->left   = getViewSize ().left  + impl->offsetHandle.x;
				handleRect->top    = actualPos;
				handleRect->right  = handleRect->left + impl->widthOfSlider;
				handleRect->bottom = handleRect->top  + impl->heightOfSlider;
			}
			result += where.y - actualPos;
		}
	}
	else
	{
		if (impl->styleHorizontal ())
			result += impl->widthOfSlider / 2 - 1;
		else
			result += impl->heightOfSlider / 2 - 1;
	}
	return (float)result;
}

//------------------------------------------------------------------------
void CSlider::doRamping ()
{
	float distance = 1.f;
	float normValue = getValueNormalized ();

	auto handleRect = calculateHandleRect (normValue);
	if (impl->styleHorizontal ())
		distance = impl->mouseStartPoint.x < handleRect.getCenter ().x ? -0.1f : 0.1f;
	else
		distance = impl->mouseStartPoint.y < handleRect.getCenter ().y ? 0.1f : -0.1f;

	if (impl->styleIsInverseStyle ())
		distance *= -1.f;

	CCoord delta;
	if (impl->styleHorizontal ())
		delta = getViewSize ().left + impl->offsetHandle.x + impl->widthOfSlider / 2 - 1;
	else
		delta = getViewSize ().top + impl->offsetHandle.y + impl->heightOfSlider / 2 - 1;

	float clickValue;
	if (impl->styleHorizontal ())
		clickValue = (float)(impl->mouseStartPoint.x - delta) / (float)impl->rangeHandle;
	else
		clickValue = (float)(impl->mouseStartPoint.y - delta) / (float)impl->rangeHandle;

	if (impl->styleRight () || impl->styleBottom ())
		clickValue = 1.f - clickValue;

	normValue += distance * wheelInc;
	if ((clickValue > normValue && distance < 0.f) || (clickValue < normValue && distance > 0.f))
	{
		normValue = clickValue;
		impl->rampTimer = nullptr;
		impl->delta = delta;
	}

	setValueNormalized (normValue);
	if (isDirty ())
	{
		valueChanged ();
		invalid ();
	}
}

//------------------------------------------------------------------------
CMouseEventResult CSlider::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	CRect handleRect;
	impl->delta = calculateDelta (
	    where, getEffectiveSliderMode () != CSliderMode::FreeClick ? &handleRect : nullptr);
	if (getEffectiveSliderMode () == CSliderMode::Touch && !handleRect.pointInside (where))
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;

	impl->oldVal    = getMin () - 1;
	impl->oldButton = buttons;

	if (!impl->pHandle ||
	    (getEffectiveSliderMode () == CSliderMode::RelativeTouch &&
	     handleRect.pointInside (where)) ||
	    getEffectiveSliderMode () != CSliderMode::RelativeTouch)
	{
		if (checkDefaultValue (buttons))
		{
			return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
		}
	}

	if (getEffectiveSliderMode () == CSliderMode::Ramp && !handleRect.pointInside (where))
	{
		impl->rampTimer = owned (new CVSTGUITimer ([this] (CVSTGUITimer*) { doRamping (); }, 16));
	}

	impl->startVal = getValue ();
	beginEdit ();
	impl->mouseStartPoint = where;
	if (buttons & kZoomModifier)
		return kMouseEventHandled;
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CSlider::onMouseCancel ()
{
	if (isEditing ())
	{
		value = impl->startVal;
		if (isDirty ())
		{
			valueChanged ();
			invalid ();
		}
		impl->oldButton = 0;
		impl->rampTimer = nullptr;
		endEdit ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CSlider::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	if (isEditing ())
	{
		impl->oldButton = 0;
		impl->rampTimer = nullptr;
		endEdit ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CSlider::onMouseMoved (CPoint& where, const CButtonState& _buttons)
{
	if (isEditing ())
	{
		CButtonState buttons (_buttons);
		if (kAlwaysUseZoomFactor)
			buttons |= kZoomModifier;
		if (buttons.isLeftButton ())
		{
			if (impl->rampTimer)
			{
				impl->mouseStartPoint = where;
				return kMouseEventHandled;
			}
			if (kAlwaysUseZoomFactor)
			{
				CCoord distance = fabs (impl->styleHorizontal () ? where.y - impl->mouseStartPoint.y : where.x - impl->mouseStartPoint.x);
				float newZoomFactor = 1.f;
				if (distance > (impl->styleHorizontal () ? getHeight () : getWidth ()))
				{
					newZoomFactor = (float)(distance / (impl->styleHorizontal () ? getHeight () : getWidth ()));
					newZoomFactor = static_cast<int32_t>(newZoomFactor * 10.f) / 10.f;
				}
				if (impl->zoomFactor != newZoomFactor)
				{
					impl->zoomFactor = newZoomFactor;
					impl->oldVal = (value - getMin ()) / getRange ();
					impl->delta = calculateDelta (where);
				}
			}
			
			if (impl->oldVal == getMin () - 1)
				impl->oldVal = (value - getMin ()) / getRange ();
				
			if ((impl->oldButton != buttons) && (buttons & kZoomModifier))
			{
				impl->oldVal = (value - getMin ()) / getRange ();
				impl->oldButton = buttons;
			}
			else if (!(buttons & kZoomModifier))
				impl->oldVal = (value - getMin ()) / getRange ();

			float normValue;
			if (impl->styleHorizontal ())
				normValue = (float)(where.x - impl->delta) / (float)impl->rangeHandle;
			else
				normValue = (float)(where.y - impl->delta) / (float)impl->rangeHandle;

			if (impl->styleRight () || impl->styleBottom ())
				normValue = 1.f - normValue;

			if (buttons & kZoomModifier)
				normValue = impl->oldVal + ((normValue - impl->oldVal) / impl->zoomFactor);

			setValueNormalized (normValue);
				
			if (isDirty ())
			{
				valueChanged ();
				invalid ();
			}
		}
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
bool CSlider::onWheel (const CPoint& where, const float &distance, const CButtonState &buttons)
{
	if (!getMouseEnabled ())
		return false;

	float _distance = distance;
	if (impl->styleIsInverseStyle ())
		_distance *= -1.f;
	float normValue = getValueNormalized ();
	if (buttons & kZoomModifier)
		normValue += 0.1f * _distance * wheelInc;
	else
		normValue += _distance * wheelInc;

	setValueNormalized (normValue);

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
int32_t CSlider::onKeyDown (VstKeyCode& keyCode)
{
	switch (keyCode.virt)
	{
		case VKEY_UP :
		case VKEY_RIGHT :
		case VKEY_DOWN :
		case VKEY_LEFT :
		{
			float distance = 1.f;
			bool isInverse = impl->styleIsInverseStyle ();
			if ((keyCode.virt == VKEY_DOWN && !isInverse) 
			 || (keyCode.virt == VKEY_UP && isInverse)
			 || (keyCode.virt == VKEY_LEFT && !isInverse)
			 || (keyCode.virt == VKEY_RIGHT && isInverse))
			{
				distance = -distance;
			}

			float normValue = getValueNormalized ();
			if (mapVstKeyModifier (keyCode.modifier) & kZoomModifier)
				normValue += 0.1f * distance * wheelInc;
			else
				normValue += distance * wheelInc;

			setValueNormalized (normValue);

			if (isDirty ())
			{
				invalid ();

				// begin of edit parameter
				beginEdit ();
			
				valueChanged ();
			
				// end of edit parameter
				endEdit ();
			}
			return 1;
		}
	}
	return -1;
}

//------------------------------------------------------------------------
void CSlider::setHandle (CBitmap *_pHandle)
{
	impl->pHandle = _pHandle;
	if (impl->pHandle)
	{
		impl->widthOfSlider  = impl->pHandle->getWidth ();
		impl->heightOfSlider = impl->pHandle->getHeight ();
		setViewSize (getViewSize (), true);
	}
	else
	{
		impl->widthOfSlider = impl->heightOfSlider = 1.;
	}
}

//------------------------------------------------------------------------
CBitmap* CSlider::getHandle () const
{
	return impl->pHandle;
}

//------------------------------------------------------------------------
void CSlider::setZoomFactor (float val)
{
	impl->zoomFactor = val;
}

//------------------------------------------------------------------------
float CSlider::getZoomFactor () const
{
	return impl->zoomFactor;
}

//------------------------------------------------------------------------
void CSlider::setDrawStyle (int32_t style)
{
	if (style != impl->drawStyle)
	{
		impl->drawStyle = style;
		invalid ();
	}
}

//------------------------------------------------------------------------
void CSlider::setFrameWidth (CCoord width)
{
	if (impl->frameWidth != width)
	{
		impl->frameWidth = width;
		invalid ();
	}
}

//------------------------------------------------------------------------
void CSlider::setFrameColor (CColor color)
{
	if (color != impl->frameColor)
	{
		impl->frameColor = color;
		invalid ();
	}
}

//------------------------------------------------------------------------
void CSlider::setBackColor (CColor color)
{
	if (color != impl->backColor)
	{
		impl->backColor = color;
		invalid ();
	}
}

//------------------------------------------------------------------------
void CSlider::setValueColor (CColor color)
{
	if (color != impl->valueColor)
	{
		impl->valueColor = color;
		invalid ();
	}
}

//------------------------------------------------------------------------
int32_t CSlider::getDrawStyle () const
{
	return impl->drawStyle;
}

//------------------------------------------------------------------------
CCoord CSlider::getFrameWidth () const
{
	return impl->frameWidth;
}

//------------------------------------------------------------------------
CColor CSlider::getFrameColor () const
{
	return impl->frameColor;
}

//------------------------------------------------------------------------
CColor CSlider::getBackColor () const
{
	return impl->backColor;
}

//------------------------------------------------------------------------
CColor CSlider::getValueColor () const
{
	return impl->valueColor;
}

//------------------------------------------------------------------------
void CSlider::setSliderSize (CCoord width, CCoord height)
{
	impl->widthOfSlider = width;
	impl->heightOfSlider = height;
}

//------------------------------------------------------------------------
CPoint CSlider::getSliderSize () const
{
	return {impl->widthOfSlider, impl->heightOfSlider};
}

//------------------------------------------------------------------------
CPoint CSlider::getControlSize () const
{
	return {impl->widthControl, impl->heightControl};
}

//------------------------------------------------------------------------
// CVerticalSlider
//------------------------------------------------------------------------
/*! @class CVerticalSlider
This is the vertical slider. See CSlider.
*/
//------------------------------------------------------------------------
/**
 * CVerticalSlider constructor.
 * @param rect the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param iMinPos min position in pixel
 * @param iMaxPos max position in pixel
 * @param handle bitmap of the slider
 * @param background bitmap of the background
 * @param offset offset of the background
 * @param style style (kLeft, kRight)
 */
//------------------------------------------------------------------------
CVerticalSlider::CVerticalSlider (const CRect &rect, IControlListener* listener, int32_t tag, int32_t iMinPos, int32_t iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset, const int32_t style)
: CSlider (rect, listener, tag, iMinPos, iMaxPos, handle, background, offset, style|kVertical)
{}

//------------------------------------------------------------------------
/**
 * CVerticalSlider constructor.
 * @param rect the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param offsetHandle handle offset
 * @param rangeHandle size of handle range
 * @param handle bitmap of the slider
 * @param background bitmap of the background
 * @param offset offset of the background
 * @param style style (kLeft, kRight)
 */
//------------------------------------------------------------------------
CVerticalSlider::CVerticalSlider (const CRect &rect, IControlListener* listener, int32_t tag, const CPoint& offsetHandle, int32_t rangeHandle, CBitmap* handle, CBitmap* background, const CPoint& offset, const int32_t style)
: CSlider (rect, listener, tag, offsetHandle, rangeHandle, handle, background, offset, style|kVertical)
{}

//------------------------------------------------------------------------
// CHorizontalSlider
//------------------------------------------------------------------------
/*! @class CHorizontalSlider
This is the horizontal slider. See CSlider.
*/
//------------------------------------------------------------------------
/**
 * CHorizontalSlider constructor.
 * @param rect the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param iMinPos min position in pixel
 * @param iMaxPos max position in pixel
 * @param handle bitmap of the slider
 * @param background bitmap of the background
 * @param offset offset of the background
 * @param style style (kLeft, kRight)
 */
//------------------------------------------------------------------------
CHorizontalSlider::CHorizontalSlider (const CRect &rect, IControlListener* listener, int32_t tag, int32_t iMinPos, int32_t iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset, const int32_t style)
: CSlider (rect, listener, tag, iMinPos, iMaxPos, handle, background, offset, style|kHorizontal)
{}

//------------------------------------------------------------------------
/**
 * CHorizontalSlider constructor.
 * @param rect the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param offsetHandle handle offset
 * @param rangeHandle size of handle range
 * @param handle bitmap of the slider
 * @param background bitmap of the background
 * @param offset offset of the background
 * @param style style (kLeft, kRight)
 */
//------------------------------------------------------------------------
CHorizontalSlider::CHorizontalSlider (const CRect &rect, IControlListener* listener, int32_t tag, const CPoint& offsetHandle, int32_t rangeHandle, CBitmap* handle, CBitmap* background, const CPoint& offset, const int32_t style)
: CSlider (rect, listener, tag, offsetHandle, rangeHandle, handle, background, offset, style|kHorizontal)
{}

} // namespace
