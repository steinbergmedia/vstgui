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

#ifndef __cspecialdigit__
#define __cspecialdigit__

#include "ccontrol.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CSpecialDigit Declaration
//! @brief special display with custom numbers (0...9)
/// @ingroup views
//-----------------------------------------------------------------------------
class CSpecialDigit : public CControl
{
public:
	CSpecialDigit (const CRect& size, IControlListener* listener, int32_t tag, int32_t dwPos, int32_t iNumbers, int32_t* xpos, int32_t* ypos, int32_t width, int32_t height, CBitmap* background);
	CSpecialDigit (const CSpecialDigit& digit);
	
	virtual void  draw (CDrawContext*) VSTGUI_OVERRIDE_VMETHOD;

	CLASS_METHODS(CSpecialDigit, CControl)
protected:
	~CSpecialDigit ();
	int32_t     iNumbers;
	int32_t     xpos[7];
	int32_t     ypos[7];
	int32_t     width;
	int32_t     height;
};

} // namespace

#endif
