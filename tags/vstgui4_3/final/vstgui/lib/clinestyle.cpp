//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

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
			dashLengths.push_back (_dashLengths[i]);
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
CLineStyle::~CLineStyle ()
{
}

#if VSTGUI_RVALUE_REF_SUPPORT
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
#endif

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
const CLineStyle kLineSolid;
const CLineStyle kLineOnOffDash (CLineStyle::kLineCapButt, CLineStyle::kLineJoinMiter, 0, 2, kDefaultOnOffDashLength);

}