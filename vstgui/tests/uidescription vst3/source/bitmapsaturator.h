// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
