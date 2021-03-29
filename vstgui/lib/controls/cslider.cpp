// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../cbitmap.h"
#include "../cdrawcontext.h"
#include "../cgraphicspath.h"
#include "../cvstguitimer.h"
#include "cslider.h"
#include <cmath>

namespace VSTGUI {

//------------------------------------------------------------------------
bool CSliderBase::kAlwaysUseZoomFactor = false;

//------------------------------------------------------------------------
struct CSliderBase::Impl
{
	SharedPointer<CVSTGUITimer> rampTimer;

	int32_t style {0};
	float zoomFactor {10.f};

	CSliderMode mode {CSliderMode::UseGlobal};

	CPoint offsetHandle;
	CPoint handleSize {1., 1.};

	CCoord rangeHandle {0.};
	CCoord minTmp {0.};
	CCoord maxTmp {0.};
	CCoord minPos {0.};

	// mouse editing values
	CPoint meStartPoint;
	float mePreviousVal;
	float meStartValue;
	CButtonState meOldButton;
	CCoord meDelta {0.};

	static CSliderMode globalMode;
};

//------------------------------------------------------------------------
CSliderMode CSliderBase::Impl::globalMode = CSliderMode::FreeClick;

//------------------------------------------------------------------------
CSliderBase::CSliderBase (const CRect& size, IControlListener* listener, int32_t tag)
: CControl (size, listener, tag)
{
	impl = std::unique_ptr<Impl> (new Impl);
}

//------------------------------------------------------------------------
CSliderBase::CSliderBase (const CSliderBase& v) : CControl (v)
{
	impl = std::unique_ptr<Impl> (new Impl (*v.impl));
}

//------------------------------------------------------------------------
CSliderBase::~CSliderBase () noexcept
{
}

//------------------------------------------------------------------------
void CSliderBase::setHandleRangePrivate (CCoord range)
{
	impl->rangeHandle = range;
	updateInternalHandleValues ();
}

//------------------------------------------------------------------------
void CSliderBase::setHandleMinPosPrivate (CCoord pos)
{
	impl->minPos = pos;
	updateInternalHandleValues ();
}

//------------------------------------------------------------------------
CCoord CSliderBase::getHandleMinPosPrivate () const
{
	return impl->minPos;
}

//------------------------------------------------------------------------
void CSliderBase::setOffsetHandle (const CPoint& val)
{
	impl->offsetHandle = val;
	updateInternalHandleValues ();
}

//------------------------------------------------------------------------
CPoint CSliderBase::getOffsetHandle () const
{
	return impl->offsetHandle;
}

//------------------------------------------------------------------------
void CSliderBase::setHandleSizePrivate (CCoord width, CCoord height)
{
	impl->handleSize.x = width;
	impl->handleSize.y = height;
	updateInternalHandleValues ();
}

//------------------------------------------------------------------------
CPoint CSliderBase::getHandleSizePrivate () const
{
	return impl->handleSize;
}

//------------------------------------------------------------------------
CPoint CSliderBase::getControlSizePrivate () const
{
	return {getWidth (), getHeight ()};
}

//------------------------------------------------------------------------
void CSliderBase::setStyle (int32_t _style)
{
	vstgui_assert (((_style & kHorizontal) || (_style & kVertical)) &&
	               !((_style & kVertical) && (_style & kHorizontal)));

	impl->style = _style;
}

//------------------------------------------------------------------------
int32_t CSliderBase::getStyle () const
{
	return impl->style;
}

//------------------------------------------------------------------------
bool CSliderBase::isStyleHorizontal () const
{
	return impl->style & kHorizontal;
}

//------------------------------------------------------------------------
bool CSliderBase::isStyleRight () const
{
	return impl->style & kRight;
}

//------------------------------------------------------------------------
bool CSliderBase::isStyleBottom () const
{
	return impl->style & kBottom;
}

//------------------------------------------------------------------------
bool CSliderBase::isInverseStyle () const
{
	if (isStyleHorizontal ())
		return getStyle () & kRight;
	return getStyle () & kTop;
}

//------------------------------------------------------------------------
void CSliderBase::setZoomFactor (float val)
{
	impl->zoomFactor = val;
}

//------------------------------------------------------------------------
float CSliderBase::getZoomFactor () const
{
	return impl->zoomFactor;
}

//------------------------------------------------------------------------
void CSliderBase::setSliderMode (CSliderMode newMode)
{
	impl->mode = newMode;
}

//------------------------------------------------------------------------
CSliderMode CSliderBase::getSliderMode () const
{
	return impl->mode;
}

//------------------------------------------------------------------------
CSliderMode CSliderBase::getEffectiveSliderMode () const
{
	if (impl->mode == CSliderMode::UseGlobal)
		return Impl::globalMode;
	return impl->mode;
}

//------------------------------------------------------------------------
void CSliderBase::setGlobalMode (CSliderMode mode)
{
	vstgui_assert (mode != CSliderMode::UseGlobal, "do not set the global mode to use global");
	Impl::globalMode = mode;
}

//------------------------------------------------------------------------
CSliderMode CSliderBase::getGlobalMode ()
{
	return Impl::globalMode;
}

//------------------------------------------------------------------------
void CSliderBase::setViewSize (const CRect& rect, bool invalid)
{
	CControl::setViewSize (rect, invalid);
	if (isStyleHorizontal ())
	{
		impl->minPos = rect.left - getViewSize ().left;
		impl->rangeHandle = rect.getWidth () - (impl->handleSize.x + impl->offsetHandle.x * 2);
	}
	else
	{
		impl->minPos = rect.top - getViewSize ().top;
		impl->rangeHandle = rect.getHeight () - (impl->handleSize.y + impl->offsetHandle.y * 2);
	}
	updateInternalHandleValues ();
}

//------------------------------------------------------------------------
void CSliderBase::updateInternalHandleValues ()
{
	if (isStyleHorizontal ())
	{
		impl->minTmp = impl->offsetHandle.x + impl->minPos;
		impl->maxTmp = impl->minTmp + impl->rangeHandle + impl->handleSize.x;
	}
	else
	{
		impl->minTmp = impl->offsetHandle.y + impl->minPos;
		impl->maxTmp = impl->minTmp + impl->rangeHandle + impl->handleSize.y;
	}
}

//------------------------------------------------------------------------
CRect CSliderBase::calculateHandleRect (float normValue) const
{
	if (isStyleRight () || isStyleBottom ())
		normValue = 1.f - normValue;

	CRect r;
	if (isStyleHorizontal ())
	{
		r.top = impl->offsetHandle.y;
		r.bottom = r.top + impl->handleSize.y;

		r.left = impl->offsetHandle.x + floor (normValue * impl->rangeHandle);
		r.left = (r.left < impl->minTmp) ? impl->minTmp : r.left;

		r.right = r.left + impl->handleSize.x;
		r.right = (r.right > impl->maxTmp) ? impl->maxTmp : r.right;
	}
	else
	{
		r.left = impl->offsetHandle.x;
		r.right = r.left + impl->handleSize.x;

		r.top = impl->offsetHandle.y + floor (normValue * impl->rangeHandle);
		r.top = (r.top < impl->minTmp) ? impl->minTmp : r.top;

		r.bottom = r.top + impl->handleSize.y;
		r.bottom = (r.bottom > impl->maxTmp) ? impl->maxTmp : r.bottom;
	}
	r.offset (getViewSize ().left, getViewSize ().top);
	return r;
}

//------------------------------------------------------------------------
float CSliderBase::calculateDelta (const CPoint& where, CRect* handleRect) const
{
	CCoord result;
	if (isStyleHorizontal ())
		result = getViewSize ().left + impl->offsetHandle.x;
	else
		result = getViewSize ().top + impl->offsetHandle.y;
	if (getEffectiveSliderMode () != CSliderMode::FreeClick)
	{
		float normValue = getValueNormalized ();
		if (isStyleRight () || isStyleBottom ())
			normValue = 1.f - normValue;
		CCoord actualPos;

		actualPos = result + (int32_t) (normValue * impl->rangeHandle);

		if (isStyleHorizontal ())
		{
			if (handleRect)
			{
				handleRect->left = actualPos;
				handleRect->top = getViewSize ().top + impl->offsetHandle.y;
				handleRect->right = handleRect->left + impl->handleSize.x;
				handleRect->bottom = handleRect->top + impl->handleSize.y;
			}
			result += where.x - actualPos;
		}
		else
		{
			if (handleRect)
			{
				handleRect->left = getViewSize ().left + impl->offsetHandle.x;
				handleRect->top = actualPos;
				handleRect->right = handleRect->left + impl->handleSize.x;
				handleRect->bottom = handleRect->top + impl->handleSize.y;
			}
			result += where.y - actualPos;
		}
	}
	else
	{
		if (isStyleHorizontal ())
			result += impl->handleSize.x / 2 - 1;
		else
			result += impl->handleSize.y / 2 - 1;
	}
	return (float)result;
}

//------------------------------------------------------------------------
void CSliderBase::doRamping ()
{
	float distance = 1.f;
	float normValue = getValueNormalized ();

	auto handleRect = calculateHandleRect (normValue);
	if (isStyleHorizontal ())
		distance = impl->meStartPoint.x < handleRect.getCenter ().x ? -0.1f : 0.1f;
	else
		distance = impl->meStartPoint.y < handleRect.getCenter ().y ? 0.1f : -0.1f;

	if (isInverseStyle ())
		distance *= -1.f;

	CCoord delta;
	if (isStyleHorizontal ())
		delta = getViewSize ().left + impl->offsetHandle.x + impl->handleSize.x / 2 - 1;
	else
		delta = getViewSize ().top + impl->offsetHandle.y + impl->handleSize.y / 2 - 1;

	float clickValue;
	if (isStyleHorizontal ())
		clickValue = (float)(impl->meStartPoint.x - delta) / (float)impl->rangeHandle;
	else
		clickValue = (float)(impl->meStartPoint.y - delta) / (float)impl->rangeHandle;

	if (isStyleRight () || isStyleBottom ())
		clickValue = 1.f - clickValue;

	normValue += distance * getWheelInc ();
	if ((clickValue > normValue && distance < 0.f) || (clickValue < normValue && distance > 0.f))
	{
		normValue = clickValue;
		impl->rampTimer = nullptr;
		impl->meDelta = delta;
	}

	setValueNormalized (normValue);
	if (isDirty ())
	{
		valueChanged ();
		invalid ();
	}
}

//------------------------------------------------------------------------
CMouseEventResult CSliderBase::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	invalidMouseWheelEditTimer (this);

