//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2009, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __cocoatextedit__
#define __cocoatextedit__

#include "../../iplatformtextedit.h"

#if MAC_COCOA

#ifdef __OBJC__
@class NSView;
@class NSTextField;
#else
struct NSView;
struct NSTextField;
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
class CocoaTextEdit : public IPlatformTextEdit
{
public:
	CocoaTextEdit (NSView* parent, IPlatformTextEditCallback* textEdit);
	~CocoaTextEdit ();
	
	bool getText (char* text, long maxSize);
	bool setText (const char* text);
	bool updateSize ();

	NSTextField* getPlatformControl () const { return platformControl; }
	NSView* getParent () const { return parent; }
	IPlatformTextEditCallback* getTextEdit () const { return textEdit; }

//-----------------------------------------------------------------------------
protected:
	static void initClass ();

	NSTextField* platformControl;
	NSView* parent;
};

} // namespace

#endif // MAC_COCOA

#endif // __cocoatextedit__
