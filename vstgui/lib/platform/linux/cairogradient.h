// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../common/gradientbase.h"
#include "../../cpoint.h"
#include "cairoutils.h"
#include <cairo/cairo.h>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Cairo {

//------------------------------------------------------------------------
class Gradient : public PlatformGradientBase
{
public:
	~Gradient () noexcept override;

	const PatternHandle& getLinearGradient (CPoint start, CPoint end) const;
	const PatternHandle& getRadialGradient (CPoint center, CCoord radius,
											CPoint originOffset) const;

private:
	void changed () override;

	/* we want to calculate a normalized linear and radial gradiant */
	mutable PatternHandle linearGradient;
	mutable PatternHandle radialGradient;

	mutable CPoint linearGradientStart;
	mutable CPoint linearGradientEnd;
};

//------------------------------------------------------------------------
} // Cairo
} // VSTGUI
