// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../lib/vstguibase.h"

#if VSTGUI_LIVE_EDITING

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class IGridProcessor : virtual public IReference
{
public:
	virtual void process (CPoint& p) = 0;
};

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
