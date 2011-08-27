//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins :
//
// Version 4.1
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

#ifndef __iactionoperation__
#define __iactionoperation__

#include "../../lib/cview.h"

#if VSTGUI_LIVE_EDITING
#include "../../lib/ccolor.h"
#include "../../lib/cfont.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class IActionOperation
{
public:
	virtual ~IActionOperation () {}
	
	virtual UTF8StringPtr getName () = 0;
	virtual void perform () = 0;
	virtual void undo () = 0;
};

//----------------------------------------------------------------------------------------------------
class IActionOperator
{
public:
	virtual ~IActionOperator () {}
	virtual void performAction (IActionOperation* action) = 0;

	virtual void performColorChange (UTF8StringPtr colorName, const CColor& newColor, bool remove = false) = 0;
	virtual void performTagChange (UTF8StringPtr tagName, int32_t tag, bool remove = false) = 0;
	virtual void performBitmapChange (UTF8StringPtr bitmapName, UTF8StringPtr bitmapPath, bool remove = false) = 0;
	virtual void performFontChange (UTF8StringPtr fontName, CFontRef newFont, bool remove = false) = 0;

	virtual void performColorNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) = 0;
	virtual void performTagNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) = 0;
	virtual void performFontNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) = 0;
	virtual void performBitmapNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) = 0;

	virtual void performBitmapNinePartTiledChange (UTF8StringPtr bitmapName, const CRect* offsets) = 0;
	virtual void makeSelection (CView* view) = 0;
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __iactionoperation__