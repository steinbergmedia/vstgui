// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cgradient.h"
#include "platform/iplatformgradient.h"
#include "platform/platformfactory.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
CGradient* CGradient::create (const GradientColorStopMap& colorStopMap)
{
	if (auto pg = getPlatformFactory ().createGradient ())
	{
		pg->setColorStops (colorStopMap);
		return new CGradient (std::move (pg));
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
CGradient* CGradient::create (double color1Start, double color2Start, const CColor& color1,
                              const CColor& color2)
{
	GradientColorStopMap map;
	map.emplace (color1Start, color1);
	map.emplace (color2Start, color2);
	return create (map);
}

//-----------------------------------------------------------------------------
CGradient::CGradient (PlatformGradientPtr&& platformGradient)
: platformGradient (std::move (platformGradient))
{
}

//-----------------------------------------------------------------------------
CGradient::~CGradient () noexcept = default;

//-----------------------------------------------------------------------------
void CGradient::addColorStop (double start, const CColor& color)
{
	addColorStop (std::make_pair (start, color));
}

//-----------------------------------------------------------------------------
void CGradient::addColorStop (const GradientColorStop& colorStop)
{
	platformGradient->addColorStop (colorStop);
}

//-----------------------------------------------------------------------------
const GradientColorStopMap& CGradient::getColorStops () const
{
	return platformGradient->getColorStops ();
}

//-----------------------------------------------------------------------------
const PlatformGradientPtr& CGradient::getPlatformGradient () const
{
	return platformGradient;
}

} // VSTGUI
