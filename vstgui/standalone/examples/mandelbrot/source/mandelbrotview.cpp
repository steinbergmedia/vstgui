
#include "mandelbrotview.h"
#include "vstgui/lib/cbitmap.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/cdrawcontext.h"

//------------------------------------------------------------------------
namespace Mandelbrot {

using namespace VSTGUI;

//------------------------------------------------------------------------
View::View (const ChangedFunc& func) : CView ({}), changed (func)
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
int32_t View::onKeyDown (VstKeyCode& keyCode)
{
	if (keyCode.virt == VKEY_ESCAPE)
	{
		onMouseCancel ();
		return 1;
	}
	return -1;
}

//------------------------------------------------------------------------
void View::draw (CDrawContext* context)
{
	if (auto bitmap = getBackground ())
	{
		auto width = bitmap->getWidth ();
		auto height = bitmap->getHeight ();
		CGraphicsTransform transform;
		transform.scale (getWidth () / width, getHeight () / height);
		transform.translate (getViewSize ().left, getViewSize ().top);
		CDrawContext::Transform t (*context, transform);
		bitmap->draw (context, CRect (0, 0, width, height));
	}
	if (box.isEmpty ())
		return;
	auto hairlineSize = context->getHairlineSize ();
	ConcatClip cc (*context, box);
	context->setLineWidth (hairlineSize);
	context->setDrawMode (kAliasing);
	context->setFrameColor (kBlackCColor);
	context->drawRect (box);
	CRect b2 (box);
	b2.inset (hairlineSize, hairlineSize);
	context->setFrameColor (kWhiteCColor);
	context->drawRect (b2);
}

//------------------------------------------------------------------------
} // Mandelbrot
