// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __iscalefactorchangedlistener__
#define __iscalefactorchangedlistener__

#include "vstguifwd.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
class IScaleFactorChangedListener
{
public:
	virtual ~IScaleFactorChangedListener () noexcept = default;

	virtual void onScaleFactorChanged (CFrame* frame, double newScaleFactor) = 0;
};

} // namespace

#endif // __iscalefactorchangedlistener__
