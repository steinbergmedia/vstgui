// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "d2dbitmap.h"

#if WINDOWS

namespace VSTGUI {

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

} // namespace VSTGUI

#endif // WINDOWS
