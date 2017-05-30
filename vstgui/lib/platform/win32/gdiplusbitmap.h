// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __gdiplusbitmap__
#define __gdiplusbitmap__

#include "win32bitmapbase.h"

#if WINDOWS

#include "../../cpoint.h"
#include "win32support.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
class GdiplusBitmap : public Win32BitmapBase
{
public:
	GdiplusBitmap ();
	GdiplusBitmap (const CPoint& size);
	~GdiplusBitmap () noexcept;

	bool load (const CResourceDescription& desc) override;
	const CPoint& getSize () const override { return size; }
	SharedPointer<IPlatformBitmapPixelAccess> lockPixels (bool alphaPremultiplied) override;
	void setScaleFactor (double factor) override {}
	double getScaleFactor () const override { return 1.; }

	Gdiplus::Bitmap* getBitmap () const { return bitmap; }
	HBITMAP createHBitmap () override;
	bool loadFromStream (IStream* stream) override;
	PNGBitmapBuffer createMemoryPNGRepresentation () override;

//-----------------------------------------------------------------------------
protected:
	class PixelAccess : public IPlatformBitmapPixelAccess
	{
	public:
		PixelAccess ();
		~PixelAccess () noexcept;

		bool init (GdiplusBitmap* bitmap, bool alphaPremulitplied);

		uint8_t* getAddress () const { return (uint8_t*)data.Scan0; }
		uint32_t getBytesPerRow () const { return static_cast<uint32_t> (data.Stride); }
		PixelFormat getPixelFormat () const { return kBGRA; }
	protected:
		GdiplusBitmap* bitmap;
		Gdiplus::BitmapData data;
	};
	Gdiplus::Bitmap* bitmap;
	CPoint size;
	bool allocatedByGdi;
};

} // namespace

#endif // WINDOWS

#endif // __gdiplusbitmap__

