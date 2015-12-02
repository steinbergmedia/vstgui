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

#include "cspecialdigit.h"
#include "../cdrawcontext.h"
#include "../cbitmap.h"
#include <cmath>

namespace VSTGUI {

//------------------------------------------------------------------------
// CSpecialDigit
//------------------------------------------------------------------------
/*! @class CSpecialDigit
Can be used to display a counter with maximum 7 digits.
All digit have the same size and are stacked in height in the bitmap.
*/
//------------------------------------------------------------------------
/**
 * CSpecialDigit constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param dwPos actual value
 * @param iNumbers amount of numbers (max 7)
 * @param xpos array of all X positions, can be NULL
 * @param ypos array of all Y positions, can be NULL
 * @param width width of one number in the bitmap
 * @param height height of one number in the bitmap
 * @param background bitmap
 */
//------------------------------------------------------------------------
CSpecialDigit::CSpecialDigit (const CRect& size, IControlListener* listener, int32_t tag, int32_t dwPos, int32_t iNumbers, int32_t* xpos, int32_t* ypos, int32_t width, int32_t height, CBitmap* background)
: CControl (size, listener, tag, background)
, iNumbers (iNumbers)
, width (width)
, height (height)
{
	setValue ((float)dwPos);          // actual value

	if (iNumbers > 7)
		iNumbers = 7;

	if (xpos == NULL)
	{
		// automatically init xpos/ypos if not provided by caller
		const int32_t numw = (const int32_t)background->getWidth();
		int32_t x = (int32_t)size.left;
		for (int32_t i = 0; i < iNumbers; i++)
		{
			this->xpos[i] = x; 
			this->ypos[i] = (int32_t)size.top;
			x += numw;
		}
	} 
	else if (xpos && ypos)
	{
		// store coordinates of x/y pos of each digit
		for (int32_t i = 0; i < iNumbers; i++)
		{
			this->xpos[i] = xpos[i];
			this->ypos[i] = ypos[i];
		}
	}

	setMax ((float)pow (10.f, (float)iNumbers) - 1.0f);
	setMin (0.0f);
}

//------------------------------------------------------------------------
CSpecialDigit::CSpecialDigit (const CSpecialDigit& v)
: CControl (v)
, iNumbers (v.iNumbers)
, width (v.width)
, height (v.height)
{
	for (int32_t i = 0; i < 7; i++)
	{
		xpos[i] = v.xpos[i];
		ypos[i] = v.ypos[i];
	}
}

//------------------------------------------------------------------------
CSpecialDigit::~CSpecialDigit ()
{}

//------------------------------------------------------------------------
void CSpecialDigit::draw (CDrawContext *pContext)
{
	CPoint  where;
	CRect   rectDest;
	int32_t    i, j;
	int32_t    dwValue;
	int32_t     one_digit[16];
  
	if ((int32_t)value >= getMax ()) 
		dwValue = (int32_t)getMax ();
	else if ((int32_t)value < getMin ()) 
		dwValue = (int32_t)getMin ();
	else
		dwValue = (int32_t)value;
	
	for (i = 0, j = ((int32_t)getMax () + 1) / 10; i < iNumbers; i++, j /= 10)
	{
		one_digit[i] = dwValue / j;
		dwValue -= (one_digit[i] * j);
	}
	
	where.x = 0;
	for (i = 0; i < iNumbers; i++)
	{	
		j = one_digit[i];
		if (j > 9)
			j = 9;
		
		rectDest.left   = (CCoord)xpos[i];
		rectDest.top    = (CCoord)ypos[i];
		
		rectDest.right  = rectDest.left + width;
		rectDest.bottom = rectDest.top  + height;		
		
		// where = src from bitmap
		where.y = (CCoord)j * height;
		if (getDrawBackground ())
		{
			getDrawBackground ()->draw (pContext, rectDest, where);
		}
	}
		
	setDirty (false);
}

} // namespace
