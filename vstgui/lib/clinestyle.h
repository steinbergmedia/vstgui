// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __clinestyle__
#define __clinestyle__

#include "vstguifwd.h"
#include <vector>

namespace VSTGUI {

//-----------
// @brief Line Style
//-----------
class CLineStyle
{
public:
	using CoordVector = std::vector<CCoord>;

	enum LineCap
	{
		kLineCapButt = 0,
		kLineCapRound,
		kLineCapSquare
	};

	enum LineJoin
	{
		kLineJoinMiter = 0,
		kLineJoinRound,
		kLineJoinBevel
	};

	CLineStyle () = default;
	explicit CLineStyle (LineCap cap, LineJoin join = kLineJoinMiter, CCoord dashPhase = 0., uint32_t dashCount = 0, const CCoord* dashLengths = nullptr);
	CLineStyle (LineCap cap, LineJoin join, CCoord dashPhase, const CoordVector& dashLengths);
	CLineStyle (const CLineStyle& lineStyle);
	~CLineStyle () noexcept = default;

	CLineStyle (LineCap cap, LineJoin join, CCoord dashPhase, CoordVector&& dashLengths) noexcept;
	CLineStyle (CLineStyle&& cls) noexcept;
	CLineStyle& operator= (CLineStyle&& cls) noexcept;
	
	LineCap getLineCap () const { return cap; }
	LineJoin getLineJoin () const { return join; }
	CCoord getDashPhase () const { return dashPhase; }
	uint32_t getDashCount () const { return static_cast<uint32_t> (dashLengths.size ()); }
	CoordVector& getDashLengths () { return dashLengths; }
	const CoordVector& getDashLengths() const { return dashLengths; }

	void setLineCap (LineCap newCap) { cap = newCap; }
	void setLineJoin (LineJoin newJoin) { join = newJoin; }
	void setDashPhase (CCoord phase) { dashPhase = phase; }

	bool operator== (const CLineStyle& cls) const;
	bool operator!= (const CLineStyle& cls) const { return !(*this == cls); }
	CLineStyle& operator= (const CLineStyle& cls);

protected:
	LineCap cap {kLineCapButt};
	LineJoin join {kLineJoinMiter};
	CCoord dashPhase {0.};
	CoordVector dashLengths;
};

extern const CLineStyle kLineSolid;
extern const CLineStyle kLineOnOffDash;

}

#endif // __clinestyle__
