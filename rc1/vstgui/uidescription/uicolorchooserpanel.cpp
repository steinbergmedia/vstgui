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

#if VSTGUI_LIVE_EDITING

#include "uicolorchooserpanel.h"
#include "editingcolordefs.h"

namespace VSTGUI {

CRect UIColorChooserPanel::lastSize;

//-----------------------------------------------------------------------------
UIColorChooserPanel::UIColorChooserPanel (CBaseObject* owner, IPlatformColorChangeCallback* callback, void* parentPlatformWindow)
: UIPanelBase (owner, parentPlatformWindow)
, callback (callback)
{
	CRect size (0, 0, 300, 500);
	if (init (size, "VSTGUI Color Chooser", PlatformWindow::kClosable|PlatformWindow::kResizable))
	{
		platformWindow->center ();
		if (!lastSize.isEmpty ())
			platformWindow->setSize (lastSize);
		platformWindow->show ();
	}
}

//-----------------------------------------------------------------------------
CFrame* UIColorChooserPanel::createFrame (void* platformWindow, const CCoord& width, const CCoord& height)
{
	CRect size (0, 0, width, height);
	CFrame* frame = new CFrame (size, platformWindow, this);
	frame->setBackgroundColor (uidPanelBackgroundColor);

	frame->setFocusDrawingEnabled (true);
	frame->setFocusColor (uidFocusColor);
	frame->setFocusWidth (1.2);

	const CCoord kMargin = 12;
	colorChooser = new CColorChooser (this);
	CRect r = colorChooser->getViewSize ();
	r.offset (kMargin, kMargin);
	colorChooser->setViewSize (r);
	colorChooser->setMouseableArea (r);
	r.inset (-kMargin, -kMargin);
	minSize.x = r.getWidth ();
	minSize.y = r.getHeight ();
	frame->setSize (r.getWidth (), r.getHeight ());
	frame->addView (colorChooser);
	return frame;
}

//-----------------------------------------------------------------------------
UIColorChooserPanel::~UIColorChooserPanel ()
{
}

//-----------------------------------------------------------------------------
void UIColorChooserPanel::setColorChangeCallback (IPlatformColorChangeCallback* newCallback)
{
	callback = newCallback;
}

//-----------------------------------------------------------------------------
void UIColorChooserPanel::setColor (const CColor& newColor)
{
	colorChooser->setColor (newColor);
}

//-----------------------------------------------------------------------------
void UIColorChooserPanel::windowClosed (PlatformWindow* _platformWindow)
{
	if (_platformWindow == platformWindow)
	{
		lastSize = platformWindow->getSize ();
	}
	UIPanelBase::windowClosed (_platformWindow);
}

//-----------------------------------------------------------------------------
void UIColorChooserPanel::checkWindowSizeConstraints (CPoint& size, PlatformWindow* platformWindow)
{
	if (size.x < minSize.x)
		size.x = minSize.x;
	if (size.y < minSize.y)
		size.y = minSize.y;
}

// IColorChooserDelegate
//-----------------------------------------------------------------------------
void UIColorChooserPanel::colorChanged (CColorChooser* chooser, const CColor& color)
{
	if (callback)
		callback->colorChanged (color);
}


} // namespace

#endif // VSTGUI_LIVE_EDITING
