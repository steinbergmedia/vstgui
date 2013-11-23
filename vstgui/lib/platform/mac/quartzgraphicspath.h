//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
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

#ifndef __quartzgraphicspath__
#define __quartzgraphicspath__

#include "../../cgraphicspath.h"

#if MAC

#include "macglobals.h"
#if TARGET_OS_IPHONE
	#include <CoreGraphics/CoreGraphics.h>
	#include <ImageIO/ImageIO.h>
#else
	#include <ApplicationServices/ApplicationServices.h>
#endif

namespace VSTGUI {

//------------------------------------------------------------------------------------
class QuartzGraphicsPath : public CGraphicsPath
{
public:
	QuartzGraphicsPath ();
	~QuartzGraphicsPath ();

	CGPathRef getCGPathRef ();
	void dirty () VSTGUI_OVERRIDE_VMETHOD;

	bool hitTest (const CPoint& p, bool evenOddFilled = false, CGraphicsTransform* transform = 0) VSTGUI_OVERRIDE_VMETHOD;
	CPoint getCurrentPosition () VSTGUI_OVERRIDE_VMETHOD;
	CRect getBoundingBox () VSTGUI_OVERRIDE_VMETHOD;

	CGradient* createGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2) VSTGUI_OVERRIDE_VMETHOD;

	static CGAffineTransform createCGAfflineTransform (const CGraphicsTransform& t);

//------------------------------------------------------------------------------------
protected:
	CGMutablePathRef path;
};

//-----------------------------------------------------------------------------
class QuartzGradient : public CGradient
{
public:
	QuartzGradient (double _color1Start, double _color2Start, const CColor& _color1, const CColor& _color2)
	: CGradient (_color1Start, _color2Start, _color1, _color2)
	, gradient (0)
	{
		CGFloat color1Components[] = {color1.red/255.f, color1.green/255.f, color1.blue/255.f, color1.alpha/255.f};
		CGColorRef cgColor1 = CGColorCreate (GetCGColorSpace (), color1Components);
		CGFloat color2Components[] = {color2.red/255.f, color2.green/255.f, color2.blue/255.f, color2.alpha/255.f};
		CGColorRef cgColor2 = CGColorCreate (GetCGColorSpace (), color2Components);
		const void* colors[] = { cgColor1, cgColor2 };
		CFArrayRef colorArray = CFArrayCreate (0, colors, 2, &kCFTypeArrayCallBacks);

		if (color1Start < 0) color1Start = 0;
		else if (color1Start > 1) color1Start = 1;
		if (color2Start < 0) color2Start = 0;
		else if (color2Start > 1) color2Start = 1;
		CGFloat locations[] = { static_cast<CGFloat>(color1Start), static_cast<CGFloat>(color2Start) };
		
		gradient = CGGradientCreateWithColors (GetCGColorSpace (), colorArray, locations);

		CFRelease (cgColor1);
		CFRelease (cgColor2);
		CFRelease (colorArray);
	}
	
	~QuartzGradient ()
	{
		if (gradient)
			CFRelease (gradient);
	}

	operator CGGradientRef () const { return gradient; }

protected:
	CGGradientRef gradient;
};

} // namespace

#endif

#endif // __quartzgraphicspath__
