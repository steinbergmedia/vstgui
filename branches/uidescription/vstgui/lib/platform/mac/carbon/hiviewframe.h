//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
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

#ifndef __hiviewframe__
#define __hiviewframe__

#include "../../../cframe.h"

#if MAC_CARBON

#include <Carbon/Carbon.h>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class HIViewFrame : public IPlatformFrame
{
public:
	static void setAddToContentView (bool addToContentView); // defaults to true

	HIViewFrame (IPlatformFrameCallback* frame, const CRect& size, WindowRef parent);
	~HIViewFrame ();

	HIViewRef getPlatformControl () const { return controlRef; }
	const CPoint& getScrollOffset () const { return hiScrollOffset; }

	// IPlatformFrame
	bool getGlobalPosition (CPoint& pos) const;
	bool setSize (const CRect& newSize);
	bool getSize (CRect& size) const;
	bool getCurrentMousePosition (CPoint& mousePosition) const;
	bool getCurrentMouseButtons (long& buttons) const;
	bool setMouseCursor (CCursorType type);
	bool invalidRect (const CRect& rect);
	bool scrollRect (const CRect& src, const CPoint& distance);
	unsigned long getTicks () const;
	bool showTooltip (const CRect& rect, const char* utf8Text);
	bool hideTooltip ();
	void* getPlatformRepresentation () const { return controlRef; }
	IPlatformTextEdit* createPlatformTextEdit (IPlatformTextEditCallback* textEdit);
	IPlatformOptionMenu* createPlatformOptionMenu ();
	COffscreenContext* createOffscreenContext (CCoord width, CCoord height);
	CGraphicsPath* createGraphicsPath ();

//-----------------------------------------------------------------------------
protected:
	static pascal OSStatus carbonMouseEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
	static pascal OSStatus carbonEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
	
	WindowRef window;
	HIViewRef controlRef;
	bool hasFocus;
	bool isInMouseTracking;
	EventHandlerRef mouseEventHandler;
	EventHandlerRef keyboardEventHandler;
	CPoint hiScrollOffset;
};

} // namespace

#endif // MAC_CARBON
#endif // __hiviewframe__
