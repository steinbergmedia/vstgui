// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "d2dbitmapcache.h"

#if WINDOWS

#include "../comptr.h"

#include <algorithm>
#include <d2d1.h>
#include <vector>

namespace VSTGUI {
namespace D2DBitmapCache {
namespace {

//-----------------------------------------------------------------------------
struct Cache
{
	Cache () = default;
	~Cache () = default;

	ID2D1Bitmap* getBitmap (D2DBitmap* bitmap, ID2D1RenderTarget* renderTarget,
							ID2D1Device* device);
	void removeBitmap (D2DBitmap* bitmap);
	void removeRenderTarget (ID2D1RenderTarget* renderTarget);
	void removeDevice (ID2D1Device* device);

private:
	COM::Ptr<ID2D1Bitmap> createBitmap (D2DBitmap* bitmap, ID2D1RenderTarget* renderTarget) const;

	struct ResourceLocation
	{
		ID2D1Device* device {nullptr};
		ID2D1RenderTarget* renderTarget {nullptr};

		bool operator== (const ResourceLocation& o) const
		{
			return o.device == device && o.renderTarget == renderTarget;
		}
	};

	using ResourceLocationList = std::vector<std::pair<ResourceLocation, COM::Ptr<ID2D1Bitmap>>>;
	using BitmapMap = std::map<D2DBitmap*, ResourceLocationList>;
	BitmapMap bitmaps;
};

//-----------------------------------------------------------------------------
ID2D1Bitmap* Cache::getBitmap (D2DBitmap* bitmap, ID2D1RenderTarget* renderTarget,
							   ID2D1Device* device)
{
	ResourceLocation resLoc {device, device ? nullptr : renderTarget};

	auto bitmapIt = bitmaps.find (bitmap);
	if (bitmapIt != bitmaps.end ())
	{

		auto resLocIt = std::find_if (bitmapIt->second.begin (), bitmapIt->second.end (),
									  [&] (const auto& loc) { return loc.first == resLoc; });
		if (resLocIt != bitmapIt->second.end ())
		{
			return resLocIt->second.get ();
		}
		auto b = createBitmap (bitmap, renderTarget);
		if (b)
			bitmapIt->second.emplace_back (std::make_pair (resLoc, b));
		return b.get ();
	}
	auto insertSuccess = bitmaps.emplace (bitmap, ResourceLocationList ());
	if (insertSuccess.second == true)
	{
		auto b = createBitmap (bitmap, renderTarget);
		if (b)
		{
			insertSuccess.first->second.emplace_back (std::make_pair (resLoc, b));
			return b.get ();
		}
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
void Cache::removeBitmap (D2DBitmap* bitmap)
{
	auto bitmapIt = bitmaps.find (bitmap);
	if (bitmapIt != bitmaps.end ())
		bitmaps.erase (bitmapIt);
}

//-----------------------------------------------------------------------------
void Cache::removeRenderTarget (ID2D1RenderTarget* renderTarget)
{
	auto bitmapIt = bitmaps.begin ();
	while (bitmapIt != bitmaps.end ())
	{
		auto end =
			std::remove_if (bitmapIt->second.begin (), bitmapIt->second.end (),
							[&] (const auto& el) { return el.first.renderTarget == renderTarget; });

		bitmapIt->second.erase (end, bitmapIt->second.end ());

		bitmapIt++;
	}
}

//-----------------------------------------------------------------------------
void Cache::removeDevice (ID2D1Device* device)
{
	auto bitmapIt = bitmaps.begin ();
	while (bitmapIt != bitmaps.end ())
	{
		auto end = std::remove_if (bitmapIt->second.begin (), bitmapIt->second.end (),
								   [&] (const auto& el) { return el.first.device == device; });

		bitmapIt->second.erase (end, bitmapIt->second.end ());

		bitmapIt++;
	}
}

//-----------------------------------------------------------------------------
COM::Ptr<ID2D1Bitmap> Cache::createBitmap (D2DBitmap* bitmap, ID2D1RenderTarget* renderTarget) const
{
	if (bitmap->getSource () == nullptr)
		return {};
	COM::Ptr<ID2D1Bitmap> d2d1Bitmap;
	renderTarget->CreateBitmapFromWicBitmap (bitmap->getSource (), d2d1Bitmap.adoptPtr ());
	return d2d1Bitmap;
}

std::unique_ptr<Cache> gCache;

//-----------------------------------------------------------------------------
} // anonymous

//-----------------------------------------------------------------------------
void init ()
{
	gCache = std::make_unique<Cache> ();
}

//-----------------------------------------------------------------------------
void terminate ()
{
	gCache.reset ();
}

//-----------------------------------------------------------------------------
ID2D1Bitmap* getBitmap (D2DBitmap* bitmap, ID2D1RenderTarget* renderTarget, ID2D1Device* device)
{
	if (!gCache)
		return nullptr;
	return gCache->getBitmap (bitmap, renderTarget, device);
}

//-----------------------------------------------------------------------------
void removeBitmap (D2DBitmap* bitmap)
{
	if (!gCache)
		return;
	gCache->removeBitmap (bitmap);
}

//-----------------------------------------------------------------------------
void removeRenderTarget (ID2D1RenderTarget* renderTarget)
{
	if (!gCache)
		return;
	gCache->removeRenderTarget (renderTarget);
}

//-----------------------------------------------------------------------------
void removeDevice (ID2D1Device* device)
{
	if (!gCache)
		return;
	gCache->removeDevice (device);
}

//-----------------------------------------------------------------------------
} // D2DBitmapCache
} // VSTGUI

#endif // WINDOWS
