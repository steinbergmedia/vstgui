// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../common/gradientbase.h"

#if WINDOWS

#include <d2d1.h>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class D2DGradient : public PlatformGradientBase
{
public:
	ID2D1GradientStopCollection* create (ID2D1RenderTarget* renderTarget, float globalAlpha) const;
};

} // VSTGUI

#endif // WINDOWS
