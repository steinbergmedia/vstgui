//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2011, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "uifontchooserpanel.h"

#if VSTGUI_LIVE_EDITING

#include "editingcolordefs.h"

namespace VSTGUI {

UIFontChooserPanel* UIFontChooserPanel::gInstance = 0;
CRect UIFontChooserPanel::lastSize;

//-----------------------------------------------------------------------------
void UIFontChooserPanel::show (CFontRef initialFont, CBaseObject* owner, void* parentPlatformWindow)
{
	if (gInstance == 0)
		new UIFontChooserPanel (initialFont, owner, parentPlatformWindow);
	else
	{
		gInstance->setOwner (owner);
		gInstance->setFont (initialFont);
	}
}

//-----------------------------------------------------------------------------
void UIFontChooserPanel::hide ()
{
	if (gInstance)
	{
		gInstance->setOwner (0);
		gInstance->forget ();
	}
}

//-----------------------------------------------------------------------------
UIFontChooserPanel::UIFontChooserPanel (CFontRef initialFont, CBaseObject* owner, void* parentPlatformWindow)
: UIPanelBase (owner, parentPlatformWindow)
, fontChooser (0)
{
	gInstance = this;
	CRect size (0, 0, 500, 300);
	if (init (size, "VSTGUI Font Chooser", PlatformWindow::kClosable|PlatformWindow::kResizable))
	{
		if (initialFont)
			fontChooser->setFont (initialFont);
		platformWindow->center ();
		if (lastSize.getWidth () > 0 && lastSize.getHeight () > 0)
			platformWindow->setSize (lastSize);
		platformWindow->show ();
	}
}

//-----------------------------------------------------------------------------
UIFontChooserPanel::~UIFontChooserPanel ()
{
	gInstance = 0;
}

//-----------------------------------------------------------------------------
CFrame* UIFontChooserPanel::createFrame (void* platformWindow, const CCoord& width, const CCoord& height)
{
	CRect size (0, 0, width, height);
	CFrame* frame = new CFrame (size, platformWindow, this);
	frame->setBackgroundColor (uidPanelBackgroundColor);

	frame->setFocusDrawingEnabled (true);
	frame->setFocusColor (uidFocusColor);
	frame->setFocusWidth (1.2);

	const CCoord kMargin = 12;
	size.left += kMargin;
	size.top += kMargin;
	size.right -= kMargin;
	size.bottom -= kMargin;

	CFontChooserUIDefinition uiDef;
	uiDef.rowHeight = 20;
	uiDef.rowlineColor = uidDataBrowserLineColor;
	uiDef.selectionColor = uidFocusColor;
	uiDef.scrollbarWidth = 10;
	uiDef.scrollbarScrollerColor = uidScrollerColor;
	uiDef.scrollbarBackgroundColor = kTransparentCColor;
	uiDef.scrollbarFrameColor = kTransparentCColor;
	uiDef.font = kNormalFont;

	fontChooser = new CFontChooser (this, 0, uiDef);
	fontChooser->setTransparency (true);
	fontChooser->setAutosizeFlags (kAutosizeAll);
	fontChooser->setViewSize (size);
	fontChooser->setMouseableArea (size);
	frame->addView (fontChooser);
	minSize = frame->getViewSize ().getBottomRight ();
	return frame;
}

//-----------------------------------------------------------------------------
void UIFontChooserPanel::windowClosed (PlatformWindow* _platformWindow)
{
	if (_platformWindow)
		lastSize = _platformWindow->getSize ();
	UIPanelBase::windowClosed (_platformWindow);
}

//-----------------------------------------------------------------------------
void UIFontChooserPanel::checkWindowSizeConstraints (CPoint& size, PlatformWindow* platformWindow)
{
	if (size.x < minSize.x)
		size.x = minSize.x;
	if (size.y < minSize.y)
		size.y = minSize.y;
}

//-----------------------------------------------------------------------------
void UIFontChooserPanel::fontChanged (CFontChooser* chooser, CFontRef newFont)
{
	IFontChooserDelegate* delegate = owner ? dynamic_cast<IFontChooserDelegate*> (owner) : 0;
	if (delegate)
		delegate->fontChanged (chooser, newFont);
}

//-----------------------------------------------------------------------------
void UIFontChooserPanel::setOwner (CBaseObject* _owner)
{
	owner = _owner;
}

//-----------------------------------------------------------------------------
void UIFontChooserPanel::setFont (CFontRef font)
{
	fontChooser->setFont (font);
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
