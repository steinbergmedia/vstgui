// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../cgradient.h"
#include "../../cpoint.h"
#include "cairoutils.h"
#include <cairo/cairo.h>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Cairo {

//------------------------------------------------------------------------
class Gradient : public CGradient
{
public:
	Gradient (const ColorStopMap& colorStopMap);
	~Gradient ();

	void addColorStop (const std::pair<double, CColor>& colorStop) override
	{
		destroy ();
		CGradient::addColorStop (colorStop);
	}

#if VSTGUI_RVALUE_REF_SUPPORT
	void addColorStop (std::pair<double, CColor>&& colorStop) override
	{
		destroy ();
		CGradient::addColorStop (std::move (colorStop));
	}
#endif

	const PatternHandle& getLinearGradient (CPoint start, CPoint end) const;
	const PatternHandle& getRadialGradient () const;

private:
	void destroy () const;

	/* we want to calculate a normalized linear and radial gradiant */
	mutable PatternHandle linearGradient;
	mutable PatternHandle radialGradient;

	mutable CPoint linearGradientStart;
	mutable CPoint linearGradientEnd;
};

//------------------------------------------------------------------------
} // Cairo
} // VSTGUI
