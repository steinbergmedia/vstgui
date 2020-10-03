// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cairobitmap.h"
#include "../iplatformfont.h"
#include "../iplatformframe.h"
#include "../iplatformframecallback.h"
#include "../iplatformresourceinputstream.h"
#include "linuxstring.h"
#include "x11timer.h"
#include "linuxfactory.h"
#include <list>
#include <memory>
#include <chrono>

//-----------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
uint64_t LinuxFactory::getTicks () const noexcept
{
	using namespace std::chrono;
	return duration_cast<milliseconds> (steady_clock::now ().time_since_epoch ()).count ();
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
	return makeOwned<Cairo::Bitmap> (size);
}

//-----------------------------------------------------------------------------
PlatformBitmapPtr LinuxFactory::createBitmap (const CResourceDescription& desc) const noexcept
{
	if (auto bitmap = makeOwned<Cairo::Bitmap> ())
	{
		if (bitmap->load (desc))
			return bitmap;
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
PlatformBitmapPtr LinuxFactory::createBitmapFromPath (UTF8StringPtr absolutePath) const noexcept
{
	return Cairo::Bitmap::create (absolutePath);
}

//-----------------------------------------------------------------------------
PlatformBitmapPtr LinuxFactory::createBitmapFromMemory (const void* ptr,
														uint32_t memSize) const noexcept
{
	return Cairo::Bitmap::create (ptr, memSize);
}

//-----------------------------------------------------------------------------
PNGBitmapBuffer
LinuxFactory::createBitmapMemoryPNGRepresentation (const PlatformBitmapPtr& bitmap) const noexcept
{
	if (auto cairoBitmap = dynamic_cast<Cairo::Bitmap*> (bitmap.get ()))
	{
		return cairoBitmap->createMemoryPNGRepresentation ();
	}
	return {};
}

//-----------------------------------------------------------------------------
PlatformResourceInputStreamPtr
LinuxFactory::createResourceInputStream (const CResourceDescription& desc) const noexcept
{
	return IPlatformResourceInputStream::create (desc);
}

//-----------------------------------------------------------------------------
PlatformStringPtr LinuxFactory::createString (UTF8StringPtr utf8String) const noexcept
{
	return makeOwned<LinuxString> (utf8String);
}

//-----------------------------------------------------------------------------
PlatformTimerPtr LinuxFactory::createTimer (IPlatformTimerCallback* callback) const noexcept
{
	return makeOwned<X11::Timer> (callback);
}

//-----------------------------------------------------------------------------
} // VSTGUI
