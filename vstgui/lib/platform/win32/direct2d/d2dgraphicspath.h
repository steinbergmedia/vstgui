// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../../cgraphicspath.h"

#if WINDOWS

#include <list>

struct ID2D1PathGeometry;
struct ID2D1Geometry;
struct D2D1_GRADIENT_STOP;

namespace VSTGUI {
class D2DFont;
class D2DDrawContext;

//-----------------------------------------------------------------------------
class D2DGraphicsPath final : public CGraphicsPath
{
public:
	D2DGraphicsPath ();
	D2DGraphicsPath (const D2DFont* font, UTF8StringPtr text);
	~D2DGraphicsPath ();
	
	ID2D1Geometry* createPath (int32_t fillMode, D2DDrawContext* context = nullptr, CGraphicsTransform* transform = nullptr);

	CGradient* createGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2) override;

	bool hitTest (const CPoint& p, bool evenOddFilled = false, CGraphicsTransform* transform = nullptr) override;
	CPoint getCurrentPosition () override;
	CRect getBoundingBox () override;
	void dirty () override;
protected:
	ID2D1Geometry* path;
	int32_t currentPathFillMode;
};

} // VSTGUI

#endif // WINDOWS
