// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformgradient.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
class PlatformGradientBase : public IPlatformGradient
{
public:
	void setColorStops (const GradientColorStopMap& colorStops) override
	{
		map = colorStops;
		changed ();
	}
	void addColorStop (const GradientColorStop& colorStop) override
	{
		map.emplace (colorStop);
		changed ();
	}
	const GradientColorStopMap& getColorStops () const override
	{
		return map;
	}

	virtual void changed () {}
private:
	GradientColorStopMap map;
};

} // VSTGUI

