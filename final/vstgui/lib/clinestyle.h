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
	typedef std::vector<CCoord> CoordVector;

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

	CLineStyle (LineCap cap = kLineCapButt, LineJoin join = kLineJoinMiter, CCoord dashPhase = 0., uint32_t dashCount = 0, const CCoord* dashLengths = 0);
	CLineStyle (LineCap cap, LineJoin join, CCoord dashPhase, const CoordVector& dashLengths);
	CLineStyle (const CLineStyle& lineStyle);
	~CLineStyle ();

#if VSTGUI_RVALUE_REF_SUPPORT
	CLineStyle (LineCap cap, LineJoin join, CCoord dashPhase, CoordVector&& dashLengths) noexcept;
	CLineStyle (CLineStyle&& cls) noexcept;
	CLineStyle& operator= (CLineStyle&& cls) noexcept;
#endif
	
	LineCap getLineCap () const { return cap; }
	LineJoin getLineJoin () const { return join; }
	CCoord getDashPhase () const { return dashPhase; }
	uint32_t getDashCount () const { return static_cast<uint32_t> (dashLengths.size ()); }
	CoordVector& getDashLengths () { return dashLengths; }
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	const CCoord* getDashLengths() const { return &dashLengths[0]; }
#else
	const CoordVector& getDashLengths() const { return dashLengths; }
#endif

	void setLineCap (LineCap newCap) { cap = newCap; }
	void setLineJoin (LineJoin newJoin) { join = newJoin; }
	void setDashPhase (CCoord phase) { dashPhase = phase; }

	bool operator== (const CLineStyle& cls) const;
	bool operator!= (const CLineStyle& cls) const { return !(*this == cls); }
	CLineStyle& operator= (const CLineStyle& cls);

protected:
	LineCap cap;
	LineJoin join;
	CCoord dashPhase;
	CoordVector dashLengths;
};

extern const CLineStyle kLineSolid;
extern const CLineStyle kLineOnOffDash;

}

#endif // __clinestyle__
