
#include "gdiplusbitmap.h"

#if WINDOWS && VSTGUI_PLATFORM_ABSTRACTION

#include "win32support.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
IPlatformBitmap* IPlatformBitmap::create (CPoint* size)
{
	if (size)
		return new GdiplusBitmap (*size);
	return new GdiplusBitmap ();
}

//-----------------------------------------------------------------------------
GdiplusBitmap::GdiplusBitmap ()
: bitmap (0)
{
	GDIPlusGlobals::enter ();
}

//-----------------------------------------------------------------------------
GdiplusBitmap::GdiplusBitmap (const CPoint& size)
: bitmap (0)
, size (size)
{
	GDIPlusGlobals::enter ();
	bitmap = new Gdiplus::Bitmap ((INT)size.x, (INT)size.y, PixelFormat32bppARGB);
}

//-----------------------------------------------------------------------------
GdiplusBitmap::~GdiplusBitmap ()
{
	if (bitmap)
		delete bitmap;
	GDIPlusGlobals::exit ();
}

//-----------------------------------------------------------------------------
bool GdiplusBitmap::load (const CResourceDescription& desc)
{
	if (bitmap == 0)
	{
		ResourceStream* resourceStream = new ResourceStream;
		if (resourceStream->open (desc, "PNG"))
			bitmap = Gdiplus::Bitmap::FromStream (resourceStream, TRUE);
		resourceStream->Release ();
		if (!bitmap)
			bitmap = Gdiplus::Bitmap::FromResource (GetInstance (), desc.type == CResourceDescription::kIntegerType ? (WCHAR*)MAKEINTRESOURCE(desc.u.id) : (WCHAR*)desc.u.name);
		if (bitmap)
		{
			size.x = (CCoord)bitmap->GetWidth ();
			size.y = (CCoord)bitmap->GetHeight ();
			return true;
		}
	}
	return false;
}

} // namespace

#endif // WINDOWS && VSTGUI_PLATFORM_ABSTRACTION
