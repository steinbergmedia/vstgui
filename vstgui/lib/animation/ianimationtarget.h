// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../vstguifwd.h"

namespace VSTGUI {
namespace Animation {

//-----------------------------------------------------------------------------
/// @brief Animation target interface
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class IAnimationTarget
{
public:
	virtual ~IAnimationTarget () noexcept = default;

	/** animation starts */
	virtual void animationStart (CView* view, IdStringPtr name) = 0;
	/** pos is a normalized value between zero and one */
	virtual void animationTick (CView* view, IdStringPtr name, float pos) = 0;
	/** animation ended */
	virtual void animationFinished (CView* view, IdStringPtr name, bool wasCanceled) = 0;
};

} // Animation
} // VSTGUI