	CRect handleRect;
	impl->meDelta = calculateDelta (
	    where, getEffectiveSliderMode () != CSliderMode::FreeClick ? &handleRect : nullptr);
	if (getEffectiveSliderMode () == CSliderMode::Touch && !handleRect.pointInside (where))
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;

	impl->mePreviousVal = getMin () - 1;
	impl->meOldButton = buttons;

	if ((getEffectiveSliderMode () == CSliderMode::RelativeTouch &&
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

	impl->meStartValue = getValue ();
	beginEdit ();
	impl->meStartPoint = where;
	if (buttons & kZoomModifier)
		return kMouseEventHandled;
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CSliderBase::onMouseCancel ()
{
	if (isEditing ())
	{
		value = impl->meStartValue;
		if (isDirty ())
		{
			valueChanged ();
			invalid ();
		}
		impl->meOldButton = 0;
		impl->rampTimer = nullptr;
		endEdit ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CSliderBase::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	if (isEditing ())
	{
		impl->meOldButton = 0;
		impl->rampTimer = nullptr;
		endEdit ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CSliderBase::onMouseMoved (CPoint& where, const CButtonState& _buttons)
{
	if (_buttons.isLeftButton () && isEditing ())
	{
		CButtonState buttons (_buttons);
		if (kAlwaysUseZoomFactor)
			buttons |= kZoomModifier;
		if (buttons.isLeftButton ())
		{
			if (impl->rampTimer)
			{
				impl->meStartPoint = where;
				return kMouseEventHandled;
			}
			if (kAlwaysUseZoomFactor)
			{
				CCoord distance = fabs (isStyleHorizontal () ? where.y - impl->meStartPoint.y :
				                                               where.x - impl->meStartPoint.x);
				float newZoomFactor = 1.f;
				if (distance > (isStyleHorizontal () ? getHeight () : getWidth ()))
				{
					newZoomFactor =
					    (float)(distance / (isStyleHorizontal () ? getHeight () : getWidth ()));
					newZoomFactor = static_cast<int32_t> (newZoomFactor * 10.f) / 10.f;
				}
				if (impl->zoomFactor != newZoomFactor)
				{
					impl->zoomFactor = newZoomFactor;
					impl->mePreviousVal = (value - getMin ()) / getRange ();
					impl->meDelta = calculateDelta (where);
				}
			}

			if (impl->mePreviousVal == getMin () - 1)
				impl->mePreviousVal = (value - getMin ()) / getRange ();

			if ((impl->meOldButton != buttons) && (buttons & kZoomModifier))
			{
				impl->mePreviousVal = (value - getMin ()) / getRange ();
				impl->meOldButton = buttons;
			}
			else if (!(buttons & kZoomModifier))
				impl->mePreviousVal = (value - getMin ()) / getRange ();

			float normValue;
			if (isStyleHorizontal ())
				normValue = (float)(where.x - impl->meDelta) / (float)impl->rangeHandle;
			else
				normValue = (float)(where.y - impl->meDelta) / (float)impl->rangeHandle;

			if (isStyleRight () || isStyleBottom ())
				normValue = 1.f - normValue;

			if (buttons & kZoomModifier)
				normValue = impl->mePreviousVal + ((normValue - impl->mePreviousVal) / impl->zoomFactor);

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
bool CSliderBase::onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance,
                           const CButtonState& buttons)
{
	if (!getMouseEnabled ())
		return false;

	if ((isStyleHorizontal () && axis == kMouseWheelAxisY) ||
	    (!isStyleHorizontal () && axis == kMouseWheelAxisX))
		return false;

	onMouseWheelEditing (this);

	float _distance = distance;
	if (isStyleHorizontal ())
		_distance *= -1.f;
	if (isInverseStyle ())
		_distance *= -1.f;
	float normValue = getValueNormalized ();
	if (buttons & kZoomModifier)
		normValue += 0.1f * _distance * getWheelInc ();
	else
		normValue += _distance * getWheelInc ();

	setValueNormalized (normValue);

	if (isDirty ())
	{
		invalid ();

		valueChanged ();
	}

	return true;
}

//------------------------------------------------------------------------
int32_t CSliderBase::onKeyDown (VstKeyCode& keyCode)
{
	switch (keyCode.virt)
	{
		case VKEY_UP:
		case VKEY_RIGHT:
		case VKEY_DOWN:
		case VKEY_LEFT:
		{
			float distance = 1.f;
			bool isInverse = isInverseStyle ();
			if ((keyCode.virt == VKEY_DOWN && !isInverse) ||
			    (keyCode.virt == VKEY_UP && isInverse) ||
			    (keyCode.virt == VKEY_LEFT && !isInverse) ||
			    (keyCode.virt == VKEY_RIGHT && isInverse))
			{
				distance = -distance;
			}

			float normValue = getValueNormalized ();
			if (mapVstKeyModifier (keyCode.modifier) & kZoomModifier)
				normValue += 0.1f * distance * getWheelInc ();
			else
				normValue += distance * getWheelInc ();

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
		case VKEY_ESCAPE:
		{
			if (isEditing ())
			{
				onMouseCancel ();
				return 1;
			}
			break;
		}
	}
	return -1;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
struct CSlider::Impl
{
	CPoint backgroundOffset;

	SharedPointer<CBitmap> pHandle;
	CCoord frameWidth {1.};

	int32_t drawStyle {0};
	CColor frameColor {kGreyCColor};
	CColor backColor {kBlackCColor};
	CColor valueColor {kWhiteCColor};
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
CSlider::CSlider (const CRect& rect, IControlListener* listener, int32_t tag, int32_t iMinPos,
                  int32_t iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset,
                  const int32_t style)
: CSliderBase (rect, listener, tag)
{
	impl = std::unique_ptr<Impl> (new Impl);

	setBackgroundOffset (offset);
	setBackground (background);
	setStyle (style);
	setHandle (handle);

	if (style & kHorizontal)
	{
		setHandleMinPosPrivate (iMinPos - getViewSize ().left);
		setHandleRangePrivate ((CCoord)iMaxPos - iMinPos);
	}
	else
	{
		setHandleMinPosPrivate (iMinPos - getViewSize ().top);
		setHandleRangePrivate ((CCoord)iMaxPos - iMinPos);
	}

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
CSlider::CSlider (const CRect& rect, IControlListener* listener, int32_t tag,
                  const CPoint& offsetHandle, int32_t _rangeHandle, CBitmap* handle,
                  CBitmap* background, const CPoint& offset, const int32_t style)
: CSliderBase (rect, listener, tag)
{
	impl = std::unique_ptr<Impl> (new Impl);

	setBackgroundOffset (offset);
	setBackground (background);
	setStyle (style);
	setHandle (handle);

	if (isStyleHorizontal ())
		setHandleRangePrivate (_rangeHandle - getHandleSizePrivate ().x);
	else
		setHandleRangePrivate (_rangeHandle - getHandleSizePrivate ().y);

	setOffsetHandle (offsetHandle);

	setWantsFocus (true);
}

//------------------------------------------------------------------------
CSlider::CSlider (const CSlider& v) : CSliderBase (v)
{
	impl = std::unique_ptr<Impl> (new Impl (*v.impl));
}

//------------------------------------------------------------------------
CSlider::~CSlider () noexcept
{
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
void CSlider::setOffset (const CPoint& val)
{
	setBackgroundOffset (val);
}

//------------------------------------------------------------------------
CPoint CSlider::getOffset () const
{
	return getBackgroundOffset ();
}
#endif

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
void CSlider::setBackgroundOffset (const CPoint& offset)
{
	impl->backgroundOffset = offset;
}

//------------------------------------------------------------------------
CPoint CSlider::getBackgroundOffset () const
{
	return impl->backgroundOffset;
}

//------------------------------------------------------------------------
void CSlider::draw (CDrawContext* pContext)
{
	CDrawContext* drawContext = pContext;

	// draw background
	if (getDrawBackground ())
	{
		CRect rect (0, 0, getControlSizePrivate ().x, getControlSizePrivate ().y);
		rect.offset (getViewSize ().left, getViewSize ().top);
		getDrawBackground ()->draw (drawContext, rect, getBackgroundOffset ());
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
void CSlider::setHandle (CBitmap* _pHandle)
{
	impl->pHandle = _pHandle;
	if (impl->pHandle)
	{
		setHandleSizePrivate (impl->pHandle->getWidth (), impl->pHandle->getHeight ());
		setViewSize (getViewSize (), true);
	}
	else
	{
		setHandleSizePrivate (1., 1.);
	}
}

//------------------------------------------------------------------------
CBitmap* CSlider::getHandle () const
{
	return impl->pHandle;
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
CVerticalSlider::CVerticalSlider (const CRect& rect, IControlListener* listener, int32_t tag,
                                  int32_t iMinPos, int32_t iMaxPos, CBitmap* handle,
                                  CBitmap* background, const CPoint& offset, const int32_t style)
: CSlider (rect, listener, tag, iMinPos, iMaxPos, handle, background, offset, style | kVertical)
{
}

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
CVerticalSlider::CVerticalSlider (const CRect& rect, IControlListener* listener, int32_t tag,
                                  const CPoint& offsetHandle, int32_t rangeHandle, CBitmap* handle,
                                  CBitmap* background, const CPoint& offset, const int32_t style)
: CSlider (rect, listener, tag, offsetHandle, rangeHandle, handle, background, offset,
           style | kVertical)
{
}

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
CHorizontalSlider::CHorizontalSlider (const CRect& rect, IControlListener* listener, int32_t tag,
                                      int32_t iMinPos, int32_t iMaxPos, CBitmap* handle,
                                      CBitmap* background, const CPoint& offset,
                                      const int32_t style)
: CSlider (rect, listener, tag, iMinPos, iMaxPos, handle, background, offset, style | kHorizontal)
{
}

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
CHorizontalSlider::CHorizontalSlider (const CRect& rect, IControlListener* listener, int32_t tag,
                                      const CPoint& offsetHandle, int32_t rangeHandle,
                                      CBitmap* handle, CBitmap* background, const CPoint& offset,
                                      const int32_t style)
: CSlider (rect, listener, tag, offsetHandle, rangeHandle, handle, background, offset,
           style | kHorizontal)
{
}

} // VSTGUI
