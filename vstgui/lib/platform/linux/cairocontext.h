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

#pragma once

#include "cairoutils.h"

#include "../../coffscreencontext.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Cairo {

class Bitmap;

//------------------------------------------------------------------------
class Context : public COffscreenContext
{
public:
	using super = COffscreenContext;

	Context (CRect& rect, const SurfaceHandle& surface);
	Context (CRect r, cairo_t* context);
	Context (Bitmap* bitmap);

	~Context ();

	bool valid () const { return cr != nullptr; }
	CRect getSurfaceRect () const { return surfaceRect; }
	const SurfaceHandle& getSurface () const { return surface; }
	const ContextHandle& getCairo () const { return cr; }

	void drawLine (const LinePair& line) override;
	void drawLines (const LineList& lines) override;
	void drawPolygon (const PointList& polygonPointList, const CDrawStyle drawStyle) override;
	void drawRect (const CRect& rect, const CDrawStyle drawStyle) override;
	void drawArc (const CRect& rect, const float startAngle1, const float endAngle2,
	              const CDrawStyle drawStyle) override;
	void drawEllipse (const CRect& rect, const CDrawStyle drawStyle) override;
	void drawPoint (const CPoint& point, const CColor& color) override;
	void drawBitmap (CBitmap* bitmap, const CRect& dest, const CPoint& offset,
	                 float alpha) override;
	void clearRect (const CRect& rect) override;
	CGraphicsPath* createGraphicsPath () override;
	CGraphicsPath* createTextPath (const CFontRef font, UTF8StringPtr text) override;
	void drawGraphicsPath (CGraphicsPath* path, PathDrawMode mode,
	                       CGraphicsTransform* transformation) override;
	void fillLinearGradient (CGraphicsPath* path, const CGradient& gradient,
	                         const CPoint& startPoint, const CPoint& endPoint, bool evenOdd,
	                         CGraphicsTransform* transformation) override;
	void fillRadialGradient (CGraphicsPath* path, const CGradient& gradient, const CPoint& center,
	                         CCoord radius, const CPoint& originOffset, bool evenOdd,
	                         CGraphicsTransform* transformation) override;

	void saveGlobalState () override;
	void restoreGlobalState () override;

	void beginDraw () override;
	void endDraw () override;

private:
	void init () override;
	void setSourceColor (CColor color);
	void setupCurrentStroke ();
	void draw (CDrawStyle drawstyle);

	SurfaceHandle surface;
	ContextHandle cr;
};

//------------------------------------------------------------------------
struct DrawBlock
{
	static DrawBlock begin (Context& context);

	~DrawBlock ();
	operator bool () { return !clipIsEmpty; }
private:
	explicit DrawBlock (Context& context);
	Context& context;
	bool clipIsEmpty {false};
};

//-----------------------------------------------------------------------------
template <typename T>
inline T pixelAlign (const CGraphicsTransform& tm, T obj)
{
	tm.transform (obj);
	obj.makeIntegral ();
	tm.inverse ().transform (obj);
	return obj;
}

//------------------------------------------------------------------------
} // Cairo
} // VSTGUI
