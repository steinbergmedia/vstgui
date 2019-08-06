// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../lib/cpoint.h"

#if VSTGUI_LIVE_EDITING

#include "igridprocessor.h"
#include <cmath>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UIGrid : public IGridProcessor, public NonAtomicReferenceCounted
{
public:
	UIGrid (const CPoint& size = CPoint (10, 10)) : size (size) {}
	
	void process (CPoint& p) override
	{
		auto x = static_cast<int32_t> (std::round (p.x / size.x));
		p.x = x * size.x;
		auto y = static_cast<int32_t> (std::round (p.y / size.y));
		p.y = y * size.y;
	}

	virtual void setSize (const CPoint& p) { size = p; }
	const CPoint& getSize () const { return size; }
protected:
	CPoint size;
};

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
