// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
class IScaleFactorChangedListener
{
public:
	virtual ~IScaleFactorChangedListener () noexcept = default;

	virtual void onScaleFactorChanged (CFrame& frame, double newScaleFactor) = 0;
};

//------------------------------------------------------------------------
class ScaleFactorChangedListenerAdapter : public IScaleFactorChangedListener
{
public:
	void onScaleFactorChanged (CFrame& frame, double newScaleFactor) override {}
};

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
struct LegacyScaleFactorChangedListenerAdapter : ScaleFactorChangedListenerAdapter
{
	void onScaleFactorChanged (CFrame& frame, double newScaleFactor) final
	{
		onScaleFactorChanged (&frame, newScaleFactor);
	}

	virtual void onScaleFactorChanged (CFrame* frame, double newScaleFactor) {}
};
#endif

} // VSTGUI
