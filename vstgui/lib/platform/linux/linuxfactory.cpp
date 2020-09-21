// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../iplatformbitmap.h"
#include "../iplatformfont.h"
#include "../iplatformframe.h"
#include "../iplatformframecallback.h"
#include "../iplatformresourceinputstream.h"
#include "../iplatformstring.h"
#include "../iplatformtimer.h"
#include "linuxfactory.h"
#include <list>
#include <memory>

//-----------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
uint64_t LinuxFactory::getTicks () const noexcept
{
	return IPlatformFrame::getTicks ();
}

//-----------------------------------------------------------------------------
PlatformFramePtr LinuxFactory::createFrame (IPlatformFrameCallback* frame, const CRect& size,
                                            void* parent, PlatformType parentType,
                                            IPlatformFrameConfig* config) const noexcept
{
	return owned (IPlatformFrame::createPlatformFrame (frame, size, parent, parentType, config));
}

//-----------------------------------------------------------------------------
PlatformFontPtr LinuxFactory::createFont (const UTF8String& name, const CCoord& size,
                                          const int32_t& style) const noexcept
{
	return IPlatformFont::create (name, size, style);
}

//-----------------------------------------------------------------------------
bool LinuxFactory::getAllFontFamilies (const FontFamilyCallback& callback) const noexcept
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
PlatformBitmapPtr LinuxFactory::createBitmap (const CPoint& size) const noexcept
{
	return IPlatformBitmap::create (&const_cast<CPoint&> (size));
}

//-----------------------------------------------------------------------------
PlatformBitmapPtr LinuxFactory::createBitmap (const CResourceDescription& desc) const noexcept
{
	if (auto bitmap = IPlatformBitmap::create ())
	{
		if (bitmap->load (desc))
			return bitmap;
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
PlatformBitmapPtr LinuxFactory::createBitmapFromPath (UTF8StringPtr absolutePath) const noexcept
{
	return IPlatformBitmap::createFromPath (absolutePath);
}

//-----------------------------------------------------------------------------
PlatformBitmapPtr LinuxFactory::createBitmapFromMemory (const void* ptr, uint32_t memSize) const
    noexcept
{
	return IPlatformBitmap::createFromMemory (ptr, memSize);
}

//-----------------------------------------------------------------------------
PNGBitmapBuffer LinuxFactory::createBitmapMemoryPNGRepresentation (
    const PlatformBitmapPtr& bitmap) const noexcept
{
	return IPlatformBitmap::createMemoryPNGRepresentation (bitmap);
}

//-----------------------------------------------------------------------------
PlatformResourceInputStreamPtr LinuxFactory::createResourceInputStream (
    const CResourceDescription& desc) const noexcept
{
	return IPlatformResourceInputStream::create (desc);
}

//-----------------------------------------------------------------------------
PlatformStringPtr LinuxFactory::createString (UTF8StringPtr utf8String) const noexcept
{
	return IPlatformString::createWithUTF8String (utf8String);
}

//-----------------------------------------------------------------------------
PlatformTimerPtr LinuxFactory::createTimer (IPlatformTimerCallback* callback) const noexcept
{
	return IPlatformTimer::create (callback);
}

//-----------------------------------------------------------------------------
} // VSTGUI
