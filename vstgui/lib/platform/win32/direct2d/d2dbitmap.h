// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __d2dbitmap__
#define __d2dbitmap__

#include "../win32bitmapbase.h"

#if WINDOWS && VSTGUI_DIRECT2D_SUPPORT

#include "../../../cpoint.h"

struct IWICBitmapSource;
struct ID2D1Bitmap;
struct ID2D1RenderTarget;
struct IWICBitmap;
struct IWICBitmapLock;

#include <map>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class D2DBitmap : public Win32BitmapBase
{
public:
	D2DBitmap ();
	D2DBitmap (const CPoint& size);
	~D2DBitmap ();

	bool load (const CResourceDescription& desc) override;
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

	class PixelAccess : public IPlatformBitmapPixelAccess
	{
	public:
		PixelAccess ();
		~PixelAccess ();

		bool init (D2DBitmap* bitmap, bool alphaPremultiplied);

		uint8_t* getAddress () const { return (uint8_t*)ptr; }
		uint32_t getBytesPerRow () const { return bytesPerRow; }
		PixelFormat getPixelFormat () const { return kBGRA; }

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

//-----------------------------------------------------------------------------
class D2DBitmapCache
{
public:
	ID2D1Bitmap* getBitmap (D2DBitmap* bitmap, ID2D1RenderTarget* renderTarget);
	
	void removeBitmap (D2DBitmap* bitmap);
	void removeRenderTarget (ID2D1RenderTarget* renderTarget);

	static D2DBitmapCache* instance ();
//-----------------------------------------------------------------------------
protected:
	D2DBitmapCache ();
	~D2DBitmapCache ();
	ID2D1Bitmap* createBitmap (D2DBitmap* bitmap, ID2D1RenderTarget* renderTarget);
	using RenderTargetBitmapMap = std::map<ID2D1RenderTarget*, ID2D1Bitmap*>;
	using BitmapCache = std::map<D2DBitmap*, RenderTargetBitmapMap>;
	BitmapCache cache;
};

} // namespace

#endif // WINDOWS

#endif // __d2dbitmap__
