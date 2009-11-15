
#ifndef __gdiplusbitmap__
#define __gdiplusbitmap__

#include "../iplatformbitmap.h"

#if WINDOWS && VSTGUI_PLATFORM_ABSTRACTION

#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class GdiplusBitmap : public IPlatformBitmap
{
public:
	GdiplusBitmap ();
	GdiplusBitmap (const CPoint& size);
	~GdiplusBitmap ();

	bool load (const CResourceDescription& desc);
	const CPoint& getSize () const { return size; }

	Gdiplus::Bitmap* getBitmap () const { return bitmap; }

//-----------------------------------------------------------------------------
protected:
	Gdiplus::Bitmap* bitmap;
	CPoint size;
};

} // namespace

#endif // WINDOWS && VSTGUI_PLATFORM_ABSTRACTION

#endif // __gdiplusbitmap__

