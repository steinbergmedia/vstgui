//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "uisearchtextfield.h"

#if VSTGUI_LIVE_EDITING

#include "../../lib/cframe.h"
#include "../../lib/cgraphicspath.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
UISearchTextField::UISearchTextField (const CRect& size, IControlListener* listener, int32_t tag, UTF8StringPtr txt, CBitmap* background, const int32_t style)
: CTextEdit (size, listener, tag, 0, background, style)
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
void UISearchTextField::drawClearMark (CDrawContext* context) const
{
	if (getText ().empty ())
		return;

	SharedPointer<CGraphicsPath> path = owned (context->createGraphicsPath ());
	if (path == 0)
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
	context->setDrawMode (kAntiAliasing|kNonIntegralMode);
	context->drawGraphicsPath (path, CDrawContext::kPathStroked);
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

	pContext->setDrawMode (kAntiAliasing);

	CColor origFontColor (fontColor);
	if (getText ().empty ())
	{
		CColor color (fontColor);
		color.alpha /= 2;
		setFontColor (color);
		drawPlatformText(pContext, CString ("Search").getPlatformString ());
	}
	else
		drawPlatformText (pContext, getText ().getPlatformString ());

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
		rect = static_cast<CViewContainer*>(pParentView)->getVisibleSize (rect);
	else if (pParentFrame)
		rect = pParentFrame->getVisibleSize (rect);

	CPoint p (0, 0);
	localToFrame (p);
	rect.offset (p.x, p.y);
	return rect;
}


} // namespace

#endif // VSTGUI_LIVE_EDITING
