// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "gdiplusbitmap.h"

#if WINDOWS

#include "win32support.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
GdiplusBitmap::GdiplusBitmap ()
: bitmap (0)
, allocatedByGdi (false)
{
	GDIPlusGlobals::enter ();
}

//-----------------------------------------------------------------------------
GdiplusBitmap::GdiplusBitmap (const CPoint& size)
: bitmap (0)
, size (size)
, allocatedByGdi (false)
{
	GDIPlusGlobals::enter ();
	bitmap = ::new Gdiplus::Bitmap ((INT)size.x, (INT)size.y, PixelFormat32bppARGB);
}

//-----------------------------------------------------------------------------
GdiplusBitmap::~GdiplusBitmap () noexcept
{
	if (bitmap)
	{
		if (allocatedByGdi)
			delete bitmap;
		else
			::delete bitmap;
	}
	GDIPlusGlobals::exit ();
}

//-----------------------------------------------------------------------------
bool GdiplusBitmap::loadFromStream (IStream* stream)
{
	if (bitmap)
		return false;
	bitmap = Gdiplus::Bitmap::FromStream (stream, TRUE);
	if (bitmap)
	{
		allocatedByGdi = true;
		size.x = (CCoord)bitmap->GetWidth ();
		size.y = (CCoord)bitmap->GetHeight ();
	}
	return bitmap != 0;
}

//-----------------------------------------------------------------------------
PNGBitmapBuffer GdiplusBitmap::createMemoryPNGRepresentation ()
{
	return {};
}

//-----------------------------------------------------------------------------
bool GdiplusBitmap::load (const CResourceDescription& desc)
{
	if (bitmap == 0)
	{
		ResourceStream* resourceStream = new ResourceStream;
		if (resourceStream->open (desc, "PNG"))
			loadFromStream (resourceStream);
		resourceStream->Release ();
		if (!bitmap)
			bitmap = Gdiplus::Bitmap::FromResource (GetInstance (), desc.type == CResourceDescription::kIntegerType ? (WCHAR*)MAKEINTRESOURCE(desc.u.id) : (WCHAR*)desc.u.name);
		if (bitmap)
		{
			allocatedByGdi = true;
			size.x = (CCoord)bitmap->GetWidth ();
			size.y = (CCoord)bitmap->GetHeight ();
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
HBITMAP GdiplusBitmap::createHBitmap ()
{
	HBITMAP hBmp = 0;
	if (bitmap->GetHBITMAP (Gdiplus::Color (0, 0, 0, 0), &hBmp) == Gdiplus::Ok)
		return hBmp;
	return 0;
}

//-----------------------------------------------------------------------------
SharedPointer<IPlatformBitmapPixelAccess> GdiplusBitmap::lockPixels (bool alphaPremultiplied)
{
	if (bitmap)
	{
		auto pixelAccess = owned (new PixelAccess ());
		if (pixelAccess->init (this, alphaPremultiplied))
			return shared<IPlatformBitmapPixelAccess> (pixelAccess);
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
GdiplusBitmap::PixelAccess::PixelAccess ()
: bitmap (0)
{
}

//-----------------------------------------------------------------------------
GdiplusBitmap::PixelAccess::~PixelAccess () noexcept
{
	if (bitmap)
	{
		bitmap->bitmap->UnlockBits (&data);
		bitmap->forget ();
	}
}

//-----------------------------------------------------------------------------
bool GdiplusBitmap::PixelAccess::init (GdiplusBitmap* _bitmap, bool _alphaPremulitplied)
{
	Gdiplus::Rect r (0, 0, (INT)_bitmap->getSize ().x, (INT)_bitmap->getSize ().y);
	Gdiplus::PixelFormat pixelFormat = _alphaPremulitplied ? PixelFormat32bppPARGB : PixelFormat32bppARGB;
	if (_bitmap->bitmap->LockBits (&r, Gdiplus::ImageLockModeRead|Gdiplus::ImageLockModeWrite, pixelFormat, &data) == Gdiplus::Ok)
	{
		bitmap = _bitmap;
		bitmap->remember ();
		return true;
	}
	return false;
}

} // namespace

#endif // WINDOWS
