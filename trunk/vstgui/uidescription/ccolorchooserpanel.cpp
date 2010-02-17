//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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

#include "ccolorchooserpanel.h"
#include "editingcolordefs.h"

namespace VSTGUI {

const char* CColorChooserPanel::kMsgWindowClosed = "kMsgWindowClosed";
CRect CColorChooserPanel::lastSize;

//-----------------------------------------------------------------------------
CColorChooserPanel::CColorChooserPanel (CBaseObject* owner, IPlatformColorChangeCallback* callback, void* parentPlatformWindow)
: platformWindow (0)
, owner (owner)
, callback (callback)
{
	CRect size (0, 0, 300, 500);
	platformWindow = PlatformWindow::create (size, "VSTGUI Color Chooser", PlatformWindow::kPanelType, PlatformWindow::kClosable|PlatformWindow::kResizable, this, parentPlatformWindow);
	if (platformWindow)
	{
		#if MAC_CARBON && MAC_COCOA
		CFrame::setCocoaMode (true);
		#endif

		frame = new CFrame (size, platformWindow->getPlatformHandle (), this);
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
		platformWindow->setSize (r);
		platformWindow->center ();
		if (!lastSize.isEmpty ())
			platformWindow->setSize (lastSize);
		platformWindow->show ();
	}
}

//-----------------------------------------------------------------------------
CColorChooserPanel::~CColorChooserPanel ()
{
	owner = 0;
	if (frame)
		frame->close ();
	if (platformWindow)
		windowClosed (platformWindow);
}

//-----------------------------------------------------------------------------
void CColorChooserPanel::setColorChangeCallback (IPlatformColorChangeCallback* newCallback)
{
	callback = newCallback;
}

//-----------------------------------------------------------------------------
void CColorChooserPanel::setColor (const CColor& newColor)
{
	colorChooser->setColor (newColor);
}

// IPlatformWindowDelegate
//-----------------------------------------------------------------------------
void CColorChooserPanel::windowSizeChanged (const CRect& newSize, PlatformWindow* platformWindow)
{
	frame->setSize (newSize.getWidth (), newSize.getHeight ());
}

//-----------------------------------------------------------------------------
void CColorChooserPanel::windowClosed (PlatformWindow* _platformWindow)
{
	if (_platformWindow == platformWindow)
	{
		lastSize = platformWindow->getSize ();
		platformWindow->forget ();
		platformWindow = 0;
	}
	if (owner)
		owner->notify (this, kMsgWindowClosed);
}

//-----------------------------------------------------------------------------
void CColorChooserPanel::checkWindowSizeConstraints (CPoint& size, PlatformWindow* platformWindow)
{
	if (size.x < minSize.x)
		size.x = minSize.x;
	if (size.y < minSize.y)
		size.y = minSize.y;
}

// IColorChooserDelegate
//-----------------------------------------------------------------------------
void CColorChooserPanel::colorChanged (CColorChooser* chooser, const CColor& color)
{
	if (callback)
		callback->colorChanged (color);
}


} // namespace

#endif // VSTGUI_LIVE_EDITING
