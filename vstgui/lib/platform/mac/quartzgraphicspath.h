//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
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

#ifndef __quartzgraphicspath__
#define __quartzgraphicspath__

#include "../../cgraphicspath.h"

#if MAC

#include <ApplicationServices/ApplicationServices.h>

namespace VSTGUI {

//------------------------------------------------------------------------------------
class QuartzGraphicsPath : public CGraphicsPath
{
public:
	QuartzGraphicsPath ();
	~QuartzGraphicsPath ();

	CGPathRef getCGPathRef () const { return path; }

	// CGraphicsPath
	CGradient* createGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2);
	void addArc (const CRect& rect, double startAngle, double endAngle, bool clockwise);
	void addCurve (const CPoint& start, const CPoint& control1, const CPoint& control2, const CPoint& end);
	void addEllipse (const CRect& rect);
	void addLine (const CPoint& start, const CPoint& end);
	void addRect (const CRect& rect);
	void addPath (const CGraphicsPath& path, CGraphicsTransform* transformation = 0);
	void closeSubpath ();
	CPoint getCurrentPosition () const;
	CRect getBoundingBox () const;

	static CGAffineTransform createCGAfflineTransform (const CGraphicsTransform& t);

//------------------------------------------------------------------------------------
protected:
	CGMutablePathRef path;
};

#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_4
//-----------------------------------------------------------------------------
class QuartzGradient : public CGradient
{
public:
	QuartzGradient (double _color1Start, double _color2Start, const CColor& _color1, const CColor& _color2)
	: CGradient (_color1Start, _color2Start, _color1, _color2)
	, gradient (0)
	{
		CGColorRef cgColor1 = CGColorCreateGenericRGB (color1.red/255.f, color1.green/255.f, color1.blue/255.f, color1.alpha/255.f);
		CGColorRef cgColor2 = CGColorCreateGenericRGB (color2.red/255.f, color2.green/255.f, color2.blue/255.f, color2.alpha/255.f);
		const void* colors[] = { cgColor1, cgColor2 };
		CFArrayRef colorArray = CFArrayCreate (0, colors, 2, &kCFTypeArrayCallBacks);

		if (color1Start < 0) color1Start = 0;
		else if (color1Start > 1) color1Start = 1;
		if (color2Start < 0) color2Start = 0;
		else if (color2Start > 1) color2Start = 1;
		CGFloat locations[] = { color1Start, color2Start };
		
		gradient = CGGradientCreateWithColors (0, colorArray, locations);

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
#endif

} // namespace

#endif

#endif // __quartzgraphicspath__
