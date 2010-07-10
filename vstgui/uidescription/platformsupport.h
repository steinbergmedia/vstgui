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

#ifndef __platformsupport__
#define __platformsupport__

#include "../lib/crect.h"
#include "uidescription.h"
#include <string>
#include <list>

/// @cond ignore

namespace VSTGUI {
class CBitmap;
class CFrame;
class PlatformWindow;

//-----------------------------------------------------------------------------
class IPlatformWindowDelegate
{
public:
	virtual void windowSizeChanged (const CRect& newSize, PlatformWindow* platformWindow) = 0;
	virtual void windowClosed (PlatformWindow* platformWindow) = 0;
	virtual void checkWindowSizeConstraints (CPoint& size, PlatformWindow* platformWindow) = 0;
};

//-----------------------------------------------------------------------------
class PlatformWindow : public CBaseObject
{
public:
	enum WindowType {
		kPanelType,
		kWindowType
	};
	enum WindowStyleFlags {
		kClosable = 1 << 0,
		kResizable  = 1 << 1
	};
	
	static PlatformWindow* create (const CRect& size, const char* title = 0, WindowType type = kPanelType, int32_t styleFlags = 0, IPlatformWindowDelegate* delegate = 0, void* parentWindow = 0);
	
	virtual void* getPlatformHandle () const = 0;
	virtual void show () = 0;
	virtual void center () = 0;
	virtual CRect getSize () = 0;
	virtual void setSize (const CRect& size) = 0;
	
	virtual void runModal () = 0;
	virtual void stopModal () = 0;
};

//-----------------------------------------------------------------------------
class IPlatformColorChangeCallback
{
public:
	virtual void colorChanged (const CColor& color) = 0;
};

//-----------------------------------------------------------------------------
class PlatformUtilities
{
public:
	static void colorChooser (const CColor* oldColor, IPlatformColorChangeCallback* callback);
	static void gatherResourceBitmaps (std::list<std::string>& filenames);
};

} // namespace

/// @endcond ignore

#endif
