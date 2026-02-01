// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "ccontrol.h"
#include "icontrollistener.h"
#include "../algorithm.h"
#include "../events.h"
#include "../cframe.h"
#include "../cbitmap.h"
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
	IControlListener* listener;
	float value;
	float oldValue {1};
	float defaultValue {0.5};
	float vmin {0};
	float vmax {1.f};
	float wheelInc {0.1f};
	int32_t editing {0};
	int32_t tag {-1};

	void viewOnEvent (CView& view, Event& event) override
	{
		if (event.type != EventType::MouseDown)
			return;
		auto control = static_cast<CControl*> (&view);
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
				control->invalid ();
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
CControl::CControl (const CRect& size, IControlListener* listener, int32_t tag,
					const SharedPointer<CBitmap>& pBackground)
: CView (size)
{
	impl = std::unique_ptr<Impl> (new Impl);
	impl->listener = listener;
	impl->tag = tag;
	setTransparency (false);
	setMouseEnabled (true);
	setBackground (pBackground);
}

//------------------------------------------------------------------------
CControl::CControl (const CControl& c) : CView (c)
{
	impl = std::unique_ptr<Impl> (new Impl);
	impl->oldValue = c.impl->oldValue;
	impl->defaultValue = c.impl->defaultValue;
	impl->vmin = c.impl->vmin;
	impl->vmax = c.impl->vmax;
	impl->wheelInc = c.impl->wheelInc;
	impl->listener = c.impl->listener;
	impl->tag = c.impl->tag;
	impl->value = c.impl->value;
}

//------------------------------------------------------------------------
CControl::~CControl () noexcept {}

//------------------------------------------------------------------------
bool CControl::attached (const SharedPointer<CViewContainer>& parent)
{
	if (CView::attached (parent))
	{
		registerViewEventListener (impl.get ());
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool CControl::removed (const SharedPointer<CViewContainer>& parent)
{
	unregisterViewEventListener (impl.get ());
	return CView::removed (parent);
}

//------------------------------------------------------------------------
IControlListener* CControl::getListener () const { return impl->listener; }

//------------------------------------------------------------------------
void CControl::setListener (IControlListener* l) { impl->listener = l; }

//------------------------------------------------------------------------
void CControl::registerControlListener (IControlListener* subListener)
{
	vstgui_assert (impl->listener != subListener, "the subListener is already the main listener");
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
	if (impl->listener)
		impl->listener->controlTagWillChange (*this);
	impl->tag = val;
	if (impl->listener)
		impl->listener->controlTagDidChange (*this);
}

//------------------------------------------------------------------------
int32_t CControl::getTag () const { return impl->tag; }

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
		if (impl->listener)
			impl->listener->controlBeginEdit (*this);
		impl->subListeners.forEach ([this] (IControlListener* l) { l->controlBeginEdit (*this); });
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
		if (impl->listener)
			impl->listener->controlEndEdit (*this);
		impl->subListeners.forEach ([this] (IControlListener* l) { l->controlEndEdit (*this); });
	}
#if VSTGUI_CCONTROL_LOG_EDITING
	DebugPrint("endEdit [%d] - %d\n", tag, impl->editing);
#endif
}

//------------------------------------------------------------------------
float CControl::getValue () const { return impl->value; }

//------------------------------------------------------------------------
bool CControl::setValue (float val)
{
	val = clamp (val, getMin (), getMax ());
	if (val != impl->value)
	{
		impl->value = val;
		invalid ();
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool CControl::setValueNormalized (float val)
{
	if (getRange () == 0.f)
	{
		return setValue (getMin ());
	}
	val = clampNorm (val);
	return setValue (normalizedToPlain (val, getMin (), getMax ()));
}

//------------------------------------------------------------------------
float CControl::getValueNormalized () const
{
	auto range = getRange ();
	if (range == 0.f)
		return 0.f;
	return plainToNormalized<float> (impl->value, getMin (), getMax ());
}

//------------------------------------------------------------------------
void CControl::valueChanged ()
{
	if (impl->listener)
		impl->listener->valueChanged (*this);
	impl->subListeners.forEach ([this] (IControlListener* l) { l->valueChanged (*this); });
}

//------------------------------------------------------------------------
void CControl::bounceValue () { impl->value = clamp (impl->value, getMin (), getMax ()); }

//------------------------------------------------------------------------
CControl::CheckDefaultValueEventFuncT CControl::CheckDefaultValueEventFunc =
	[] (CControl* c, MouseDownEvent& event) {
#if TARGET_OS_IPHONE
		return event.buttonState.isLeft () && event.clickCount == 2;
#else
		return event.buttonState.isLeft () && event.modifiers.is (ModifierKey::Control);
#endif // TARGET_OS_IPHONE
	};

//------------------------------------------------------------------------
bool CControl::drawFocusOnTop ()
{
	return false;
}

//------------------------------------------------------------------------
bool CControl::getFocusPath (CGraphicsPath& outPath, CCoord focusLineWidth)
{
	if (wantsFocus ())
	{
		CRect r (getVisibleViewSize ());
		if (!r.isEmpty ())
		{
			outPath.addRect (r);
			r.extend (focusLineWidth, focusLineWidth);
			outPath.addRect (r);
		}
	}
	return true;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
void CMouseWheelEditingSupport::onMouseWheelEditing (CControl& control)
{
	if (!control.isEditing ())
		control.beginEdit ();
	endEditTimer = makeShared<CVSTGUITimer> (
		[&] (CVSTGUITimer* timer) {
			control.endEdit ();
			timer->stop ();
		},
		500);
}

//------------------------------------------------------------------------
void CMouseWheelEditingSupport::invalidMouseWheelEditTimer (CControl& control)
{
	endEditTimer.reset ();
	if (control.isEditing ())
		control.endEdit ();
}

} // VSTGUI
