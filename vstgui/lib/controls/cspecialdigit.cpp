// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
CSpecialDigit::CSpecialDigit (const CRect& size, IControlListener* listener, int32_t tag, int32_t dwPos, int32_t inNumbers, int32_t* xpos, int32_t* ypos, int32_t width, int32_t height, CBitmap* background)
: CControl (size, listener, tag, background)
, iNumbers (inNumbers)
, width (width)
, height (height)
{
	setValue ((float)dwPos);          // actual value

	if (iNumbers > 7)
		iNumbers = 7;

	if (xpos == nullptr)
	{
		// automatically init xpos/ypos if not provided by caller
		const int32_t numw = (const int32_t)background->getWidth();
		int32_t x = (int32_t)size.left;
		for (int32_t i = 0; i < inNumbers; i++)
		{
			this->xpos[i] = x; 
			this->ypos[i] = (int32_t)size.top;
			x += numw;
		}
	} 
	else if (xpos && ypos)
	{
		// store coordinates of x/y pos of each digit
		for (int32_t i = 0; i < inNumbers; i++)
		{
			this->xpos[i] = xpos[i];
			this->ypos[i] = ypos[i];
		}
	}

	setMax ((float)pow (10.f, (float)inNumbers) - 1.0f);
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
void CSpecialDigit::draw (CDrawContext *pContext)
{
	CPoint where;
	CRect rectDest;
	int32_t i, j;
	int32_t one_digit[16] = {};

	int32_t dwValue = static_cast<int32_t> (getValue ());
	int32_t intMax = static_cast<int32_t> (getMax ());
	if (dwValue > intMax)
		dwValue = intMax;
	else if (dwValue < static_cast<int32_t> (getMin ()))
		dwValue = static_cast<int32_t> (getMin ());
	
	for (i = 0, j = (intMax + 1) / 10; i < iNumbers; i++, j /= 10)
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

} // VSTGUI
