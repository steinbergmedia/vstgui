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

#ifndef __editingcolordefs__
#define __editingcolordefs__

#if VSTGUI_LIVE_EDITING

#include "../lib/ccolor.h"

namespace VSTGUI {

#if MAC
static CColor uidPanelBackgroundColor (kTransparentCColor);
static CColor uidDialogBackgroundColor (kTransparentCColor);
#elif WINDOWS
static CColor uidPanelBackgroundColor (kBlackCColor);
static CColor uidDialogBackgroundColor (kWhiteCColor);
#endif

static CColor uidFocusColor (100, 100, 255, 200);

static CColor uidHilightColor (255, 255, 255, 150);
static CColor uidSelectionColor (255, 0, 0, 255);
static CColor uidSelectionHandleColor (255, 0, 0, 150);

static CColor uidCrossLinesBackground (255, 255, 255, 200);
static CColor uidCrossLinesForeground (0, 0, 0, 255);

static CColor uidDataBrowserLineColor (255, 255, 255, 30);
static CColor uidDataBrowserSelectionColor (255, 255, 255, 30);

static CColor uidViewAttributeValueBackgroundColor (255, 255, 255, 150);
static CColor uidViewAttributeDifferentValuesBackgroundColor (150, 150, 255, 150);
static CColor uidViewAttributeValueFrameColor (0, 0, 0, 180);

} // namespace VSTGUI

#endif // VSTGUI_LIVE_EDITING

#endif // __editingcolordefs__