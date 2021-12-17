// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../win32bitmapbase.h"

#if WINDOWS

#include "../../../cpoint.h"

struct IWICBitmapSource;
struct ID2D1Bitmap;
struct ID2D1RenderTarget;
struct IWICBitmap;
struct IWICBitmapLock;

#include <map>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class D2DBitmap final : public Win32BitmapBase
{
public:
	D2DBitmap ();
	D2DBitmap (const CPoint& size);
	~D2DBitmap ();

	bool load (const CResourceDescription& desc);
	const CPoint& getSize () const override { return size; }
	SharedPointer<IPlatformBitmapPixelAccess> lockPixels (bool alphaPremultiplied) override;
	void setScaleFactor (double factor) override { scaleFactor = factor; }
	double getScaleFactor () const override { return scaleFactor; }

	HBITMAP createHBitmap () override;
	bool loadFromStream (IStream* stream) override;

	IWICBitmapSource* getSource () const { return source; }
	IWICBitmap* getBitmap ();
	PNGBitmapBuffer createMemoryPNGRepresentation () override;
//-----------------------------------------------------------------------------
protected:
	void replaceBitmapSource (IWICBitmapSource* newSourceBitmap);

	class PixelAccess final : public IPlatformBitmapPixelAccess
	{
	public:
		PixelAccess ();
		~PixelAccess ();

		bool init (D2DBitmap* bitmap, bool alphaPremultiplied);

		uint8_t* getAddress () const override { return (uint8_t*)ptr; }
		uint32_t getBytesPerRow () const override { return bytesPerRow; }
		PixelFormat getPixelFormat () const override { return kBGRA; }

	protected:
		static void premultiplyAlpha (BYTE* ptr, UINT bytesPerRow, const CPoint& size);
		static void unpremultiplyAlpha (BYTE* ptr, UINT bytesPerRow, const CPoint& size);

		D2DBitmap* bitmap;
		IWICBitmapLock* bLock;
		BYTE* ptr;
		UINT bytesPerRow;
		bool alphaPremultiplied;
	};

	CPoint size;
	double scaleFactor;
	IWICBitmapSource* source;
};

} // VSTGUI

#endif // WINDOWS
