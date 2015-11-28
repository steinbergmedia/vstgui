//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

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
class CGradient : public CBaseObject
{
public:
	typedef std::multimap<double, CColor> ColorStopMap;

	static CGradient* create (const ColorStopMap& colorStopMap);
	static CGradient* create (double color1Start, double color2Start, const CColor& color1, const CColor& color2)
	{
		ColorStopMap map;
		map.insert (std::make_pair (color1Start, color1));
		map.insert (std::make_pair (color2Start, color2));
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
		colorStops.insert (colorStop);
	}

#if VSTGUI_RVALUE_REF_SUPPORT
	virtual void addColorStop (std::pair<double, CColor>&& colorStop)
	{
		colorStops.insert (std::move (colorStop));
	}
#endif
	
	const ColorStopMap& getColorStops () const { return colorStops; }
	//@}
//-----------------------------------------------------------------------------
	CLASS_METHODS_NOCOPY(CGradient, CBaseObject)
protected:
	CGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2)
	{
		addColorStop (color1Start, color1);
		addColorStop (color2Start, color2);
	}
	CGradient (const ColorStopMap& colorStopMap) : colorStops (colorStopMap) {}

	ColorStopMap colorStops;
};

}

#endif // __cgradient__
