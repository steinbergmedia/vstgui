// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../cgraphicspath.h"
#include "cairoutils.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Cairo {

//------------------------------------------------------------------------
class Path : public CGraphicsPath
{
public:
	Path (const ContextHandle& cr) noexcept;
	~Path () noexcept;

	cairo_path_t* getPath (const ContextHandle& handle, bool align = false);

	CGradient* createGradient (double color1Start, double color2Start, const CColor& color1,
							   const CColor& color2) override;

	bool hitTest (const CPoint& p, bool evenOddFilled = false,
				  CGraphicsTransform* transform = 0) override;
	CPoint getCurrentPosition () override;
	CRect getBoundingBox () override;

	void dirty () override;

//------------------------------------------------------------------------
private:
	ContextHandle cr;
	cairo_path_t* path {nullptr};
};

//------------------------------------------------------------------------
} // Cairo
} // VSTGUI
