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

#ifndef __iplatformtextedit__
#define __iplatformtextedit__

/// @cond ignore

#include "../cfont.h"
#include "../ccolor.h"
#include "../crect.h"
#include "../cdrawcontext.h"

struct VstKeyCode;

namespace VSTGUI {

//-----------------------------------------------------------------------------
class IPlatformTextEditCallback
{
public:
	virtual CColor platformGetBackColor () const = 0;
	virtual CColor platformGetFontColor () const = 0;
	virtual CFontRef platformGetFont () const = 0;
	virtual CHoriTxtAlign platformGetHoriTxtAlign () const = 0; 
	virtual UTF8StringPtr platformGetText () const = 0;
	virtual CRect platformGetSize () const = 0;
	virtual CRect platformGetVisibleSize () const = 0;
	virtual CPoint platformGetTextInset () const = 0;
	virtual void platformLooseFocus (bool returnPressed) = 0;
	virtual bool platformOnKeyDown (const VstKeyCode& key) = 0;
	virtual void platformTextDidChange () = 0;

//------------------------------------------------------------------------------------
};

//-----------------------------------------------------------------------------
class IPlatformTextEdit : public CBaseObject
{
public:
	virtual UTF8StringPtr getText () = 0;
	virtual bool setText (UTF8StringPtr text) = 0;
	virtual bool updateSize () = 0;

//-----------------------------------------------------------------------------
protected:
	IPlatformTextEdit (IPlatformTextEditCallback* textEdit) : textEdit (textEdit) {}
	IPlatformTextEditCallback* textEdit;
};

} // namespace

/// @endcond

#endif // __iplatformtextedit__
