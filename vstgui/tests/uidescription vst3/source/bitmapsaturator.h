/*
 *  bitmapsaturator.h
 *  uidescription test
 *
 *  Created by Arne Scheffler on 4/5/10.
 *  Copyright 2010 Arne Scheffler. All rights reserved.
 *
 */

#ifndef __bitmapsaturator__
#define __bitmapsaturator__

#include "vstgui/lib/cbitmap.h"

namespace VSTGUI {
namespace Bitmap {

//------------------------------------------------------------------------
static bool hueSaturateValue (CBitmap* bitmap, double hueAngle, double saturation, double value)
{
	bool result = false;
	CBitmapPixelAccess* accessor = CBitmapPixelAccess::create (bitmap);
	if (accessor)
	{
		CColor c;
		double h,s,v;
		do
		{
			accessor->getColor (c);
			c.toHSV (h, s, v);
			h += hueAngle;
			s += saturation;
			v += value;
			c.fromHSV (h, s, v);
			accessor->setColor (c);
		} while (++*accessor);
		accessor->forget ();
		result = true;
	}
	return result;
}

} } // namespaces

#endif
