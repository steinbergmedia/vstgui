//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins :
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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

#include "cview.h"
#include "cgraphicspath.h"

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
	void setFrameColor (const CColor& newColor);
	void setGradientStartColor (const CColor& newColor);
	void setGradientEndColor (const CColor& newColor);
	void setGradientAngle (double angle);
	void setGradientStartColorOffset (double offset);
	void setGradientEndColorOffset (double offset);
	void setRoundRectRadius (CCoord radius);
	void setFrameWidth (CCoord width);
	void setDrawAntialiased (bool state);
	
	const CColor& getFrameColor () const { return frameColor; }
	const CColor& getGradientStartColor () const { return gradientStartColor; }
	const CColor& getGradientEndColor () const { return gradientEndColor; }
	double getGradientAngle () const { return gradientAngle; }
	double getGradientStartColorOffset () const { return gradientStartColorOffset; }
	double getGradientEndColorOffset () const { return gradientEndColorOffset; }
	CCoord getRoundRectRadius () const { return roundRectRadius; }
	CCoord getFrameWidth () const { return frameWidth; }
	bool getDrawAntialised () const { return drawAntialiased; }
	//@}

	// override
	virtual void setViewSize (const CRect& rect, bool invalid = true) VSTGUI_OVERRIDE_VMETHOD;
	void draw (CDrawContext* context) VSTGUI_OVERRIDE_VMETHOD;
protected:
	CColor frameColor;
	CColor gradientStartColor;
	CColor gradientEndColor;
	double gradientAngle;
	double gradientStartColorOffset;
	double gradientEndColorOffset;
	CCoord roundRectRadius;
	CCoord frameWidth;
	bool drawAntialiased;

	OwningPointer<CGraphicsPath> path;
	OwningPointer<CGradient> gradient;
};
	
} // namespace

#endif // __cgradientview__
