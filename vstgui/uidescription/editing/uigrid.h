// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __uigrid__
#define __uigrid__

#include "../../lib/cpoint.h"

#if VSTGUI_LIVE_EDITING

#include <cmath>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UIGrid : public NonAtomicReferenceCounted
{
public:
	UIGrid (const CPoint& size = CPoint (10, 10)) : size (size) {}
	
	virtual void process (CPoint& p)
	{
		p.x -= size.x / 2.;
		p.y -= size.y / 2.;
		int32_t x = (int32_t) std::floor (p.x / size.x + 0.5);
		p.x = x * size.x;
		int32_t y = (int32_t) std::floor (p.y / size.y + 0.5);
		p.y = y * size.y;
	}

	virtual void setSize (const CPoint& p) { size = p; }
	const CPoint& getSize () const { return size; }
protected:
	CPoint size;
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uigrid__
