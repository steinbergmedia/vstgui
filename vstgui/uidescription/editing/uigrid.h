
#ifndef __uigrid__
#define __uigrid__

#include "../../lib/cpoint.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UIGrid : public CBaseObject
{
public:
	UIGrid (const CPoint& size = CPoint (10, 10)) : size (size) {}
	
	virtual void process (CPoint& p)
	{
		int32_t x = (int32_t) (p.x / size.x);
		p.x = x * size.x;
		int32_t y = (int32_t) (p.y / size.y);
		p.y = y * size.y;
	}

	virtual void setSize (const CPoint& p) { size = p; }
	const CPoint& getSize () const { return size; }
protected:
	CPoint size;
};

} // namespace

#endif // __uigrid__
