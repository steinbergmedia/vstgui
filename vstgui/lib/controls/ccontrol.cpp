// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "ccontrol.h"
#include "icontrollistener.h"
#include "../cframe.h"
#include "../cgraphicspath.h"
#include <cassert>

#define VSTGUI_CCONTROL_LOG_EDITING 0 //DEBUG

namespace VSTGUI {

#if VSTGUI_ENABLE_DEPRECATED_METHODS
IdStringPtr CControl::kMessageValueChanged = "kMessageValueChanged";
IdStringPtr CControl::kMessageBeginEdit = "kMessageBeginEdit";
IdStringPtr CControl::kMessageEndEdit = "kMessageEndEdit";
#endif

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
, oldValue (1)
, defaultValue (0.5f)
, value (0)
, vmin (0)
, vmax (1.f)
, wheelInc (0.1f)
, editing (0)
{
	setTransparency (false);
	setMouseEnabled (true);
	setBackground (pBackground);
}

//------------------------------------------------------------------------
CControl::CControl (const CControl& c)
: CView (c)
, listener (c.listener)
, tag (c.tag)
, oldValue (c.oldValue)
, defaultValue (c.defaultValue)
, value (c.value)
, vmin (c.vmin)
, vmax (c.vmax)
, wheelInc (c.wheelInc)
, editing (0)
{
}

//------------------------------------------------------------------------
void CControl::registerControlListener (IControlListener* subListener)
{
	vstgui_assert (listener != subListener, "the subListener is already the main listener");
	subListeners.add (subListener);
}

//------------------------------------------------------------------------
void CControl::unregisterControlListener (IControlListener* subListener)
{
	subListeners.remove (subListener);
}

//------------------------------------------------------------------------
int32_t CControl::kZoomModifier = kShift;
int32_t CControl::kDefaultValueModifier = kControl;

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
void CControl::beginEdit ()
{
	// begin of edit parameter
	editing++;
	if (editing == 1)
	{
		if (listener)
			listener->controlBeginEdit (this);
		subListeners.forEach ([this] (IControlListener* l) { l->controlBeginEdit (this); });
#if VSTGUI_ENABLE_DEPRECATED_METHODS
		changed (kMessageBeginEdit);
#endif
		if (getFrame ())
			getFrame ()->beginEdit (tag);
	}
#if VSTGUI_CCONTROL_LOG_EDITING
	DebugPrint("beginEdit [%d] - %d\n", tag, editing);
#endif
}

//------------------------------------------------------------------------
void CControl::endEdit ()
{
	editing--;
	vstgui_assert(editing >= 0);
	if (editing == 0)
	{
		if (getFrame ())
			getFrame ()->endEdit (tag);
		if (listener)
			listener->controlEndEdit (this);
		subListeners.forEach ([this] (IControlListener* l) { l->controlEndEdit (this); });
#if VSTGUI_ENABLE_DEPRECATED_METHODS
		changed (kMessageEndEdit);
#endif
	}
#if VSTGUI_CCONTROL_LOG_EDITING
	DebugPrint("endEdit [%d] - %d\n", tag, editing);
#endif
}

//------------------------------------------------------------------------
void CControl::setValue (float val)
{
	if (val < getMin ())
		val = getMin ();
	else if (val > getMax ())
		val = getMax ();
	if (val != value)
	{
		value = val;
#if VSTGUI_ENABLE_DEPRECATED_METHODS
		changed (kMessageValueChanged);
#endif
	}
}

//------------------------------------------------------------------------
void CControl::setValueNormalized (float val)
{
	if (val > 1.f)
		val = 1.f;
	else if (val < 0.f)
		val = 0.f;
	setValue (getRange () * val + getMin ());
}

//------------------------------------------------------------------------
float CControl::getValueNormalized () const
{
	return (value - getMin ()) / getRange ();
}

//------------------------------------------------------------------------
void CControl::valueChanged ()
{
	if (listener)
		listener->valueChanged (this);
	subListeners.forEach ([this] (IControlListener* l) { l->valueChanged (this); });
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	changed (kMessageValueChanged);
#endif
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
void CControl::bounceValue ()
{
	if (value > getMax ())
		value = getMax ();
	else if (value < getMin ())
		value = getMin ();
}

//-----------------------------------------------------------------------------
bool CControl::checkDefaultValue (CButtonState button)
{
#if TARGET_OS_IPHONE
	if (button.isDoubleClick ())
#else
	if (button.isLeftButton () && button.getModifierState () == kDefaultValueModifier)
#endif
	{
		float defValue = getDefaultValue ();
		if (defValue != getValue ())
		{
			// begin of edit parameter
			beginEdit ();
		
			setValue (defValue);
			valueChanged ();

			// end of edit parameter
			endEdit ();
			
			setDirty ();
		}
		return true;
	}
	return false;
}

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

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
void IMultiBitmapControl::autoComputeHeightOfOneImage ()
{
	CView* view = dynamic_cast<CView*>(this);
	if (view)
	{
		CRect viewSize = view->getViewSize ();
		heightOfOneImage = viewSize.getHeight ();
	}
}

} // namespace
