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


#ifndef __cgradientview__
#define __cgradientview__

#include "vstguifwd.h"
#include "cview.h"
#include "ccolor.h"
#include "cgradient.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
///	@brief View which draws a gradient
///	@ingroup new_in_4_2
//-----------------------------------------------------------------------------
class CGradientView : public CView
{
public:
	CGradientView (const CRect& size);
	~CGradientView ();

	//-----------------------------------------------------------------------------
	/// @name Gradient Style Methods
	//-----------------------------------------------------------------------------
	//@{
	enum GradientStyle {
		kLinearGradient,
		kRadialGradient
	};
	
	void setGradientStyle (GradientStyle style);
	void setGradient (CGradient* gradient);
	void setFrameColor (const CColor& newColor);
	void setGradientAngle (double angle);
	void setRoundRectRadius (CCoord radius);
	void setFrameWidth (CCoord width);
	void setDrawAntialiased (bool state);
	void setRadialCenter (const CPoint& center);
	void setRadialRadius (CCoord radius);

	GradientStyle getGradientStyle () const { return gradientStyle; }
	CGradient* getGradient () const { return gradient; }
	const CColor& getFrameColor () const { return frameColor; }
	double getGradientAngle () const { return gradientAngle; }
	CCoord getRoundRectRadius () const { return roundRectRadius; }
	CCoord getFrameWidth () const { return frameWidth; }
	bool getDrawAntialised () const { return drawAntialiased; }
	const CPoint& getRadialCenter () const { return radialCenter; }
	CCoord getRadialRadius () const { return radialRadius; }
	//@}

	// override
	virtual void setViewSize (const CRect& rect, bool invalid = true) VSTGUI_OVERRIDE_VMETHOD;
	void draw (CDrawContext* context) VSTGUI_OVERRIDE_VMETHOD;
protected:
	GradientStyle gradientStyle;
	CColor frameColor;
	double gradientAngle;
	CCoord roundRectRadius;
	CCoord frameWidth;
	CCoord radialRadius;
	CPoint radialCenter;
	bool drawAntialiased;

	SharedPointer<CGraphicsPath> path;
	SharedPointer<CGradient> gradient;
};
	
} // namespace

#endif // __cgradientview__
