//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2016, Steinberg Media Technologies, All Rights Reserved
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
#pragma once

#include "../../crect.h"
#include "../iplatformframe.h"
#include <memory>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace X11 {

//------------------------------------------------------------------------
class Frame : public IPlatformFrame, public IPlatformFrameRunLoopExt
{
public:
	Frame (IPlatformFrameCallback* frame, const CRect& size, uint32_t parent);
	~Frame ();

	bool getGlobalPosition (CPoint& pos) const override;
	bool setSize (const CRect& newSize) override;
	bool getSize (CRect& size) const override;
	bool getCurrentMousePosition (CPoint& mousePosition) const override;
	bool getCurrentMouseButtons (CButtonState& buttons) const override;
	bool setMouseCursor (CCursorType type) override;
	bool invalidRect (const CRect& rect) override;
	bool scrollRect (const CRect& src, const CPoint& distance) override;
	bool showTooltip (const CRect& rect, const char* utf8Text) override;
	bool hideTooltip () override;
	void* getPlatformRepresentation () const override;
	IPlatformTextEdit* createPlatformTextEdit (IPlatformTextEditCallback* textEdit) override;
	IPlatformOptionMenu* createPlatformOptionMenu () override;
#if VSTGUI_OPENGL_SUPPORT
	IPlatformOpenGLView* createPlatformOpenGLView () override;
#endif
	IPlatformViewLayer* createPlatformViewLayer (IPlatformViewLayerDelegate* drawDelegate,
												 IPlatformViewLayer* parentLayer) override;
	COffscreenContext* createOffscreenContext (CCoord width, CCoord height,
											   double scaleFactor) override;
	DragResult doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap) override;
	void setClipboard (IDataPackage* data) override;
	IDataPackage* getClipboard () override;
	void handleNextEvents () override;

	void* getGtkWindow (); // return is Gtk::Window*
private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // X11
} // VSTGUI
