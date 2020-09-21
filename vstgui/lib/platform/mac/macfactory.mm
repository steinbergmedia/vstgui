// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cgbitmap.h"
#include "../iplatformfont.h"
#include "../iplatformframe.h"
#include "../iplatformframecallback.h"
#include "../iplatformresourceinputstream.h"
#include "../iplatformstring.h"
#include "../iplatformtimer.h"
#include "macfactory.h"
#include <list>
#include <memory>

//-----------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
uint64_t MacFactory::getTicks () const noexcept
{
	return IPlatformFrame::getTicks ();
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
	return IPlatformFont::create (name, size, style);
}

//-----------------------------------------------------------------------------
bool MacFactory::getAllFontFamilies (const FontFamilyCallback& callback) const noexcept
{
	std::list<std::string> names;
	if (IPlatformFont::getAllPlatformFontFamilies (names))
	{
		for (const auto& n : names)
		{
			if (!callback (n))
				break;
		}
		return true;
	}
	return false;
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
