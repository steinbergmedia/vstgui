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
class Pixel
{
public:
	inline void setBuffer (unsigned char* address) { data = address; }
	inline void advance () { data += 4; }
	virtual void getColor (CColor& c) const = 0;
	virtual void setColor (const CColor& c) = 0;

	static Pixel* create (IPlatformBitmapPixelAccess::PixelFormat pixelFormat);
protected:
	Pixel () : data (0) {}
	unsigned char* data;
};

//------------------------------------------------------------------------
template <int redPosition, int greenPosition, int bluePosition, int alphaPosition>
class PixelOrder : public Pixel
{
public:
	void getColor (CColor& c) const
	{
		c.red = data[redPosition];
		c.green = data[greenPosition];
		c.blue = data[bluePosition];
		c.alpha = data[alphaPosition];
	}
	void setColor (const CColor& c)
	{
		data[redPosition] = c.red;
		data[greenPosition] = c.green;
		data[bluePosition] = c.blue;
		data[alphaPosition] = c.alpha;
	}
};

//------------------------------------------------------------------------
Pixel* Pixel::create (IPlatformBitmapPixelAccess::PixelFormat pixelFormat)
{
	switch (pixelFormat)
	{
		case IPlatformBitmapPixelAccess::kARGB: return new PixelOrder<1,2,3,0> ();
		case IPlatformBitmapPixelAccess::kRGBA: return new PixelOrder<0,1,2,3> ();
		case IPlatformBitmapPixelAccess::kABGR: return new PixelOrder<3,2,1,0> ();
		case IPlatformBitmapPixelAccess::kBGRA: return new PixelOrder<2,1,0,3> ();
	}
	return 0;
}

//------------------------------------------------------------------------
static bool hueSaturateValue (CBitmap* bitmap, double hueAngle, double saturation, double value)
{
	bool result = false;
	IPlatformBitmapPixelAccess* bipa = bitmap->getPlatformBitmap ()->lockPixels (false);
	if (bipa)
	{
		Pixel* pixel = Pixel::create (bipa->getPixelFormat ());
		CColor c;
		double h,s,v;
		for (long y = 0; y < bitmap->getHeight (); y++)
		{
			unsigned char* scanline = bipa->getAddress () + y * bipa->getBytesPerRow ();
			pixel->setBuffer (scanline);
			for (long x = 0; x < bitmap->getWidth (); x++, pixel->advance ())
			{
				pixel->getColor (c);
				c.toHSV (h, s, v);
				h += hueAngle;
				s += saturation;
				v += value;
				c.fromHSV (h, s, v);
				pixel->setColor (c);
			}
		}
		delete pixel;
		result = true;
		bipa->forget ();
	}
	return result;
}

} } // namespaces

#endif
