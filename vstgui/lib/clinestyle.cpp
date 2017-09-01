// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "clinestyle.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CLineStyle::CLineStyle (LineCap _cap, LineJoin _join, CCoord _dashPhase, uint32_t _dashCount, const CCoord* _dashLengths)
: cap (_cap)
, join (_join)
, dashPhase (_dashPhase)
{
	if (_dashCount && _dashLengths)
	{
		for (uint32_t i = 0; i < _dashCount; i++)
			dashLengths.emplace_back (_dashLengths[i]);
	}
}

//-----------------------------------------------------------------------------
CLineStyle::CLineStyle (LineCap _cap, LineJoin _join, CCoord _dashPhase, const CoordVector& _dashLengths)
: cap (_cap)
, join (_join)
, dashPhase (_dashPhase)
, dashLengths (_dashLengths)
{
}

//-----------------------------------------------------------------------------
CLineStyle::CLineStyle (const CLineStyle& lineStyle)
{
	*this = lineStyle;
}

//-----------------------------------------------------------------------------
CLineStyle::CLineStyle (LineCap _cap, LineJoin _join, CCoord _dashPhase, CoordVector&& _dashLengths) noexcept
: cap (_cap)
, join (_join)
, dashPhase (_dashPhase)
, dashLengths (std::move (_dashLengths))
{
}

//-----------------------------------------------------------------------------
CLineStyle::CLineStyle (CLineStyle&& cls) noexcept
{
	*this = std::move (cls);
}

//-----------------------------------------------------------------------------
CLineStyle& CLineStyle::operator= (CLineStyle&& cls) noexcept
{
	dashLengths.clear ();
	cap = cls.cap;
	join = cls.join;
	dashPhase = cls.dashPhase;
	dashLengths = std::move (cls.dashLengths);
	return *this;
}

//-----------------------------------------------------------------------------
bool CLineStyle::operator== (const CLineStyle& cls) const
{
	if (cap == cls.cap && join == cls.join && dashPhase == cls.dashPhase && dashLengths == cls.dashLengths)
	{
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
CLineStyle& CLineStyle::operator= (const CLineStyle& cls)
{
	dashLengths.clear ();
	cap = cls.cap;
	join = cls.join;
	dashPhase = cls.dashPhase;
	dashLengths = cls.dashLengths;
	return *this;
}

//-----------------------------------------------------------------------------
static const CCoord kDefaultOnOffDashLength[] = {1, 1};
const CLineStyle kLineSolid {};
const CLineStyle kLineOnOffDash (CLineStyle::kLineCapButt, CLineStyle::kLineJoinMiter, 0, 2, kDefaultOnOffDashLength);

}
