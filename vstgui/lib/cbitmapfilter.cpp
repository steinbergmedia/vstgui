#include "cbitmapfilter.h"
#include "cbitmap.h"
#include "ccolor.h"

namespace VSTGUI {

namespace BitmapFilter {

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
void BoxBlur::calculate (CColor& color, CColor* colors, uint32_t numColors)
{
	int32_t red = 0;
	int32_t green = 0;
	int32_t blue = 0;
	int32_t alpha = 0;
	for (int64_t i = numColors-1; i >= 0; i--)
	{
		red += colors[i].red;
		green += colors[i].green;
		blue += colors[i].blue;
		alpha += colors[i].alpha;
		if (i+1 < numColors)
			colors[i+1] = colors[i];
	}
	red /= numColors;
	green /= numColors;
	blue /= numColors;
	alpha /= numColors;
	color.red = (int8_t)red;
	color.green = (int8_t)green;
	color.blue = (int8_t)blue;
	color.alpha = (int8_t)alpha;
}

//----------------------------------------------------------------------------------------------------
CBitmapPixelAccess& BoxBlur::process (CBitmapPixelAccess& accessor, uint32_t boxSize)
{
	uint32_t x,y,x1,y1;
	uint32_t width = accessor.getBitmapWidth ();
	uint32_t height = accessor.getBitmapHeight ();
	CColor* nc = new CColor[boxSize];
	for (y = 0; y < height; y++)
	{
		accessor.setPosition (0, y);
		for (uint32_t i = 1; i < boxSize; i++)
			nc[i] = kTransparentCColor;
		for (x1 = 0; x1 < boxSize / 2; x1++)
		{
			accessor.setPosition (x1, y);
			accessor.getColor (nc[0]);
			calculate(nc[0], nc, boxSize);
		}
		for (x = 0; x < width; x++, x1++)
		{
			if (accessor.setPosition (x1, y))
				accessor.getColor (nc[0]);
			else
				nc[0] = kTransparentCColor;
			calculate (nc[0], nc, boxSize);
			accessor.setPosition (x, y);
			accessor.setColor (nc[0]);
		}
	}
	for (x = 0; x < width; x++)
	{
		accessor.setPosition (x, 0);
		for (uint32_t i = 1; i < boxSize; i++)
			nc[i] = kTransparentCColor;
		for (y1 = 0; y1 < boxSize / 2; y1++)
		{
			accessor.setPosition (x, y1);
			accessor.getColor (nc[0]);
			calculate(nc[0], nc, boxSize);
		}
		for (y = 0; y < height; y++, y1++)
		{
			if (accessor.setPosition (x, y1))
				accessor.getColor (nc[0]);
			else
				nc[0] = kTransparentCColor;
			calculate (nc[0], nc, boxSize);
			accessor.setPosition (x, y);
			accessor.setColor (nc[0]);
		}
	}
	delete [] nc;
	return accessor;
}

//----------------------------------------------------------------------------------------------------
bool BoxBlur::process (CBitmap* bitmap, uint32_t boxSize)
{
	OwningPointer<CBitmapPixelAccess> accessor = CBitmapPixelAccess::create (bitmap, false);
	if (accessor)
	{
		process (*accessor, boxSize);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
CBitmapPixelAccess& Grayscale::process (CBitmapPixelAccess& accessor)
{
	accessor.setPosition (0, 0);
	CColor color;
	uint32_t width = accessor.getBitmapWidth ();
	uint32_t height = accessor.getBitmapHeight ();
	for (uint32_t y = 0; y < height; y++)
	{
		for (uint32_t x = 0; x < width; x++, accessor++)
		{
			accessor.getColor (color);
			color.red = color.green = color.blue = color.getLuma ();
			accessor.setColor (color);
		}
	}
	return accessor;
}

//----------------------------------------------------------------------------------------------------
bool Grayscale::process (CBitmap* bitmap)
{
	OwningPointer<CBitmapPixelAccess> accessor = CBitmapPixelAccess::create (bitmap, false);
	if (accessor)
	{
		process (*accessor);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
CBitmap* Copy::process (CBitmap* bitmap)
{
	CBitmap* copyBitmap = new CBitmap (bitmap->getWidth (), bitmap->getHeight ());
	OwningPointer<CBitmapPixelAccess> originalAccessor = CBitmapPixelAccess::create (bitmap, false);
	OwningPointer<CBitmapPixelAccess> copyAccessor = CBitmapPixelAccess::create (copyBitmap, false);
	if (originalAccessor && copyAccessor)
	{
		process (*originalAccessor, *copyAccessor);
		return copyBitmap;
	}
	copyBitmap->forget ();
	return 0;
}

//----------------------------------------------------------------------------------------------------
CBitmapPixelAccess& Copy::process (CBitmapPixelAccess& originalAccessor, CBitmapPixelAccess& copyAccessor)
{
	uint32_t width = originalAccessor.getBitmapWidth ();
	uint32_t height = originalAccessor.getBitmapHeight ();
	if (width != copyAccessor.getBitmapWidth () || height != copyAccessor.getBitmapHeight ())
		return copyAccessor;

	IPlatformBitmapPixelAccess* pa1 = originalAccessor.getPlatformBitmapPixelAccess ();
	IPlatformBitmapPixelAccess* pa2 = copyAccessor.getPlatformBitmapPixelAccess ();

	if (pa1->getPixelFormat () == pa2->getPixelFormat () && pa1->getBytesPerRow () == pa2->getBytesPerRow ())
	{
		uint32_t bytes = pa1->getBytesPerRow () * height;
		memcpy (pa2->getAddress (), pa1->getAddress (), bytes);
	}
	else
	{
		originalAccessor.setPosition (0, 0);
		copyAccessor.setPosition (0, 0);
		CColor color;
		for (uint32_t y = 0; y < height; y++)
		{
			for (uint32_t x = 0; x < width; x++, originalAccessor++, copyAccessor++)
			{
				originalAccessor.getColor (color);
				copyAccessor.setColor (color);
			}
		}
	}
	return copyAccessor;
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
CBitmap* Extract::process (CBitmap* bitmap, const CRect& _size)
{
	CRect r (0, 0, bitmap->getWidth (), bitmap->getHeight ());
	CRect size (_size);
	size.bound (r);
	if (r.isEmpty () == false)
	{
		CBitmap* copyBitmap = new CBitmap (size.getWidth (), size.getHeight ());
		OwningPointer<CBitmapPixelAccess> originalAccessor = CBitmapPixelAccess::create (bitmap, false);
		OwningPointer<CBitmapPixelAccess> copyAccessor = CBitmapPixelAccess::create (copyBitmap, false);
		if (originalAccessor && copyAccessor)
		{
			process (*originalAccessor, *copyAccessor, size.getTopLeft ());
			return copyBitmap;
		}
		copyBitmap->forget ();
	}
	return 0;
}

//----------------------------------------------------------------------------------------------------
CBitmapPixelAccess& Extract::process (CBitmapPixelAccess& originalBitmapAccessor, CBitmapPixelAccess& copyBitmapAccessor, const CPoint& leftTop)
{
	uint32_t width = copyBitmapAccessor.getBitmapWidth ();
	uint32_t height = copyBitmapAccessor.getBitmapHeight ();
	copyBitmapAccessor.setPosition (0, 0);
	for (int64_t y = 0; y < height; y++)
	{
		originalBitmapAccessor.setPosition ((uint32_t)leftTop.x, (uint32_t)leftTop.y + (uint32_t)y);
		for (int64_t x = 0; x < width; x++, copyBitmapAccessor++, originalBitmapAccessor++)
		{
			CColor color;
			originalBitmapAccessor.getColor (color);
			copyBitmapAccessor.setColor (color);
		}
	}
	return copyBitmapAccessor;
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
CBitmap* Expand::process (CBitmap* bitmap, CCoord left, CCoord top, CCoord right, CCoord bottom)
{
	CRect size (0, 0, bitmap->getWidth () + left + right, bitmap->getHeight () + top + bottom);
	if (size.isEmpty () == false)
	{
		CBitmap* copyBitmap = new CBitmap (size.getWidth (), size.getHeight ());
		OwningPointer<CBitmapPixelAccess> originalAccessor = CBitmapPixelAccess::create (bitmap, false);
		OwningPointer<CBitmapPixelAccess> copyAccessor = CBitmapPixelAccess::create (copyBitmap, false);
		if (originalAccessor && copyAccessor)
		{
			process (*originalAccessor, *copyAccessor, left, top, right, bottom);
			return copyBitmap;
		}
		copyBitmap->forget ();
	}
	return 0;
}

//----------------------------------------------------------------------------------------------------
CBitmapPixelAccess& Expand::process (CBitmapPixelAccess& originalBitmapAccessor, CBitmapPixelAccess& copyBitmapAccessor, CCoord left, CCoord top, CCoord right, CCoord bottom)
{
	uint32_t width = originalBitmapAccessor.getBitmapWidth ();
	uint32_t height = originalBitmapAccessor.getBitmapHeight ();
	originalBitmapAccessor.setPosition (0, 0);

	for (int64_t y = 0; y < height; y++)
	{
		copyBitmapAccessor.setPosition ((uint32_t)left, (uint32_t)y + (uint32_t)top);
		for (int64_t x = 0; x < width; x++, originalBitmapAccessor++, copyBitmapAccessor++)
		{
			CColor color;
			originalBitmapAccessor.getColor (color);
			copyBitmapAccessor.setColor (color);
		}
	}
	
	return copyBitmapAccessor;
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
CBitmapPixelAccess& Scale::processBilinear (CBitmapPixelAccess& originalBitmap, CBitmapPixelAccess& copyBitmap)
{
	originalBitmap.setPosition (0, 0);
	copyBitmap.setPosition (0, 0);

	int32_t origWidth = (int32_t)originalBitmap.getBitmapWidth ();
	int32_t origHeight = (int32_t)originalBitmap.getBitmapHeight ();
	int32_t newWidth = (int32_t)copyBitmap.getBitmapWidth ();
	int32_t newHeight = (int32_t)copyBitmap.getBitmapHeight ();

    float xRatio = ((float)(origWidth-1)) / (float)newWidth;
    float yRatio = ((float)(origHeight-1)) / (float)newHeight;
    float xDiff, yDiff, r, g, b, a;
	uint32_t x, y;
	CColor color[4];
	CColor result;

	for (int32_t i = 0; i < newHeight; i++)
	{
		y = (int32_t)(yRatio * i);
		yDiff = (yRatio * i) - y;

		for (int32_t j = 0; j < newWidth; j++, copyBitmap++)
		{
			x = (int32_t)(xRatio * j);
			xDiff = (xRatio * j) - x;
			originalBitmap.setPosition (x, y);
			originalBitmap.getColor (color[0]);
			originalBitmap.setPosition (x+1, y);
			originalBitmap.getColor (color[1]);
			originalBitmap.setPosition (x, y+1);
			originalBitmap.getColor (color[2]);
			originalBitmap.setPosition (x+1, y+1);
			originalBitmap.getColor (color[3]);
			r = color[0].red * (1.f - xDiff) * (1.f - yDiff) + color[1].red * xDiff * (1.f - yDiff)
			  + color[2].red * yDiff * (1.f - xDiff) + color[3].red * xDiff * yDiff;
			g = color[0].green * (1.f - xDiff) * (1.f - yDiff) + color[1].green * xDiff * (1.f - yDiff)
			  + color[2].green * yDiff * (1.f - xDiff) + color[3].green * xDiff * yDiff;
			b = color[0].blue * (1.f - xDiff) * (1.f - yDiff) + color[1].blue * xDiff * (1.f - yDiff)
			  + color[2].blue * yDiff * (1.f - xDiff) + color[3].blue * xDiff * yDiff;
			a = color[0].alpha * (1.f - xDiff) * (1.f - yDiff) + color[1].alpha * xDiff * (1.f - yDiff)
			  + color[2].alpha * yDiff * (1.f - xDiff) + color[3].alpha * xDiff * yDiff;
			result = CColor ((uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a);
			copyBitmap.setColor (result);
		}
	}

	return copyBitmap;
}

//----------------------------------------------------------------------------------------------------
CBitmap* Scale::processBilinear (CBitmap* bitmap, const CPoint& newSize)
{
	CBitmap* copyBitmap = new CBitmap (newSize.x, newSize.y);
	OwningPointer<CBitmapPixelAccess> originalAccessor = CBitmapPixelAccess::create (bitmap, false);
	OwningPointer<CBitmapPixelAccess> copyAccessor = CBitmapPixelAccess::create (copyBitmap, false);
	if (originalAccessor && copyAccessor)
	{
		processBilinear (*originalAccessor, *copyAccessor);
		return copyBitmap;
	}
	copyBitmap->forget ();
	return 0;
}

}} // namespaces
