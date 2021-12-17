// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "d2dbitmapcache.h"

#if WINDOWS

#include <d2d1.h>

namespace VSTGUI {
//------------------------------------------------------------------------
namespace D2DBitmapCache {
//------------------------------------------------------------------------
namespace {

struct Cache
{
	ID2D1Bitmap* getBitmap (D2DBitmap* bitmap, ID2D1RenderTarget* renderTarget);
	void removeBitmap (D2DBitmap* bitmap);
	void removeRenderTarget (ID2D1RenderTarget* renderTarget);
	static Cache* instance ()
	{
		static Cache gInstance;
		return &gInstance;
	}

private:
	Cache ();
	~Cache ();
	ID2D1Bitmap* createBitmap (D2DBitmap* bitmap, ID2D1RenderTarget* renderTarget);
	using RenderTargetBitmapMap = std::map<ID2D1RenderTarget*, ID2D1Bitmap*>;
	using BitmapMap = std::map<D2DBitmap*, RenderTargetBitmapMap>;
	BitmapMap map;
};

//-----------------------------------------------------------------------------
ID2D1Bitmap* Cache::getBitmap (D2DBitmap* bitmap, ID2D1RenderTarget* renderTarget)
{
	BitmapMap::iterator it = map.find (bitmap);
	if (it != map.end ())
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
	auto insertSuccess = map.emplace (bitmap, RenderTargetBitmapMap ());
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
void Cache::removeBitmap (D2DBitmap* bitmap)
{
	BitmapMap::iterator it = map.find (bitmap);
	if (it != map.end ())
	{
		RenderTargetBitmapMap::iterator it2 = it->second.begin ();
		while (it2 != it->second.end ())
		{
			it2->second->Release ();
			it2++;
		}
		map.erase (it);
	}
}

//-----------------------------------------------------------------------------
void Cache::removeRenderTarget (ID2D1RenderTarget* renderTarget)
{
	BitmapMap::iterator it = map.begin ();
	while (it != map.end ())
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
ID2D1Bitmap* Cache::createBitmap (D2DBitmap* bitmap, ID2D1RenderTarget* renderTarget)
{
	if (bitmap->getSource () == nullptr)
		return nullptr;
	ID2D1Bitmap* d2d1Bitmap = nullptr;
	renderTarget->CreateBitmapFromWicBitmap (bitmap->getSource (), &d2d1Bitmap);
	return d2d1Bitmap;
}

//-----------------------------------------------------------------------------
Cache::Cache () {}

//-----------------------------------------------------------------------------
Cache::~Cache ()
{
#if DEBUG
	for (BitmapMap::const_iterator it = map.begin (); it != map.end (); it++)
	{
		vstgui_assert (it->second.empty ());
	}
#endif
}

//-----------------------------------------------------------------------------
} // anonymous

//-----------------------------------------------------------------------------
ID2D1Bitmap* getBitmap (D2DBitmap* bitmap, ID2D1RenderTarget* renderTarget)
{
	return Cache::instance ()->getBitmap (bitmap, renderTarget);
}

//-----------------------------------------------------------------------------
void removeBitmap (D2DBitmap* bitmap)
{
	Cache::instance ()->removeBitmap (bitmap);
}

//-----------------------------------------------------------------------------
void removeRenderTarget (ID2D1RenderTarget* renderTarget)
{
	Cache::instance ()->removeRenderTarget (renderTarget);
}

//-----------------------------------------------------------------------------
} // D2DBitmapCache
} // VSTGUI

#endif // WINDOWS
