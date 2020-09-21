// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../iplatformfont.h"
#include "../iplatformframe.h"
#include "../iplatformframecallback.h"
#include "../iplatformresourceinputstream.h"
#include "../iplatformstring.h"
#include "../iplatformtimer.h"
#include "cfontmac.h"
#include "cgbitmap.h"
#include "macfactory.h"
#include <list>
#include <mach/mach_time.h>
#include <memory>

//-----------------------------------------------------------------------------
namespace VSTGUI {
namespace MacFactoryDetail {
static struct mach_timebase_info timebaseInfo;
} // MacFactoryDetail

//------------------------------------------------------------------------
MacFactory::MacFactory ()
{
	mach_timebase_info (&MacFactoryDetail::timebaseInfo);
}

//-----------------------------------------------------------------------------
uint64_t MacFactory::getTicks () const noexcept
{
	uint64_t absTime = mach_absolute_time ();
	auto d =
	    ((absTime * MacFactoryDetail::timebaseInfo.numer) / MacFactoryDetail::timebaseInfo.denom) /
	    1000000;
	return d;
}

//-----------------------------------------------------------------------------
PlatformFramePtr MacFactory::createFrame (IPlatformFrameCallback* frame, const CRect& size,
                                          void* parent, PlatformType parentType,
                                          IPlatformFrameConfig* config) const noexcept
{
	return owned (IPlatformFrame::createPlatformFrame (frame, size, parent, parentType, config));
}

//-----------------------------------------------------------------------------
PlatformFontPtr MacFactory::createFont (const UTF8String& name, const CCoord& size,
                                        const int32_t& style) const noexcept
{
	auto font = makeOwned<CoreTextFont> (name, size, style);
	if (font->getFontRef ())
		return std::move (font);
	return nullptr;
}

//-----------------------------------------------------------------------------
bool MacFactory::getAllFontFamilies (const FontFamilyCallback& callback) const noexcept
{
	return CoreTextFont::getAllFontFamilies (callback);
}

//-----------------------------------------------------------------------------
PlatformBitmapPtr MacFactory::createBitmap (const CPoint& size) const noexcept
{
	return CGBitmap::create (&const_cast<CPoint&> (size));
}

//-----------------------------------------------------------------------------
PlatformBitmapPtr MacFactory::createBitmap (const CResourceDescription& desc) const noexcept
{
	if (auto bitmap = makeOwned<CGBitmap> ())
	{
		if (bitmap->load (desc))
			return bitmap;
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
PlatformBitmapPtr MacFactory::createBitmapFromPath (UTF8StringPtr absolutePath) const noexcept
{
	return CGBitmap::createFromPath (absolutePath);
}

//-----------------------------------------------------------------------------
PlatformBitmapPtr MacFactory::createBitmapFromMemory (const void* ptr, uint32_t memSize) const
    noexcept
{
	return CGBitmap::createFromMemory (ptr, memSize);
}

//-----------------------------------------------------------------------------
PNGBitmapBuffer MacFactory::createBitmapMemoryPNGRepresentation (
    const PlatformBitmapPtr& bitmap) const noexcept
{
	return CGBitmap::createMemoryPNGRepresentation (bitmap);
}

//-----------------------------------------------------------------------------
PlatformResourceInputStreamPtr MacFactory::createResourceInputStream (
    const CResourceDescription& desc) const noexcept
{
	return IPlatformResourceInputStream::create (desc);
}

//-----------------------------------------------------------------------------
PlatformStringPtr MacFactory::createString (UTF8StringPtr utf8String) const noexcept
{
	return IPlatformString::createWithUTF8String (utf8String);
}

//-----------------------------------------------------------------------------
PlatformTimerPtr MacFactory::createTimer (IPlatformTimerCallback* callback) const noexcept
{
	return IPlatformTimer::create (callback);
}

//-----------------------------------------------------------------------------
} // VSTGUI
