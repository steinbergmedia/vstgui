//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
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

#ifndef __editingcolordefs__
#define __editingcolordefs__

#if VSTGUI_LIVE_EDITING

#include "../lib/ccolor.h"

namespace VSTGUI {

#if MAC
static const CColor uidPanelBackgroundColor (kTransparentCColor);
static const CColor uidDialogBackgroundColor (kTransparentCColor);
#elif WINDOWS
static const CColor uidPanelBackgroundColor (0, 0, 0, 200);
static const CColor uidDialogBackgroundColor (kBlackCColor);
#endif

static const CColor uidFocusColor (100, 100, 255, 200);

static const CColor uidHilightColor (255, 255, 255, 150);
static const CColor uidSelectionColor (255, 0, 0, 255);
static const CColor uidSelectionHandleColor (255, 0, 0, 150);

static const CColor uidCrossLinesBackground (255, 255, 255, 100);
static const CColor uidCrossLinesForeground (0, 0, 0, 255);

static const CColor uidDataBrowserLineColor (255, 255, 255, 30);
static const CColor uidDataBrowserSelectionColor (255, 255, 255, 30);

static const CColor uidViewAttributeValueBackgroundColor (255, 255, 255, 150);
static const CColor uidViewAttributeDifferentValuesBackgroundColor (150, 150, 255, 150);
static const CColor uidViewAttributeValueFrameColor (0, 0, 0, 180);

static const CColor uidScrollerColor (255, 255, 255, 140);


} // namespace VSTGUI

#endif // VSTGUI_LIVE_EDITING

#endif // __editingcolordefs__