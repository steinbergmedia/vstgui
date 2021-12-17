// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "d2dbitmapcache.h"

#if WINDOWS

#include <d2d1.h>

namespace VSTGUI {

//-----------------------------------------------------------------------------
ID2D1Bitmap* D2DBitmapCache::getBitmap (D2DBitmap* bitmap, ID2D1RenderTarget* renderTarget)
{
	BitmapCache::iterator it = cache.find (bitmap);
	if (it != cache.end ())
	{
		RenderTargetBitmapMap::iterator it2 = it->second.find (renderTarget);
		if (it2 != it->second.end ())
		{
			return it2->second;
		}
		ID2D1Bitmap* b = createBitmap (bitmap, renderTarget);
		if (b)
			it->second.emplace (renderTarget, b);
		return b;
	}
	auto insertSuccess = cache.emplace (bitmap, RenderTargetBitmapMap ());
	if (insertSuccess.second == true)
	{
		ID2D1Bitmap* b = createBitmap (bitmap, renderTarget);
		if (b)
		{
			insertSuccess.first->second.emplace (renderTarget, b);
			return b;
		}
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
void D2DBitmapCache::removeBitmap (D2DBitmap* bitmap)
{
	BitmapCache::iterator it = cache.find (bitmap);
	if (it != cache.end ())
	{
		RenderTargetBitmapMap::iterator it2 = it->second.begin ();
		while (it2 != it->second.end ())
		{
			it2->second->Release ();
			it2++;
		}
		cache.erase (it);
	}
}

//-----------------------------------------------------------------------------
void D2DBitmapCache::removeRenderTarget (ID2D1RenderTarget* renderTarget)
{
	BitmapCache::iterator it = cache.begin ();
	while (it != cache.end ())
	{
		RenderTargetBitmapMap::iterator it2 = it->second.begin ();
		while (it2 != it->second.end ())
		{
			if (it2->first == renderTarget)
			{
				it2->second->Release ();
				it->second.erase (it2++);
			}
			else
				it2++;
		}
		it++;
	}
}

//-----------------------------------------------------------------------------
ID2D1Bitmap* D2DBitmapCache::createBitmap (D2DBitmap* bitmap, ID2D1RenderTarget* renderTarget)
{
	if (bitmap->getSource () == nullptr)
		return nullptr;
	ID2D1Bitmap* d2d1Bitmap = nullptr;
	renderTarget->CreateBitmapFromWicBitmap (bitmap->getSource (), &d2d1Bitmap);
	return d2d1Bitmap;
}

static D2DBitmapCache* gD2DBitmapCache = nullptr;
//-----------------------------------------------------------------------------
D2DBitmapCache::D2DBitmapCache ()
{
	gD2DBitmapCache = this;
}

//-----------------------------------------------------------------------------
D2DBitmapCache::~D2DBitmapCache ()
{
#if DEBUG
	for (BitmapCache::const_iterator it = cache.begin (); it != cache.end (); it++)
	{
		vstgui_assert (it->second.empty ());
	}
#endif
	gD2DBitmapCache = nullptr;
}

//-----------------------------------------------------------------------------
D2DBitmapCache* D2DBitmapCache::instance ()
{
	static D2DBitmapCache gInstance;
	return gD2DBitmapCache;
}

} // VSTGUI

#endif // WINDOWS
