// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __gdiplusgraphicspath__
#define __gdiplusgraphicspath__

#include "../../cgraphicspath.h"

#if WINDOWS

namespace Gdiplus {
class GraphicsPath;
class LinearGradientBrush;
}

namespace VSTGUI {
class GdiPlusFont;

//-----------------------------------------------------------------------------
class GdiplusGraphicsPath : public CGraphicsPath
{
public:
	GdiplusGraphicsPath ();
	GdiplusGraphicsPath (const GdiPlusFont* font, UTF8StringPtr text);
	~GdiplusGraphicsPath () noexcept;

	Gdiplus::GraphicsPath* getGraphicsPath ();

	CGradient* createGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2) override;
	void dirty () override;
	bool hitTest (const CPoint& p, bool evenOddFilled = false, CGraphicsTransform* transform = nullptr) override;
	CPoint getCurrentPosition () override;
	CRect getBoundingBox () override;
protected:
	Gdiplus::GraphicsPath* platformPath;
};

} // namespace

#endif // WINDOWS

#endif // __gdiplusgraphicspath__
