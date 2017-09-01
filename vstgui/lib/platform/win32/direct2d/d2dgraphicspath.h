// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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

	CGradient* createGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2) override;

	bool hitTest (const CPoint& p, bool evenOddFilled = false, CGraphicsTransform* transform = 0) override;
	CPoint getCurrentPosition () override;
	CRect getBoundingBox () override;
	void dirty () override;
protected:
	ID2D1Geometry* path;
	int32_t currentPathFillMode;
};

} // namespace

#endif // WINDOWS

#endif // __d2dgraphicspath__
