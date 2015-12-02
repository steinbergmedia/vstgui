//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
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

#ifndef __macstring__
#define __macstring__

#include "../iplatformstring.h"

#if MAC
#include <CoreFoundation/CoreFoundation.h>
#include "cfontmac.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
class MacString : public IPlatformString
{
public:
	MacString (UTF8StringPtr utf8String);
	~MacString ();
	
	void setUTF8String (UTF8StringPtr utf8String) VSTGUI_OVERRIDE_VMETHOD;

	CFStringRef getCFString () const { return cfString; }

	CTLineRef getCTLine () const { return ctLine; }
	const void* getCTLineFontRef () const { return ctLineFontRef; }
	const CColor& getCTLineColor () const { return ctLineColor; }

	void setCTLine (CTLineRef line, const void* fontRef, const CColor& color);
//-----------------------------------------------------------------------------
protected:
	CFStringRef cfString;
	CTLineRef ctLine;
	const void* ctLineFontRef;
	CColor ctLineColor;
};

} // namespace

#endif // MAC

#endif // __macstring__
