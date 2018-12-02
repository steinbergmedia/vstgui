// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "csearchtextedit.h"

#include "../cframe.h"
#include "../cgraphicspath.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
CSearchTextEdit::CSearchTextEdit (const CRect& size, IControlListener* listener, int32_t tag,
                                  UTF8StringPtr txt, CBitmap* background, const int32_t style)
: CTextEdit (size, listener, tag, nullptr, background, style)
{
	setPlaceholderString ("Search");
}

//------------------------------------------------------------------------
void CSearchTextEdit::setClearMarkInset (CPoint inset)
{
	if (inset != clearMarkInset)
	{
		clearMarkInset = inset;
		invalid ();
	}
}

//------------------------------------------------------------------------
CPoint CSearchTextEdit::getClearMarkInset () const
{
	return clearMarkInset;
}

//----------------------------------------------------------------------------------------------------
CRect CSearchTextEdit::getClearMarkRect () const
{
	CRect r (getViewSize ());
	if (getHoriAlign () == kRightText)
		r.right = r.left + getHeight ();
	else
		r.left = r.right - getHeight ();
	r.inset (getClearMarkInset ());
	return r;
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult CSearchTextEdit::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton ())
	{
		if (!getText ().empty ())
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
void CSearchTextEdit::drawClearMark (CDrawContext* context) const
{
	if (!((platformControl && !platformControl->getText ().empty ()) || !getText ().empty ()))
		return;

	auto path = owned (context->createGraphicsPath ());
	if (path == nullptr)
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
	path->beginSubpath (r.getTopLeft ());
	path->addLine (r.getBottomRight ());
	path->beginSubpath (r.getBottomLeft ());
	path->addLine (r.getTopRight ());
	context->setDrawMode (kAntiAliasing);
	context->drawGraphicsPath (path, CDrawContext::kPathStroked);
}

//----------------------------------------------------------------------------------------------------
void CSearchTextEdit::draw (CDrawContext *pContext)
{
	drawBack (pContext);
	drawClearMark (pContext);

	if (platformControl)
	{
		setDirty (false);
		return;
	}

	pContext->setDrawMode (kAntiAliasing);

	CColor origFontColor (fontColor);
	if (getText ().empty ())
	{
		CColor color (fontColor);
		color.alpha /= 2;
		setFontColor (color);
		drawPlatformText (pContext, getPlaceholderString ().getPlatformString (), getTextRect ());
	}
	else
		drawPlatformText (pContext, getText ().getPlatformString (), getTextRect ());

	setDirty (false);
	setFontColor (origFontColor);
}

//------------------------------------------------------------------------
CRect CSearchTextEdit::getTextRect () const
{
	CRect rect = getViewSize ();
	CRect cmr = getClearMarkRect ();
	if (getHoriAlign () == kRightText)
		rect.left = cmr.right;
	else
		rect.right = cmr.left;
	return rect;
}

//------------------------------------------------------------------------
CRect CSearchTextEdit::platformGetSize () const
{
	return translateToGlobal (getTextRect ());
}

//------------------------------------------------------------------------
CRect CSearchTextEdit::platformGetVisibleSize () const
{
	CRect rect = getTextRect ();
	if (getParentView ())
		rect = getParentView ()->asViewContainer ()->getVisibleSize (rect);
	else if (getFrame ())
		rect = getFrame ()->getVisibleSize (rect);

	return translateToGlobal (rect);
}

//------------------------------------------------------------------------
void CSearchTextEdit::platformTextDidChange ()
{
	invalidRect (getClearMarkRect ());
	CTextEdit::platformTextDidChange ();
}

//------------------------------------------------------------------------
} // VSTGUI
