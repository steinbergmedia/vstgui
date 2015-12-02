//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins :
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

#ifndef __cxypad__
#define __cxypad__

#include "cparamdisplay.h"
#include <cmath>

namespace VSTGUI {

//------------------------------------------------------------------------
class CXYPad : public CParamDisplay
{
public:
	CXYPad (const CRect& size = CRect (0, 0, 0, 0));

	void setStopTrackingOnMouseExit (bool state) { stopTrackingOnMouseExit = state; }
	bool getStopTrackingOnMouseExit () const { return stopTrackingOnMouseExit; }

	void draw (CDrawContext* context) VSTGUI_OVERRIDE_VMETHOD;
	
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;

	static float calculateValue (float x, float y)
	{
		x = std::floor (x * 1000.f + 0.5f) * 0.001f;
		y = std::floor (y * 1000.f + 0.5f) * 0.0000001f;
		return x + y;
	}
	
	static void calculateXY (float value, float& x, float& y)
	{
		x = std::floor (value * 1000.f + 0.5f) * 0.001f;
		y = std::floor ((value - x)  * 10000000.f + 0.5f) * 0.001f;
	}
	
protected:
	virtual void setMin (float val) VSTGUI_OVERRIDE_VMETHOD { }
	virtual void setMax (float val) VSTGUI_OVERRIDE_VMETHOD { }

	void boundValues (float& x, float& y);
	
	CPoint mouseChangeStartPoint;
	CPoint lastMouseChangePoint;
	bool stopTrackingOnMouseExit;
};


} // namespace

#endif // __cxypad__