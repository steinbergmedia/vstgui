// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

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
	static CGradient* create (const GradientColorStopMap& colorStopMap);
	static CGradient* create (double color1Start, double color2Start, const CColor& color1, const CColor& color2);
	
	CGradient (PlatformGradientPtr&& platformGradient);
	~CGradient () noexcept override;

	//-----------------------------------------------------------------------------
	/// @name Member Access
	//-----------------------------------------------------------------------------
	//@{
	
	void addColorStop (double start, const CColor& color);
	void addColorStop (const GradientColorStop& colorStop);
	
	const GradientColorStopMap& getColorStops () const;
	//@}
	
	const PlatformGradientPtr& getPlatformGradient () const;
protected:
	PlatformGradientPtr platformGradient;
};

} // VSTGUI
