//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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

#include "crect.h"
#include "cpoint.h"
#include <cmath>

namespace VSTGUI {

//-----------------------------------------------------------------------------
bool CRect::pointInside (const CPoint& where) const
{
	return where.h >= left && where.h < right && where.v >= top && where.v < bottom;
}

//-----------------------------------------------------------------------------
bool CRect::isEmpty () const 
{
	if (right <= left)
		return true;
	if (bottom <= top)
		return true;
	return false;
}

//-----------------------------------------------------------------------------
void CRect::bound (const CRect& rect)
{
	if (left < rect.left)
		left = rect.left;
	if (top < rect.top)
		top = rect.top;
	if (right > rect.right)
		right = rect.right;
	if (bottom > rect.bottom)
		bottom = rect.bottom;
	if (bottom < top)
		bottom = top;
	if (right < left)
		right = left;
}

//-----------------------------------------------------------------------------
void CRect::unite (const CRect& rect)
{
	if (left > rect.left)
		left = rect.left;
	if (right < rect.right)
		right = rect.right;
	if (top > rect.top)
		top = rect.top;
	if (bottom < rect.bottom)
		bottom = rect.bottom;
}

//-----------------------------------------------------------------------------
void CRect::makeIntegral ()
{
	left = floor (left + 0.5);
	right = floor (right + 0.5);
	top = floor (top + 0.5);
	bottom = floor (bottom + 0.5);
}

//-----------------------------------------------------------------------------
CPoint CRect::getTopLeft () const
{
	CPoint myPoint (left, top);
	return myPoint;
}

//-----------------------------------------------------------------------------
CPoint CRect::getTopRight () const
{
	CPoint myPoint (right, top);
	return myPoint;
}

//-----------------------------------------------------------------------------
CPoint CRect::getBottomLeft () const
{
	CPoint myPoint (left, bottom);
	return myPoint;
}

//-----------------------------------------------------------------------------
CPoint CRect::getBottomRight () const
{
	CPoint myPoint (right, bottom);
	return myPoint;
}

//-----------------------------------------------------------------------------
void CRect::setTopLeft (const CPoint& inPoint)
{
	left = inPoint.h;
	top = inPoint.v;
}

//-----------------------------------------------------------------------------
void CRect::setTopRight (const CPoint& inPoint)
{
	right = inPoint.h;
	top = inPoint.v;
}

//-----------------------------------------------------------------------------
void CRect::setBottomLeft (const CPoint& inPoint)
{
	left = inPoint.h;
	bottom = inPoint.v;
}

//-----------------------------------------------------------------------------
void CRect::setBottomRight (const CPoint& inPoint)
{
	right = inPoint.h;
	bottom = inPoint.v;
}

//-----------------------------------------------------------------------------
CPoint CRect::getCenter () const
{
	CPoint myPoint (left + getWidth () / 2, top + getHeight () / 2);
	return myPoint;
}

//-----------------------------------------------------------------------------
CPoint CRect::getSize () const
{
	CPoint myPoint (getWidth (), getHeight ());
	return myPoint;
}

//-----------------------------------------------------------------------------
void CRect::setSize (const CPoint& size)
{
	setWidth (size.x);
	setHeight (size.y);
}

//-----------------------------------------------------------------------------
void CRect::centerInside (const CRect& r)
{
	CPoint cp = r.getCenter ();
	CPoint cp2 = getCenter ();
	offset (cp.x-cp2.x, cp.y-cp2.y);
}

} // namespace
