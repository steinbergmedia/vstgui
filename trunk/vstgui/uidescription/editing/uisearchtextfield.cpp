#include "uisearchtextfield.h"

#if VSTGUI_LIVE_EDITING

#include "../../lib/cframe.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
UISearchTextField::UISearchTextField (const CRect& size, CControlListener* listener, int32_t tag, UTF8StringPtr txt, CBitmap* background, const int32_t style)
: CTextEdit (size, listener, tag, txt, background, style)
{
}

//----------------------------------------------------------------------------------------------------
CRect UISearchTextField::getClearMarkRect () const
{
	CRect r (getViewSize ());
	r.left = r.right - getHeight ();
	r.inset (2, 2);
	return r;
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UISearchTextField::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton ())
	{
		if (getText () != 0 && strlen (getText ()) != 0)
		{
			if (getClearMarkRect ().pointInside (where))
			{
				beginEdit ();
				setText ("");
				valueChanged ();
				endEdit ();
				return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
			}
		}
	}
	return CTextEdit::onMouseDown (where, buttons);
}

//----------------------------------------------------------------------------------------------------
void UISearchTextField::drawClearMark (CDrawContext* context) const
{
	if (getText () == 0 || strlen (getText ()) == 0)
		return;

	CRect r = getClearMarkRect ();
	CColor color (fontColor);
	color.alpha /= 2;
	context->setFillColor (color);
	context->setDrawMode (kAntiAliasing);
	context->drawEllipse (r, kDrawFilled);
	double h,s,v;
	color.toHSV (h, s, v);
	v = 1. - v;
	color.fromHSV (h, s, v);
	context->setFrameColor (color);
	context->setLineWidth (2.);
	r.inset (r.getWidth () / (M_PI * 2.) + 1, r.getHeight () / (M_PI * 2.) + 1);
	context->moveTo (r.getTopLeft ());
	context->lineTo (r.getBottomRight ());
	context->moveTo (r.getBottomLeft ());
	context->lineTo (r.getTopRight ());
}

//----------------------------------------------------------------------------------------------------
void UISearchTextField::draw (CDrawContext *pContext)
{
	drawBack (pContext);
	drawClearMark (pContext);

	if (platformControl)
	{
		setDirty (false);
		return;
	}

	const char* string = getText ();
	CColor origFontColor (fontColor);
	if (string == 0 || strlen (string) == 0)
	{
		CColor color (fontColor);
		color.alpha /= 2;
		setFontColor (color);
		string = "Search";
	}

	drawText (pContext, string);
	setDirty (false);

	setFontColor (origFontColor);
}

//------------------------------------------------------------------------
CRect UISearchTextField::platformGetSize () const
{
	CRect rect = getViewSize ();
	CRect cmr = getClearMarkRect ();
	rect.right = cmr.left;
	CPoint p (0, 0);
	localToFrame (p);
	rect.offset (p.x, p.y);
	return rect;
}

//------------------------------------------------------------------------
CRect UISearchTextField::platformGetVisibleSize () const
{
	CRect rect = getViewSize ();
	CRect cmr = getClearMarkRect ();
	rect.right = cmr.left;
	if (pParentView)
		rect = reinterpret_cast<CViewContainer*>(pParentView)->getVisibleSize (rect);
	else if (pParentFrame)
		rect = pParentFrame->getVisibleSize (rect);

	CPoint p (0, 0);
	localToFrame (p);
	rect.offset (p.x, p.y);
	return rect;
}


} // namespace

#endif // VSTGUI_LIVE_EDITING
