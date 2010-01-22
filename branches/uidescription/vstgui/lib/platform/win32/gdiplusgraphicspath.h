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

#ifndef __gdiplusgraphicspath__
#define __gdiplusgraphicspath__

#include "../../cgraphicspath.h"

#if WINDOWS

namespace Gdiplus {
class GraphicsPath;
}

namespace VSTGUI {

class GdiplusGraphicsPath : public CGraphicsPath
{
public:
	GdiplusGraphicsPath ();
	~GdiplusGraphicsPath ();

	Gdiplus::GraphicsPath* getGraphicsPath () const { return platformPath; }

	// CGraphicsPath
	CGradient* createGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2);
	void addArc (const CRect& rect, double startAngle, double endAngle);
	void addCurve (const CPoint& start, const CPoint& control1, const CPoint& control2, const CPoint& end);
	void addEllipse (const CRect& rect);
	void addLine (const CPoint& start, const CPoint& end);
	void addRect (const CRect& rect);
	void addPath (const CGraphicsPath& path, CGraphicsTransformation* transformation = 0);
	void addString (const char* utf8String, CFontRef font, const CPoint& position);
	void closeSubpath ();
	void draw (CDrawContext* context, PathDrawMode mode = kFilled, CGraphicsTransformation* transformation = 0);
	void fillLinearGradient (CDrawContext* context, const CGradient& gradient, const CPoint& startPoint, const CPoint& endPoint, bool evenOdd = false, CGraphicsTransformation* transformation = 0);
	CPoint getCurrentPosition () const;
	CRect getBoundingBox () const;
protected:
	Gdiplus::GraphicsPath* platformPath;
};

} // namespace

#endif // WINDOWS

#endif // __gdiplusgraphicspath__
