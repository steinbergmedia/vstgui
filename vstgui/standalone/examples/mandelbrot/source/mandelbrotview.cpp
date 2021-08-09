// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "mandelbrotview.h"
#include "vstgui/lib/cbitmap.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/events.h"

//------------------------------------------------------------------------
namespace Mandelbrot {

using namespace VSTGUI;

//------------------------------------------------------------------------
View::View (ChangedFunc&& func) : CView ({}), changed (std::move (func))
{
}

//------------------------------------------------------------------------
CMouseEventResult View::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton ())
	{
		setWantsFocus (true);
		getFrame ()->setFocusView (this);
		box.setTopLeft (where);
		box.setBottomRight (where);
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
CMouseEventResult View::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	if (wantsFocus () && buttons.isLeftButton () && !box.isEmpty ())
	{
		if (changed)
		{
			CRect b {box};
			b.offsetInverse (getViewSize ().getTopLeft ());
			changed (b);
		}
		onMouseCancel ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult View::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (!wantsFocus () || !buttons.isLeftButton ())
		return kMouseEventNotHandled;
	CRect r (box);
	box.setBottomRight (where);
	r.unite (box);
	invalidRect (r);
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult View::onMouseCancel ()
{
	invalidRect (box);
	box = {};
	setWantsFocus (false);
	getFrame ()->setFocusView (nullptr);
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
void View::onKeyboardEvent (KeyboardEvent& event)
{
	if (event.type == EventType::KeyDown && event.virt == VirtualKey::Escape)
	{
		onMouseCancel ();
		event.consumed = true;
		return;
	}
}

//------------------------------------------------------------------------
void View::draw (CDrawContext* context)
{
	if (auto bitmap = getBackground ())
	{
		auto bitmapSize = bitmap->getSize ();
		CGraphicsTransform transform;
		transform.scale (getWidth () / bitmapSize.x, getHeight () / bitmapSize.y);
		transform.translate (getViewSize ().left, getViewSize ().top);
		CDrawContext::Transform t (*context, transform);
		bitmap->draw (context, CRect (0, 0, bitmapSize.x, bitmapSize.y));
	}
	if (box.isEmpty ())
		return;
	auto hairlineSize = context->getHairlineSize ();
	ConcatClip cc (*context, box);
	context->setLineWidth (hairlineSize);
	context->setDrawMode (kAliasing | kNonIntegralMode);
	context->setFrameColor (kBlackCColor);
	context->drawRect (box);
	CRect b2 (box);
	b2.inset (hairlineSize, hairlineSize);
	context->setFrameColor (kWhiteCColor);
	context->drawRect (b2);
}

//------------------------------------------------------------------------
} // Mandelbrot
