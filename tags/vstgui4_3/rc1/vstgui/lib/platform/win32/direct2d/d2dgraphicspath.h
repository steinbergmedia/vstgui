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

#ifndef __d2dgraphicspath__
#define __d2dgraphicspath__

#include "../../../cgraphicspath.h"

#if WINDOWS && VSTGUI_DIRECT2D_SUPPORT

#include <list>

struct ID2D1PathGeometry;
struct ID2D1Geometry;
struct D2D1_GRADIENT_STOP;

namespace VSTGUI {
class D2DFont;
class D2DDrawContext;

//-----------------------------------------------------------------------------
class D2DGraphicsPath : public CGraphicsPath
{
public:
	D2DGraphicsPath ();
	D2DGraphicsPath (const D2DFont* font, UTF8StringPtr text);
	~D2DGraphicsPath ();
	
	ID2D1Geometry* createPath (int32_t fillMode, D2DDrawContext* context = 0, CGraphicsTransform* transform = 0);

	CGradient* createGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2) VSTGUI_OVERRIDE_VMETHOD;

	bool hitTest (const CPoint& p, bool evenOddFilled = false, CGraphicsTransform* transform = 0) VSTGUI_OVERRIDE_VMETHOD;
	CPoint getCurrentPosition () VSTGUI_OVERRIDE_VMETHOD;
	CRect getBoundingBox () VSTGUI_OVERRIDE_VMETHOD;
	void dirty () VSTGUI_OVERRIDE_VMETHOD;
protected:
	ID2D1Geometry* path;
	int32_t currentPathFillMode;
};

} // namespace

#endif // WINDOWS

#endif // __d2dgraphicspath__
