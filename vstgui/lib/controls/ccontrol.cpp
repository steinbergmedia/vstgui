// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "ccontrol.h"
#include "icontrollistener.h"
#include "../algorithm.h"
#include "../events.h"
#include "../cframe.h"
#include "../cgraphicspath.h"
#include "../cvstguitimer.h"
#include "../dispatchlist.h"
#include "../iviewlistener.h"
#include <cassert>

#define VSTGUI_CCONTROL_LOG_EDITING 0 //DEBUG

namespace VSTGUI {

//------------------------------------------------------------------------
struct CControl::Impl : ViewEventListenerAdapter
{
	using SubListenerDispatcher = DispatchList<IControlListener*>;

	SubListenerDispatcher subListeners;
	float oldValue {1};
	float defaultValue {0.5};
	float vmin {0};
	float vmax {1.f};
	float wheelInc {0.1f};
	int32_t editing {0};

	void viewOnEvent (CView* view, Event& event) override
	{
		if (event.type != EventType::MouseDown)
			return;
		auto control = static_cast<CControl*> (view);
		auto& mouseDownEvent = castMouseDownEvent (event);
		if (CControl::CheckDefaultValueEventFunc (control, mouseDownEvent))
		{
			auto defValue = control->getDefaultValue ();
			if (defValue != control->getValue ())
			{
				control->beginEdit ();
				control->setValue (defValue);
				control->valueChanged ();
				control->endEdit ();
				control->setDirty ();
			}
			mouseDownEvent.consumed = true;
			mouseDownEvent.ignoreFollowUpMoveAndUpEvents (true);
		}
	}
};

//------------------------------------------------------------------------
// CControl
//------------------------------------------------------------------------
/*! @class CControl
This object manages the tag identification and the value of a control object.
*/
CControl::CControl (const CRect& size, IControlListener* listener, int32_t tag, CBitmap *pBackground)
: CView (size)
, listener (listener)
, tag (tag)
, value (0)
{
	impl = std::unique_ptr<Impl> (new Impl);
	setTransparency (false);
	setMouseEnabled (true);
	setBackground (pBackground);
	registerViewEventListener (impl.get ());
}

//------------------------------------------------------------------------
CControl::CControl (const CControl& c)
: CView (c)
, listener (c.listener)
, tag (c.tag)
, value (c.value)
{
	impl = std::unique_ptr<Impl> (new Impl);
	impl->oldValue = c.impl->oldValue;
	impl->defaultValue = c.impl->defaultValue;
	impl->vmin = c.impl->vmin;
	impl->vmax = c.impl->vmax;
	impl->wheelInc = c.impl->wheelInc;
	registerViewEventListener (impl.get ());
}

//------------------------------------------------------------------------
CControl::~CControl () noexcept
{
	unregisterViewEventListener (impl.get ());
}

//------------------------------------------------------------------------
void CControl::registerControlListener (IControlListener* subListener)
{
	vstgui_assert (listener != subListener, "the subListener is already the main listener");
	impl->subListeners.add (subListener);
}

//------------------------------------------------------------------------
void CControl::unregisterControlListener (IControlListener* subListener)
{
	impl->subListeners.remove (subListener);
}

//------------------------------------------------------------------------
void CControl::setWheelInc (float val)
{
	impl->wheelInc = val;
}

//------------------------------------------------------------------------
float CControl::getWheelInc () const
{
	return impl->wheelInc;
}

//------------------------------------------------------------------------
void CControl::setMin (float val)
{
	impl->vmin = val;
	bounceValue ();
}
//------------------------------------------------------------------------
float CControl::getMin () const
{
	return impl->vmin;
}
//------------------------------------------------------------------------
void CControl::setMax (float val)
{
	impl->vmax = val;
	bounceValue ();
}
//------------------------------------------------------------------------
float CControl::getMax () const
{
	return impl->vmax;
}

//------------------------------------------------------------------------
void CControl::setOldValue (float val)
{
	impl->oldValue = val;
}

//------------------------------------------------------------------------
float CControl::getOldValue (void) const
{
	return impl->oldValue;
}

//------------------------------------------------------------------------
void CControl::setDefaultValue (float val)
{
	impl->defaultValue = val;
}

//------------------------------------------------------------------------
float CControl::getDefaultValue (void) const
{
	return impl->defaultValue;
}

//------------------------------------------------------------------------
void CControl::setTag (int32_t val)
{
	if (listener)
		listener->controlTagWillChange (this);
	tag = val;
	if (listener)
		listener->controlTagDidChange (this);
}

//------------------------------------------------------------------------
bool CControl::isEditing () const
{
	return impl->editing > 0;
}

//------------------------------------------------------------------------
void CControl::beginEdit ()
{
	// begin of edit parameter
	impl->editing++;
	if (impl->editing == 1)
	{
		if (listener)
			listener->controlBeginEdit (this);
		impl->subListeners.forEach ([this] (IControlListener* l) { l->controlBeginEdit (this); });
		if (getFrame ())
			getFrame ()->beginEdit (tag);
	}
#if VSTGUI_CCONTROL_LOG_EDITING
	DebugPrint("beginEdit [%d] - %d\n", tag, impl->editing);
#endif
}

//------------------------------------------------------------------------
void CControl::endEdit ()
{
	if (!isEditing ())
		return;
	--impl->editing;
	if (impl->editing == 0)
	{
		if (getFrame ())
			getFrame ()->endEdit (tag);
		if (listener)
			listener->controlEndEdit (this);
		impl->subListeners.forEach ([this] (IControlListener* l) { l->controlEndEdit (this); });
	}
#if VSTGUI_CCONTROL_LOG_EDITING
	DebugPrint("endEdit [%d] - %d\n", tag, impl->editing);
#endif
}

//------------------------------------------------------------------------
void CControl::setValue (float val) { value = clamp (val, getMin (), getMax ()); }

//------------------------------------------------------------------------
void CControl::setValueNormalized (float val)
{
	if (getRange () == 0.f)
	{
		value = getMin ();
		return;
	}
	val = clampNorm (val);
	setValue (normalizedToPlain (val, getMin (), getMax ()));
}

//------------------------------------------------------------------------
float CControl::getValueNormalized () const
{
	auto range = getRange ();
	if (range == 0.f)
		return 0.f;
	return plainToNormalized<float> (value, getMin (), getMax ());
}

//------------------------------------------------------------------------
void CControl::valueChanged ()
{
	if (listener)
		listener->valueChanged (this);
	impl->subListeners.forEach ([this] (IControlListener* l) { l->valueChanged (this); });
}

//------------------------------------------------------------------------
bool CControl::isDirty () const
{
	if (getOldValue () != value || CView::isDirty ())
		return true;
	return false;
}

//------------------------------------------------------------------------
void CControl::setDirty (bool val)
{
	CView::setDirty (val);
	if (val)
	{
		if (value != -1.f)
			setOldValue (-1.f);
		else
			setOldValue (0.f);
	}
	else
		setOldValue (value);
}

//------------------------------------------------------------------------
void CControl::bounceValue () { value = clamp (value, getMin (), getMax ()); }

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
CControl::CheckDefaultValueFuncT CControl::CheckDefaultValueFunc = [] (CControl*,
																	   CButtonState button) {
#if TARGET_OS_IPHONE
	return button.isDoubleClick ();
#else
	return (button.isLeftButton () && button.getModifierState () == kDefaultValueModifier);
#endif // TARGET_OS_IPHONE
};

#endif // VSTGUI_ENABLE_DEPRECATED_METHODS

//------------------------------------------------------------------------
CControl::CheckDefaultValueEventFuncT CControl::CheckDefaultValueEventFunc =
	[] (CControl* c, MouseDownEvent& event) {
#if VSTGUI_ENABLE_DEPRECATED_METHODS
		if (event.buttonState.isLeft ())
		{
			return CheckDefaultValueFunc (c, buttonStateFromMouseEvent (event));
		}
		return false;
#else
#if TARGET_OS_IPHONE
		return event.buttonState.isLeft () && event.clickCount == 2;
#else
		return event.buttonState.isLeft () && event.modifiers.is (ModifierKey::Control);
#endif // TARGET_OS_IPHONE
#endif // VSTGUI_ENABLE_DEPRECATED_METHODS
	};

//------------------------------------------------------------------------
bool CControl::drawFocusOnTop ()
{
	return false;
}

//------------------------------------------------------------------------
bool CControl::getFocusPath (CGraphicsPath& outPath)
{
	if (wantsFocus ())
	{
		CCoord focusWidth = getFrame ()->getFocusWidth ();
		CRect r (getVisibleViewSize ());
		if (!r.isEmpty ())
		{
			outPath.addRect (r);
			r.extend (focusWidth, focusWidth);
			outPath.addRect (r);
		}
	}
	return true;
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
int32_t CControl::mapVstKeyModifier (int32_t vstModifier)
{
	int32_t modifiers = 0;
	if (vstModifier & MODIFIER_SHIFT)
		modifiers |= kShift;
	if (vstModifier & MODIFIER_ALTERNATE)
		modifiers |= kAlt;
	if (vstModifier & MODIFIER_COMMAND)
		modifiers |= kApple;
	if (vstModifier & MODIFIER_CONTROL)
		modifiers |= kControl;
	return modifiers;
}
#endif // VSTGUI_ENABLE_DEPRECATED_METHODS

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
void IMultiBitmapControl::autoComputeHeightOfOneImage ()
{
	auto* view = dynamic_cast<CView*>(this);
	if (view)
	{
		const CRect& viewSize = view->getViewSize ();
		heightOfOneImage = viewSize.getHeight ();
	}
}
#endif

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
void CMouseWheelEditingSupport::onMouseWheelEditing (CControl* control)
{
	if (!control->isEditing ())
		control->beginEdit ();
	endEditTimer = makeOwned<CVSTGUITimer> (
	    [control] (CVSTGUITimer* timer) {
		    control->endEdit ();
		    timer->stop ();
	    },
	    500);
}

//------------------------------------------------------------------------
void CMouseWheelEditingSupport::invalidMouseWheelEditTimer (CControl* control)
{
	if (endEditTimer)
		endEditTimer = nullptr;
	if (control->isEditing ())
		control->endEdit ();
}

} // VSTGUI
