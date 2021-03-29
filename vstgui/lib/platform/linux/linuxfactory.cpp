// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cairobitmap.h"
#include "cairofont.h"
#include "cairocontext.h"
#include "x11frame.h"
#include "../iplatformframecallback.h"
#include "../common/fileresourceinputstream.h"
#include "../iplatformresourceinputstream.h"
#include "linuxstring.h"
#include "x11timer.h"
#include "linuxfactory.h"
#include <list>
#include <memory>
#include <chrono>
#include <X11/X.h>
#include <dlfcn.h>
#include <link.h>

//-----------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
struct LinuxFactory::Impl
{
	std::string resPath;

	void setupResPath (void* handle)
	{
		if (handle && resPath.empty ())
		{
			struct link_map* map;
			if (dlinfo (handle, RTLD_DI_LINKMAP, &map) == 0)
			{
				auto path = std::string (map->l_name);
				for (int i = 0; i < 3; i++)
				{
					int delPos = path.find_last_of ('/');
					if (delPos == -1)
					{
						fprintf (stderr, "Could not determine bundle location.\n");
						return; // unexpected
					}
					path.erase (delPos, path.length () - delPos);
				}
				auto rp = realpath (path.data (), nullptr);
				path = rp;
				free (rp);
				path += "/Contents/Resources/";
				std::swap (resPath, path);
			}
		}
	}
};

//-----------------------------------------------------------------------------
LinuxFactory::LinuxFactory (void* soHandle)
{
	impl = std::unique_ptr<Impl> (new Impl);
	impl->setupResPath (soHandle);
}

//-----------------------------------------------------------------------------
void LinuxFactory::setResourcePath (const std::string& path) const noexcept
{
	impl->resPath = path;
}

//-----------------------------------------------------------------------------
std::string LinuxFactory::getResourcePath () const noexcept
{
	return impl->resPath;
}

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
	if (parentType == PlatformType::kDefaultNative || parentType == PlatformType::kX11EmbedWindowID)
	{
		auto x11Parent = reinterpret_cast<XID> (parent);
		return makeOwned<X11::Frame> (frame, size, x11Parent, config);
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
PlatformFontPtr LinuxFactory::createFont (const UTF8String& name, const CCoord& size,
										  const int32_t& style) const noexcept
{
	return makeOwned<Cairo::Font> (name, size, style);
}

//-----------------------------------------------------------------------------
bool LinuxFactory::getAllFontFamilies (const FontFamilyCallback& callback) const noexcept
{
	return Cairo::Font::getAllFamilies (callback);
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
PNGBitmapBuffer LinuxFactory::createBitmapMemoryPNGRepresentation (
	const PlatformBitmapPtr& bitmap) const noexcept
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
	if (desc.type == CResourceDescription::kIntegerType)
		return {};
	auto path = impl->resPath;
	path += desc.u.name;
	return FileResourceInputStream::create (path);
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

//------------------------------------------------------------------------
bool LinuxFactory::setClipboard (const DataPackagePtr& data) const noexcept
{
	// TODO: Linux Clipboard Implementation
	return false;
}

//------------------------------------------------------------------------
auto LinuxFactory::getClipboard () const noexcept -> DataPackagePtr
{
	// TODO: Linux Clipboard Implementation
	return nullptr;
}

//------------------------------------------------------------------------
auto LinuxFactory::createOffscreenContext (const CPoint& size, double scaleFactor) const noexcept
	-> COffscreenContextPtr
{
	auto bitmap = new Cairo::Bitmap (size * scaleFactor);
	bitmap->setScaleFactor (scaleFactor);
	auto context = owned (new Cairo::Context (bitmap));
	bitmap->forget ();
	if (context->valid ())
		return context;
	return nullptr;
}

//-----------------------------------------------------------------------------
const LinuxFactory* LinuxFactory::asLinuxFactory () const noexcept
{
	return this;
}

//-----------------------------------------------------------------------------
const MacFactory* LinuxFactory::asMacFactory () const noexcept
{
	return nullptr;
}

//-----------------------------------------------------------------------------
const Win32Factory* LinuxFactory::asWin32Factory () const noexcept
{
	return nullptr;
}

//-----------------------------------------------------------------------------
} // VSTGUI
