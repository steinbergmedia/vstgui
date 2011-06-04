//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
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

#ifndef __win32frame__
#define __win32frame__

#include "../../cframe.h"

#if WINDOWS

#include <windows.h>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class Win32Frame : public IPlatformFrame
{
public:
	Win32Frame (IPlatformFrameCallback* frame, const CRect& size, HWND parent);
	~Win32Frame ();

	HWND getPlatformWindow () const { return windowHandle; }
	HWND getParentPlatformWindow () const { return parentWindow; }
	HWND getOuterWindow () const;
	IPlatformFrameCallback* getFrame () const { return frame; }
	
	// IPlatformFrame
	bool getGlobalPosition (CPoint& pos) const;
	bool setSize (const CRect& newSize);
	bool getSize (CRect& size) const;
	bool getCurrentMousePosition (CPoint& mousePosition) const;
	bool getCurrentMouseButtons (CButtonState& buttons) const;
	bool setMouseCursor (CCursorType type);
	bool invalidRect (const CRect& rect);
	bool scrollRect (const CRect& src, const CPoint& distance);
	bool showTooltip (const CRect& rect, const char* utf8Text);
	bool hideTooltip ();
	void* getPlatformRepresentation () const { return windowHandle; }
	IPlatformTextEdit* createPlatformTextEdit (IPlatformTextEditCallback* textEdit);
	IPlatformOptionMenu* createPlatformOptionMenu ();
	COffscreenContext* createOffscreenContext (CCoord width, CCoord height);
	CView::DragResult doDrag (CDropSource* source, const CPoint& offset, CBitmap* dragBitmap);

//-----------------------------------------------------------------------------
protected:
	void initTooltip ();
	void paint (HWND hwnd);

	static void initWindowClass ();
	static void destroyWindowClass ();
	static LONG_PTR WINAPI WindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static int32_t gUseCount;

	HWND parentWindow;
	HWND windowHandle;
	HWND tooltipWindow;

	COffscreenContext* backBuffer;
	CDrawContext* deviceContext;

	bool mouseInside;

	RGNDATA* updateRegionList;
	DWORD updateRegionListSize;
};

} // namespace

#endif // WINDOWS

#endif // __win32frame__
