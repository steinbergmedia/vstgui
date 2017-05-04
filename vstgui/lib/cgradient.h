// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __cgradient__
#define __cgradient__

#include "vstguifwd.h"
#include "ccolor.h"
#include <map>
#include <algorithm>

namespace VSTGUI {

//-----------------------------------------------------------------------------
///	@brief Gradient Object [new in 4.0]
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class CGradient : public AtomicReferenceCounted
{
public:
	using ColorStopMap = std::multimap<double, CColor>;

	static CGradient* create (const ColorStopMap& colorStopMap);
	static CGradient* create (double color1Start, double color2Start, const CColor& color1, const CColor& color2)
	{
		ColorStopMap map;
		map.emplace (color1Start, color1);
		map.emplace (color2Start, color2);
		return create (map);
	}
	
	//-----------------------------------------------------------------------------
	/// @name Member Access
	//-----------------------------------------------------------------------------
	//@{
	
	void addColorStop (double start, const CColor& color)
	{
		addColorStop (std::make_pair (start, color));
	}
	
	virtual void addColorStop (const std::pair<double, CColor>& colorStop)
	{
		colorStops.emplace (colorStop);
	}

	virtual void addColorStop (std::pair<double, CColor>&& colorStop)
	{
		colorStops.emplace (std::move (colorStop));
	}
	
	const ColorStopMap& getColorStops () const { return colorStops; }
	//@}
protected:
	CGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2)
	{
		addColorStop (color1Start, color1);
		addColorStop (color2Start, color2);
	}
	explicit CGradient (const ColorStopMap& colorStopMap) : colorStops (colorStopMap) {}

	ColorStopMap colorStops;
};

}

#endif // __cgradient__
