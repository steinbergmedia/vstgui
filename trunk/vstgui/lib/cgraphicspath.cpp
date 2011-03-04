//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "cgraphicspath.h"
#include "cdrawcontext.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
void CGraphicsTransform::rotate (double angle)
{
	angle = radians (angle);
	*this = CGraphicsTransform (cos (angle), sin (angle), -sin (angle), cos (angle), 0, 0) * this;
}
	
//-----------------------------------------------------------------------------
void CGraphicsPath::addRoundRect (const CRect& size, CCoord radius)
{
	CRect rect2 (size);
	rect2.normalize ();
	const CCoord X = rect2.left;
	const CCoord X3 = rect2.right; 
	const CCoord X2 = X3 - radius;
	const CCoord Y = rect2.top;
	const CCoord Y3 = rect2.bottom; 
	const CCoord Y2 = Y3 - radius;
	const CPoint arcsize (radius, radius);
	addArc (CRect (CPoint (X2, Y), arcsize), 270., 360., false);
	addArc (CRect (CPoint (X2, Y2), arcsize), 0., 90., false);
	addArc (CRect (CPoint (X, Y2), arcsize), 90., 180., false);
	addArc (CRect (CPoint (X, Y), arcsize), 180., 270., false);
	closeSubpath();
}

} // namespace
